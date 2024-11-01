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
		std::string sql_view_name;
		std::string sql_table_name;

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			_dest.put_member("connection_name", connection_name);
			_dest.put_member("sql_view_name", sql_view_name);
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
			sql_view_name = _src["sql_view_name"];
			sql_table_name = _src["sql_table_name"];

			json jmappings = _src["mappings"];
			for (auto mapping : jmappings)
			{
				sql_field_mapping sfm;
				sfm.put_json(_errors, mapping);
			}
		}
		
		sql_field_mappings::iterator find(std::string _corona_name)
		{
			sql_field_mappings::iterator it = 				
				std::find(mappings.begin(), mappings.end(), [&_corona_name](sql_field_mapping& _mapping) -> bool
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
			[$TABLE_NAME$] T
			WHERE $SELECT_KEY$
	)
	BEGIN
		UPDATE $TABLE_NAME$ 		
		SET	$UPDATE_FIELDS$
		WHERE $UPDATE_KEY$
	END
	ELSE
	BEGIN
		INSERT [$TABLE_NAME$] 
		($INSERT_FIELDS$) 
		VALUES ($INSERT_VALUES$)
	END
)";

			get_template = R"(
	SELECT $SELECT_FIELDS$
	FROM 
			[$VIEW_NAME$] T
	WHERE $WHERE_CLAUSE$
	
)";

			erase_template = R"(
	DELETE 
	FROM 
			[$TABLE_NAME$] T
	WHERE 
			$DELETE_KEY$
	
)";

		}

	public:

		std::string apply_template(std::string src, json& _values)
		{
			auto members = _values.get_members();
			for (auto member : members) {
				replace(src, member.first, (std::string)member.second);
			}
		}

		virtual sql_statement put_statement(json& _object, std::shared_ptr<sql_integration>& _integration) override
		{
			sql_statement stmt;

			json_parser jp;

			json jvariables = jp.create_object();
			jvariables.put_member("$TABLE_NAME$", _integration->sql_table_name);
			jvariables.put_member("$VIEW_NAME$", _integration->sql_view_name);

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
					stmt.parameters.push_back(ssp);

					update_fields << comma;
					comma = ", ";
					update_fields << std::format("{0} = {1}", fld.sql_field_name, ssp.sql_field_name);
				}
			}

			// update_key
			comma = "";
			for (auto fld : _integration->mappings)
			{
				if (_object.has_member(fld.corona_field_name)) {
					sql_statement_parameter ssp;
					ssp.corona_field_name = fld.corona_field_name;
					ssp.parameter_index = index++;
					ssp.sql_field_name = "?";
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
					insert_fields << comma;
					comma = ", ";
					update_fields << fld.sql_field_name;
				}
			}

			// insert_values
			comma = "";
			for (auto fld : _integration->mappings)
			{
				if (_object.has_member(fld.corona_field_name)) {
					sql_statement_parameter ssp;
					ssp.corona_field_name = fld.corona_field_name;
					ssp.parameter_index = index++;
					ssp.sql_field_name = "?";
					stmt.parameters.push_back(ssp);

					update_fields << comma;
					comma = ", ";
					update_fields << ssp.sql_field_name;
				}
			}

			jvariables.put_member("$SELECT_KEY$", select_key.str());
			jvariables.put_member("$UPDATE_FIELDS$", update_fields.str());
			jvariables.put_member("$UPDATE_KEY", update_key.str());
			jvariables.put_member("$INSERT_FIELDS$", insert_fields.str());
			jvariables.put_member("$INSERT_VALUES", insert_values.str());
			stmt.string_to_execute = apply_template(stmt.string_to_execute, jvariables);
			return stmt;
		}

		virtual sql_statement get_statement(json& _object, std::shared_ptr<sql_integration>& _integration) override
		{
			sql_statement stmt;

			json_parser jp;

			json jvariables = jp.create_object();
			jvariables.put_member("$TABLE_NAME$", _integration->sql_table_name);
			jvariables.put_member("$VIEW_NAME$", _integration->sql_view_name);

			std::stringstream select_fields, select_key;
			std::string comma;

			int index = 1;

			comma = "";
			// select fields
			for (auto fld : _integration->mappings)
			{
				if (_object.has_member(fld.corona_field_name)) {
					select_key << comma;
					comma = ", ";
					select_key << fld.sql_field_name;
					stmt.result_mappings.push_back(fld);
					stmt.result_keys.push_back(fld.corona_field_name);
				}
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
					stmt.parameters.push_back(ssp);

					select_key << comma;
					comma = " AND ";
					select_key << std::format("{0} = {1}", fld.sql_field_name, ssp.sql_field_name);
				}
			}

			jvariables.put_member("$SELECT_KEY$", select_key.str());
			jvariables.put_member("$SELECT_FIELDS$", select_fields.str());
			stmt.string_to_execute = apply_template(stmt.string_to_execute, jvariables);
			return stmt;
		}

		virtual sql_statement erase_statement(json& _object, std::shared_ptr<sql_integration>& _integration) override
		{
			sql_statement stmt;

			json_parser jp;

			json jvariables = jp.create_object();
			jvariables.put_member("$TABLE_NAME$", _integration->sql_table_name);
			jvariables.put_member("$VIEW_NAME$", _integration->sql_view_name);

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

			jvariables.put_member("$DELETE_KEY", erase_key.str());
			stmt.string_to_execute = apply_template(stmt.string_to_execute, jvariables);
			return stmt;
		}
	};

	class sql_connection
	{
		SQLHENV hEnv;
		SQLHDBC hDbc;
		SQLHSTMT hStmt;
		SQLRETURN ret;
		std::shared_ptr<sql_integration> sql;

		char conn_string[2048];

		json sql_execute(std::string _statement, std::vector<std::string>& _paramters, json& _src)
		{
			json_parser jp;
		}

		bool sql_error(	SQLHANDLE      hHandle,
						SQLSMALLINT    hType,
						RETCODE        RetCode)
		{
			SQLSMALLINT iRec = 0;
			SQLINTEGER  iError;
			char		szMessage[4096];
			char        szState[SQL_SQLSTATE_SIZE + 1];

			if (RetCode == SQL_INVALID_HANDLE)
			{
				return;
			}

			while (SQLGetDiagRec(hType,
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
					std::string error_message = std::format("[{0:5.5s}] {1} ({%d})", szState, szMessage, iError);

				}
			}
		}

		json execute(sql_statement& _statement)
		{
			json_parser jp;
			json results_array = jp.create_array();

			SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);

			SQLPrepare(hStmt, (SQLCHAR*)_statement.string_to_execute.c_str(), SQL_NTS);

			xrecord xparams;
			std::vector<int> offsets;

			for (auto& param : _statement.parameters)
			{
				auto binding = sql->find(param.corona_field_name);
				if (binding != std::end(sql->mappings)) {
					switch (binding->field_type) 
					{
					case field_types::ft_string:
						{
							std::string field = (std::string)_statement[param.corona_field_name];
							int nsz = field.size() + 1;
							int offset = xparams.bind(field);
							offsets.push_back(offset);
						}
						break;
					case field_types::ft_double:
						{
							int offset = xparams.bind((double)_statement[param.corona_field_name]);
							offsets.push_back(offset);
						}
						break;
					case field_types::ft_datetime:
						{
							int offset = xparams.bind((date_time)_statement[param.corona_field_name]);
							offsets.push_back(offset);
						}
						break;
					case field_types::ft_int64:
						{
							int offset = xparams.bind((int64_t)_statement[param.corona_field_name]);
							offsets.push_back(offset);
						}
						break;
					}
				}
			}

			int offset_idx = 0;
			for (auto& param : _statement.parameters)
			{
				char *dest = xparams.get_ptr(offsets[offset_idx]);
				auto binding = sql->find(param.corona_field_name);
				if (binding != std::end(sql->mappings)) {
					switch (binding->field_type)
					{
					case field_types::ft_string:
					{
						std::string field = (std::string)_statement[param.corona_field_name];
						int nsz = field.size() + 1;
						SQLBindParameter(hStmt, param.parameter_index, SQL_PARAM_INPUT, SQL_C_CHAR, SQL_CHAR, nsz, 0, dest, 0, NULL);
					}
					break;
					case field_types::ft_double:
					{
						SQLBindParameter(hStmt, param.parameter_index, SQL_PARAM_INPUT, SQL_C_DOUBLE, SQL_DOUBLE, 0, 0, dest, 0, NULL);
					}
					break;
					case field_types::ft_datetime:
					{
						SQLBindParameter(hStmt, param.parameter_index, SQL_PARAM_INPUT, SQL_C_TYPE_TIMESTAMP, SQL_TIMESTAMP, 0, 0, dest, 0, NULL);
					}
					break;
					case field_types::ft_int64:
					{
						SQLBindParameter(hStmt, param.parameter_index, SQL_PARAM_INPUT, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, dest, 0, NULL);
					}
					break;
					}
				}
				offset_idx++;

			}

			xrecord xresults;
			offsets.clear();

			for (auto& binding : _statement.result_mappings)
			{
				switch (binding.field_type)
				{
				case field_types::ft_string:
				{
					std::string temp(binding.string_size * 2, ' ');
					int offset = xresults.bind(temp);
					offsets.push_back(offset);
				}
				break;
				case field_types::ft_double:
				{
					int offset = xresults.bind((double)0.0);
					offsets.push_back(offset);
				}
				break;
				case field_types::ft_datetime:
				{
					int offset = xresults.bind(date_time::now());
					offsets.push_back(offset);
				}
				break;
				case field_types::ft_int64:
				{
					int offset = xresults.bind((int64_t)0);
					offsets.push_back(offset);
				}
				break;
				}
			}

			int offset_idx = 0;
			int result_idx = 1;
			for (auto& binding : _statement.result_mappings)
			{
				char* dest = xresults.get_ptr(offsets[offset_idx]);
				offset_idx++;

				switch (binding.field_type)
					{
					case field_types::ft_string:
					{
						SQLBindCol(hStmt, offset_idx, SQL_C_CHAR, dest, binding.string_size, nullptr);
					}
					break;
					case field_types::ft_double:
					{
						SQLBindCol(hStmt, offset_idx, SQL_C_DOUBLE, dest, 0, nullptr);
					}
					break;
					case field_types::ft_datetime:
					{
						SQLBindCol(hStmt, offset_idx, SQL_C_TYPE_TIMESTAMP, dest, nullptr);
					}
					break;
					case field_types::ft_int64:
					{
						SQLBindCol(hStmt, offset_idx, SQL_C_SBIGINT, dest, 0, nullptr);
					}
					break;
					}
			}

			SQLExecute(hStmt);

			if (_statement.result_mappings.size() > 0) 
			{
				while (SQLFetch(hStmt) == SQL_SUCCESS) 
				{
					json new_object = jp.create_object();
					xresults.get_json(new_object, _statement.result_keys);
					results_array.push_back(new_object);
				};
			}
		}

		std::shared_ptr<sql_statements> statement_gen;

	public:

		sql_connection(std::shared_ptr<sql_integration> _sql)
		{
			sql = _sql;
			hEnv = nullptr;
			hDbc = nullptr;
			hStmt = nullptr;
			conn_string[0] = 0;
			statement_gen = std::make_shared<sql_server_statements>();
		}

		~sql_connection()
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

		SQLRETURN connect(std::string _connection)
		{
			int max_conn_size = (sizeof(conn_string) - 16);
			if (_connection.empty() or _connection.size() >= max_conn_size)
				return;

			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			SQLCHAR outstr[1024];
			SQLSMALLINT outstrlen = 0;

			std::strcpy(conn_string, _connection.c_str());

			ret = SQLDriverConnect(hDbc, NULL, (SQLCHAR *) & conn_string[0], SQL_NTS,
				outstr, sizeof(outstr), &outstrlen,
				SQL_DRIVER_NOPROMPT);

			return ret;
		}

		virtual void put(json& values)
		{
			sql_statement stmt;

			stmt = statement_gen->put_statement(values, sql);
			execute(stmt);

		}

		virtual void put_array(json& values)
		{
			for (auto obj : values) {
				put(obj);
			}
		}

		virtual json get(json key)
		{
			sql_statement stmt;
			json results;

			stmt = statement_gen->get_statement(key, sql);
			results = execute(stmt);
			return results;
		}

		virtual json erase(json key)
		{
			sql_statement stmt;
			json results;

			stmt = statement_gen->get_statement(key, sql);
			results = execute(stmt);
			return results;
		}

		virtual json erase_array(json values)
		{
			for (auto obj : values) {
				erase(obj);
			}
		}

	};

}
