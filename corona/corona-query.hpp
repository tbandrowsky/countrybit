
#ifndef CORONA_QUERY_H
#define CORONA_QUERY_H

namespace corona
{

	class query_context_base
	{
		json froms;

	public:

		virtual void get_json(json& _dest)
		{
			_dest.put_member("class_name", "query_context");
			_dest.put_member("froms", froms);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (not _src.has_members(missing, { "class_name", "froms" })) {
				system_monitoring_interface::global_mon->log_warning("query_context missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			froms = _src["froms"];
		}


		virtual json get_data(std::string _query) 
		{
			json_parser jp;
			json query_s = jp.parse_query(_query);
			json j;

			std::string query_si = query_s["source_name"];
			std::string query_p = query_s["query_path"];

			if (query_si.empty()) 
			{
				std::string msg = std::format("{0} query data request does not have a source_name: prefix", _query);
				system_monitoring_interface::global_mon->log_warning(msg);
			}
			else 
			{
				j = froms[query_si];
				if (j.empty()) {
					std::string msg = std::format("{0} query data source not found", _query);
					system_monitoring_interface::global_mon->log_warning(msg);
				}

				j = j.query(query_p);
				j = j["value"];
			}

			return j;
		}

		virtual void set_data_source(std::string _name, json _data)
		{
			if (froms.empty()) {
				json_parser jp;
				froms = jp.create_object();
			}
			froms.put_member(_name, _data);
		}

		virtual void fill_data_sources(std::function<json(std::string _name)> _filler)
		{
			if (froms.object()) {
				auto frms = froms.get_members();
				for (auto frm : frms) {
					json j = _filler(frm.first);
					froms.put_member(frm.first, j);
				}
			}
		}
	};

	class query_stage
	{
	public:
		std::string stage_name;
		json		stage_output;
		double		execution_time_seconds;

		virtual std::string term_name() { return "stage"; }
		virtual json process(query_context_base *_src) { return stage_output; }

		virtual void get_json(json& _dest)
		{
			_dest.put_member("class_name", "query_stage");
			_dest.put_member("stage_name", stage_name);
			_dest.put_member("stage_output", stage_output);
			_dest.put_member("execution_time_seconds", execution_time_seconds);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (not _src.has_members(missing, { "class_name", "stage_name" })) {
				system_monitoring_interface::global_mon->log_warning("query_project missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			stage_name = _src["stage_name"];
			stage_output = _src["stage_output"];
		}

	};

	void get_json(json& _dest, std::shared_ptr<query_stage>& _src);
	void put_json(std::shared_ptr<query_stage>& _dest, json _src);

	class query_context : public query_context_base
	{
	public:

		std::vector<std::shared_ptr<query_stage>> stages;

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			query_context_base::get_json(_dest);

			_dest.put_member("class_name", "query_context");
			json stage_array = jp.create_array();
			for (auto st : stages) {
				json stj = jp.create_object();
				st->get_json(stj);
				stage_array.push_back(stj);
			}
			_dest.put_member("stages", stage_array);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			stages.clear();

			if (not _src.has_members(missing, { "class_name", "stages" })) {
				system_monitoring_interface::global_mon->log_warning("query_context missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			query_context_base::put_json(_src);

			json jstgs = _src["stages"];
			if (jstgs.array()) {
				for (auto item : jstgs) {
					std::shared_ptr<query_stage> new_query;
					corona::put_json(new_query, item);
					if (new_query) {
						stages.push_back(new_query);
					}
				}
			}
			else if (jstgs.object()) {
				std::shared_ptr<query_stage> new_query;
				corona::put_json(new_query, jstgs);
				if (new_query) {
					stages.push_back(new_query);
				}
			}
		}

		virtual json get_data(std::string _data_name) {
			json j;

			j = query_context_base::get_data(_data_name);
			if (j.empty()) {
				auto found = std::find_if(
					stages.begin(), 
					stages.end(), 
					[_data_name](std::shared_ptr<query_stage>& _item) ->bool {
						return _item->stage_name == _data_name;
					});
				while (found != std::end(stages) and j.empty()) {
					j = found->get()->stage_output;
					found++;
				}
			}
			return j;
		}

		virtual json run()
		{
			json output_op;
			for (auto stage : stages) {
				output_op = stage->process(this);
			}
			return output_op;
		}
	};

	class query_predicate
	{
	public:
		virtual std::string term_name() { return "predicate"; }
		virtual bool accepts(query_context_base *_qcb, json _src) { return true; }

		virtual void get_json(json& _dest)
		{
		}

		virtual void put_json(json& _src)
		{
		}

	};

	void put_json(std::shared_ptr<query_predicate>& _dest, json _src);

	class query_filter : public query_stage
	{
	public:

		std::string stage_input_name;

		std::shared_ptr<query_predicate> predicate;
		virtual std::string term_name() { return "filter"; }

		virtual json process(query_context_base* _src) {
			timer tx;
			json_parser jp;
			json stage_input = _src->get_data(stage_input_name);
			if (stage_input.object()) {
				json result = jp.create_array();
				if (predicate and predicate->accepts(_src, stage_input)) {
					result.push_back(stage_input);
					stage_output = result;
					return stage_output;
				}
			}
			else if (stage_input.array()) {
				json result = jp.create_array();
				for (auto item : stage_input) {
					if (predicate and predicate->accepts(_src, item)) {
						result.push_back(item);
					}
				}
				stage_output = result;
				return result;
			}
			execution_time_seconds = tx.get_elapsed_seconds();
			return stage_output;  
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			query_stage::get_json(_dest);
			json jpredicate = jp.create_object();
			if (predicate) {
				predicate->get_json(jpredicate);
				_dest.put_member("predicate", jpredicate);
			}
			_dest.put_member("class_name", "filter");
			_dest.put_member("stage_input_name", stage_input_name);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (not _src.has_members(missing, { "class_name", "predicate" })) {
				system_monitoring_interface::global_mon->log_warning("query_filter missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			query_stage::put_json(_src);
			json jpredicate = _src["predicate"];
			corona::put_json(predicate, jpredicate);
			stage_input_name = _src["stage_input_name"];
		}

	};

	class query_join : public query_stage
	{
	public:

		std::string resultname1;
		std::string resultname2;
		std::string source1;
		std::string source2;
		std::vector<std::string> keys;

		virtual std::string term_name() { return "join"; }

		virtual json process(query_context_base* _src) {
			timer tx;
			json_parser jp;
			stage_output = jp.create_array();

			json query1 = _src->get_data(source1);
			json query2 = _src->get_data(source2);

			if (query1.object()) {
				json t = jp.create_array();
				t.push_back(query1);
				query1 = t;
			}
		
			if (query2.object()) {
				json t = jp.create_array();
				t.push_back(query2);
				query2 = t;
			}

			stage_output = jp.create_array();

			if (query1.array() and query2.array()) {
				for (auto item1 : query1) {
					json key1 = item1.extract(keys);
					for (auto item2 : query2) {
						json key2 = item2.extract(keys);
						if (key1.compare(key2) == 0) {
							json new_item = jp.create_object();
							new_item.merge(key1);
							new_item.put_member(resultname1, item1);
							new_item.put_member(resultname2, item2);
							stage_output.push_back(new_item);
						}
					}
				}
			}
			execution_time_seconds = tx.get_elapsed_seconds();

			return stage_output;
		}

		virtual void get_json(json& _dest)
		{
			query_stage::get_json(_dest);
			_dest.put_member("class_name", "join");
			_dest.put_member("resultname1", resultname1);
			_dest.put_member("resultname2", resultname2);
			_dest.put_member("source1", source1);
			_dest.put_member("source2", source2);
			std::string skeys = join(keys, ",");
			_dest.put_member("keys", skeys);
			_dest.put_member("execution_time_seconds", execution_time_seconds);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (not _src.has_members(missing, { "class_name", "resultname1", "resultname2", "source1", "source2", "keys" })) {
				system_monitoring_interface::global_mon->log_warning("join missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}
			query_stage::put_json(_src);

			resultname1 = _src["resultname1"];
			resultname2 = _src["resultname2"];
			source1 = _src["source1"];
			source2 = _src["source2"];
			std::string skeys = _src["keys"];
			keys = split(skeys, ',');

		}

	};

	class filter_contains : public query_predicate
	{
	public:

		std::string textsrc;

		virtual std::string term_name() { return "contains"; }

		virtual bool accepts(query_context_base* _qcb, json _src)
		{
			std::string search_text = _qcb->get_data(textsrc);
			return _src.contains_text(search_text);
		}

		virtual void get_json(json& _dest)
		{
			query_predicate::get_json(_dest);
			_dest.put_member("class_name", "contains");
			_dest.put_member("textsrc", textsrc);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "class_name", "textsrc" })) {
				system_monitoring_interface::global_mon->log_warning("filter missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			query_predicate::put_json(_src);
			textsrc = _src["textsrc"];
		}
	};

	class filter_gt : public query_predicate
	{
	public:

		std::string valuepath;
		json value;

		virtual std::string term_name() { return "gt"; }

		virtual bool accepts(query_context_base* _qcb, json _src)
		{
			json s = _src.query(valuepath);
			return s["value"].gt(value);
		}

		virtual void get_json(json& _dest)
		{
			query_predicate::get_json(_dest);
			_dest.put_member("class_name", "gt");
			_dest.put_member("valuepath", valuepath);
			_dest.put_member("value", value);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "class_name", "valuepath", "value" })) {
				system_monitoring_interface::global_mon->log_warning("filter missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			query_predicate::put_json(_src);
			valuepath = _src["valuepath"];
			value = _src["value"];
		}

	};

	class filter_lt : public query_predicate
	{
	public:

		std::string valuepath;
		json value;

		virtual std::string term_name() { return "lt"; }

		virtual bool accepts(query_context_base* _qcb, json _src)
		{
			json s = _src.query(valuepath);
			return s["value"].lt(value);
		}

		virtual void get_json(json& _dest)
		{
			query_predicate::get_json(_dest);
			_dest.put_member("class_name", "lt");
			_dest.put_member("valuepath", valuepath);
			_dest.put_member("value", value);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "class_name", "valuepath", "value" })) {
				system_monitoring_interface::global_mon->log_warning("filter missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			query_predicate::put_json(_src);
			valuepath = _src["valuepath"];
			value = _src["value"];
		}

	};

	class filter_eq : public query_predicate
	{
	public:

		std::string valuepath;
		json value;

		virtual std::string term_name() { return "eq"; }

		virtual bool accepts(query_context_base* _qcb, json _src)
		{
			json s = _src.query(valuepath);
			return s["value"].eq(value);
		}

		virtual void get_json(json& _dest)
		{
			query_predicate::get_json(_dest);
			_dest.put_member("class_name", "eq");
			_dest.put_member("valuepath", valuepath);
			_dest.put_member("value", value);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "class_name", "valuepath", "value" })) {
				system_monitoring_interface::global_mon->log_warning("filter missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			query_predicate::put_json(_src);
			valuepath = _src["valuepath"];
			value = _src["value"];
		}
	};

	class filter_gte : public query_predicate
	{
	public:

		std::string valuepath;
		json value;

		virtual std::string term_name() { return "gte"; }

		virtual bool accepts(query_context_base* _qcb, json _src)
		{
			json s = _src.query(valuepath);
			return s["value"].gt(value) or s["value"].eq(value);
		}

		virtual void get_json(json& _dest)
		{
			query_predicate::get_json(_dest);
			_dest.put_member("class_name", "gte");
			_dest.put_member("valuepath", valuepath);
			_dest.put_member("value", value);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "class_name", "valuepath", "value" })) {
				system_monitoring_interface::global_mon->log_warning("filter missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			query_predicate::put_json(_src);
			valuepath = _src["valuepath"];
			value = _src["value"];
		}
	};

	class filter_lte: public query_predicate
	{
	public:

		std::string valuepath;
		json value;

		virtual std::string term_name() { return "lte"; }

		virtual bool accepts(query_context_base* _qcb, json _src)
		{
			json s = _src.query(valuepath);
			return s["value"].gt(value) or s["value"].eq(value);
		}

		virtual void get_json(json& _dest)
		{
			query_predicate::get_json(_dest);
			_dest.put_member("class_name", "lte");
			_dest.put_member("valuepath", valuepath);
			_dest.put_member("value", value);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "class_name", "valuepath", "value" })) {
				system_monitoring_interface::global_mon->log_warning("filter missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			query_predicate::put_json(_src);
			valuepath = _src["valuepath"];
			value = _src["value"];
		}
	};

	class filter_between : public query_predicate
	{
	public:

		std::shared_ptr<filter_gte> start;
		std::shared_ptr<filter_lt> stop;

		virtual std::string term_name() { return "between"; }

		virtual bool accepts(query_context_base* _srcx, json _src)
		{
			return	start->accepts(_srcx, _src) and stop->accepts(_srcx, _src);
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			query_predicate::get_json(_dest);

			_dest.put_member("class_name", "between");

			json jstart = jp.create_object();
			if (start) {
				start->get_json(jstart);
				_dest.put_member("start", jstart);
			}

			json jstop = jp.create_object();
			if (stop) {
				stop->get_json(jstop);
				_dest.put_member("stop", jstop);
			}
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "class_name", "start", "stop" })) {
				system_monitoring_interface::global_mon->log_warning("filter missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			query_predicate::put_json(_src);

			json jstart = _src["start"];
			start = std::make_shared<filter_gte>();
			start->put_json(jstart);

			json jstop = _src["stop"];
			stop = std::make_shared<filter_lt>();
			stop->get_json(jstop);

		}

	};

	class filter_in : public query_predicate
	{
	public:

		std::string src_path;
		std::string items_path;

		virtual std::string term_name() { return "in"; }

		virtual bool accepts(query_context_base* _qcb, json _src)
		{
			json tsrcc = _src.query(src_path);
			json srcc = tsrcc["value"];
			json items = _qcb->get_data(items_path);
			for (auto item : items) {
				if (item.compare(srcc) == 0)
					return true;
			}
			return false;
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			query_predicate::get_json(_dest);

			_dest.put_member("class_name", "in");
			_dest.put_member("src_path", src_path);
			_dest.put_member("items_path", items_path);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "class_name", "src_path", "items_path" })) {
				system_monitoring_interface::global_mon->log_warning("filter missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			query_predicate::put_json(_src);
		}

	};

	class filter_all : public query_predicate
	{
	public:

		virtual std::string term_name() { return "all"; }
		std::vector<std::shared_ptr<query_predicate>> conditions;

		virtual bool accepts(query_context_base* _qcb, json _src)
		{
			return std::all_of(conditions.begin(), conditions.end(), [_qcb, _src](std::shared_ptr<query_predicate>& _condition)-> bool
				{
					return _condition->accepts(_qcb, _src);
				});
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			query_predicate::get_json(_dest);

			_dest.put_member("class_name", "in");
			json jconditions = jp.create_array();
			for (auto cond : conditions) {
				json jcond = jp.create_object();
				cond->get_json(jcond);
			}
			_dest.put_member("conditions", jconditions);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "class_name", "conditions" })) {
				system_monitoring_interface::global_mon->log_warning("filter missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			query_predicate::put_json(_src);

			json_parser jp;
			json jconditions = _src["conditions"];
			conditions.clear();

			for (auto jcondition : jconditions)
			{
				std::shared_ptr<query_predicate> new_condition;
				corona::put_json(new_condition, jcondition);
				conditions.push_back(new_condition);
			}
		}
	};

	class filter_any : public query_predicate
	{
	public:
		virtual std::string term_name() { return "any"; }

		std::vector<std::shared_ptr<query_predicate>> conditions;

		virtual bool accepts(query_context_base* _qcb, json _src)
		{
			return std::any_of(conditions.begin(), conditions.end(), [_qcb, _src](std::shared_ptr<query_predicate>& _condition) -> bool
				{
					return _condition->accepts(_qcb, _src);
				});
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			query_predicate::get_json(_dest);

			_dest.put_member("class_name", "in");
			json jconditions = jp.create_array();
			for (auto cond : conditions) {
				if (cond) {
					json jcond = jp.create_object();
					cond->get_json(jcond);
					jconditions.push_back(jcond);
				}
			}
			_dest.put_member("conditions", jconditions);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;
			if (not _src.has_members(missing, { "class_name", "conditions" })) {
				system_monitoring_interface::global_mon->log_warning("filter missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			query_predicate::put_json(_src);

			json_parser jp;
			json jconditions = _src["conditions"];
			conditions.clear();

			for (auto jcondition : jconditions)
			{
				std::shared_ptr<query_predicate> new_condition;
				corona::put_json(new_condition, jcondition);
				if (new_condition) {
					conditions.push_back(new_condition);
				}
			}
		}
	};

	class query_project : public query_stage {
	public:

		std::string source_name;
		json		projection;

		virtual std::string term_name() { return "project"; }

		virtual void get_json(json& _dest)
		{
			_dest.put_member("class_name", "project");
			_dest.put_member("source_name", source_name);
			_dest.put_member("projection", projection);
		}

		virtual void put_json(json& _src)
		{
			std::vector<std::string> missing;

			if (not _src.has_members(missing, { "class_name", "source_name", "projection" })) {
				system_monitoring_interface::global_mon->log_warning("query_project missing:");
				std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
					system_monitoring_interface::global_mon->log_warning(s);
					});
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
				return;
			}

			source_name = _src["source_name"];
			projection = _src["projection"];
		}

		virtual json process(query_context_base* _src) {
			timer tx;
			json_parser jp;
			auto members = projection.get_members();

			if (source_name.empty()) {
				stage_output = jp.create_object();
				for (auto member : members) {
					std::string path = member.second;
					json query = jp.parse_query(path);
					if (query.has_member("source_name")) {
						json data = _src->get_data(path);
						stage_output.put_member(member.first, data);
					}
					else 
					{
						std::string msg = std::format("projection member '{0}' has a path '{1}' that requires a projection source and none was supplied.", member.first, path);
						comm_bus_interface::global_bus->log_warning(msg, __FILE__, __LINE__);
					}
				}
			}
			else 
			{
				json projection_source = _src->get_data(source_name);
				if (projection_source.object())
				{
					stage_output = jp.create_object();

					for (auto member : members) {
						std::string path = member.second;
						json query = jp.parse_query(path);
						if (query.has_member("source_name")) {
							json data = _src->get_data(path);
							stage_output.put_member(member.first, data);
						}
						else
						{
							std::string data_path = query["query_path"];
							json t = projection_source.query(data_path);
							t = t["value"];
							stage_output.put_member(member.first, t);
						}
					}
				}
				else if (projection_source.array())
				{
					stage_output = jp.create_array();
					for (auto arr_item : projection_source) {
						json new_item = jp.create_object();
						for (auto member : members) {
							std::string path = member.second;
							json query = jp.parse_query(path);
							if (query.has_member("source_name")) {
								std::string data_path = query["query_path"];
								json data = _src->get_data(path);
								json t = data.query(data_path);
								t = t["value"];
								new_item.put_member(member.first, t);
							}
							else
							{
								std::string data_path = query["query_path"];
								json t = arr_item.query(data_path);
								t = t["value"];
								new_item.put_member(member.first, t);
							}
						}
						stage_output.push_back(new_item);
					}
				}
				else 
				{
					stage_output = jp.create_object();
				}
			}

			execution_time_seconds = tx.get_elapsed_seconds();
			return stage_output; 
		}
	};

	void get_json(json& _dest, std::shared_ptr<query_stage>& _src)
	{
		_src->get_json(_dest);
	}

	void put_json(std::shared_ptr<query_stage>& _dest, json _src)
	{
		std::vector<std::string> missing;

		if (_src.empty())
			return;

		if (not _src.has_members(missing, { "class_name" })) {
			system_monitoring_interface::global_mon->log_warning("query stage missing class_name.");
			if (_src.size()) {
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
			}
			return;
		}

		if (_src.has_member("class_name"))
		{
			std::string class_name = _src["class_name"];

			if (class_name == "filter")
			{
				_dest = std::make_shared<query_filter>();
				_dest->put_json(_src);
			}
			else if (class_name == "project")
			{
				_dest = std::make_shared<query_project>();
				_dest->put_json(_src);
			}
			else if (class_name == "join")
			{
				_dest = std::make_shared<query_join>();
				_dest->put_json(_src);
			}
			else {
				std::string msg = std::format("class_name {0} is not a valid query stage.  Use 'filter', 'project', 'join'.",  class_name );
				system_monitoring_interface::global_mon->log_warning(msg, __FILE__, __LINE__);
			}
		}
	}

	void get_json(json& _dest, std::shared_ptr<query_filter>& _src)
	{
		if (_src)
			_src->get_json(_dest);
	}

	void put_json(std::shared_ptr<query_predicate>& _dest, json _src)
	{
		std::vector<std::string> missing;

		if (_src.empty())
			return;

		if (not _src.has_members(missing, { "class_name" })) {
			system_monitoring_interface::global_mon->log_warning("query filter missing class_name.");
			if (_src.size()) {
				system_monitoring_interface::global_mon->log_information("the source json is:");
				system_monitoring_interface::global_mon->log_json<json>(_src, 2);
			}
			return;
		}

		if (_src.has_member("class_name"))
		{
			std::string class_name = _src["class_name"];

			if (class_name == "eq")
			{
				_dest = std::make_shared<filter_eq>();
				_dest->put_json(_src);
			}
			else if (class_name == "gt")
			{
				_dest = std::make_shared<filter_gt>();
				_dest->put_json(_src);
			}
			else if (class_name == "gte")
			{
				_dest = std::make_shared<filter_gte>();
				_dest->put_json(_src);
			}
			else if (class_name == "lt")
			{
				_dest = std::make_shared<filter_lt>();
				_dest->put_json(_src);
			}
			else if (class_name == "lte")
			{
				_dest = std::make_shared<filter_lte>();
				_dest->put_json(_src);
			}
			else if (class_name == "any")
			{
				_dest = std::make_shared<filter_any>();
				_dest->put_json(_src);
			}
			else if (class_name == "all")
			{
				_dest = std::make_shared<filter_all>();
				_dest->put_json(_src);
			}
			else if (class_name == "contains")
			{
				_dest = std::make_shared<filter_contains>();
				_dest->put_json(_src);
			}
		}
	}

}

#endif
