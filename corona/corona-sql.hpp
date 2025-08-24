#pragma once

namespace corona
{

	struct sql_statement_parameter
	{
	public:
		std::string sql_field_name;
		std::string corona_field_name;
		int			parameter_index;
		field_types			field_type;
		int					string_size;
	};

	class sql_field_mapping
	{
	public:
		std::string			corona_field_name;
		std::string			sql_field_name;
		bool				primary_key;
		field_types			field_type;
		int					string_size;
		bool				is_expression;

		sql_field_mapping()
		{
			primary_key = 0;
			string_size = 100;
			field_type = field_types::ft_string;
		}

		virtual void get_json(json& _dest)
		{
			_dest.put_member("corona_field_name", corona_field_name);
			_dest.put_member("sql_field_name", sql_field_name);
			_dest.put_member("primary_key", primary_key);
			_dest.put_member("string_size", string_size);
		}

		virtual void put_json(std::vector<validation_error>& _errors, json& _src)
		{
			corona_field_name = _src["corona_field_name"];
			sql_field_name = _src["sql_field_name"];
			primary_key = (bool)_src["primary_key"];
			string_size = (int)_src["string_size"];
			is_expression = false;
			for (auto s : sql_field_name) {
				if (s == '(' or s == ')' or s == '*' or s == '+' or s == '/' or s == '-' or s == '!') {
					is_expression = true;
				}
			}
		}
	};

	using sql_field_mappings = std::vector<sql_field_mapping>;

	struct sql_statement
	{
	public:
		json		source_object;
		std::string string_to_execute;
		std::vector<sql_statement_parameter> parameters;
		std::vector<sql_field_mapping>		 result_mappings;
		std::vector<std::string>			 result_keys;
	};

	class sql_integration
	{
	public:
		sql_field_mappings mappings;
		std::string connection_name;
		std::string sql_table_name;
		std::vector<std::string> primary_key;
		std::vector<std::string> all_fields;

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			_dest.put_member("connection_name", connection_name);
			_dest.put_member("sql_table_name", sql_table_name);

			json jmappings = jp.create_array();
			for (auto mapping : mappings)
			{
				json jmapping = jp.create_object();
				mapping.get_json(jmapping);
				jmappings.push_back(jmapping);
			}
			_dest.put_member("mappings", jmappings);
		}

		virtual void put_json(std::vector<validation_error>& _errors, json& _src)
		{
			mappings.clear();
			connection_name = _src["connection_name"];
			sql_table_name = _src["sql_table_name"];
			primary_key.clear();

			if (connection_name.empty()) {
				validation_error ve;
				ve.class_name = "sql";
				ve.field_name = "connection";
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "missing connection name";
				_errors.push_back(ve);
			}

			if (sql_table_name.empty()) {
				validation_error ve;
				ve.class_name = "sql";
				ve.field_name = "table";
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "missing connection name";
				_errors.push_back(ve);
			}

			json jmappings = _src["mappings"];

			if (jmappings.array()) {
				for (auto mapping : jmappings)
				{
					sql_field_mapping sfm;
					sfm.put_json(_errors, mapping);
					if (sfm.primary_key) {
						primary_key.push_back(sfm.corona_field_name);
					}
					all_fields.push_back(sfm.corona_field_name);
					mappings.push_back(sfm);
				}
			}
			else {
				validation_error ve;
				ve.class_name = "sql";
				ve.field_name = "mappings";
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "sql mappings not found";
				_errors.push_back(ve);
			}
		}
		
		sql_field_mappings::iterator find(std::string _corona_name)
		{
			sql_field_mappings::iterator it = 				
				std::find_if(mappings.begin(), mappings.end(), [&_corona_name](sql_field_mapping& _mapping) -> bool
				{
					return _mapping.corona_field_name == _corona_name;
				});
			return it;
		}

	};

	class sql_statements
	{

	public:
		std::string put_template;
		std::string get_template;
		std::string erase_template;

		virtual std::string get_name() = 0;
		virtual sql_statement put_statement(json& _object, std::shared_ptr<sql_integration>& _integration) = 0;
		virtual sql_statement get_statement(json& _object, std::shared_ptr<sql_integration>& _integration) = 0;
		virtual sql_statement erase_statement(json& _object, std::shared_ptr<sql_integration>& _integration) = 0;
	};

	class sql_server_statements : public sql_statements
	{
	public:

		sql_server_statements()
		{
			put_template = R"(
	IF EXISTS( SELECT 1 
			FROM 
			$TABLE_NAME$ T
			WHERE $SELECT_KEY$
	)
	BEGIN
		UPDATE $TABLE_NAME$ 		
		SET	$UPDATE_FIELDS$
		WHERE $UPDATE_KEY$
	END
	ELSE
	BEGIN
		INSERT $TABLE_NAME$
		($INSERT_FIELDS$) 
		VALUES ($INSERT_VALUES$)
	END
)";

			get_template = R"(
	SELECT $SELECT_FIELDS$
	FROM   $TABLE_NAME$ T
	WHERE $SELECT_KEY$
	
)";

			erase_template = R"(
	DELETE 
	FROM 
			$TABLE_NAME$ T
	WHERE 
			$DELETE_KEY$
	
)";

		}

	public:

		std::string apply_template(std::string src, json& _values)
		{
			auto members = _values.get_members();
			for (auto member : members) {
				src = replace(src, member.first, (std::string)member.second);
			}
			return src;
		}

		virtual sql_statement put_statement(json& _object, std::shared_ptr<sql_integration>& _integration) override
		{
			sql_statement stmt;

			stmt.string_to_execute = put_template;
			stmt.source_object = _object;

			json_parser jp;

			json jvariables = jp.create_object();
			jvariables.put_member("$TABLE_NAME$", _integration->sql_table_name);

			std::stringstream select_key, update_fields, update_key, insert_fields, insert_values;
			std::string comma;

			int index = 1;

			// exists
			for (auto fld : _integration->mappings)
			{
				if (_object.has_member(fld.corona_field_name)) {
					if (fld.primary_key) {
						sql_statement_parameter ssp;
						ssp.corona_field_name = fld.corona_field_name;
						ssp.parameter_index = index++;
						ssp.sql_field_name = "?";
						ssp.field_type = fld.field_type;
						ssp.string_size = fld.string_size;

						stmt.parameters.push_back(ssp);

						select_key << comma;
						comma = " AND ";
						select_key << std::format("{0} = {1}", fld.sql_field_name, ssp.sql_field_name);
					}
				}
			}

			// update_fields
			comma = "";
			for (auto fld : _integration->mappings)
			{
				if (_object.has_member(fld.corona_field_name)) {
					sql_statement_parameter ssp;
					ssp.corona_field_name = fld.corona_field_name;
					ssp.parameter_index = index++;
					ssp.sql_field_name = "?";
					ssp.field_type = fld.field_type;
					ssp.string_size = fld.string_size;
					stmt.parameters.push_back(ssp);

					if (not fld.is_expression) {
						update_fields << comma;
						comma = ", ";
						update_fields << std::format("{0} = {1}", fld.sql_field_name, ssp.sql_field_name);
					}

				}
			}

			// update_key
			comma = "";
			for (auto fld : _integration->mappings)
			{
				if (_object.has_member(fld.corona_field_name) && fld.primary_key) {
					sql_statement_parameter ssp;
					ssp.corona_field_name = fld.corona_field_name;
					ssp.parameter_index = index++;
					ssp.sql_field_name = "?";
					ssp.field_type = fld.field_type;
					ssp.string_size = fld.string_size;
					stmt.parameters.push_back(ssp);

					update_key << comma;
					comma = " AND ";
					update_key << std::format("{0} = {1}", fld.sql_field_name, ssp.sql_field_name);
				}
			}

			// insert_fields
			comma = "";
			for (auto fld : _integration->mappings)
			{
				if (_object.has_member(fld.corona_field_name)) {
					if (not fld.is_expression) {
						insert_fields << comma;
						comma = ", ";
						insert_fields << fld.sql_field_name;
					}
				}
			}


			// insert_values
			comma = "";
			for (auto fld : _integration->mappings)
			{
				if (not fld.is_expression) {
					if (_object.has_member(fld.corona_field_name)) {
						sql_statement_parameter ssp;
						ssp.corona_field_name = fld.corona_field_name;
						ssp.parameter_index = index++;
						ssp.sql_field_name = "?";
						ssp.field_type = fld.field_type;
						ssp.string_size = fld.string_size;
						stmt.parameters.push_back(ssp);

						insert_values << comma;
						comma = ", ";
						insert_values << ssp.sql_field_name;
					}
				}
			}

			jvariables.put_member("$SELECT_KEY$", select_key.str());
			jvariables.put_member("$UPDATE_FIELDS$", update_fields.str());
			jvariables.put_member("$UPDATE_KEY$", update_key.str());
			jvariables.put_member("$INSERT_FIELDS$", insert_fields.str());
			jvariables.put_member("$INSERT_VALUES$", insert_values.str());
			stmt.string_to_execute = apply_template(stmt.string_to_execute, jvariables);
			return stmt;
		}

		virtual sql_statement get_statement(json& _object, std::shared_ptr<sql_integration>& _integration) override
		{
			sql_statement stmt;
			stmt.string_to_execute = get_template;
			stmt.source_object = _object;

			json_parser jp;

			json jvariables = jp.create_object();
			jvariables.put_member("$TABLE_NAME$", _integration->sql_table_name);
			
			std::stringstream select_fields, select_key;
			std::string comma;

			int index = 1;

			comma = "";
			// select fields
			for (auto fld : _integration->mappings)
			{
				select_fields << comma;
				comma = ", ";
				select_fields << fld.sql_field_name;
				stmt.result_mappings.push_back(fld);
				stmt.result_keys.push_back(fld.corona_field_name);
			}

			// select_key
			comma = "";
			for (auto fld : _integration->mappings)
			{
				if (_object.has_member(fld.corona_field_name)) {
					sql_statement_parameter ssp;
					ssp.corona_field_name = fld.corona_field_name;
					ssp.parameter_index = index++;
					ssp.sql_field_name = "?";
					ssp.field_type = fld.field_type;
					ssp.string_size = fld.string_size;
					stmt.parameters.push_back(ssp);

					select_key << comma;
					comma = " AND ";
					select_key << std::format("{0} = {1}", fld.sql_field_name, ssp.sql_field_name);
				}
			}

			std::string sselect_key = select_key.str();
			if (sselect_key.size() < 2) {
				sselect_key = "1 = 1";
			}
			jvariables.put_member("$SELECT_KEY$", sselect_key);
			jvariables.put_member("$SELECT_FIELDS$", select_fields.str());
			stmt.string_to_execute = apply_template(stmt.string_to_execute, jvariables);
			return stmt;
		}

		virtual sql_statement erase_statement(json& _object, std::shared_ptr<sql_integration>& _integration) override
		{
			sql_statement stmt;
			stmt.source_object = _object;
			stmt.string_to_execute = erase_template;

			json_parser jp;
			json jvariables = jp.create_object();
			jvariables.put_member("$TABLE_NAME$", _integration->sql_table_name);

			std::stringstream erase_key;
			std::string comma;

			int index = 1;

			// erase_key
			comma = "";
			for (auto fld : _integration->mappings)
			{
				if (_object.has_member(fld.corona_field_name)) {
					sql_statement_parameter ssp;
					ssp.corona_field_name = fld.corona_field_name;
					ssp.parameter_index = index++;
					ssp.sql_field_name = "?";
					stmt.parameters.push_back(ssp);

					erase_key << comma;
					comma = " AND ";
					erase_key << std::format("{0} = {1}", fld.sql_field_name, ssp.sql_field_name);
				}
			}


			std::string serase_key = erase_key.str();
			if (serase_key.size() < 2) {
				serase_key = "1 = 1";
			}
			jvariables.put_member("$DELETE_KEY", serase_key);
			stmt.string_to_execute = apply_template(stmt.string_to_execute, jvariables);
			return stmt;
		}

		virtual std::string get_name()
		{
			return "SQL Server";
		}
	};

	class sql_table : public xtable_interface
	{
		SQLHENV hEnv;
		SQLHDBC hDbc;
		SQLHSTMT hStmt;
		SQLRETURN ret;
		std::shared_ptr<sql_integration> sql;

		istring<2048> conn_string;
		istring<2048> completed_string;

		json sql_execute(std::string _statement, std::vector<std::string>& _paramters, json& _src)
		{
			json_parser jp;
		}

		bool sql_error(	SQLSMALLINT    hType,
						SQLHANDLE      hHandle,						
						RETCODE        retcode)
		{
			SQLSMALLINT iRec = 0;
			SQLINTEGER  iError;
			char		szMessage[4096];
			char        szState[SQL_SQLSTATE_SIZE + 1];

			if ((retcode == SQL_SUCCESS) or (retcode == SQL_SUCCESS_WITH_INFO))
				return false;

			if (retcode == SQL_INVALID_HANDLE)
			{
				return false;
			}

			bool warned = false;

			while (SQLGetDiagRecA(hType,
				hHandle,
				++iRec,
				(SQLCHAR *)szState,
				&iError,
				(SQLCHAR*)szMessage,
				(SQLSMALLINT)(sizeof(szMessage) / sizeof(char)),
				(SQLSMALLINT*)NULL) == SQL_SUCCESS)
			{
				// Hide data truncated..
				if (strncmp(szState, "01004", 5))
				{
					if (not warned) {
						system_monitoring_interface::active_mon->log_warning("SQL Statement Error");
						warned = true;
					}
					std::string error_message = std::format("[{0:5.5s}] {1} ({2})", std::string_view(szState), std::string_view(szMessage), (int)iError);
					system_monitoring_interface::active_mon->log_information(error_message, __FILE__, __LINE__);

					
				}
			}
		}

		json execute(sql_statement& _statement)
		{
			json_parser jp;
			json results_array = jp.create_array();

			SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

			SQLRETURN status;

			status = SQLPrepareA(hStmt, (SQLCHAR*)_statement.string_to_execute.c_str(), SQL_NTS);
			sql_error(SQL_HANDLE_STMT, hStmt, status);

			xrecord xparams;
			std::vector<point> offsets;

			for (auto& param : _statement.parameters)
			{
				auto binding = sql->find(param.corona_field_name);
				if (binding != std::end(sql->mappings)) {
					switch (binding->field_type) 
					{
					case field_types::ft_string:
						{
							std::string field = (std::string)_statement.source_object[param.corona_field_name];
							int nsz = field.size() + 1;
							int offset = xparams.bind(field);
							offsets.push_back({ offset, nsz });
						}
						break;
					case field_types::ft_double:
						{
							int offset = xparams.bind((double)_statement.source_object[param.corona_field_name]);
							offsets.push_back({ offset, 0 });

						}
						break;
					case field_types::ft_datetime:
						{
							int offset = xparams.bind((date_time)_statement.source_object[param.corona_field_name]);
							offsets.push_back({ offset, 0 });
					}
						break;
					case field_types::ft_int64:
						{
							int offset = xparams.bind((int64_t)_statement.source_object[param.corona_field_name]);
							offsets.push_back({ offset, 0 });
						}
						break;
					}
				}
			}

			int offset_idx = 0;
			for (auto& param : _statement.parameters)
			{
				char *dest = xparams.get_ptr(offsets[offset_idx].x);
				auto binding = sql->find(param.corona_field_name);
				if (binding != std::end(sql->mappings)) {
					switch (binding->field_type)
					{
					case field_types::ft_string:
					{
						status = SQLBindParameter(hStmt, param.parameter_index, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, offsets[offset_idx].y, 0, dest, 0, NULL);
						sql_error(SQL_HANDLE_STMT, hStmt, status);
					}
					break;
					case field_types::ft_double:
					{
						status = SQLBindParameter(hStmt, param.parameter_index, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, dest, 0, NULL);
						sql_error(SQL_HANDLE_STMT, hStmt, status);
					}
					break;
					case field_types::ft_datetime:
					{
						status = SQLBindParameter(hStmt, param.parameter_index, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP, 0, 0, dest, 0, NULL);
						sql_error(SQL_HANDLE_STMT, hStmt, status);
					}
					break;
					case field_types::ft_int64:
					{
						status = SQLBindParameter(hStmt, param.parameter_index, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, dest, 0, NULL);
						sql_error(SQL_HANDLE_STMT, hStmt, status);
					}
					break;
					}
				}
				offset_idx++;

			}

			xrecord xresults;
			offsets.clear();

			std::vector<std::string> all_columns;
			std::vector<std::string> mapped_columns;

			for (auto& binding : _statement.result_mappings)
			{
				switch (binding.field_type)
				{
				case field_types::ft_string:
				{
					std::string temp(binding.string_size, ' ');
					int offset = xresults.bind(temp);
					offsets.push_back({ offset, binding.string_size + 1 });
					// for the indicator
					offset = xresults.bind(0i64);
					offsets.push_back({ offset, 0 });
					all_columns.push_back(binding.corona_field_name);
					mapped_columns.push_back(binding.corona_field_name);
					all_columns.push_back(binding.corona_field_name + "_null");

				}
				break;
				case field_types::ft_double:
				{
					int offset = xresults.bind((double)0.0);
					offsets.push_back({ offset, 0 });
					// for the indicator
					offset = xresults.bind(0i64);
					offsets.push_back({ offset, 0 });
					all_columns.push_back(binding.corona_field_name);
					mapped_columns.push_back(binding.corona_field_name);
					all_columns.push_back(binding.corona_field_name + "_null");

				}
				break;
				case field_types::ft_datetime:
				{
					int offset = xresults.bind(date_time::now());
					offsets.push_back({ offset, 0 });
					// for the indicator
					offset = xresults.bind(0i64);
					offsets.push_back({ offset, 0 });
					all_columns.push_back(binding.corona_field_name);
					mapped_columns.push_back(binding.corona_field_name);
					all_columns.push_back(binding.corona_field_name + "_null");

				}
				break;
				case field_types::ft_int64:
				{
					int offset = xresults.bind((int64_t)0);
					offsets.push_back({ offset, 0 });
					// for the indicator
					offset = xresults.bind(0i64);
					offsets.push_back({ offset, 0 });
					all_columns.push_back(binding.corona_field_name);
					mapped_columns.push_back(binding.corona_field_name);
					all_columns.push_back(binding.corona_field_name + "_null");

				}
				break;
				}
			}

			offset_idx = 0;
			int result_idx = 1;
			for (auto& binding : _statement.result_mappings)
			{
				char* dest = xresults.get_ptr(offsets[offset_idx].x);
				SQLLEN* destind = (SQLLEN*)xresults.get_ptr(offsets[offset_idx + 1].x);

				switch (binding.field_type)
					{
					case field_types::ft_string:
					{
						status = SQLBindCol(hStmt, result_idx, SQL_C_CHAR, dest, binding.string_size, destind);
						sql_error(SQL_HANDLE_STMT, hStmt, status);
					}
					break;
					case field_types::ft_double:
					{
						status = SQLBindCol(hStmt, result_idx, SQL_C_DOUBLE, dest, 0, destind);
						sql_error(SQL_HANDLE_STMT, hStmt, status);
					}
					break;
					case field_types::ft_datetime:
					{
						status = SQLBindCol(hStmt, result_idx, SQL_C_TYPE_TIMESTAMP, dest, 0, destind);
						sql_error(SQL_HANDLE_STMT, hStmt, status);
					}
					break;
					case field_types::ft_int64:
					{
						status = SQLBindCol(hStmt, result_idx, SQL_C_SBIGINT, dest, 0, destind);
						sql_error(SQL_HANDLE_STMT, hStmt, status);
					}
					break;
					}
				offset_idx+=2;
				result_idx++;
			}

			status = SQLExecute(hStmt);
			sql_error(SQL_HANDLE_STMT, hStmt, status);
			
			if (_statement.result_mappings.size() > 0) 
			{
				status = SQLFetch(hStmt);
				sql_error(SQL_HANDLE_STMT, hStmt, status);
				while (status == SQL_SUCCESS or status == SQL_SUCCESS_WITH_INFO)
				{
					json new_object = jp.create_object();
					xresults.get_json(new_object, all_columns);
					json obj = jp.create_object();
					for (auto key : _statement.result_keys)
					{
						std::string null_key = key + "_null";
						int64_t null_ind = (int64_t)new_object[null_key];
						if (null_ind != SQL_NULL_DATA) {
							std::shared_ptr<json_value> jv = new_object.get_member_value(key);
							if (jv) {
								if (jv->get_field_type() == field_types::ft_string)
								{
									auto sjv = std::dynamic_pointer_cast<json_string>(jv);
									auto sjd = std::make_shared<json_string>();
									sjd->value = trim(sjv->value, null_ind);
									obj.object_impl()->members[key] = sjd;
								}
								else {
									obj.object_impl()->members[key] = jv;
								}
							}
						}
					}
					obj = new_object.extract(_statement.result_keys);
					results_array.push_back(obj);
					status = SQLFetch(hStmt);
					sql_error(SQL_HANDLE_STMT, hStmt, status);
				};
			}

			return results_array;
		}

		std::shared_ptr<sql_statements> statement_gen;

		SQLRETURN status;

	public:

		sql_table(std::shared_ptr<sql_integration> _sql, std::string _connection)
		{
			sql = _sql;
			hEnv = nullptr;
			hDbc = nullptr;
			hStmt = nullptr;
			statement_gen = std::make_shared<sql_server_statements>();

			int max_conn_size = conn_string.capacity();
			if (_connection.empty() or _connection.size() >= max_conn_size)
				return;

			conn_string = _connection;

			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);

			SQLSMALLINT completed_len = 0;
			status = SQLDriverConnectA(hDbc, NULL, (SQLCHAR*)conn_string.c_str_w(), SQL_NTS, (SQLCHAR *)completed_string.c_str_w(), completed_string.capacity()-4, &completed_len, SQL_DRIVER_NOPROMPT);
			sql_error(SQL_HANDLE_DBC, hDbc, status);
		}

		~sql_table()
		{
			if (hStmt) {
				SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
				hStmt = nullptr;
			}

			if (hDbc) {
				SQLDisconnect(hDbc);
				SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
				hDbc = nullptr;
			}

			if (hEnv) {
				SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
				hEnv = nullptr;
			}
		}

		virtual relative_ptr_type get_location() override
		{
			return -1;
		}

		virtual json get(json& key) override
		{
			sql_statement stmt;
			json results;

			stmt = statement_gen->get_statement(key, sql);
			results = execute(stmt);
			return results;
		}

		virtual void put(json& values) override
		{
			sql_statement stmt;

			stmt = statement_gen->put_statement(values, sql);
			execute(stmt);

		}

		virtual void put_array(json& values) override
		{
			for (auto obj : values) {
				put(obj);
			}
		}

		virtual void erase(json& key)  override
		{
			sql_statement stmt;
			json results;

			stmt = statement_gen->erase_statement(key, sql);
			results = execute(stmt);
		}

		virtual void erase_array(json& values) override
		{
			for (auto obj : values) {
				erase(obj);
			}
		}

		virtual json get_info() override
		{
			json_parser jp;
			json results = jp.create_object();
			results.put_member("table", std::string("ODBC"));
			results.put_member("language", statement_gen->get_name());
			std::string connection = conn_string.c_str();
			std::vector<std::string> sections = split(connection, ';');
			for (auto section : sections) {
				std::vector<std::string> terms = split(section, '=');
				if (terms.size() == 2) {
					std::string lower_name = terms[0];
					std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
					if (lower_name == "driver" or lower_name == "server" or lower_name == "database" or lower_name == "trustservercertificate") {
						results.put_member(terms[0], terms[1]);
					}
				}
			}
			return results;
		}

		virtual xfor_each_result for_each(json _object, std::function<relative_ptr_type(json& _item)> _process) override
		{

			xfor_each_result result = {};
			json results = get(_object);
			for (auto obj : results) 
			{
				if (_process(obj) != null_row)
				{
					result.count++;
					result.is_any = true;
				}
				else {
					result.is_any = false;
				}
			}
			return result;
		}

		virtual json select(json _object, std::function<json(json& _item)> _process) override
		{
			json_parser jp;
			json raw_results = get(_object);
			json results = jp.create_array();
			for (auto obj : raw_results)
			{
				json jresult = _process(obj);
				if (jresult.object()) {
					results.push_back(jresult);
				}
			}
			return results;
		}

		virtual void clear() override
		{
			sql_statement stmt;
			stmt.string_to_execute = "DELETE FROM [" + sql->sql_table_name + "]";
			execute(stmt);
		}

	};

	class corona_connections
	{

		std::map<std::string, std::string> connections;
		shared_lockable locker;

	public:

		std::string get_connection(std::string _source)
		{
			read_scope_lock lockit(locker);
			std::string result;
			std::string env_path = "connection_" + _source;
			std::transform(env_path.begin(), env_path.end(), env_path.begin(), ::toupper);

			result = get_environment(env_path);
			if (result.empty()) {
				result = connections[_source];
			}
			return result;
		}

		void set_connection(std::string _source, std::string _connection)
		{
			write_scope_lock lockit(locker);
			connections.insert_or_assign(_source, _connection);
		}

	};

}
