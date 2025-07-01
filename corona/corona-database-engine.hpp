/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
This is the core database engine for the corona database server.  

*/


#ifndef CORONA_DATABASE_ENGINE_HPP
#define CORONA_DATABASE_ENGINE_HPP

/*********************************************** 

Journal

3/14/2024 - Objectives
Create and list teams and users, 
Team permission checks for all objects,
Create 

3/15/2024 - log
Create and edit 

3/28/2024 - 
API close to complete, rounding out for testing.

4/6/2024 -
A good chunk of this is coded, but I am dealing 
with getting the headers straightened out from 
the great module disaster.

9/18/2024 - rewrote most of the above because the design was stupid.
9/25/2024 - factored classes and indexes out to help with concurrency.
10/28/2024 - now with xtable implementation for faster and more efficient storage,
			and querying that works increasingly well.

***********************************************/


namespace corona
{

	const std::string class_permission_put = "put";
	const std::string class_permission_get = "get";
	const std::string class_permission_delete = "delete";
	const std::string class_permission_alter = "alter";

	const std::string class_grant_any = "any"; // any object can be modified by someone on the team
	const std::string class_grant_own = "own"; // only objects you own can be modified if you are on the team

	enum class_grants {
		grant_none = 0,
		grant_any = 1,
		grant_own = 2
	};

	class child_object_class
	{
	public:
		std::string class_name;
		std::map<std::string, std::string> copy_values;
	};

	class child_object_definition 
	{
	public:
		bool					 is_undefined;
		bool					 is_array;
		std::vector<std::shared_ptr<child_object_class>> child_classes;

		child_object_definition()
		{
			is_undefined = true;
			is_array = false;
		}

		child_object_definition(const child_object_definition& _src) = default;
		child_object_definition(child_object_definition&& _src) = default;
		child_object_definition& operator =(const child_object_definition& _src) = default;
		child_object_definition& operator =(child_object_definition&& _src) = default;

		static child_object_definition parse_definition(const char* _src)
		{
			parser_base pb;
			child_object_definition cod;

			// parse symbol until end or [
			// if there is a [
			// if end, end 
			// 
			// parse classname until end or ] or ,
			// if comma, add cn, reset classname
			// if ] end declaration
			// if end, end 

			if ((not _src) or (*_src == 0))
			{
				return cod;
			}

			_src = pb.eat_white(_src);
			if (*_src == '[') 
			{
				_src++;
				cod.is_array = true;
			}

			enum parse_states {
				parsing_class_name,
				parsing_dst_field,
				parsing_src_field,
				parsing_complete,
				parsing_error
			} status;

			status = parsing_class_name;

			std::string class_name;
			std::string dest_field;
			std::string src_field;
			std::shared_ptr<child_object_class> new_class = std::make_shared<child_object_class>();

			while (status != parsing_complete and status != parsing_error)
			{
				_src = pb.eat_white(_src);

				if (*_src == 0)
				{
					status = parsing_complete;
				}
				else if (status == parsing_class_name)
				{
					if (pb.parse_symbol(class_name, _src, &_src)) {
						_src = pb.eat_white(_src);
						new_class->class_name = class_name;
						if (*_src == ':')
						{
							_src++;
							status = parsing_dst_field;
						}
						else if (*_src == ';')
						{
							_src++;
							cod.child_classes.push_back(new_class);
							new_class = std::make_shared<child_object_class>();
							status = parsing_class_name;
						}
						else if (*_src == 0 || *_src == ']')
						{
							cod.child_classes.push_back(new_class);
							status = parsing_complete;
						}
						else
						{
							pb.error("declaration", "syntax error after class name");
							status = parsing_error;
						}
					}
					else
					{
						pb.error("declaration", "invalid class name");
						status = parsing_error;

					}
				}
				else if (status == parsing_dst_field) 
				{
					if (pb.parse_symbol(dest_field, _src, &_src)) {
						_src = pb.eat_white(_src);
						if (*_src == '=')
						{
							_src++;
							status = parsing_src_field;
						}
						else if (*_src == ',')
						{
							_src++;
							status = parsing_dst_field;
							new_class->copy_values.insert_or_assign(dest_field, "object_id");
						}
						else if (*_src == ';')
						{
							_src++;
							status = parsing_class_name;
							new_class->copy_values.insert_or_assign(dest_field, "object_id");
							cod.child_classes.push_back(new_class);
							new_class = std::make_shared<child_object_class>();
						}
						else if (*_src == 0 || *_src == ']')
						{
							new_class->copy_values.insert_or_assign(dest_field, "object_id");
							cod.child_classes.push_back(new_class);
							status = parsing_complete;
						}
						else 
						{
							pb.error("declaration", "syntax error after dst field");
							status = parsing_error;
						}
					}
					else
					{
						pb.error("declaration", "invalid dest symbol");
						status = parsing_error;
					}
				}
				else if (status == parsing_src_field)
				{
					if (pb.parse_symbol(src_field, _src, &_src)) {
						new_class->copy_values.insert_or_assign(dest_field, src_field);
						_src = pb.eat_white(_src);
						if (*_src == ',')
						{
							_src++;
							status = parsing_dst_field;
						}
						else if (*_src == ';')
						{
							_src++;
							status = parsing_class_name;
							new_class->copy_values.insert_or_assign(dest_field, src_field);
							cod.child_classes.push_back(new_class);
						}
						else if (*_src == 0 || *_src == ']')
						{
							new_class->copy_values.insert_or_assign(dest_field, src_field);
							cod.child_classes.push_back(new_class);
							new_class = std::make_shared<child_object_class>();
							status = parsing_complete;
						}
						else
						{
							pb.error("declaration", "syntax error after src field");
							status = parsing_error;
						}
					}
					else 
					{
						pb.error("declaration", "invalid src symbol");
						status = parsing_error;
					}
				}
			}

			if (not cod.is_array and *_src == ']') 
			{
				pb.error("declaration", "invalid array terminator");
				status = parsing_error;
			}

			if (not pb.has_errors())
			{
				cod.is_undefined = false;
			}

			return cod;
		}
	};

	class reference_definition
	{
	public:
		bool is_undefined;
		std::string base_class;

		reference_definition()
		{
			is_undefined = true;
		}

		reference_definition(const reference_definition& _src) = default;
		reference_definition(reference_definition&& _src) = default;
		reference_definition& operator =(const reference_definition& _src) = default;
		reference_definition& operator =(reference_definition&& _src) = default;

		static reference_definition parse_definition(const char* _src)
		{
			parser_base pb;
			reference_definition cod;

			_src = pb.eat_white(_src);

			if (_src[0] == '-' and _src[1] == '>') {
				_src += 2;
				_src = pb.eat_white(_src);
				cod.base_class = _src;
				cod.is_undefined = not cod.base_class.empty();
			}
			else 
			{
				pb.error("reference", "reference field definitions must have form ->class_name");
			}

			if (not pb.has_errors())
			{
				cod.is_undefined = false;
			}

			return cod;
		}
	};

	void test_parse_child_field(std::shared_ptr<test_set> _tests)
	{
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("parse_child_field", "start", st, __FILE__, __LINE__);

		child_object_definition cd;
		bool result;

		const char *case1 = "class1";
		cd = child_object_definition::parse_definition(case1);
		result = not cd.is_undefined and cd.child_classes.size() == 1 and cd.child_classes[0]->class_name == case1 and cd.child_classes[0]->copy_values.size()==0;
		_tests->test({ std::format("child object {0}", case1), result , __FILE__, __LINE__ });

		const char* case2 = "class1:target";
		cd = child_object_definition::parse_definition(case2);
		result = not cd.is_undefined and cd.child_classes.size() == 1 and cd.child_classes[0]->class_name == "class1" and cd.child_classes[0]->copy_values.contains("target");
		_tests->test({ std::format("child object {0}", case1), result , __FILE__, __LINE__ });

		const char* case3 = "class1:target = src";
		cd = child_object_definition::parse_definition(case3);
		result = not cd.is_undefined and cd.child_classes.size() == 1 
			and cd.child_classes[0]->class_name == "class1"
			and cd.child_classes[0]->copy_values.contains("target")
			and cd.child_classes[0]->copy_values["target"] == "src";
		_tests->test({ std::format("child object {0}", case3), result , __FILE__, __LINE__ });

		const char *case4 = "class1:target1;class2:target2 = src2;";
		cd = child_object_definition::parse_definition(case4);
		result = not cd.is_undefined and cd.child_classes.size() == 2
			and cd.child_classes[0]->class_name == "class1"
			and cd.child_classes[1]->class_name == "class2"
			and cd.child_classes[0]->copy_values.contains("target1")
			and cd.child_classes[1]->copy_values.contains("target2")
			and cd.child_classes[0]->copy_values["target1"] == "object_id"
			and cd.child_classes[1]->copy_values["target2"] == "src2";
		_tests->test({ std::format("child object {0}", case4), result , __FILE__, __LINE__ });

		const char* acase1 = "[ class1 ]";
		cd = child_object_definition::parse_definition(case1);
		result = not cd.is_undefined and cd.child_classes.size() == 1 and cd.child_classes[0]->class_name == "class1" and cd.child_classes[0]->copy_values.size() == 0;
		_tests->test({ std::format("child object {0}", case1), result , __FILE__, __LINE__ });

		const char* acase2 = "[ class2:target ]";
		cd = child_object_definition::parse_definition(acase2);
		result = not cd.is_undefined 
			and cd.child_classes.size() == 1 
			and cd.child_classes[0]->class_name == "class2" 
			and cd.child_classes[0]->copy_values.contains("target");
		_tests->test({ std::format("child object {0}", acase2), result , __FILE__, __LINE__ });

		const char* acase3 = "[ class2:target = src ]";
		cd = child_object_definition::parse_definition(acase3);
		result = not cd.is_undefined and cd.child_classes.size() == 1
			and cd.child_classes[0]->class_name == "class2"
			and cd.child_classes[0]->copy_values.contains("target")
			and cd.child_classes[0]->copy_values["target"] == "src";
		_tests->test({ std::format("child object {0}", acase3), result , __FILE__, __LINE__ });

		const char* acase4 = "[ class1:target1;class2:target2 = src2 ]";
		cd = child_object_definition::parse_definition(acase4);
		result = not cd.is_undefined and cd.child_classes.size() == 2
			and cd.child_classes[0]->class_name == "class1"
			and cd.child_classes[1]->class_name == "class2"
			and cd.child_classes[0]->copy_values.contains("target1")
			and cd.child_classes[1]->copy_values.contains("target2")
			and cd.child_classes[0]->copy_values["target1"] == "object_id"
			and cd.child_classes[1]->copy_values["target2"] == "src2";
		_tests->test({ std::format("child object {0}", acase4), result , __FILE__, __LINE__ });

		const char* acase5 = "[ class2:target ]";
		cd = child_object_definition::parse_definition(acase5);
		result = not cd.is_undefined
			and cd.child_classes.size() == 1
			and cd.child_classes[0]->class_name == "class2"
			and cd.child_classes[0]->copy_values.contains("target");
		_tests->test({ std::format("child object {0}", acase5), result , __FILE__, __LINE__ });

		const char* acase6 = "[ class2:target = src ]";
		cd = child_object_definition::parse_definition(acase6);
		result = not cd.is_undefined 
			and cd.child_classes.size() == 1 
			and cd.child_classes[0]->class_name == "class2" 
			and cd.child_classes[0]->copy_values.contains("target") 
			and cd.child_classes[0]->copy_values["target"] == "src";
		_tests->test({ std::format("child object {0}", acase6), result , __FILE__, __LINE__ });

		system_monitoring_interface::global_mon->log_function_stop("parse_child_field", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
	}

	class class_permissions {
	public:
		std::string		user_name;
		class_grants	put_grant;
		class_grants	get_grant;
		class_grants	delete_grant;
		class_grants	alter_grant;
	};

	class corona_database_interface;
	
	class to_field
	{
	public:
		std::string			  source_name;
		std::string			  field_name;
	};

	class from_field
	{
	public:
		std::string			  field_name;
		std::vector<std::shared_ptr<to_field>> targets;
	};

	class from_source
	{
	public:
		int													index;
		std::string											source_name;
		std::string											parent_source_name;
		std::map<std::string, std::shared_ptr<from_field>>  fields;
		std::shared_ptr<json_object>						context;
		std::vector<std::shared_ptr<to_field>>				origins;
		std::map<int64_t, std::shared_ptr<json_object>>		data;
	};

	using from_sources = std::map<std::string, std::shared_ptr<from_source>>;

	class from_join
	{
		from_sources			sources;
		json					class_filter;
		json					dest_array;
		int						join_index;

		void get_filters_impl(json& dest_array, json& class_filter, from_sources::iterator _it)
		{
			json_parser jp;

			// this basically permutes the data we have in our join tree
			// at the end, we then take the chain of contexts, and from 
			// that result, we load up our new filter objects

			if (_it != std::end(sources)) {
				auto next_it = _it;
				next_it++;
				if (_it->second->data.size()) {
					for (auto item : _it->second->data)
					{
						_it->second->context = item.second;
						get_filters_impl(dest_array, class_filter, next_it);
					}
				}
				else {
					_it->second->context = std::make_shared<json_object>();
					get_filters_impl(dest_array, class_filter, next_it);
				}
			}
			else 
			{
				json_parser jp;
				json new_filter_object = class_filter.clone();
				std::string source_name = new_filter_object["name"];

				auto jobj = new_filter_object["filter"];
				if (jobj.empty()) {
					jobj = jp.create_object();
					new_filter_object.put_member("filter", jobj);
				}
				auto jmembers = jobj.get_members();

				from_source* parent = nullptr;

				for (auto member : jmembers)
				{
					if (member.second.is_string())
					{
						std::string value = (std::string)member.second;
						if (value.starts_with("$"))
						{
							std::string path(value.substr(1));
							std::vector<std::string> split_path = split(path, '.');
							if (split_path.size() == 2) {
								std::string& source_from_name = split_path[0];
								std::string& source_from_member = split_path[1];
								auto fsi = sources.find(source_from_name);
								if (fsi != sources.end())
								{
									jobj.object_impl()->members.insert_or_assign(source_from_member, fsi->second->context->members[source_from_member]);
									if (not parent or fsi->second->index > parent->index) {
										parent = fsi->second.get();
									}
								}
							}
						}
					}
				}

				if (parent) {
					new_filter_object.put_member("parent_source", parent->source_name);
					int64_t object_id = parent->context->members[object_id_field]->to_int64();
					new_filter_object.put_member_i64("parent_id", object_id);
				}

				dest_array.push_back(new_filter_object);
			}
		}

	public:

		from_join()
		{
			json_parser jp;
			join_index = 0;
		}
		
		void add_join(std::string _from_source, std::string _from_field, std::string _to_source, std::string _to_field)
		{
			std::shared_ptr<to_field> dest = std::make_shared<to_field>();
			dest->source_name = _to_source;
			dest->field_name = _to_field;

			std::shared_ptr<to_field> src = std::make_shared<to_field>();
			src->source_name = _from_source;
			src->field_name = _from_field;

			// from->to
			auto fsi = sources.find(_from_source);
			if (fsi != sources.end()) {
				auto& fs = fsi->second;
				auto ffi = fs->fields.find(_from_field);
				if (ffi != fs->fields.end()) 
				{
					ffi->second->targets.push_back(dest);
				}
				else 
				{
					std::shared_ptr<from_field> new_field = std::make_shared<from_field>();
					new_field->field_name = _from_field;
					new_field->targets.push_back(dest);
					fs->fields.insert_or_assign(_from_field, new_field);
				}
			}
			else 
			{
				std::shared_ptr<from_source> new_source = std::make_shared<from_source>();
				std::shared_ptr<from_field> new_field = std::make_shared<from_field>();
				new_field->field_name = _from_field;
				new_field->targets.push_back(dest);
				new_source->index = join_index++;
				new_source->source_name = _from_source;
				new_source->fields.insert_or_assign(_from_field, new_field);
				sources.insert_or_assign(_from_source, new_source);
			}

			// now update for origins
			auto fsio = sources.find(dest->source_name);
			if (fsio != sources.end()) {
				fsio->second->origins.push_back(src);
			}
			else 
			{
				std::shared_ptr<from_source> new_source = std::make_shared<from_source>();
				std::shared_ptr<from_field> new_field = std::make_shared<from_field>();
				new_field->field_name = _from_field;
				new_field->targets.push_back(dest);
				new_source->index = join_index++;
				new_source->source_name = dest->source_name;
				new_source->origins.push_back(src);
				new_source->fields.insert_or_assign(_from_field, new_field);
				sources.insert_or_assign(dest->source_name, new_source);
			}

		}

		json get_filters(json _class_filter)
		{
			json_parser jp;

			json class_filter = _class_filter;
			json dest_array = jp.create_array();

			from_sources::iterator _it = sources.begin();
			get_filters_impl(dest_array, class_filter, _it);
			return dest_array;
		}


		json get_data(std::string _source_name)
		{
			std::shared_ptr<json_array> jsarray = std::make_shared<json_array>();
			auto sit = sources.find(_source_name);
			if (sit != sources.end()) {
				for (auto dit : sit->second->data) {
					jsarray->elements.push_back(dit.second);
				}
			}
			return json(jsarray);
		}

		void add_data(json _class_filter, json& _src_array)
		{
			json_parser jp;
			
			std::shared_ptr<from_source> csp, cs;

			std::string source_name = _class_filter["name"];
			json parent_object;

			// first, own data gets added;

			auto csi = sources.find(source_name);
			if (csi == sources.end())
			{
				std::shared_ptr<from_source> new_source = std::make_shared<from_source>();
				std::shared_ptr<from_field> new_field = std::make_shared<from_field>();
				new_source->index = join_index++;
				new_source->source_name = source_name;
				sources.insert_or_assign(source_name, new_source);
			}
			csi = sources.find(source_name);
			if (csi != sources.end())
			{
				for (auto item : _src_array) {
					int64_t object_id = item.get_member(object_id_field);
					csi->second->data.insert_or_assign(object_id, item.object_impl());
				}

				std::string parent_source = _class_filter["parent_source"];
				int64_t parent_id = (int64_t)_class_filter["parent_id"];
				if (not parent_source.empty()) {
					auto cspi = sources.find(parent_source);
					if (cspi != sources.end())
					{
						auto fit = cspi->second->data.find(parent_id);
						if (fit != cspi->second->data.end())
						{
							std::shared_ptr<json_array> children;
							auto ci = fit->second->members.find(source_name);
							if (ci == fit->second->members.end()) {
								children = std::make_shared<json_array>();
								fit->second->members.insert_or_assign(source_name, children);
							}
							else 
							{
								children = std::dynamic_pointer_cast<json_array>(ci->second);
								if (not children) {
									children = std::make_shared<json_array>();
									fit->second->members.insert_or_assign(source_name, children);
								}
							}
							children->elements.insert(children->elements.end(), _src_array.array_impl()->elements.begin(), _src_array.array_impl()->elements.end());
						}
					}
				}

			}

		}
	};

	class child_bridge_interface
	{
	public:
		virtual std::string get_class_name() = 0;
		virtual void set_class_name(std::string _class_name) = 0;
		virtual void get_json(json& _dest) = 0;
		virtual void put_json(json& _src) = 0;
		virtual void copy(json& _dest, json& _src) = 0;
		virtual json get_field(json& src) = 0;
	};

	class child_bridges_interface
	{
	public:
		virtual void get_json(json& _dest) = 0;
		virtual void put_json(json& _src) = 0;

		virtual std::shared_ptr<child_bridge_interface> get_bridge(std::string _class_name) = 0;
		virtual std::vector<std::string> get_bridge_list() = 0;

		virtual void init_validation() = 0;
		virtual void init_validation(corona_database_interface* _db, class_permissions _permissions) = 0;
		virtual json get_children(corona_database_interface* _db, json _parent_object, class_permissions _permissions) = 0;
	};

	class field_options_interface
	{
	public:

		virtual void get_json(json& _dest) = 0;
		virtual void put_json(json& _src) = 0;
		virtual void init_validation() = 0;
		virtual void init_validation(corona_database_interface* _db, class_permissions _permissions) = 0;
		virtual json run_queries(corona_database_interface* _db, std::string& _token, std::string& _class_name, json & _object) = 0;
		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test) = 0;
		virtual std::shared_ptr<child_bridges_interface> get_bridges() = 0;
	};

	class field_interface {
	protected:
		std::string field_name;
		field_types field_type;
		std::shared_ptr<field_options_interface> options;

	public:

		virtual void init_validation() = 0;
		virtual void init_validation(corona_database_interface* _db, class_permissions _permissions) = 0;
		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test) = 0;
		virtual void get_json(json& _dest) = 0;
		virtual void put_json(std::vector<validation_error>& _errors, json& _src) = 0;

		virtual std::string get_field_name()
		{
			return field_name;
		}

		virtual field_types get_field_type() 
		{
			return field_type;
		}

		virtual std::shared_ptr<field_options_interface> get_options() {
			return options;
		}
		
		virtual std::shared_ptr<field_options_interface> set_options(std::shared_ptr<field_options_interface> _src) {
			options = _src;
			return options;
		}

		virtual json run_queries(corona_database_interface* _db, std::string& _token, std::string& _class_name, json& _object) = 0;
		virtual std::shared_ptr<child_bridges_interface> get_bridges() = 0;
	};


	class index_interface
	{
	protected:
		relative_ptr_type								table_location;
	public:
		virtual void get_json(json& _dest) = 0;
		virtual void put_json(std::vector<validation_error>& _errors, json& _src) = 0;

		virtual int64_t									get_index_id() = 0;
		virtual std::string								get_index_name() = 0;
		virtual std::vector<std::string>				&get_index_keys() = 0;
		virtual std::shared_ptr<xtable>					get_xtable(corona_database_interface* _db) = 0;
		virtual std::string								get_index_key_string() = 0;

	};

	class activity;

	class class_interface : public shared_lockable
	{
	protected:
		relative_ptr_type								table_location;
	public:

		virtual void get_json(json& _dest) = 0;
		virtual void put_json(std::vector<validation_error>& _errors, json& _src) = 0;

		virtual int64_t									get_class_id() const = 0;
		virtual std::string								get_class_name()  const = 0;
		virtual std::string								get_class_description()  const = 0;
		virtual std::string								get_base_class_name()  const = 0;
		virtual std::map<std::string, bool>  const&		get_descendants()  const = 0;
		virtual std::map<std::string, bool>  const&		get_ancestors()  const = 0;
		virtual std::map<std::string, bool>  &			update_descendants() = 0;
		virtual std::map<std::string, bool>  &			update_ancestors()  = 0;
		virtual std::vector<std::string>				get_parents() const = 0;
		virtual std::shared_ptr<xtable_interface>		get_table(corona_database_interface* _db) = 0;
		virtual std::shared_ptr<sql_table>				get_stable(corona_database_interface* _db) = 0;
		virtual std::shared_ptr<xtable>					get_xtable(corona_database_interface* _db) = 0;
		virtual std::shared_ptr<xtable>					alter_xtable(corona_database_interface* _db) = 0;
		virtual std::shared_ptr<xtable>					find_index(corona_database_interface* _db, json& _keys) const = 0;
		virtual	bool									update(activity* _context, json _changed_class) = 0;
		virtual std::vector<std::string>				get_table_fields()  const = 0;

		virtual	void									put_field(std::shared_ptr<field_interface>& _name) = 0;
		virtual std::shared_ptr<field_interface>		get_field(const std::string& _name)  const = 0;
		virtual std::vector<std::shared_ptr<field_interface>> get_fields()  const = 0;

		virtual std::shared_ptr<index_interface>		get_index(const std::string& _name)  const = 0;
		virtual std::vector<std::shared_ptr<index_interface>> get_indexes()  const = 0;

		virtual int64_t	get_location()  const = 0;
		virtual void	init_validation(corona_database_interface* _db, class_permissions _permissions) = 0;

		virtual json	get_object(corona_database_interface* _db, int64_t _object_id, class_permissions _grant, bool& _exists) = 0;
		virtual void	put_objects(corona_database_interface* _db, json& _children, json& _src_objects, class_permissions _grant) = 0;
		virtual json	get_objects(corona_database_interface* _db, json _key, bool _include_children, class_permissions _grant) = 0;
		virtual json	delete_objects(corona_database_interface* _db, json _key, bool _include_children, class_permissions _grant) = 0;

		virtual void	run_queries(corona_database_interface* _db, std::string& _token, std::string& _class_name, json& _target) = 0;
		virtual void	clear_queries(json& _target) = 0;

		virtual json	get_info(corona_database_interface* _db) = 0;
	};

	using read_class_sp = read_locked_sp<class_interface>;
	using write_class_sp = write_locked_sp<class_interface>;

	class activity
	{
	public:
		std::map<std::string, write_class_sp> classes;
		corona_database_interface* db;
		std::vector<validation_error> errors;

		class_interface* get_class(std::string _class_name);
		class_interface* create_class(std::string _class_name);
	};

	class corona_simulation_interface
	{
	public:

		virtual void on_frame(json& _commands) = 0;
	};

	class corona_database_interface : public file_block
	{
	protected:

		std::unique_ptr<xblock_cache> cache;

	public:
		corona_connections connections;

		corona_database_interface(std::shared_ptr<file> _fb) :
			file_block(_fb)
		{
			
		}

		xblock_cache* get_cache()
		{
			return cache.get();
		}

		virtual json create_database() = 0;
		virtual relative_ptr_type open_database(relative_ptr_type _header_location) = 0;

		virtual void apply_config(json _config) = 0;
		virtual json apply_schema(json _schema) = 0;

		virtual std::string get_random_code() = 0;

		virtual class_permissions get_class_permission(
			std::string _user_name,
			std::string _class_name) = 0;

		virtual class_permissions get_class_permission(
			const class_permissions& _src,
			std::string _class_name) = 0;

		virtual json create_user(json create_user_request, bool _system_user) = 0;
		virtual json login_user(json _login_request) = 0;
		virtual json user_confirm_user_code(json _login_request) = 0;
		virtual json send_user_confirm_code(json _send_user_request) = 0;
		virtual json set_user_password(json _set_password_request) = 0;
		virtual json get_classes(json get_classes_request) = 0;
		virtual json get_class(json get_class_request) = 0;
		virtual json put_class(json put_class_request) = 0;

		virtual json edit_object(json _edit_object_request) = 0;
		virtual json run_object(json _edit_object_request) = 0;
		virtual json create_object(json create_object_request) = 0;
		virtual json put_object(json put_object_request) = 0;
		virtual json get_object(json get_object_request) = 0;
		virtual json delete_object(json delete_object_request) = 0;

		virtual json copy_object(json copy_request) = 0;
		virtual json query(json query_request) = 0;

		// these two are for internal use only

		virtual int64_t get_next_object_id() = 0;

		virtual json select_object(json _key, bool _include_children, class_permissions _permissions) = 0;
		virtual json select_single_object(json _key, bool _include_children, class_permissions _permissions) = 0;
		virtual read_class_sp read_lock_class(const std::string& _class_name) = 0;
		virtual write_class_sp write_lock_class(const std::string& _class_name) = 0;
		virtual write_class_sp create_lock_class(const std::string& _class_name) = 0;
		virtual json save_class(write_class_sp& _class_to_save) = 0;
		virtual json save_class(class_interface* _class_to_save) = 0;
		virtual bool check_message(json& _message, std::vector<std::string> _authorizations, std::string& _user_name) = 0;
	};

	class corona_db_header_struct
	{
	public:
		int64_t								object_id;
		relative_ptr_type					classes_location;
		iarray<list_block_header, 100>		free_lists;

		corona_db_header_struct() 
		{
			object_id = -1;
			classes_location = -1;
		}
	};

	using class_method_key = std::tuple<std::string, std::string>;

	class corona_db_header : public poco_node<corona_db_header_struct>
	{
	public:

		int64_t write_free_list(file_block* _file, int _index)
		{
			auto offset = offsetof(corona_db_header_struct, free_lists) + _index * sizeof(list_block_header);
			auto size = sizeof(list_block_header);
			auto r = write_piece(_file, offset, size);
			return r;
		}
	};


	class field_options_base : public field_options_interface
	{
	public:
		bool required;

		field_options_base() = default;
		field_options_base(const field_options_base& _src) = default;
		field_options_base(field_options_base&& _src) = default;
		field_options_base& operator = (const field_options_base& _src) = default;
		field_options_base& operator = (field_options_base&& _src) = default;
		virtual ~field_options_base() = default;

		virtual void get_json(json& _dest)
		{
			_dest.put_member("required", required);
		}

		virtual void put_json(json& _src)
		{
			required = (bool)_src["required"];
		}

		virtual void init_validation() override
		{

		}

		virtual void init_validation(corona_database_interface* _db, class_permissions _permissions)  override
		{
			;
		}

		virtual json run_queries(corona_database_interface* _db, std::string& _token, std::string& _classname, json & _object)  override
		{
			json empty;
			return empty;
		}

		virtual bool accepts(corona_database_interface *_db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)  override
		{
			bool is_empty = _object_to_test.empty();
			if (required and is_empty) {
				validation_error ve;
				ve.class_name = _class_name;
				ve.field_name = _field_name;
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "required field";
				_validation_errors.push_back(ve);
				return false;
			};
			return false;

		}

		virtual std::shared_ptr<child_bridges_interface> get_bridges() override
		{
			return nullptr;
		}
	};

	class child_bridge_implementation : public child_bridge_interface
	{
		std::string							child_class_name;
		json								copy_values;

	public:

		virtual void set_class_name(std::string _class_name) override
		{
			child_class_name = _class_name;
		}

		virtual std::string get_class_name() override
		{
			return child_class_name;
		}

		virtual void get_json(json& _dest)  override
		{
			_dest.put_member("child_class_name", child_class_name);
			_dest.put_member("copy_values", copy_values);
		}

		virtual void put_json(json& _src)  override
		{
			child_class_name = _src["child_class_name"];
			copy_values = _src["copy_values"];
		}

		virtual void put_child_object(std::string& _child_class_name, json& _copy_values)
		{
			child_class_name = _child_class_name;
			copy_values = _copy_values;
		}

		virtual void copy(json& _dest, json& _src)  override
		{
			if (copy_values.object()) {
				auto members = copy_values.get_members();

				for (auto member : members)
				{
					std::string _dest_key = member.first;
					std::string _src_key = member.second;
					if (not (_src_key.empty() or _dest_key.empty()))
					{
						json value = _src[_src_key];
						_dest.put_member(_dest_key, value);
					}
				}
			}
		}

		virtual json get_field(json& _src) override
		{
			json_parser jp;
			json key;
			key = jp.create_object();
			auto members = copy_values.get_members();

			for (auto member : members)
			{
				std::string _dest_key = member.first;
				std::string _src_key = member.second;
				if (not (_src_key.empty() or _dest_key.empty()))
				{
					json value = _src[_src_key];
					key.put_member(_dest_key, value);
				}
			}

			return key;
		}

	};

	class child_bridges: public child_bridges_interface
	{
	public:
		std::map<std::string, std::shared_ptr<child_bridge_interface>> base_constructors;
		std::map<std::string, std::shared_ptr<child_bridge_interface>> all_constructors;

		virtual void get_json(json& _dest) override
		{
			json_parser jp;
			for (auto ctor : base_constructors) {
				json obj = jp.create_object();
				ctor.second->get_json(obj);
				_dest.put_member(ctor.first, obj);
			}
		}

		virtual void put_child_object(child_object_definition& _cod)
		{
			base_constructors.clear();
			all_constructors.clear();
			for (auto class_def : _cod.child_classes) {
				std::shared_ptr<child_bridge_implementation> new_bridge = std::make_shared<child_bridge_implementation>();
				json_parser jp;
				json copy_values = jp.create_object();
				for (auto pair : class_def->copy_values)
				{
					copy_values.put_member(pair.first, pair.second);
				}
				
				new_bridge->put_child_object(class_def->class_name, copy_values);
				new_bridge->set_class_name(class_def->class_name);
				base_constructors.insert_or_assign(class_def->class_name, new_bridge);
			}
		}

		virtual void put_json(json& _src) override
		{
			base_constructors.clear();
			all_constructors.clear();
			auto members = _src.get_members();
			for (auto member : members) {
				json obj = member.second;
				std::shared_ptr<child_bridge_implementation> new_bridge = std::make_shared<child_bridge_implementation>();
				std::string class_name = member.first;
				new_bridge->put_json(obj);
				new_bridge->set_class_name(class_name);
				base_constructors.insert_or_assign(class_name, new_bridge);
			}
		}

		virtual std::shared_ptr<child_bridge_interface> get_bridge(std::string _class_name) override
		{
			std::shared_ptr<child_bridge_interface> result;
			auto iter = all_constructors.find(_class_name);
			if (iter != std::end(all_constructors))
				result = iter->second;
			return result;
		}

  		virtual std::vector<std::string> get_bridge_list() override
		{
			std::vector<std::string> results;
			for (auto item : all_constructors) {
				results.push_back(item.first);
			}
			return results;
		}

		virtual void init_validation() override
		{

		}

		virtual void init_validation(corona_database_interface* _db, class_permissions _permissions)
		{
			all_constructors.clear();
			for (auto class_name_pair : base_constructors) {
				auto ci = _db->read_lock_class(class_name_pair.first);
				if (ci) {
					auto descendants = ci->get_descendants();
					for (auto descendant : descendants) {
						all_constructors.insert_or_assign(descendant.first, class_name_pair.second);
					}
				}
			}
		}

		virtual json get_children(corona_database_interface* _db, json _parent_object, class_permissions _permissions) override
		{
			json_parser jp;
			json result_array = jp.create_array();

			for (auto class_name_pair : all_constructors) 
			{
				json key = class_name_pair.second->get_field(_parent_object);
				read_class_sp classy = _db->read_lock_class(class_name_pair.first);
				if (classy) {
					json temp_array = classy->get_objects(_db, key, true, _permissions);
					if (temp_array.array()) {
						for (auto obj : temp_array) {
							result_array.push_back(obj);
						}
					}
				}
			}

			return result_array;
		}




	};

	class array_field_options : public field_options_base
	{
	public:

		std::shared_ptr<child_bridges> bridges;
		field_types fundamental_type;

		array_field_options() = default;
		array_field_options(const array_field_options& _src) = default;
		array_field_options(array_field_options&& _src) = default;
		array_field_options& operator = (const array_field_options& _src) = default;
		array_field_options& operator = (array_field_options&& _src) = default;
		virtual ~array_field_options() = default;

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);

			json_parser jp;

			if (bridges) {
				json jctors = jp.create_object();
				bridges->get_json(jctors);
				_dest.put_member("child_objects", jctors);
			}
		}

		virtual void put_json(json& _src)
		{
			field_options_base::put_json(_src);

			json jctors = _src["child_objects"];
			json fte = _src["fundamental_type"];

			bridges = std::make_shared<child_bridges>();
			if (jctors.object()) {
				bridges->put_json(jctors);
			}
		}

		virtual void put_definition(child_object_definition& _cod)
		{
			if (_cod.child_classes.size() == 1) 
			{
				std::string class_name = _cod.child_classes[0]->class_name;
				if (class_name == "string")
				{
					fundamental_type = field_types::ft_string;
				}
				else if (class_name == "number")
				{
					fundamental_type = field_types::ft_double;
				}
				else if (class_name == "datetime")
				{
					fundamental_type = field_types::ft_datetime;
				}
				else if (class_name == "reference")
				{
					fundamental_type = field_types::ft_reference;
				}
				else if ((class_name == "float" || class_name == "double"))
				{
					fundamental_type = field_types::ft_double;
				}
			}
			bridges = std::make_shared<child_bridges>();
			bridges->put_child_object(_cod);
		}

		virtual void init_validation() override
		{

		}

		virtual void init_validation(corona_database_interface* _db, class_permissions _permissions)
		{
			if (bridges)
				bridges->init_validation(_db, _permissions);
		}

		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;

				if (_object_to_test.array()) 
				{
					for (auto obj : _object_to_test)
					{
						std::string object_class_name;
						if (bridges)
						{
							if (obj.object()) {
								object_class_name = obj[class_name_field];
								auto ctor = bridges->get_bridge(object_class_name);
								if (not ctor) {
									validation_error ve;
									ve.class_name = _class_name;
									ve.field_name = _field_name;
									ve.filename = __FILE__;
									ve.line_number = __LINE__;
									ve.message = "This array does not accept child objects of " + object_class_name;
									_validation_errors.push_back(ve);
									return false;
								}
							}
							else {
								validation_error ve;
								ve.class_name = _class_name;
								ve.field_name = _field_name;
								ve.filename = __FILE__;
								ve.line_number = __LINE__;
								ve.message = "elements of this array must be objects.";
								_validation_errors.push_back(ve);
								return false;

							}
						}
						else if (fundamental_type == field_types::ft_string) 
						{
							bool acceptable = obj.is_string();
							if (not acceptable) {
								validation_error ve;
								ve.class_name = _class_name;
								ve.field_name = _field_name;
								ve.filename = __FILE__;
								ve.line_number = __LINE__;
								ve.message = "Element must be a string.";
								_validation_errors.push_back(ve);
							}
						}
						else if (fundamental_type == field_types::ft_int64)
						{
							bool acceptable = obj.is_int64();
							if (not acceptable) {
								validation_error ve;
								ve.class_name = _class_name;
								ve.field_name = _field_name;
								ve.filename = __FILE__;
								ve.line_number = __LINE__;
								ve.message = "Element must be a int64.";
								_validation_errors.push_back(ve);
							}
						}
						else if (fundamental_type == field_types::ft_double)
						{
							bool acceptable = obj.is_double();
							if (not acceptable) {
								validation_error ve;
								ve.class_name = _class_name;
								ve.field_name = _field_name;
								ve.filename = __FILE__;
								ve.line_number = __LINE__;
								ve.message = "Element must be a double.";
								_validation_errors.push_back(ve);
							}
						}
						else if (fundamental_type == field_types::ft_datetime)
						{
							bool acceptable = obj.is_int64();
							if (not acceptable) {
								validation_error ve;
								ve.class_name = _class_name;
								ve.field_name = _field_name;
								ve.filename = __FILE__;
								ve.line_number = __LINE__;
								ve.message = "Element must be a datetime.";
								_validation_errors.push_back(ve);
							}
						}
					}
				}
				else 
				{
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "Value must be an array for an array field.";
					_validation_errors.push_back(ve);
					return false;
				}
			}
			return true;
		}

		virtual std::shared_ptr<child_bridges_interface> get_bridges() override
		{
			return bridges;
		}
	};

	class object_field_options : public field_options_base
	{
	public:
		std::shared_ptr<child_bridges> bridges;

		object_field_options() = default;
		object_field_options(const object_field_options& _src) = default;
		object_field_options(object_field_options&& _src) = default;
		object_field_options& operator = (const object_field_options& _src) = default;
		object_field_options& operator = (object_field_options&& _src) = default;
		virtual ~object_field_options() = default;

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);

			json_parser jp;

			if (bridges) {
				json jctors = jp.create_object();
				bridges->get_json(jctors);
				_dest.put_member("child_objects", jctors);
			}
		}

		virtual void put_json(json& _src)
		{
			field_options_base::put_json(_src);

			json jctors = _src["child_objects"];
			bridges = std::make_shared<child_bridges>();
			if (jctors.object()) {
				bridges->put_json(jctors);
			}
		}

		virtual void put_definition(child_object_definition& _cod)
		{
			bridges = std::make_shared<child_bridges>();
			bridges->put_child_object(_cod);
		}

		virtual void init_validation(corona_database_interface* _db, class_permissions _permissions)
		{
			if (bridges)
				bridges->init_validation(_db, _permissions);
		}

		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;

				if (bridges)
				{
					std::string object_class_name;

					if (_object_to_test.object()) {
						object_class_name = _object_to_test[class_name_field];

						auto ctor = bridges->get_bridge(object_class_name);
						if (not ctor) {
							is_legit = false;
						}
					}
					else {
							is_legit = false;
					}
				}
				else
				{
					is_legit = true;
				}
				if (not is_legit) {
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "value must be an object of correct type.";
					_validation_errors.push_back(ve);
					return false;
				};

				return is_legit;
			}
		}

		virtual std::shared_ptr<child_bridges_interface> get_bridges() override
		{
			return bridges;
		}

	};

	class string_field_options : public field_options_base
	{
	public:
		int maximum_length;
		int minimum_length;
		std::string match_pattern;
		std::vector<std::string> allowed_values;

		string_field_options() = default;
		string_field_options(const string_field_options& _src) = default;
		string_field_options(string_field_options&& _src) = default;
		string_field_options& operator = (const string_field_options& _src) = default;
		string_field_options& operator = (string_field_options&& _src) = default;
		virtual ~string_field_options() = default;

		virtual void get_json(json& _dest)
		{
			json_parser jp;
			field_options_base::get_json(_dest);
			_dest.put_member_i64("max_length", maximum_length);
			_dest.put_member_i64("min_length", minimum_length);
			_dest.put_member("match_pattern", match_pattern);
			if (allowed_values.size()) {
				json jallowed_values = jp.create_array();
				for (auto s : allowed_values) {
					jallowed_values.push_back(s);
				}
				_dest.put_member("enum", jallowed_values);
			}
		}

		virtual void put_json(json& _src)
		{
			json_parser jp;
			field_options_base::put_json(_src);
			minimum_length = _src["min_length"];
			maximum_length = _src["max_length"];
			match_pattern = _src["match_pattern"];
			allowed_values.clear();
			json jallowed_values = _src["enum"];
			if (jallowed_values.array()) {
				for (auto s : jallowed_values) {
					allowed_values.push_back(s);
				}
			}
		}

		bool is_allowed_value(const std::string& _src)
		{
			bool is_legit = true;
			if (allowed_values.size() > 0) {
				auto iter = std::find(allowed_values.begin(), allowed_values.end(), _src);
				if (iter == std::end(allowed_values)) {
					is_legit = false;
				}
			}
			return is_legit;
		}

		virtual bool accepts(corona_database_interface *_db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;
				std::string chumpy = (std::string)_object_to_test;

				if (chumpy.size() >= minimum_length and chumpy.size() <= maximum_length)
				{
					if (not match_pattern.empty()) {
						std::regex rgx(match_pattern);
						if (std::regex_match(chumpy, rgx)) {
							is_legit = is_allowed_value(chumpy);
						}
						else {
							is_legit = false;
						}
					}
					else {
						is_legit = is_allowed_value(chumpy);
					}
				}
				else
				{
					is_legit = false;
				}
				if (not is_legit) {
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = std::format( "Value '{0}' must be between {1} and {2} characters long", chumpy, minimum_length, maximum_length);
					if (match_pattern.size() > 0) {
						ve.message += std::format(" and must match '{0}'", match_pattern);
					}
					if (allowed_values.size())
					{
						std::string setx = join(allowed_values, ", ");
						ve.message += std::format(" and must be within '{0}'", setx);
					}
					_validation_errors.push_back(ve);
					return false;
				};
			}
			return false;

		}

	};

	class int64_field_options : public field_options_base
	{
	public:
		int64_t min_value;
		int64_t max_value;

		int64_field_options() = default;
		int64_field_options(const int64_field_options& _src) = default;
		int64_field_options(int64_field_options&& _src) = default;
		int64_field_options& operator = (const int64_field_options& _src) = default;
		int64_field_options& operator = (int64_field_options&& _src) = default;
		virtual ~int64_field_options() = default;

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);
			_dest.put_member_i64("min_value", min_value);
			_dest.put_member_i64("max_value", max_value);
		}

		virtual void put_json(json& _src)
		{
			field_options_base::put_json(_src);
			min_value = (int64_t)_src["min_value"];
			max_value = (int64_t)_src["max_value"];
		}

		virtual bool accepts(corona_database_interface *_db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;
				int64_t chumpy = (int64_t)_object_to_test;

				if (chumpy >= min_value and chumpy <= max_value)
				{
					is_legit = true;
				}
				else
				{
					is_legit = false;
				}
				if (not is_legit) {
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = std::format("Value '{0}' must be between {1} and {2}", chumpy, min_value, max_value);
					_validation_errors.push_back(ve);
					return false;
				};
			}
			return false;
		}

	};

	class reference_field_options : public field_options_base
	{
	public:
		std::string reference_class;
		std::map<std::string, bool> reference_class_descendants;

		reference_field_options() = default;
		reference_field_options(const reference_field_options& _src) = default;
		reference_field_options(reference_field_options&& _src) = default;
		reference_field_options& operator = (const reference_field_options& _src) = default;
		reference_field_options& operator = (reference_field_options&& _src) = default;
		virtual ~reference_field_options() = default;

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);
			_dest.put_member("ref_class", reference_class);
		}

		virtual void put_definition(reference_definition& _rd)
		{
			reference_class = _rd.base_class;
			reference_class_descendants.clear();
		}

		virtual void put_json(json& _src)
		{
			field_options_base::put_json(_src);
			reference_class = _src["ref_class"];
			reference_class_descendants.clear();
		}

		virtual void init_validation(corona_database_interface* _db, class_permissions _permissions)
		{
			reference_class_descendants.clear();
			auto ci = _db->read_lock_class(reference_class);
			if (ci) {
				auto descendants = ci->get_descendants();
				for (auto descendant : descendants) {
					reference_class_descendants.insert_or_assign(descendant.first, true);
				}
			}
		}

		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;
				object_reference_type chumpy = (object_reference_type)_object_to_test;

				if (chumpy and reference_class_descendants.contains(chumpy.class_name))
				{
					is_legit = true;
				}
				else
				{
					is_legit = false;
				}
				if (not is_legit) {
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = std::format("Value '{0}' must be derived from {1}", (std::string)chumpy, reference_class);
					_validation_errors.push_back(ve);
					return false;
				};
			}
			return false;
		}

	};

	template <typename scalar_type> 
	class general_field_options : public field_options_base
	{
	public:
		scalar_type min_value;
		scalar_type max_value;

		general_field_options() = default;
		general_field_options(const general_field_options& _src) = default;
		general_field_options(general_field_options&& _src) = default;
		general_field_options& operator = (const general_field_options& _src) = default;
		general_field_options& operator = (general_field_options&& _src) = default;
		virtual ~general_field_options() = default;

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);
			_dest.put_member("min_value", min_value);
			_dest.put_member("max_value", max_value);
		}

		virtual void put_json(json& _src)
		{
			field_options_base::put_json(_src);
			min_value = (scalar_type)_src["min_value"];
			max_value = (scalar_type)_src["max_value"];
		}

		virtual bool accepts(corona_database_interface *_db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_legit = true;
				scalar_type chumpy = (scalar_type)_object_to_test;

				if (chumpy >= min_value and chumpy <= max_value)
				{
					is_legit = true;
				}
				else
				{
					is_legit = false;
				}
				if (not is_legit) {
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "value out of range";
					_validation_errors.push_back(ve);
					return false;
				};
			}
			return false;
		}

	};

	class choice_field_options : public field_options_base
	{
	public:
		json		filter;
		json		options;

		std::string	id_field_name;
		std::string	description_field_name;

		choice_field_options() = default;
		choice_field_options(const choice_field_options& _src) = default;
		choice_field_options(choice_field_options&& _src) = default;
		choice_field_options& operator = (const choice_field_options& _src) = default;
		choice_field_options& operator = (choice_field_options&& _src) = default;
		virtual ~choice_field_options() = default;

		virtual void init_validation() override
		{

		}

		virtual void init_validation(corona_database_interface* _db, class_permissions _permissions)
		{
			json_parser jp;
			json key = jp.create_object();
			options = _db->select_object(filter, false, _permissions);
		}

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);
			_dest.put_member("filter", filter);
			_dest.put_member("id_field_name", id_field_name);
			_dest.put_member("description_field_name", description_field_name);
			_dest.put_member("options", options);
		}

		virtual void put_json(json& _src)
		{
			json_parser jp;
			field_options_base::put_json(_src);
			filter = _src["filter"];
			id_field_name = _src["id_field_name"];
			description_field_name = _src["description_field_name"];
			options = jp.create_array();
		}

		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			if (field_options_base::accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test)) {
				bool is_empty = _object_to_test.empty();
				if (is_empty) {
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "value is empty";
					_validation_errors.push_back(ve);
					return false;
				}
				else if (options.empty()) 
				{
					validation_error ve;
					ve.class_name = _class_name;
					ve.field_name = _field_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "validation list not loaded";
					_validation_errors.push_back(ve);
					return false;
				}
				else 
				{
					std::string object_test_id;
					if (_object_to_test.object())
					{
						object_test_id = _object_to_test[id_field_name];
					}
					else if (_object_to_test.is_string())
					{
						object_test_id = _object_to_test;
					}
					else
						return false;

					for (auto option : options)
					{
						std::string id_field = option[id_field_name];
						if (id_field == object_test_id)
							return true;
					}
				}
				return true;
			}
			return false;
		}

	};

	class query_field_options : public field_options_base
	{
	public:
		json		query_body;

		query_field_options() = default;
		query_field_options(const query_field_options& _src) = default;
		query_field_options(query_field_options&& _src) = default;
		query_field_options& operator = (const query_field_options& _src) = default;
		query_field_options& operator = (query_field_options&& _src) = default;
		virtual ~query_field_options() = default;

		virtual void get_json(json& _dest)
		{
			field_options_base::get_json(_dest);
			_dest.put_member("query", query_body);
		}

		virtual void put_json(json& _src)
		{
			json_parser jp;
			field_options_base::put_json(_src);
			query_body = _src["query"];
		}

		virtual json run_queries(corona_database_interface* _db, std::string& _token, std::string& _classname, json& _object)
		{
			using namespace std::literals;
			json_parser jp;
			json this_query_body = query_body.clone();
			this_query_body.put_member("include_children", false);

			json froms = this_query_body["from"];
			if (froms.array()) {
				json new_from = jp.create_object();
				new_from.put_member(data_field, _object);
				new_from.put_member(class_name_field, _classname);
				new_from.put_member("name", "this"sv);
				auto arr = froms.array_impl();
				arr->elements.insert(arr->elements.begin(), new_from.object_impl());
			}
			this_query_body.put_member(token_field, _token);
			json query_results, query_data_results;
			query_results = _db->query(this_query_body);
			if (query_results[success_field]) {
				query_data_results = query_results[data_field];
			}
			else 
			{
				query_data_results = jp.create_array();
			}
			return query_data_results;
		}

		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test)
		{
			return false;
		}

	};

	class field_implementation : public field_interface {

	public:

		field_implementation() = default;
		field_implementation(const field_implementation& _src) = default;
		field_implementation(field_implementation&& _src) = default;
		field_implementation& operator = (const field_implementation& _src) = default;
		field_implementation& operator = (field_implementation&& _src) = default;
		virtual ~field_implementation() = default;

		virtual field_types get_field_type()
		{
			return field_type;
		}
		virtual field_implementation& set_field_type(field_types _field_type)
		{
			field_type = _field_type;
			return *this;
		}

		virtual std::string get_field_name()
		{
			return field_name;
		}
		field_implementation& set_field_name(const std::string &_field_name)
		{
			field_name = _field_name;
			return *this;
		}

		virtual std::shared_ptr<field_options_interface> get_options() {
			return options;
		}

		virtual std::shared_ptr<field_options_interface> set_options(std::shared_ptr<field_options_interface> _src) {
			options = _src;
			return options;
		}

		virtual void init_validation() override
		{
			class_permissions default_perms;
			if (options) options->init_validation();
		}

		virtual void init_validation(corona_database_interface* _db, class_permissions _permissions) override
		{
			if (options) options->init_validation(_db, _permissions);
		}

		virtual bool accepts(corona_database_interface* _db, std::vector<validation_error>& _validation_errors, std::string _class_name, std::string _field_name, json& _object_to_test) override
		{
			if (options) {
				return options->accepts(_db, _validation_errors, _class_name, _field_name, _object_to_test);
			}
			return true;
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			json joptions = jp.create_object();

			_dest.put_member("field_name", field_name);
			_dest.put_member("field_type", field_type_names[field_type]);

			if (options) {
				options->get_json(_dest);
			}
		}

		virtual void put_json(std::vector<validation_error>& _errors, json& _src)
		{

			std::string s = _src["field_type"];
			auto aft = allowed_field_types.find(s);
			if (aft != std::end(allowed_field_types)) {
				field_type = aft->second;
			}
			field_name = _src["field_name"];

			if (field_type == field_types::ft_object)
			{
				options = std::make_shared<object_field_options>();
				options->put_json(_src);
			}
			else if (field_type == field_types::ft_array)
			{
				options = std::make_shared<array_field_options>();
				options->put_json(_src);
			}
			else if (field_type == field_types::ft_double)
			{
				options = std::make_shared<general_field_options<double>>();
				options->put_json(_src);
			}
			else if (field_type == field_types::ft_int64)
			{
				options = std::make_shared<int64_field_options>();
				options->put_json(_src);
			}
			else if (field_type == field_types::ft_string)
			{
				options = std::make_shared<string_field_options>();
				options->put_json(_src);
			}
			else if (field_type == field_types::ft_bool)
			{
				options = std::make_shared<field_options_base>();
				options->put_json(_src);
			}
			else if (field_type == field_types::ft_datetime)
			{
				options = std::make_shared<general_field_options<date_time>>();
				options->put_json(_src);
			}
			else if (field_type == field_types::ft_query)
			{
				options = std::make_shared<query_field_options>();
				options->put_json(_src);
			}
			else if (field_type == field_types::ft_function)
			{
				;
			}
			else
			{
				reference_definition rd = reference_definition::parse_definition(s.c_str());

				if (not rd.is_undefined)
				{
					auto obj_options = std::make_shared<reference_field_options>();
					obj_options->put_definition(rd);
					options = obj_options;
				}
				else {

					child_object_definition cod = child_object_definition::parse_definition(s.c_str());

					if (not cod.is_undefined)
					{
						if (not cod.is_array)
						{
							auto obj_options = std::make_shared<object_field_options>();
							obj_options->put_definition(cod);
							options = obj_options;
						}
						else
						{
							auto arr_options = std::make_shared<array_field_options>();
							arr_options->put_definition(cod);
							options = arr_options;
						}
					}
				}
			}
		}

		virtual json run_queries(corona_database_interface* _db, std::string& _token, std::string& _classname, json& _object) override
		{
			json results;
			if (options) {
				results = options->run_queries(_db, _token, _classname, _object);
			}
			return results;
		}

		virtual std::shared_ptr<child_bridges_interface> get_bridges() override
		{
			if (options) {
				return options->get_bridges();
			}
			return nullptr;
		}
	};

	class index_implementation : public index_interface
	{
		int64_t index_id;
		std::string index_name;
		std::vector<std::string> index_keys;

	public:

		index_implementation()
		{
			index_id = 0;
			table_location = null_row;
		}

		index_implementation(std::shared_ptr<index_interface> _ii_index, corona_database_interface* _db)
		{
			index_id = _ii_index->get_index_id();
			index_name = _ii_index->get_index_name();
			index_keys = _ii_index->get_index_keys();

			table_location = null_row;;

			auto temp = _ii_index->get_xtable(_db);

			if (temp) {
				table_location = temp->get_location();
			}
		}

		index_implementation(std::string &_name, std::vector<std::string> &_keys, corona_database_interface* _db)
		{
			index_id = 0;
			index_name = _name;
			index_keys = _keys;

			table_location = null_row;
			if (_db) {

				auto temp = get_xtable(_db);

				if (temp) {
					table_location = temp->get_location();
				}
			}
		}

		index_implementation(const index_implementation& _src)
		{
			index_id = _src.index_id;
			index_name = _src.index_name;
			index_keys = _src.index_keys;
			table_location = _src.table_location;
		}

		index_implementation(index_implementation&& _src)
		{
			index_id = _src.index_id;
			index_name = _src.index_name;
			index_keys = _src.index_keys;
			table_location = _src.table_location;
		}

		index_implementation& operator = (const index_implementation& _src)
		{
			index_id = _src.index_id;
			index_name = _src.index_name;
			index_keys = _src.index_keys;
			table_location = _src.table_location;
			return *this;
		}

		index_implementation& operator = (index_implementation&& _src)
		{
			index_id = _src.index_id;
			index_name = _src.index_name;
			index_keys = _src.index_keys;
			table_location = _src.table_location;
			return *this;

		}

		virtual std::string get_index_key_string() override
		{
			return join(index_keys, ".");
		}
		
		virtual std::vector<std::string> &get_index_keys() override
		{
			return index_keys;
		}

		virtual void get_json(json& _dest) override
		{
			json_parser jp;
			_dest.put_member("index_name", index_name);

			json jindex_keys = jp.create_array();
			for (auto ikey : index_keys) 
			{
				jindex_keys.push_back(ikey);
			}
			_dest.put_member("index_keys", jindex_keys);
			_dest.put_member_i64("table_location", table_location);
		}

		virtual void put_json(std::vector<validation_error>& _errors, json& _src) override
		{			
			index_name = _src["index_name"];

			json jindex_keys = _src["index_keys"];
			if (jindex_keys.array())
			{
				index_keys.clear();
				bool add_object_id = true;
				for (auto key : jindex_keys) {
					std::string key_name = key;
					if (key_name == object_id_field)
						add_object_id = false;
					index_keys.push_back(key);
				}
				if (add_object_id) {
					index_keys.push_back(object_id_field);
				}
			}
			table_location = (int64_t)_src["table_location"];
			if (table_location <= 0) {
				table_location = null_row;
			}
		}

		int64_t get_index_id()  override
		{ 
			return index_id;
		}

		index_implementation& set_index_id(int64_t _index_id)
		{
			index_id = _index_id;
			return *this;
		}

		std::string get_index_name() override
		{
			return index_name;
		}

		index_implementation& set_index_name(const std::string& _name)
		{
			index_name = _name;
			return *this;
		}

		virtual std::shared_ptr<xtable> get_xtable(corona_database_interface* _db) override
		{
			std::shared_ptr<xtable> table;

			if (table_location != null_row)
			{
				table = std::make_shared<xtable>(_db->get_cache(), table_location);
				return table;
			}
			else {
				auto table_header = std::make_shared<xtable_header>();
				table_header->key_members = get_index_keys();
				table_header->object_members = { object_id_field };
				table = std::make_shared<xtable>(_db->get_cache(), table_header);
				table_location = table->get_location();
				return table;
			}
		}

	};


	class class_implementation : public class_interface
	{

	protected:
		int64_t		class_id;
		std::string class_name;
		std::string class_description;
		std::string base_class_name;
		std::vector<std::string> table_fields;
		std::vector<std::string> parents;
		std::map<std::string, std::shared_ptr<field_interface>> fields;
		std::map<std::string, std::shared_ptr<index_interface>> indexes;
		std::map<std::string, bool> ancestors;
		std::map<std::string, bool> descendants;
		std::shared_ptr<sql_integration> sql;

		void copy_from(const class_interface* _src)
		{
			class_id = _src->get_class_id();
			class_name = _src->get_class_name();
			class_description = _src->get_class_description();
			base_class_name = _src->get_base_class_name();
			table_location = _src->get_location();
			table_fields = _src->get_table_fields();
			parents = _src->get_parents();
			auto new_fields = _src->get_fields();
			for (auto fld : new_fields) {
				fields.insert_or_assign(fld->get_field_name(), fld);
			}
			auto new_indexes = _src->get_indexes();
			for (auto idx : new_indexes) {
				indexes.insert_or_assign(idx->get_index_name(), idx);
			}
			ancestors = _src->get_ancestors();
			descendants = _src->get_descendants();

		}


	public:

		class_implementation()
		{
			table_location = null_row;
			class_id = null_row;
		} 

		class_implementation(const class_interface* _src)
		{
			copy_from(_src);
		}
		class_implementation(const class_implementation& _src)
		{
			copy_from(&_src);
		};
		class_implementation(class_implementation&& _src)
		{
			copy_from(&_src);
		}
		class_implementation& operator = (const class_implementation& _src)
		{
			copy_from(&_src);
			return *this;
		}
		class_implementation& operator = (class_implementation&& _src)
		{
			copy_from(&_src);
			return *this;
		}

		virtual json get_info(corona_database_interface* _db) override
		{
			json_parser jp;
			json all_info = jp.create_object();

			auto tbl = get_table(_db);
			json info = tbl->get_info();
			all_info.put_member(class_name, info);

			for (auto idx : indexes) {
				auto idx_name = idx.second->get_index_name();
				auto idx_table = idx.second->get_xtable(_db);
				info = idx_table->get_info();
				all_info.put_member(idx_name, info);
			}

			return all_info;
		}

		virtual int64_t	get_class_id() const override
		{
			return class_id;
		}

		class_implementation& set_class_id(int64_t _class_id)
		{
			class_id = _class_id;
			return *this;
		}

		virtual std::string get_class_name() const override
		{
			return class_name;
		}

		class_implementation& set_class_name(const std::string& _class_name)
		{
			class_name = _class_name;
			return *this;
		}

		virtual std::string get_class_description() const override
		{
			return class_description;
		}

		class_implementation& set_class_description(const std::string& _class_description)
		{
			class_description = class_description;
			return *this;
		}

		virtual std::string get_base_class_name() const override
		{
			return base_class_name;
		}

		class_implementation& set_base_class_name(const std::string& _base_class_name)
		{
			base_class_name = _base_class_name;
			return *this;
		}

		virtual std::vector<std::string> get_parents() const override
		{
			return parents;
		}

		virtual std::vector<std::string> get_table_fields() const override
		{
			return table_fields;
		}

		virtual std::shared_ptr<xtable> get_xtable(corona_database_interface* _db) override
		{
			std::shared_ptr<xtable> table;
			if (table_location > null_row)
			{
				table = std::make_shared<xtable>(_db->get_cache(), table_location);
			}
			else
			{
				auto table_header = std::make_shared<xtable_header>();
				table_header->object_members = table_fields;
				table_header->key_members = { object_id_field };
				table = std::make_shared<xtable>(_db->get_cache(), table_header);
				table_location = table_header->get_location();
			}
			return table;
		}

		virtual std::shared_ptr<xtable> alter_xtable(corona_database_interface* _db) override
		{
			std::shared_ptr<xtable> current_table, new_table, table;
			if (table_location > null_row)
			{
				current_table = std::make_shared<xtable>(_db->get_cache(), table_location);
				auto table_header = std::make_shared<xtable_header>();
				table_header->object_members = table_fields;
				table_header->key_members = { object_id_field };
				new_table = std::make_shared<xtable>(_db->get_cache(), table_header);
				table_location = new_table->get_location();
				json_parser jp;
				json empty = jp.create_object();
				current_table->for_each(empty, [new_table](json& _src)->relative_ptr_type {
					new_table->put(_src);
					return 1;
					});
				table = new_table;
			}
			else
			{
				auto table_header = std::make_shared<xtable_header>();
				table_header->object_members = table_fields;
				table_header->key_members = { object_id_field };
				current_table = std::make_shared<xtable>(_db->get_cache(), table_header);
				table_location = table_header->get_location();
				table = current_table;
			}
			return current_table;
		}


		virtual std::shared_ptr<sql_table> get_stable(corona_database_interface* _db) override
		{
			if (not sql)
				return nullptr;

			std::string connection = _db->connections.get_connection(sql->connection_name);
			auto stable = std::make_shared<sql_table>(sql, connection);

			// we're going to make our xtable anyway so we can slap our object id 
			// on top of a sql server primary key
			// but we don't do this all the time, because we'd like to keep the data around.

			auto xtable = get_xtable(_db);
			return stable;
		}


		virtual std::shared_ptr<xtable_interface> get_table(corona_database_interface* _db) override
		{
			if (sql) 
			{
				return get_stable(_db);
			}
			else
				return get_xtable(_db);
		}

		virtual std::map<std::string, bool>  const& get_descendants() const override
		{
			return descendants;
		}

		virtual std::map<std::string, bool>  const& get_ancestors() const override
		{
			return ancestors;
		}

		virtual std::map<std::string, bool>  & update_descendants()  override
		{
			return descendants;
		}

		virtual std::map<std::string, bool>  & update_ancestors()  override
		{
			return ancestors;
		}

		bool empty()
		{
			return class_name.empty();
		}

		virtual void init_validation(corona_database_interface* _db, class_permissions _permissions) override
		{
			for (auto& fld : fields) {
				fld.second->init_validation(_db, _permissions);
			}
		}

		virtual void get_json(json& _dest)
		{
			json_parser jp;

			_dest.put_member(class_name_field, class_name);
			_dest.put_member("class_description", class_description);
			_dest.put_member("base_class_name", base_class_name);
			_dest.put_member_i64("table_location", table_location);

			json ja = jp.create_array();
			for (auto p : parents)
			{
				ja.push_back(p);
			}
			_dest.put_member("parents", ja);

			if (table_fields.size() > 0) {
				json jtable_fields = jp.create_array();
				for (auto tf : table_fields) {
					jtable_fields.push_back(tf);
				}
				_dest.put_member("table_fields", jtable_fields);
			}

			if (fields.size() > 0) {
				json jfield_object = jp.create_object();
				for (auto field : fields) {
					json jfield_definition = jp.create_object();
					field.second->get_json(jfield_definition);
					jfield_object.put_member(field.first, jfield_definition);
				
				}
				_dest.put_member("fields", jfield_object);
			}

			if (indexes.size() > 0) {
				json jindex_object = jp.create_object();
				for (auto index : indexes) {
					json jindex_definition = jp.create_object();
					index.second->get_json(jindex_definition);
					jindex_object.put_member(index.first, jindex_definition);
				}
				_dest.put_member("indexes", jindex_object);
			}

			if (ancestors.size() > 0) {
				json jancestor_array = jp.create_array();
				for (auto class_ancestor : ancestors) {
					jancestor_array.push_back(class_ancestor.first);
				}
				_dest.put_member("ancestors", jancestor_array);
			}

			if (descendants.size() > 0) {
				json jdescendants_array = jp.create_array();
				for (auto class_descendant : descendants) {
					jdescendants_array.push_back(class_descendant.first);
				}
				_dest.put_member("descendants", jdescendants_array);
			}

			if (sql) {
				json jsql = jp.create_object();
				sql->get_json(jsql);
				_dest.put_member("sql", jsql);
			}
		}

		virtual void put_json(std::vector<validation_error>& _errors, json& _src)
		{

			json jfields, jindexes, jancestors, jdescendants, jtable_fields;

			class_name = _src[class_name_field];
			class_description = _src["class_description"];
			base_class_name = _src["base_class_name"];
			table_location = (int64_t)_src["table_location"];

			parents.clear();
			json jparents = _src["parents"];

			if (jparents.is_string())
			{
				std::string jparento = (std::string)jparents;
				parents.push_back(jparento);
			}
			else if (jparents.array())
			{
				for (auto jparent : jparents) {
					parents.push_back((std::string)(jparent));
				}
			}

			if (table_location <= 0)
				table_location = null_row;

			jtable_fields = _src["table_fields"];

			table_fields.clear();
			if (jtable_fields.array()) {
				for (auto tf : jtable_fields) {
					table_fields.push_back((std::string)tf);
				}
			}

			ancestors.clear();
			jancestors = _src["ancestors"];
			if (jancestors.array())
			{
				for (auto jancestor : jancestors)
				{
					std::string ancestor = jancestor;
					ancestors.insert_or_assign(ancestor, true);
				}
			}
			else if (not jancestors.empty()) {
				validation_error ve;
				ve.class_name = class_name;
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "ancestors must be an array of strings";
				_errors.push_back(ve);
			}

			descendants.clear();
			jdescendants = _src["descendants"];
			if (jdescendants.array())
			{
				for (auto jdescendant : jdescendants)
				{
					std::string descendant = jdescendant;
					descendants.insert_or_assign(descendant, true);
				}
			}
			else if (not jdescendants.empty())
			{
				validation_error ve;
				ve.class_name = class_name;
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "descendants must be an array of strings";
				_errors.push_back(ve);
			}

			fields.clear();

			jfields = _src["fields"];

			if (jfields.empty())
			{
				json_parser jp;
				jfields = jp.create_object();
			}

			for (auto parent : parents)
			{
				if (jfields.has_member(parent)) {
					continue;
				}
				else {
					std::string parent_name = (std::string)parent;
					jfields.put_member(parent_name, std::string("int64"));
				}
			}

			if (jfields.object()) {
				auto jfield_members = jfields.get_members();
				for (auto jfield : jfield_members) {
					std::shared_ptr<field_implementation> field = std::make_shared<field_implementation>();
					field->set_field_type(field_types::ft_none);

					child_object_definition cod;
					reference_definition  rd;

					if (jfield.second.object()) 
					{
						field->put_json(_errors, jfield.second);
					}
					else if (jfield.second.is_string()) 
					{
						auto fi = allowed_field_types.find(jfield.second);
						if (fi != std::end(allowed_field_types)) {
							field->set_field_type( fi->second);
						}
						else {
							auto parse_temp = (std::string)jfield.second;
							cod = child_object_definition::parse_definition(parse_temp.c_str());
							if (cod.is_undefined)
							{
								rd = reference_definition::parse_definition(parse_temp.c_str());
								if (not rd.is_undefined) {
									field->set_field_type(field_types::ft_reference);
								}
							}
							else if (cod.is_array)
							{
								field->set_field_type(field_types::ft_array);
							}
							else
							{
								field->set_field_type(field_types::ft_object);
							}
						}
					}
					else if (jfield.second.array())
					{
						field->set_field_type(field_types::ft_array);
						child_object_definition cod;
						cod.is_array = true;
						for (auto jfield_grant : jfield.second) {
							if (jfield_grant.is_string())
							{
								std::shared_ptr<child_object_class> coc = std::make_shared<child_object_class>();
                                coc->class_name = jfield_grant;
								coc->copy_values.insert_or_assign(class_name, object_id_field);
								cod.child_classes.push_back( coc );
							}
                        }
						auto afo = std::make_shared<array_field_options>();
						afo->put_definition(cod);
						field->set_options(afo);
					}
					else
					{
						validation_error ve;
						ve.class_name = class_name;
						ve.field_name = field->get_field_name();
						ve.message = "Is not a valid field specification. Can either be a string or an object.";
						ve.filename = __FILE__;
						ve.line_number = __LINE__;
						_errors.push_back(ve);
					}

					if (field->get_field_name().empty())
					{
						field->set_field_name(jfield.first);
					}

					if (field->get_field_type() != field_types::ft_none) 
					{
						fields.insert_or_assign(field->get_field_name(), field);
					}
					else {
						validation_error ve;
						ve.class_name = class_name;
						ve.field_name = field->get_field_name();
						ve.message = "Invalid field type '" + (std::string)jfield.second;
						ve.filename = __FILE__;
						ve.line_number = __LINE__;
						_errors.push_back(ve);
					}
				}
			}
			else if (not jfields.empty())
			{
				validation_error ve;
				ve.class_name = class_name;
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "fields must be an object.  each field is a member, with a field as the '\"field_name\" : \"field_type\"' or '\"field_name\" : { options }'";
				_errors.push_back(ve);
			}

			indexes.clear();
			jindexes = _src["indexes"];

			if (jindexes.empty())
			{
				json_parser jp;
				jindexes = jp.create_object();
			}

			for (auto parent : parents)
			{
				std::string index_name = std::format("idx_{0}_{0}", class_name, parent);
				if (jindexes.has_member(index_name)) {
					continue;
				}
				else 
				{
					json_parser jp;
					json new_index = jp.create_object();
					new_index.put_member("index_name", index_name);
					json new_index_keys = jp.create_array();
					new_index_keys.push_back(parent);
					new_index.put_member("index_keys", new_index_keys);
					jindexes.put_member(index_name, new_index);
				}
			}

			if (jindexes.object()) {
				auto jindex_members = jindexes.get_members();
				for (auto jindex : jindex_members) {
					std::shared_ptr<index_implementation> index = std::make_shared<index_implementation>();
					index->put_json(_errors, jindex.second);
					index->set_index_name(jindex.first);

					if (index->get_index_name().empty())
					{
						validation_error ve;
						ve.class_name = class_name;
						ve.field_name = index->get_index_key_string();
						ve.message = "Missing index name.";
						ve.filename = __FILE__;
						ve.line_number = __LINE__;
						_errors.push_back(ve);
					}

					indexes.insert_or_assign(jindex.first, index);
				}
			}
			else if (not jindexes.empty())
			{
				validation_error ve;
				ve.class_name = class_name;
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "indexes must be an object.";
				_errors.push_back(ve);
			}

			json jsql = _src["sql"];
			if (jsql.object()) {
				sql = std::make_shared<sql_integration>();
				sql->put_json(_errors, jsql);
				for (auto& mp : sql->mappings) {
					auto fi = fields.find(mp.corona_field_name);
					if (fi != fields.end()) {
						mp.field_type = fi->second->get_field_type();
						if (mp.string_size <= 0) {
							mp.string_size = 100;
						}
					}
				}
				std::vector keys = sql->primary_key;
				keys.insert(keys.end(), object_id_field);
				std::string backing_index_name = sql->sql_table_name + "_idx";
				std::shared_ptr<index_implementation> idx = std::make_shared<index_implementation>(backing_index_name, keys, nullptr);
				indexes.insert_or_assign(backing_index_name, idx);
			}

		}

		virtual void clear_queries(json& _target) override
		{

			json_parser jp;
			for (auto fldpair : fields) {
				auto query_field = fldpair.second;
				if (query_field->get_field_type() == field_types::ft_query) {
					json empty_array = jp.create_array();
					_target.put_member(query_field->get_field_name(), empty_array);
				}
			}
		}

		virtual void run_queries(corona_database_interface* _db, std::string& _token, std::string& _classname, json& _target) override
		{
			for (auto fldpair : fields) {
				auto query_field = fldpair.second;
				if (query_field->get_field_type() == field_types::ft_query) {
					json objects = query_field->run_queries(_db, _token, _classname, _target);
					_target.put_member(query_field->get_field_name(), objects);
				}
			}
		}

		virtual std::shared_ptr<xtable> find_index(corona_database_interface* _db, json& _object)  const
		{
			std::shared_ptr<xtable> index_table;
			std::shared_ptr<index_interface> matched_index;
			int max_matched_key_count = 0;

			for (auto idx : indexes) 
			{
				auto& keys = idx.second->get_index_keys();

				int matched_key_count = 0;

				for (auto ikey : keys)
				{
					if (not _object.has_member(ikey))
					{
						break;
					}
					else
					{
						matched_key_count++;
					}
				}

				if (matched_key_count > max_matched_key_count)
				{
					matched_index = idx.second;
					max_matched_key_count = matched_key_count;
				}
			}

			if (matched_index) {
				index_table = matched_index->get_xtable(_db);
			}

			return index_table;
		}

		virtual int64_t get_location()  const override
		{
			return table_location;
		}

		virtual bool update(activity* _context, json _changed_class) override
		{
			class_implementation changed_class;

			changed_class.put_json(_context->errors, _changed_class);

			if (_context->errors.size())
			{
				system_monitoring_interface::global_mon->log_warning(std::format("Errors on updating class {0}", changed_class.class_name), __FILE__, __LINE__);
				for (auto error : _context->errors) {
					system_monitoring_interface::global_mon->log_information(std::format("{0} {1} {2}  @{3} {4}", error.class_name, error.field_name, error.message, error.filename, error.line_number), __FILE__, __LINE__);
				}

				return false;
			}

			if (changed_class.class_description.empty()) {
				validation_error ve;
				ve.class_name = changed_class.class_name;
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "class description not found";
				_context->errors.push_back(ve);
				return false;
			}

			if (not base_class_name.empty() and changed_class.base_class_name != base_class_name)
			{
				validation_error ve;
				ve.class_name = changed_class.class_name;
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "cannot change base class of a class.";
				_context->errors.push_back(ve);
				return false;
			}

			class_name = changed_class.class_name;
			base_class_name = changed_class.base_class_name;
			class_description = changed_class.class_description;
			sql = changed_class.sql;

			// check to see if we have changes requiring a table rebuild.

			bool alter_table = false;
			for (auto f : fields) {
				auto changed_field = changed_class.fields.find(f.first);
				if (changed_field != std::end(changed_class.fields))
				{
					if (changed_field->second->get_field_type() != f.second->get_field_type())
					{
						alter_table = true;
					}
				}
				else 
				{
					alter_table = true;
				}
			}

			std::vector<std::shared_ptr<field_interface>> new_fields;

			for (auto f : changed_class.fields) {
				auto changed_field = fields.find(f.first);
				if (changed_field == std::end(fields))
				{
					new_fields.push_back(f.second);
					alter_table = true;
				}
			}

			fields = changed_class.fields;

			ancestors.clear();

			if (not base_class_name.empty()) {

				auto base_class = _context->get_class(base_class_name);
				if (base_class) {

					ancestors = base_class->get_ancestors();
					ancestors.insert_or_assign(base_class_name, true);
					base_class->update_descendants().insert_or_assign(class_name, true);
					descendants.insert_or_assign(class_name, true);

					for (auto temp_field : base_class->get_fields())
					{
						fields.insert_or_assign(temp_field->get_field_name(), temp_field);
					}
					_context->db->save_class(base_class);
				}
				else {
					validation_error ve;
					ve.class_name = changed_class.class_name;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "base class nnot found";
					_context->errors.push_back(ve);
					return false;
				}
			}

			for (auto field : fields)
			{
				if (std::find_if(table_fields.begin(), table_fields.end(), [&field](const std::string& _src) {
						return _src == field.first;
					}) == std::end(table_fields)) {
					table_fields.push_back(field.first);
				}
			}

			auto view_descendants = descendants | std::views::filter([this](auto& pair) {
				return pair.first != class_name;
				});

			// check the indexes here, because here we have all of our fields from class anncestors.

			for (auto idx : indexes)
			{
				for (auto f : idx.second->get_index_keys()) {
					if (not fields.contains(f)) {
						validation_error ve;
						ve.class_name = class_name;
						ve.field_name = f;
						ve.message = "Invalid field for index";
						ve.filename = __FILE__;
						ve.line_number = __LINE__;
						_context->errors.push_back(ve);
					}
				}
			}

			if (_context->errors.size() > 0)
			{
				return false;
			}

			// loop through existing indexes,
			// dropping old ones that don't match
			auto existing_indexes = get_indexes();
			for (auto old_index : existing_indexes)
			{
				auto existing = changed_class.indexes.find(old_index->get_index_name());
				// if the index is going away, then get rid of it.
				if (existing == changed_class.indexes.end() or
					existing->second->get_index_key_string() != old_index->get_index_key_string()) {
					auto index_table = old_index->get_xtable(_context->db);
					if (index_table)
					{
						index_table->clear();
						index_table->save();
					}
				}
			}

			// reindex tables list
			std::map<std::string, std::shared_ptr<index_interface>> correct_indexes;

			// and once again through the indexes
			// we make a copy of the existing index, so as to keep its table,
			// while at the same time not trusting this index, which was passed in.
			for (auto& new_index : changed_class.indexes)
			{
				std::shared_ptr<index_interface> index_to_create;

				// we want to use the existing class to pick up if we have a table
				index_to_create  = get_index(new_index.first);
				if (index_to_create) {
					auto temp = std::make_shared<index_implementation>(index_to_create, _context->db);
					index_to_create = std::dynamic_pointer_cast<index_implementation, index_interface>(temp);
				}

				if (not index_to_create)
				{
					auto temp = std::make_shared<index_implementation>(new_index.second, _context->db);
					index_to_create = std::dynamic_pointer_cast<index_implementation, index_interface>(temp);
				}

				correct_indexes.insert_or_assign(new_index.first, index_to_create);
			}

			indexes = correct_indexes;

			if (class_id == 0) {
				class_id = _context->db->get_next_object_id();
			}

			_context->db->save_class(this);

			json empty_key;
			std::shared_ptr<xtable> class_data;

			if (alter_table) {
				class_data = alter_xtable(_context->db);
			}
			else 
			{
				class_data = get_xtable(_context->db);
			}

			// and populate the new indexes with any data that might exist
			for (auto idc : indexes) {
				auto my_index = idc.second;
				auto table = my_index->get_xtable(_context->db);
				auto& keys = my_index->get_index_keys();
				class_data->for_each(empty_key, [table](json& _item) -> relative_ptr_type {
					table->put(_item);
					return 1;
				});
			}

			// now update the descendants

			for (auto descendant : view_descendants)
			{
				auto desc_class = _context->get_class(descendant.first);
				if (desc_class) {
					json_parser jp;
					json descendant_json = jp.create_object();

					desc_class->update_ancestors().insert_or_assign(class_name, true);

					if (desc_class->get_base_class_name() == class_name) {

						desc_class->get_json(descendant_json);

						for (auto nf : new_fields) {
							json jfld = jp.create_object();
							nf->get_json(jfld);
							descendant_json["fields"].put_member(nf->get_field_name(), jfld);
						}

						desc_class->update(_context, descendant_json);
					}

					_context->db->save_class(desc_class);
				}
				else {
					validation_error ve;
					ve.class_name = descendant.first;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "descendant class not found";
					_context->errors.push_back(ve);
					return false;
				}
			}

			return true;
		}

		virtual void put_field(std::shared_ptr<field_interface>& _new_field) override
		{
			fields.insert_or_assign(_new_field->get_field_name(), _new_field);
		}

		virtual std::shared_ptr<field_interface>		get_field(const std::string& _name)  const override
		{
			auto found = fields.find(_name);
			if (found != std::end(fields)) {
				return found->second;
			}
			return nullptr;
		}

		virtual std::vector<std::shared_ptr<field_interface>> get_fields()  const override
		{
			std::vector<std::shared_ptr<field_interface>> fields_list;
			for (auto fld : fields) {
				fields_list.push_back(fld.second);
			}
			return fields_list;
		}

		virtual std::shared_ptr<index_interface> get_index(const std::string& _name)  const override
		{
			auto found = indexes.find(_name);
			if (found != std::end(indexes)) {
				return found->second;
			}
			return nullptr;
		}

		virtual std::vector<std::shared_ptr<index_interface>> get_indexes()  const override
		{
			std::vector<std::shared_ptr<index_interface>> indexes_list;
			for (auto fld : indexes) {
				indexes_list.push_back(fld.second);
			}
			return indexes_list;
		}

		virtual json get_object(corona_database_interface* _db, int64_t _object_id, class_permissions _grant, bool& _exists)
		{
			json_parser jp;
			json result;
			if (sql) {
				auto tb = get_xtable(_db);
				result = tb->get(_object_id);
				if (result.object()) {
					auto stb = get_stable(_db);
					json skey = result.extract(sql->primary_key);
					json sqlobja = stb->get(skey);
					json sqlobj = sqlobja.get_first_element();
					result.merge(sqlobj);
				}
			}
			else {
				auto tb = get_table(_db);
				json key = jp.create_object();
				key.put_member_i64(object_id_field, _object_id);
				result = tb->get(key);
			}

			if (result.object())
			{
				_exists = true;
				if (_grant.get_grant == class_grants::grant_own)
				{
					if ((std::string)result["created_by"] != _grant.user_name) {
						result = jp.create_object();
					}
				}
			}
			else 
			{
				_exists = false;
			}


			return result;
		}

		virtual void put_objects(corona_database_interface* _db, json& _child_objects, json& _src_list, class_permissions _grant) override
		{
			bool index_ready = true;

			json_parser jp;

			struct index_object_pair {
				std::shared_ptr<index_interface> index;
				json objects_to_add;
				json objects_to_delete;
			};

			std::vector<index_object_pair> index_updates;

			for (auto idx : indexes) 
			{
				index_object_pair iop;
				iop.index = idx.second;
				iop.objects_to_add = jp.create_array();
				iop.objects_to_delete = jp.create_array();
				index_updates.push_back(iop);
			}

			json put_list = jp.create_array();

			for (auto _src_obj : _src_list)
			{
				int64_t parent_object_id = (int64_t)_src_obj[object_id_field];

				bool exists = false;
				json old_object = get_object(_db, parent_object_id, _grant, exists);
				json write_object;

				if (old_object.object()) {
					write_object = old_object.clone();
					write_object.merge(_src_obj);
				}
				else if (not exists)
				{
					write_object = _src_obj;
				}
				else 
				{
					// in this case, exists == true, but old_object is empty. This means that the object
					// was there, but the user did not have permissions to change it.
					continue;
				}

				bool use_write_object = false;

				if (_grant.put_grant == class_grants::grant_any)
				{
					use_write_object = true;
				}
				else if (_grant.put_grant == class_grants::grant_own)
				{
					std::string owner = (std::string)write_object["created_by"];
					if (_grant.user_name == owner) {
						use_write_object = true;
					}
				}

				auto these_fields = get_fields();

				for (auto& fld : these_fields) {
					if (fld->get_field_type() == field_types::ft_array)
					{
						json array_field = write_object[fld->get_field_name()];
						if (array_field.array()) {
							auto bridges = fld->get_bridges();
							if (bridges) {
								for (auto obj : array_field) {
									std::string obj_class_name = obj[class_name_field];
									auto bridge = bridges->get_bridge(obj_class_name);
									if (bridge) {
										bridge->copy(obj, _src_obj);
									}
									_child_objects.push_back(obj);
								}
								json empty_array = jp.create_array();
								write_object.put_member(fld->get_field_name(), empty_array);
							}
						}
					}
					else if (fld->get_field_type() == field_types::ft_object)
					{
						json obj = write_object[fld->get_field_name()];
						if (obj.object()) {
							std::string obj_class_name = obj[class_name_field];
							auto bridges = fld->get_bridges();
							if (bridges) {
								auto bridge = bridges->get_bridge(obj_class_name);
								if (bridge) {
									bridge->copy(obj, write_object);
								}
								json empty;
								write_object.put_member(fld->get_field_name(), empty);
								_child_objects.push_back(obj);
							}
						}
					}
				}

				if (use_write_object) {
					put_list.array_impl()->elements.push_back(write_object.object_impl());
				}

				if (index_updates.size() > 0)
				{
					int64_t object_id = (int64_t)write_object[object_id_field];
					if (old_object.object())
					{
						for (auto& iop : index_updates)
						{
							auto& idx_keys = iop.index->get_index_keys();

							json obj_to_delete = old_object.extract(idx_keys);
							json obj_to_add = write_object.extract(idx_keys);
							if (obj_to_delete.compare(obj_to_add) != 0) {
								iop.objects_to_delete.push_back(obj_to_delete);
							}
							iop.objects_to_add.push_back(obj_to_add);
						}
					}
					else 
					{
						for (auto& iop : index_updates)
						{
							auto& idx_keys = iop.index->get_index_keys();
							// check to make sure that we have all the fields 
							// for the index
							json obj_to_add = write_object.extract(idx_keys);
							iop.objects_to_add.push_back(obj_to_add);
						}
					}
				}

			}
			
			auto tb = get_xtable(_db);

			tb->put_array(put_list);
			auto stb = get_stable(_db);
			if (stb) {
				stb->put_array(put_list);
			}
			tb->save();

			for (auto& iop : index_updates)
			{
				auto idx_table = iop.index->get_xtable(_db);
				idx_table->erase_array(iop.objects_to_delete);
				idx_table->put_array(iop.objects_to_add);
				idx_table->save();
			}
		}

		virtual json get_objects(corona_database_interface* _db, json _key, bool _include_children, class_permissions _grant)
		{
			// Now, if there is an index set specified, let's go see if we can find one and use it 
			// rather than scanning the table

			json_parser jp;
			json obj;

			if (_key.has_member(object_id_field)) {
				int64_t object_id = (int64_t)_key[object_id_field];
				bool exists;
				json temp = get_object(_db, object_id, _grant, exists);
				obj = jp.create_array();
				obj.push_back(temp);
			}
			else 
			{
				auto index_table = find_index(_db, _key);
				if (index_table)
				{
					json object_key = jp.create_object();

 					obj = index_table->select(_key, [_db, this, &object_key, &_grant](json& _item) -> json {
						int64_t object_id = (int64_t)_item[object_id_field];
						bool empty;
						json objfound = get_object(_db, object_id, _grant, empty);
						return objfound;
					});
				}
				else
				{
					auto class_data = get_table(_db);
					obj = class_data->select(_key, [&_key, &_grant](json& _j)
						{
							json result;
							if (_key.compare(_j) == 0 
								and (_grant.get_grant == class_grants::grant_any 
									or (_grant.get_grant == class_grants::grant_own 
										and (std::string)_j["created_by"] == _grant.user_name
										)
									)
								)
							result = _j;
							return result;

						});
				}
			}

			if (_include_children) {
				for (auto _src_obj : obj)
				{
					for (auto& fpair : fields) {
						auto& fld = fpair.second;
						if (fld->get_field_type() == field_types::ft_array)
						{
							auto bridges = fld->get_bridges();
							if (bridges) {
								json results = bridges->get_children(_db, _src_obj, _grant);
								_src_obj.put_member(fld->get_field_name(), results);
							}
						}
						else if (fld->get_field_type() == field_types::ft_object)
						{
							auto bridges = fld->get_bridges();
							if (bridges) {
								json results = bridges->get_children(_db, _src_obj, _grant);
								json first = results.get_first_element();
								_src_obj.put_member(fld->get_field_name(), first);
							}
						}
					}
				}
			}

			// since we got the objects from sql, we want to now get them from
			if (sql) 
			{
				auto backing_table = get_xtable(_db);
				json idx_search = jp.create_object();
				for (auto& s : sql->primary_key) {
					idx_search.put_member(s, s);
				}

				auto index_table = find_index(_db, idx_search);

				for (auto ob : obj)
				{
					json key = ob.extract(sql->primary_key);
					json ob_found;

					if (index_table)
					{
						ob_found = index_table->select(key, [this, &backing_table](json& _item) -> json {
							int64_t object_id = (int64_t)_item[object_id_field];
							auto objfound = backing_table->get(object_id);
							return objfound;
						});
					}
					else
					{
						ob_found = backing_table->select(key, [&key](json& _j)
							{
								json result;
								if (key.compare(_j) == 0)
									result = _j;
								return result;

							});
					}

					int64_t object_id = -1;
					json old_value;

					if (ob_found.array())
					{
						old_value = ob_found.get_first_element();
						if (old_value.object()) {
							object_id = (int64_t)old_value[object_id_field];
						}
						else 
						{
							object_id = _db->get_next_object_id();
						}
					}
					else
					{
						object_id = _db->get_next_object_id();
					}

					ob.put_member_i64(object_id_field, object_id);
					backing_table->put(ob);
					if (old_value.compare(ob) != 0) 
					{
						for (auto idx : indexes)
						{
							auto idx_table = idx.second->get_xtable(_db);
							if (not old_value.empty()) {
								idx_table->erase(old_value);
							}
							idx_table->put(ob);
						}
					}
				}
				backing_table->save();
			}
			return obj;
		}

		virtual json get_single_object(corona_database_interface* _db, json _key, bool _include_children, class_permissions _grant)
		{
			json temp = get_objects(_db, _key, _include_children, _grant);
			return temp.get_first_element();
		}

		virtual json delete_objects(corona_database_interface* _db, json _key, bool _include_children, class_permissions _permission)
		{
			json_parser jp;
			json child_objects = jp.create_array();
			json matching_objects = get_objects(_db, _key, _include_children, _permission);
			auto tb = get_table(_db);

			for (auto _src_obj : matching_objects) {

				for (auto& fpair : fields) {
					auto& fld = fpair.second;
					if (fld->get_field_type() == field_types::ft_array)
					{
						auto bridges = fld->get_bridges();
						json results = bridges->get_children(_db, _src_obj, _permission);
					}
					else if (fld->get_field_type() == field_types::ft_object)
					{
						auto bridges = fld->get_bridges();
						json results = bridges->get_children(_db, _src_obj, _permission);
					}
				}

				if ((_permission.delete_grant == class_grants::grant_any)
					or (_permission.delete_grant == class_grants::grant_own and (std::string)_src_obj == _permission.user_name))
				{
					tb->erase(_src_obj);
				}
			}
			return matching_objects;
		}
	};

	class corona_database : public corona_database_interface
	{
		corona_db_header header;

		shared_lockable allocation_lock,
						class_lock,
						database_lock;

		json schema;

		std::map<class_method_key, json_function_function> functions;

		crypto crypter;

		bool watch_polling;

		const std::string auth_general = "auth-general";
		const std::string auth_system = "auth-system";
		
		/*
		* authorizations in tokens, methods and progressions
		* 
		create_user->login_user
		login_user->send_confirmation_code, send_password_reset_code
		send_login_confirmation_code->receive_login_confirmation_code
		receive_login_confirmation_code->user - connected
		send_password_reset_code->receive_reset_password_code
		receive_reset_password_code->user - connected

		connected can:
		edit_object
		get_classes
		get_class
		put_class
		query_class
		create_object
		put_object
		get_object
		copy_object
		delete_object
		*/

		std::shared_ptr<json_table> classes;
		std::shared_ptr<json_table_header> classes_header;

		bool trace_check_class = false;

		allocation_index get_allocation_index(int64_t _size)
		{
			allocation_index ai = { };

			int small_size = 1024;
			int top = small_size / 16;

			if (_size < small_size) {
				int t = _size / 16;
				if (_size % 16) {
					t++;
				}
				ai.index = t;
				ai.size = t * 16;
			}
			else
			{
				int64_t sz = small_size;
				int idx = top;
				while (sz < _size) {
					sz *= 2;
					idx++;
				}
				ai.index = idx;
				ai.size = sz;
			}

			if (ai.index >= header.data.free_lists.capacity()) {
				std::string msg = std::format("{0} bytes is too big to allocate as a block.", _size);
				system_monitoring_interface::global_mon->log_warning(msg, __FILE__, __LINE__);
				ai.index = header.data.free_lists.capacity() - 1;
			}
			return ai;
		}

	public:

		virtual bool is_free_capable()
		{
			return true;
		}

		virtual relative_ptr_type allocate_space(int64_t _size, int64_t *_actual_size) override
		{
			write_scope_lock my_lock(allocation_lock);

			allocation_index ai = get_allocation_index(_size);

			*_actual_size = ai.size;

			auto& list_start = header.data.free_lists[ai.index];

			allocation_block_struct free_block = {};

			// get_allocation index always returns an index such that any block in that index
			// will have the right size.  so we know the first block is ok.
			if (list_start.first_block)
			{
				read(list_start.first_block, &free_block, sizeof(free_block));
				// this
				if (list_start.last_block == list_start.first_block)
				{
					list_start.last_block = 0;
					list_start.first_block = 0;
				}
				else
				{
					list_start.first_block = free_block.next_block;
					free_block.next_block = 0;
				}
				header.write_free_list(this, ai.index);
				return free_block.data_location;
			}

			int64_t total_size = sizeof(allocation_block_struct) + ai.size;
			int64_t base_space = add(total_size);
			if (base_space > 0) {
				free_block.data_location = base_space + sizeof(allocation_block_struct);
				free_block.next_block = 0;
				write(base_space, &free_block, sizeof(free_block));
				return free_block.data_location;
			}

			return 0;
		}


		virtual void free_space(int64_t _location) override
		{
			read_scope_lock my_lock(allocation_lock);
			relative_ptr_type block_start = _location - sizeof(allocation_block_struct);

			allocation_block_struct free_block = {};

			/* ah, the forgiveness of this can be evil.  Here we say, if we truly cannot verify this
			_location can be freed, then do not free it.  Better to keep some old block around than it is
			to stomp on a good one with a mistake.  */

			file_command_result fcr = read(block_start, &free_block, sizeof(free_block));

			// did we read the block
			if (fcr.success) {

				// is this actually an allocated space block, and, is it the block we are trying to free
				if (free_block.data_location == _location) {

					// ok, now let's have a go and see where this is in our allocation index.
					allocation_index ai = get_allocation_index(free_block.data_capacity);

					// this check says, don't try to free the block if there is a mismatch between its 
					// allocation size, and the size it says it has.
					if (free_block.data_capacity == ai.size)
					{
						// and now, we can look at the free list, and put ourselves at the 
						// end of it.  In this way, deletes and reallocates will tend to 
						// want to reuse the same space so handy for kill and fills.
						auto& list_start = header.data.free_lists[ai.index];

						// there is a last block, so we are at the end
						if (list_start.last_block) {
							relative_ptr_type old_last_block = list_start.last_block;
							allocation_block_struct last_free;
							auto fr = read(old_last_block, &last_free, sizeof(last_free));
							if (fr.success) {
								list_start.last_block = block_start;
								last_free.next_block = block_start;
								free_block.next_block = 0;
								write(old_last_block, &last_free, sizeof(last_free));
								write(block_start, &free_block, sizeof(free_block));

								// this basically says, just write out the list header for this index
								// not the whole header
								header.write_free_list(this, ai.index);
							}
						}
						else
						{
							// the list is empty and we add to it.
							free_block.next_block = 0;
							list_start.last_block = free_block.data_location - sizeof(allocation_block_struct);
							list_start.last_block = free_block.data_location - sizeof(allocation_block_struct);

							// this basically says, now save our block.
							write(block_start, &free_block, sizeof(free_block));

							// this basically says, just write out the list header for this index
							// not the whole header
							header.write_free_list(this, ai.index);
						}
					}
				}
			}
		}

		json create_database()
		{
			json result;
			timer method_timer;

			json created_classes;
			relative_ptr_type header_location;
			json_parser jp;

			date_time start_time = date_time::now();
			timer tx;

			using namespace std::literals;

			system_monitoring_interface::global_mon->log_job_start("create_database", "start", start_time, __FILE__, __LINE__);
			
			cache = std::make_unique<xblock_cache>(static_cast<file_block*>(this), maximum_record_cache_size_bytes);

			header.data.object_id = 1;
			header_location = header.append(this);

			header.data.object_id = 1;
			classes_header = std::make_shared<json_table_header>();
			classes_header->create(this);
			header.data.classes_location = classes_header->get_location();
			std::vector<std::string> class_keys = { class_name_field };
			classes = std::make_shared<json_table>(classes_header, 0, this, class_keys);

			created_classes = jp.create_object();

			header.write(this);
	
			json response =  create_class(R"(
{	
	"class_name" : "sys_object",
	"class_description" : "Base of all objects",
	"fields" : {			
			"object_id" : "int64",
			"class_name" : "string",
			"created" : "datetime",
			"created_by" : "string",
			"updated": "datetime",
 			"updated_by" :"string" 
	}
}
)");

			if (not response[success_field]) {
				system_monitoring_interface::global_mon->log_warning("create_class put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			json test =  classes->get(R"({"class_name":"sys_object"})");
			if (test.empty() or test.error()) {
				system_monitoring_interface::global_mon->log_warning("could not find class sys_object after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("sys_object", true);


			response = create_class(R"(
{	
	"class_name" : "sys_command",
	"class_description" : "Base of all commands",
	"base_class_name" : "sys_object",
	"fields" : {
	}
}
)");

			if (not response[success_field]) {
				system_monitoring_interface::global_mon->log_warning("create_class put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test = classes->get(R"({"class_name":"sys_command"})");
			if (test.empty() or test.error()) {
				system_monitoring_interface::global_mon->log_warning("could not find class sys_command after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("sys_command", true);



			response = create_class(R"(
{
	"class_name" : "sys_grant",
	"base_class_name" : "sys_object",
	"class_description" : "grants a team can have",
	"fields" : {
			"team_id"	  : "int64",
			"grant_class" : "string",
			"get" : {
				"field_type":"string",
				"field_name":"get",
				"enum" : [ "any", "none", "own" ]
			},
			"put" : {
				"field_type":"string",
				"field_name":"put",
				"enum" : [ "any", "none", "own" ]
			},
			"delete" : {
				"field_type":"string",
				"field_name":"delete",
				"enum" : [ "any", "none", "own" ]
			},
			"alter" : {
				"field_type":"string",
				"field_name":"alter",
				"enum" : [ "any", "none", "own" ]
			}
	},
	"indexes" : {
        "sys_grant_team": {
          "index_keys": [ "team_id" ]
        }
	}
}
)");

			if (not response[success_field]) {
				system_monitoring_interface::global_mon->log_warning("create_class sys_grant put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				std::cout << response.to_json() << std::endl;
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test = classes->get(R"({"class_name":"sys_grant"})");
			if (test.empty() or test.error()) {
				system_monitoring_interface::global_mon->log_warning("could not find class sys_grant after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("sys_grant", true);


			response = create_class(R"(
{
	"class_name" : "sys_team",
	"base_class_name" : "sys_object",
	"class_description" : "Teams a user can belong to",
	"fields" : {
			"team_name" : "string",
			"team_description" : "string",
			"team_domain" : "string",
			"permissions" : {
				"field_type" : "array",
				"field_name" : "permissions",
				"child_objects" : {
					"sys_grant" : {
						"child_class_name" : "sys_grant",
						"copy_values" : {
							"object_id" : "team_id"
						},
						"construct_values" : {
							"object_id" : "team_id"
						}
					}	
				}
			},
			"workflow_classes" : "array"
	},
	"indexes" : {
        "sys_team_name": {
          "index_keys": [ "team_name" ]
        },
        "sys_team_email": {
          "index_keys": [ "team_domain" ]
        }
	}
}
)");

			if (not response[success_field]) {
				system_monitoring_interface::global_mon->log_warning("create_class sys_team put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				std::cout << response.to_json() << std::endl;
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test = classes->get(R"({"class_name":"sys_team"})");
			if (test.empty() or test.error()) {
				system_monitoring_interface::global_mon->log_warning("could not find class sys_team after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("sys_team", true);





			response = create_class(R"(
{
	"class_name" : "sys_dataset",
	"base_class_name" : "sys_object",
	"class_description" : "Database script changes",
	"fields" : {
			"schema_id" : "int64",
			"dataset_name" : "string",
			"dataset_description" : "string",
			"dataset_version" : "string",
			"dataset_author" : "string",
			"dataset_source" : "string",
			"completed" : "datetime",
			"run_on_change": "bool",
			"objects": "array",
			"import" : "object"
	},
	"indexes" : {
        "sys_dataset_schema_id": {
          "index_keys": [ "schema_id" ]
        }
	}
}
)");

			if (not response[success_field]) {
				system_monitoring_interface::global_mon->log_warning("create_class sys_dataset put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				std::cout << response.to_json() << std::endl;
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test = classes->get(R"({"class_name":"sys_dataset"})");
			if (test.empty() or test.is_member("class_name", "SysParseError")) {
				system_monitoring_interface::global_mon->log_warning("could not find class sys_dataset after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("sys_dataset", true);

			response =  create_class(R"(
{
	"class_name" : "sys_schema",
	"base_class_name" : "sys_object",
	"class_description" : "Database script changes",
	"fields" : {		
			"schema_name" : "string",
			"schema_description" : "string",
			"schema_version" : "string",
			"schema_authors" : "array",
			"classes" : "array",
			"users" : "array",
			"datasets" : {
				"field_type" : "array",
				"field_name" : "datasets",
				"child_objects" : {
					"sys_dataset" : {
						"child_class_name" : "sys_dataset",
						"copy_values" : {
							"object_id" : "schema_id"
						},
						"construct_values" : {
							"object_id" : "schema_id"
						}
					}	
				}
			}
		}
	}
}
)");

			created_classes.put_member("sys_schema", true);

			if (not response[success_field]) {
				system_monitoring_interface::global_mon->log_warning("create_class put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"class_name":"sys_schema"})");
			if (test.empty() or test.error()) {
				system_monitoring_interface::global_mon->log_warning("could not find class sys_schema after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);

				return result;
			}


			response =  create_class(R"(
{	
	"base_class_name" : "sys_object",
	"class_name" : "sys_user",
	"class_description" : "A user",
	"fields" : {			
			"first_name" : {
				"field_type":"string",
				"field_name":"first_name",
				"required" : true,
				"max_length" : 50,
				"match_pattern": "[a-zA-Z0-9_\\-\\s]+"
			},
			"last_name" : {
				"field_type":"string",
				"field_name":"last_name",
				"required" : true,
				"max_length" : 50,
				"match_pattern": "[a-zA-Z0-9_\\-\\s]+"
			},
			"user_name" : {
				"field_type":"string",
				"field_name":"user_name",
				"required" : true,
				"max_length" : 40,
				"match_pattern": "[a-zA-Z0-9]+"
			},
			"email" : {
				"field_type":"string",
				"field_name":"email",
				"required" : true,
				"max_length" : 100,
				"match_pattern": "(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+"	
			},
			"mobile" : {
				"field_type":"string",
				"field_name":"mobile",
				"required" : true,
				"max_length" : 15,
				"match_pattern": "[a-zA-Z0-9_\\-\\s]+"	
			},
			"street1" : {
				"field_type":"string",
				"field_name":"street1",
				"required" : true,
				"max_length" : 100,
				"match_pattern": "[a-zA-Z0-9_\\-\\s]+"	
			},
			"street2" : {
				"field_type":"string",
				"field_name":"street2",
				"required" : true,
				"max_length" : 100,
				"match_pattern": "[a-zA-Z0-9_\\-\\s]+"	
			},
			"city" : {
				"field_type":"string",
				"field_name":"city",
				"required" : true,
				"max_length" : 100,
				"match_pattern": "[a-zA-Z0-9_\\-\\s]+"	
			},
			"state" : {
				"field_type":"string",
				"field_name":"state",
				"required" : true,
				"max_length" : 50,
				"match_pattern": "[a-zA-Z0-9_\\-\\s]+"	
			},
			"zip" : {
				"field_type":"string",
				"field_name":"zip",
				"required" : true,
				"max_length" : 15,
				"match_pattern": "^\d{5}(?:[-\s]\d{4})?$"	
			},
			"password" : "string",
			"team_name" : "string",
			"validation_code" : "string",
			"confirmed_code" : "number",
			"workflow_objects" : "object"
	}
}
)");

			if (not response[success_field]) {
				system_monitoring_interface::global_mon->log_warning("create_class sys_user put failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(response);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			test =  classes->get(R"({"class_name":"sys_user"})");
			if (test.empty() or test.error()) {
				system_monitoring_interface::global_mon->log_warning("could not find class sys_user after creation.", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			created_classes.put_member("sys_user", true);

			json gc = jp.create_object();
			json gcr = create_system_request( gc );

			json classes_array_response =  get_classes(gcr);
			json classes_array = classes_array_response[data_field];
			json classes_grouped = classes_array.group([](json& _item) -> std::string {
				return (std::string)_item[class_name_field];
				});

			auto members = created_classes.get_members();
			json missing_classes = jp.create_array();
			for (auto created_class : members) {
				const std::string& created_class_name = created_class.first;
				if (not classes_grouped.has_member(created_class_name)) {
					missing_classes.push_back(created_class_name);
				}
			}

			if (missing_classes.size() > 0) {
				system_monitoring_interface::global_mon->log_warning("system classes not saved", __FILE__, __LINE__);

				for (auto mc : missing_classes) {
					system_monitoring_interface::global_mon->log_information(mc[class_name_field], __FILE__, __LINE__);
				}

				system_monitoring_interface::global_mon->log_job_stop("create_database", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			json new_user_request;
			json new_user_data;

			new_user_data = jp.create_object();
			new_user_data.put_member(class_name_field, "sys_user"sv);
			new_user_data.put_member(user_name_field, default_user);
			new_user_data.put_member(user_email_field, default_email_address);
			new_user_data.put_member("password1", default_password);
			new_user_data.put_member("password2", default_password);

			new_user_request = create_system_request(new_user_data);
			json new_user_result =  create_user(new_user_request);
			bool success = (bool)new_user_result[success_field];
			std::vector<validation_error> errors;
			if (success) {
				json new_user = new_user_result[data_field];
				json user_return = create_response(new_user_request, true, "Ok", new_user, errors, method_timer.get_elapsed_seconds());
				response = create_response(new_user_request, true, "Database Created", user_return, errors, method_timer.get_elapsed_seconds());
			}
			else {
				system_monitoring_interface::global_mon->log_warning("system user create failed", __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json(new_user_result);
				json temp = jp.create_object();
				response = create_response(new_user_request, false, "Database user create failed.", temp, errors, method_timer.get_elapsed_seconds());
			}

			system_monitoring_interface::global_mon->log_job_stop("create_database", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

private:

		virtual void save()
		{
			cache->save();
			commit();
		}

		json create_class(std::string _text)
		{
			json_parser jp;

			json jclass_def = jp.parse_object(_text);

			json sys_request = create_system_request(jclass_def);

			json response = put_class(sys_request);

			return response;
		}

		json check_single_object(date_time &current_date, read_class_sp& class_data, json object_definition, const class_permissions& _permission)
		{
			json_parser jp;
			using namespace std::literals;
			std::vector<validation_error> validation_errors;

			json result = jp.create_object();

			if (not object_definition.object())
			{
				json warning = jp.create_object();
				validation_error ve;

				ve.class_name = class_data->get_class_name();
				ve.field_name = trim(object_definition.to_json_typed(), 50);
				ve.filename = __FILE__;
				ve.line_number = __LINE__;
				ve.message = "Not an object";
				validation_errors.push_back(ve);
			}
			else {

				if (not object_definition.has_member(class_name_field))
				{
					json warning = jp.create_object();
					validation_error ve;

					ve.class_name = trim(object_definition.to_json_typed(), 50);
					ve.field_name = class_name_field;
					ve.filename = __FILE__;
					ve.line_number = __LINE__;
					ve.message = "Missing class";
					validation_errors.push_back(ve);
				}

				db_object_id_type object_id = -1;

				if (object_definition.has_member(object_id_field))
				{
					object_id = object_definition[object_id_field];
					bool exists;
					auto existing_object = class_data->get_object(this, object_id, _permission, exists);

					if (existing_object.object()) {
						existing_object.merge(object_definition);
						object_definition = existing_object;
					}

					object_definition.put_member("updated", current_date);
					object_definition.put_member("updated_by", _permission.user_name);

				}
				else
				{
					object_id = get_next_object_id();
					object_definition.put_member_i64(object_id_field, object_id);
					object_definition.put_member("created", current_date);
					object_definition.put_member("created_by", _permission.user_name);
				}

				// if the user was a stooser and saved the query results with the object,
				// blank that out here because we will run that on load
				class_data->clear_queries(object_definition);

				// check the object against the class definition for correctness
				// first we see which fields are in the class not in the object

				for (auto fld : class_data->get_fields()) {
					if (object_definition.has_member(fld->get_field_name())) {
						auto obj_type = object_definition[fld->get_field_name()]->get_field_type();
						auto member_type = fld->get_field_type();
						if (member_type != obj_type) {
							object_definition.change_member_type(fld->get_field_name(), member_type);
						}
					}
				}

				// check to make sure that we have all the fields 
				// for the index

				for (auto &idx : class_data->get_indexes()) {
					std::vector<std::string> missing;
					std::vector<std::string> &idx_keys = idx->get_index_keys();
					if (not object_definition.has_members(missing, idx_keys))
					{
						for (auto& missed : missing) {
							validation_error ve;
							ve.field_name = missed;
							ve.class_name = class_data->get_class_name();
							ve.filename = __FILE__;
							ve.line_number = __LINE__;
							ve.message = std::format("Missing field required for index '{0}'", idx->get_index_name());
							validation_errors.push_back(ve);
						}
					}
				}

				// then we see which fields are in the object that are not 
				// in the class definition.
				auto object_members = object_definition.get_members();
				for (auto om : object_members) {
					auto fld = class_data->get_field(om.first);
					if (fld) {
						fld->accepts(this, validation_errors, class_data->get_class_name(), om.first, om.second);
					}
					else
					{
						json warning = jp.create_object();
						validation_error ve;
						ve.class_name = class_data->get_class_name();
						ve.field_name = om.first;
						ve.filename = __FILE__;
						ve.line_number = __LINE__;
						ve.message = "Field not found in class definition";
						validation_errors.push_back(ve);
					}
				}
			}

			if (validation_errors.size() > 0) {
				json warnings = jp.create_array();
				std::string msg = std::format("Object '{0}' has problems", class_data->get_class_name());
				for (auto& ve : validation_errors) {
					json jve = jp.create_object();
					ve.get_json(jve);
					warnings.push_back(jve);
				}
				result.put_member(message_field, msg);
				result.put_member(success_field, 0);
				result.put_member("errors", warnings);
				result.put_member(data_field, object_definition);
			}
			else {
				result.put_member(message_field, "Ok"sv);
				result.put_member(success_field, 1);
				result.put_member(data_field, object_definition);
			}

			return result;
		}

		json check_object(json object_load, std::string _user_name, std::vector<validation_error>& validation_errors)
		{
			timer method_timer;
			json_parser jp;
			date_time current_date = date_time::now();
			using namespace std::literals;
			json response; 

			response = jp.create_object();

			json object_definition,
				object_list,
				result_list;

			if (object_load.array()) 
			{
				object_list = object_load;
			}
			else if (object_load.object()) 
			{
				object_list = jp.create_array();
				object_list.push_back(object_load);
			}
			else
			{
				response.put_member(success_field, false);
				response.put_member(message_field, "not an object"sv);
				return response;
			}

			json classes_group = object_list.group([](json _item) -> std::string {
				return _item[class_name_field];
				});

			auto class_list = classes_group.get_members();

			bool all_objects_good = true;

			for (auto class_pair : class_list)
			{
				if (class_pair.first.empty()) {
					response.put_member(success_field, false);
					response.put_member(message_field, "empty class name"sv);
					return response;
				}
				write_class_sp cd = write_lock_class(class_pair.first);

				if (cd) {
					auto permission = get_class_permission(_user_name, class_pair.first);
					cd->init_validation(this, permission);
				}
				else 
				{
					response.put_member(success_field, false);
					response.put_member(message_field, class_pair.first + " invalid class name");
					return response;
				}
			}

			for (auto class_pair : class_list)
			{
				std::string class_name = class_pair.first;

				json class_object_list = classes_group[class_name];
				read_class_sp class_data = read_lock_class(class_name);

				if (not class_data) {
					continue;
				}

				auto permission = get_class_permission(_user_name, class_pair.first);

				if (permission.put_grant == class_grants::grant_none) {
					response.put_member(success_field, false);
					response.put_member(message_field, "denied"sv);
					return response;
				}

				result_list = jp.create_array();

				for (auto item_definition : class_object_list)
				{
					json result = check_single_object(current_date, class_data, item_definition, permission);
					result_list.push_back(result);

					if (not result[success_field]) 
					{
						all_objects_good = false;
					}
				}

				classes_group.put_member(class_name, result_list);
			}

			response.put_member(success_field, all_objects_good);
			response.put_member(message_field, "Objects processed"sv);
			response.put_member(data_field, classes_group);
			return response;
		}

		std::string get_pass_phrase()
		{
			return "This is a test pass phrase";
		}

		std::string get_iv()
		{
			return "This is a test iv";
		}

		json create_response(std::string _user_name, std::string _authorization, bool _success, std::string _message, json _data, std::vector<validation_error>& _errors, double _seconds)
		{
			json_parser jp;

			json payload = jp.create_object();
			json token = jp.create_object();
			token.put_member(user_name_field, _user_name);
			token.put_member(authorization_field, _authorization);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member(token_expires_field, expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member(signature_field, cipher_text);

			std::string token_string = token.to_json_typed();
			std::string base64_token_string = base64_encode(token_string );

			if (_success) {
				payload.put_member(token_field, base64_token_string);
			}
			payload.put_member(success_field, _success);
			payload.put_member(message_field, _message);
			payload.put_member(data_field, _data);
			payload.put_member(seconds_field, _seconds);

			if (_errors.size()) {
				json errors_array = jp.create_array();
				for (auto& ve : _errors) {
					json jve = jp.create_object();
					ve.get_json(jve);
					errors_array.push_back(jve);
				}
				payload.put_member("errors", errors_array);
			}


			return payload;
		}

		json create_user_response(json _request, bool _success, std::string _message, json _data, std::vector<validation_error>& _errors, double _seconds)
		{
			json_parser jp;
			json payload;
			json errors_array = jp.create_array();

			if (_errors.size()) {
				for (auto& ve : _errors) {
					json jve = jp.create_object();
					ve.get_json(jve);
					errors_array.push_back(jve);
				}
			}
			payload = create_user_response(_request, _success, _message, _data, errors_array, _seconds);

			return payload;
		}

		json create_user_response(json _request, bool _success, std::string _message, json _data, json _errors, double _seconds)
		{
			json_parser jp;
			json payload = jp.create_object();

			payload.put_member(success_field, _success);
			payload.put_member(message_field, _message);
			payload.put_member(data_field, _data);
			payload.put_member_double(seconds_field, _seconds);

			if (_errors.size()) {
				payload.put_member("errors", _errors);
			}

			return payload;
		}

		json create_response(json _request, bool _success, std::string _message, json _data, std::vector<validation_error>& _errors, double _seconds)
		{
			json_parser jp;
			json payload = jp.create_object();
			json token = jp.create_object();
			json src_token = _request[token_field];
			
			token.copy_member(user_name_field, src_token);
			token.copy_member(authorization_field, src_token);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member(token_expires_field, expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member(signature_field, cipher_text);

			std::string token_string = token.to_json_typed();
			std::string base64_token_string = base64_encode(token_string);

			if (_success) {
				payload.put_member(token_field, base64_token_string);
			}
			payload.put_member(success_field, _success);
			payload.put_member(message_field, _message);
			payload.put_member(data_field, _data);
			if (_errors.size()) {
				json errors_array = jp.create_array();
				for (auto& ve : _errors) {
					json jve = jp.create_object();
					ve.get_json(jve);
					errors_array.push_back(jve);
				}
				payload.put_member("errors", errors_array);
			}
			payload.put_member_double(seconds_field, _seconds);
			return payload;
		}

		json check_token(std::string _encoded_token, std::vector<std::string> _authorizations)
		{
			json_parser jp;
			json empty;
			std::string decoded_token = base64_decode(_encoded_token);
			json token;

			try 
			{
				token = jp.parse_object(decoded_token);
			}
			catch (std::exception)
			{
				return empty;
			}

			if (not token.object())
			{
				return empty;
			}

			if (not token.has_member(signature_field))
			{
				return empty;
			}

			std::string signature = token[signature_field];
			token.erase_member(signature_field);

			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());

			if (cipher_text != signature) {
				return empty;
			}

			token.put_member(signature_field, signature);

			date_time current = date_time::utc_now();
			date_time expiration = (date_time)token[token_expires_field];

			if (current > expiration)
			{
				return empty;
			}

			if (cipher_text != signature)
			{
				return empty;
			}

			if (not token.has_member(authorization_field))
			{
				return empty;
			}

			std::string authorization = token[authorization_field];
			std::string user = token[user_name_field];

			if (authorization == auth_system and user == default_user)
			{
				return token;
			}

			for (auto _authorization : _authorizations)
			{
				if (_authorization == authorization)
					return token;
			}

			return empty;
		}

		public:

		virtual bool check_message(json& _message, std::vector<std::string> _authorizations, std::string& _user_name) override
		{
			std::string token = _message[token_field];

			json result = check_token(token, _authorizations);
			bool is_ok = not result.empty();

			if (is_ok) {
				_user_name = result[user_name_field];
			}

			return is_ok;
		}

		protected:

		json select_object(json _key, bool _children, class_permissions _permission)
		{
			json_parser jp;
			json obj = jp.create_array();

			_key.set_natural_order();

			std::string class_name = _key[class_name_field];

			read_class_sp classd = read_lock_class(class_name);
			if (not classd)
				return obj;

			auto perm = get_class_permission(_permission, class_name);

			obj = classd->get_objects(this, _key, _children, perm);

			return obj;
		}

		json select_single_object(json _key, bool _children, class_permissions _permission)
		{
			json result = select_object(_key, _children, _permission);
			result = result.get_first_element();
			return result;
		}

		void put_user(json _user, class_permissions _permission)
		{
			json_parser jp;

			write_class_sp classd = write_lock_class("sys_user");
	
			json children = jp.create_array();
			json items = jp.create_array();
			items.push_back(_user);
			classd->put_objects(this, children, items, _permission);
		}

		json get_user(std::string _user_name, class_permissions _permission)
		{
			json_parser jp;

			json key = jp.create_object();
			key.put_member(user_name_field, _user_name);

			auto classd = read_get_class("sys_user");
			if (not classd)
				return jp.create_array();

			json users = classd->get_objects(this, key, true, _permission);

			return users.get_first_element();
		}

		json get_team_by_domain(std::string _domain, class_permissions _permission)
		{
			json_parser jp;

			json key = jp.create_object();
			key.put_member("team_domain", _domain);

			auto classd = read_get_class("sys_team");
			if (not classd)
				return jp.create_array();

			json users = classd->get_objects(this, key, true, _permission);

			return users.get_first_element();
		}

		json get_team(std::string _team_name, class_permissions _permission)
		{
			json_parser jp;

			json key = jp.create_object();
			key.put_member("team_name", _team_name);

			auto classd = read_get_class("sys_team");
			if (not classd)
				return jp.create_array();

			json users = classd->get_objects(this, key, true, _permission);

			return users.get_first_element();
		}

		json get_schema(std::string schema_name, std::string schema_version, class_permissions _permission)
		{
			json_parser jp;
			json key = jp.create_object();

			key.put_member("schema_name", schema_name);
			key.put_member("schema_version", schema_version);

			auto classd = read_get_class("sys_schema");
			if (not classd) {
				return jp.create_array();
			}
			json data = classd->get_objects(this, key, true, _permission);

			return data.get_first_element();
		}

		json get_dataset(std::string dataset_name, std::string dataset_version, class_permissions _permission)
		{
			json_parser jp;
			json key = jp.create_object();
			key.put_member("dataset_name", dataset_name);
			key.put_member("dataset_version", dataset_version);

			auto classd = read_get_class("sys_dataset");
			if (not classd) {
				return json();
			}
			json data = classd->get_objects(this, key, true, _permission);

			return data.get_first_element();
		}

		virtual class_permissions get_system_permission()
		{
			class_permissions grants;
			grants.user_name = default_user;
			grants.alter_grant = class_grants::grant_any;
			grants.put_grant = class_grants::grant_any;
			grants.get_grant = class_grants::grant_any;
			grants.delete_grant = class_grants::grant_any;
			return grants;
		}

		virtual class_permissions get_class_permission(
			const class_permissions &_perm,
			std::string _class_name) override
		{
			return get_class_permission(_perm.user_name, _class_name);
		}

		virtual class_permissions get_class_permission(
			std::string _user_name,
			std::string _class_name) override
		{

			class_permissions grants;

			grants.user_name = _user_name;
			grants.alter_grant = class_grants::grant_none;
			grants.put_grant = class_grants::grant_none;
			grants.get_grant = class_grants::grant_none;
			grants.delete_grant = class_grants::grant_none;

			// extract the user key from the token and get the user object
			if (_user_name == default_user) 
			{
				grants = get_system_permission();
				return grants;
			}
			auto sys_perm = get_system_permission();
			json user =  get_user(_user_name, sys_perm);

			if (not user.empty()) 
			{			
				std::string team_name = (std::string)user["team_name"];
				json jteam = get_team(team_name, sys_perm);
				if (jteam.object()) {
					json jpermissions = jteam["permissions"];
					if (jpermissions.array()) {
						for (auto jperm : jpermissions) {
							std::string jclass = jperm["grant_class"];
							auto permclass = read_lock_class(jclass);
							if (permclass->get_descendants().contains(_class_name)) {
								std::string permission = jperm[class_permission_get];
								if (permission == "any")
									grants.get_grant = class_grants::grant_any;
								else if (permission == "own")
									grants.get_grant = class_grants::grant_own;

								permission = jperm[class_permission_put];
								if (permission == "any")
									grants.put_grant = class_grants::grant_any;
								else if (permission == "own")
									grants.put_grant = class_grants::grant_own;

								permission = jperm[class_permission_delete];
								if (permission == "any")
									grants.delete_grant = class_grants::grant_any;
								else if (permission == "own")
									grants.delete_grant = class_grants::grant_own;

								permission = jperm[class_permission_alter];
								if (permission == "any")
									grants.alter_grant = class_grants::grant_any;
								else if (permission == "own")
									grants.alter_grant = class_grants::grant_own;
								break;
							}
						}
					}
				}
			}
			
			return grants;
		}

		virtual db_object_id_type get_next_object_id() override
		{
			InterlockedIncrement64(&header.data.object_id);
			return header.data.object_id;
		}

		std::map<std::string, std::shared_ptr<class_implementation>> class_cache;

		std::shared_ptr<class_implementation> get_cached_class(const std::string& _class_name)
		{
			std::shared_ptr<class_implementation> cd;
			read_scope_lock my_lock(class_lock);
			auto cache_hit = class_cache.find(_class_name);
			if (cache_hit != std::end(class_cache)) {
				cd = cache_hit->second;
			}
			return cd;
		}

		std::shared_ptr<class_implementation> cache_existing_class(const std::string& _class_name)
		{
			std::shared_ptr<class_implementation> cd;
			write_scope_lock my_lock(class_lock);
			json_parser jp;
			cd = std::make_shared<class_implementation>();
			json key = jp.create_object();
			key.put_member(class_name_field, _class_name);
			json class_def = classes->get(key);

			if (class_def.empty())
				return nullptr;

			std::vector<validation_error> errors;

			if (class_def.object()) {
				cd->put_json(errors, class_def);
				if (errors.size()) {
					system_monitoring_interface::global_mon->log_warning(std::format("Errors on deserializing class {0}", _class_name), __FILE__, __LINE__);
					for (auto error : errors) {
						system_monitoring_interface::global_mon->log_information(std::format("{0} {1} {2}  @{3} {4}", error.class_name, error.field_name, error.message, error.filename, error.line_number), __FILE__, __LINE__);
					}
				}
			}
			else
			{
				// this will be a new, empty class.
				cd->set_class_name(_class_name);
			}

			class_cache.insert_or_assign(_class_name, cd);
			return cd;
		}

		std::shared_ptr<class_implementation> cache_class(const std::string& _class_name)
		{
			std::shared_ptr<class_implementation> cd;
			write_scope_lock my_lock(class_lock);
			json_parser jp;
			cd = std::make_shared<class_implementation>();
			json key = jp.create_object();
			key.put_member(class_name_field, _class_name);
			json class_def = classes->get(key);

			std::vector<validation_error> errors;

			if (class_def.object()) {
				cd->put_json(errors, class_def);
				if (errors.size()) {
					system_monitoring_interface::global_mon->log_warning(std::format("Errors on deserializing class {0}", _class_name), __FILE__, __LINE__);
					for (auto error : errors) {
						system_monitoring_interface::global_mon->log_information(std::format("{0} {1} {2}  @{3} {4}", error.class_name, error.field_name, error.message, error.filename, error.line_number), __FILE__, __LINE__);
					}
				}
			}
			else 
			{
				// this will be a new, empty class.
				cd->set_class_name(_class_name);
			}

			class_cache.insert_or_assign(_class_name, cd);
			return cd;
		}

		int64_t maximum_record_cache_size_bytes = giga_to_bytes(1);

		bool send_user_confirmation(json user_info)
		{
			bool success = false;
			try {
				std::string new_code = get_random_code();

				user_info.put_member("validation_code", new_code);
				user_info.put_member("confirmed_code", 0);
				auto sys_perm = get_system_permission();
				put_user(user_info, sys_perm);

				sendgrid_client sc_client;
				sc_client.sender_email = sendgrid_sender_email;
				sc_client.sender_name = sendgrid_sender;
				sc_client.api_key = connections.get_connection("sendgrid");

				std::string email_template = R"(<html><body><h2>$EMAIL_TITLE$</h2><p>Username is $USERNAME$</p><p>Validation code <span style="background:grey;border:1px solid black;padding 8px;">$CODE$</p></body></html>)";

				std::string user_name = user_info[user_name_field];

				std::string email_body = replace(email_template, "$CODE$", new_code);
				email_body = replace(email_body, "$USERNAME$", user_name);
				email_body = replace(email_body, "$EMAIL_TITLE$", user_confirmation_title);
				sc_client.send_email(user_info, user_confirmation_title, email_body, "text/html");
				success = true;
			}
			catch (std::exception exc)
			{
				system_monitoring_interface::global_mon->log_warning(exc.what(), __FILE__, __LINE__);
			}
			return success;
		}

	public:

		std::string default_user;
		std::string default_password;
		std::string default_email_address;
		std::string default_guest_team;
		time_span token_life;

		std::string default_api_title;
		std::string default_api_description;
		std::string default_api_version;
		std::string default_api_author;

		std::string sendgrid_sender;
		std::string sendgrid_sender_email;
		std::string user_confirmation_title;

		// constructing and opening a database

		corona_database(std::shared_ptr<file> _database_file) :
			corona_database_interface(_database_file)
		{
			token_life = time_span(1, time_models::hours);	
		}

		virtual ~corona_database()
		{
			save();
			wait();
		}

		void apply_config(json _config)
		{
			date_time start;
			start = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_job_start("apply_config", "start", start, __FILE__, __LINE__);

			if (_config.has_member("SendGrid"))
			{
				json send_grid = _config["SendGrid"];
				std::string send_grid_api_key = send_grid["ApiKey"];
				sendgrid_sender = send_grid["SenderName"];
				sendgrid_sender_email = send_grid["SenderEmail"];
				user_confirmation_title = send_grid["UserConfirmationTitle"];
				this->connections.set_connection("sendgrid", (std::string)send_grid_api_key);
			}

			if (_config.has_member("Connections"))
			{
				json connections = _config["Connections"];
				auto members = connections.get_members();
				for (auto member : members) {
					this->connections.set_connection(member.first, (std::string)member.second);
				}
			}

			if (_config.has_member("Server"))
			{
				json server = _config["Server"];
				default_user = server[sys_user_name_field];
				default_password = server[sys_user_password_field];
				default_email_address = server[sys_user_email_field];
				default_guest_team = server[sys_default_team_field];
				default_api_title = server[sys_default_api_title_field];
				default_api_description = server[sys_default_api_description_field];
				default_api_version = server[sys_default_api_version_field];
				default_api_author = server[sys_default_api_author_field];

				if (server.has_member(sys_record_cache_field)) {
					maximum_record_cache_size_bytes = (int64_t)server[sys_record_cache_field];
					if (cache) {
						cache->save();
					}
					cache = std::make_unique<xblock_cache>(this, maximum_record_cache_size_bytes);
				}
			}

			system_monitoring_interface::global_mon->log_job_stop("apply_config", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

		}


		virtual std::string get_random_code()
		{
			std::string s_confirmation_code = "";
			int confirmation_code_digits = 6;
			char confirmation_code[32] = {};

			int rc = 0;
			int lc = 0;
			int i = 0;
			while (i < confirmation_code_digits and i < sizeof(confirmation_code))
			{
				do
				{
					rc = rand() % 26 + 'A';
					confirmation_code[i] = rc;
				} while (rc == lc);
				lc = rc;
				i++;
			}
			confirmation_code[i] = 0;
			s_confirmation_code = confirmation_code;
			return s_confirmation_code;
		}

		virtual std::shared_ptr<class_implementation> read_get_class(const std::string& _class_name) 
		{
			std::shared_ptr<class_implementation> cd;

			cd = get_cached_class(_class_name);

			if (not cd)
			{
				cd = cache_existing_class(_class_name);
			}

			return cd;
		}

		virtual read_class_sp read_lock_class(const std::string& _class_name) override
		{
			std::shared_ptr<class_implementation> cd;

			cd = get_cached_class(_class_name);

			if (not cd)
			{
				cd = cache_existing_class(_class_name);
			}

			std::shared_ptr<class_interface> cdi = std::dynamic_pointer_cast<class_interface>(cd);
			return read_class_sp(cdi);
		}

		virtual write_class_sp write_lock_class(const std::string& _class_name) override
		{
			std::shared_ptr<class_implementation> cd;

			cd = get_cached_class(_class_name);

			std::shared_ptr<class_interface> cdi = std::dynamic_pointer_cast<class_interface>(cd);

			return write_class_sp(cdi);
		}

		virtual write_class_sp create_lock_class(const std::string& _class_name) override
		{
			std::shared_ptr<class_implementation> cd;

			cd = get_cached_class(_class_name);

			if (not cd)
			{
				cd = cache_class(_class_name);
			}

			std::shared_ptr<class_interface> cdi = std::dynamic_pointer_cast<class_interface>(cd);

			return write_class_sp(cdi);
		}

		virtual json save_class(class_interface *_class_to_save)
		{
			json_parser jp;
			json class_def;

			class_def = jp.create_object();
			_class_to_save->get_json(class_def);
			classes->put(class_def);
			return class_def;
		}

		virtual json save_class(write_class_sp& _class_to_save) override
		{
			json_parser jp;
			json class_def;

			class_def = jp.create_object();
			_class_to_save->get_json(class_def);
			classes->put(class_def);
			return class_def;
		}

		void log_error_array(json put_result)
		{
			auto result_items = put_result[data_field];
			result_items.for_each_member([](const std::string& _member_name, json _member) {
				if (_member.array()) {
					_member.for_each_element([](json& _item) {
						std::string msg = std::format("{0}:", (std::string)_item[message_field]);
						system_monitoring_interface::global_mon->log_warning(msg);
						if (not _item[success_field]) {
							if (_item.has_member("errors"))
							{
								json errors = _item["errors"];
								errors.for_each_element([](json& _msg) {
									std::string msg = std::format("{0}.{1} {2}", (std::string)_msg[class_name_field], (std::string)_msg["field_name"], (std::string)_msg[message_field]);
									system_monitoring_interface::global_mon->log_information(msg);
									});
							}
						}
						});
				}
				});
		}

		virtual json apply_schema(json _schema)
		{
			date_time start_schema = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_job_start("apply_schema", "Applying schema file", start_schema, __FILE__, __LINE__);
			using namespace std::literals;
			bool new_database = true;

			if (not _schema.has_member("schema_name"))
			{
				system_monitoring_interface::global_mon->log_warning("Schema doesn't have a schema name");
			}

			if (not _schema.has_member("schema_version"))
			{
				system_monitoring_interface::global_mon->log_warning("Schema doesn't have a schema version");
			}

			if (not _schema.has_member("schema_authors"))
			{
				system_monitoring_interface::global_mon->log_warning("Schema doesn't have a schema author");
			}

			auto sys_perm = get_system_permission();

			json_parser jp;

			json schema_key = jp.create_object();
			schema_key.copy_member("schema_name", _schema);
			schema_key.copy_member("schema_version", _schema);
			schema_key.put_member(class_name_field, "sys_schema"sv);
			schema_key.set_compare_order({ "schema_name", "schema_version" });

			json schema_test =  select_object(schema_key, false, sys_perm);

			if (schema_test.object()) 
			{
				new_database = false;
			}

			if (_schema.has_member("classes"))
			{
				date_time start_section = date_time::now();
				timer txsect;
				system_monitoring_interface::global_mon->log_job_section_start("", "Classes", start_section, __FILE__, __LINE__);

				json class_array = _schema["classes"];
				if (class_array.array())
				{
					for (int i = 0; i < class_array.size(); i++)
					{
						date_time start_class = date_time::now();
						timer txc;

						json class_definition = class_array.get_element(i);

						try {

							json put_class_request = create_system_request(class_definition);
							json class_result =  put_class(put_class_request);

							if (class_result.error()) 
							{
								system_monitoring_interface::global_mon->log_warning(class_result, __FILE__, __LINE__);
							}
						}
						catch (std::exception exc)
						{
							system_monitoring_interface::global_mon->log_exception(exc);
						}
					}
				}
				system_monitoring_interface::global_mon->log_job_section_stop("", "Classes", txsect.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else
			{
				system_monitoring_interface::global_mon->log_warning("classes not found in schema");
			}

			if (_schema.has_member("users"))
			{
				date_time start_section = date_time::now();
				timer txsect;
				system_monitoring_interface::global_mon->log_job_section_start("", "Users", start_section, __FILE__, __LINE__);
				json user_array = _schema["users"];
				if (user_array.array())
				{
					for (int i = 0; i < user_array.size(); i++)
					{
						date_time start_user = date_time::now();
						timer txu;

						json user_definition = user_array.get_element(i);
						system_monitoring_interface::global_mon->log_function_start("put user", user_definition[user_name_field], start_user, __FILE__, __LINE__);
						json put_user_request = create_system_request(user_definition);
						create_user(put_user_request);
					    system_monitoring_interface::global_mon->log_function_stop("put user", user_definition[user_name_field], txu.get_elapsed_seconds(), __FILE__, __LINE__);
					}
				}
				system_monitoring_interface::global_mon->log_job_section_stop("", "Users", txsect.get_elapsed_seconds(), __FILE__, __LINE__);
			}
	
			if (_schema.has_member("datasets"))
			{
				date_time start_section = date_time::now();
				timer txsect;
				system_monitoring_interface::global_mon->log_job_section_start("", "Datasets", start_section, __FILE__, __LINE__);
				using namespace std::literals;
				json script_array = _schema["datasets"];
				if (script_array.array())
				{
					for (int i = 0; i < script_array.size(); i++)
					{
						date_time start_dataset = date_time::now();
						timer txs;

						json script_definition = script_array.get_element(i);

						script_definition.put_member(class_name_field, "sys_dataset"sv);
						script_definition.put_member_i64("schema_id", (int64_t)_schema[object_id_field]);
						std::string dataset_name = script_definition["dataset_name"];
						std::string dataset_version = script_definition["dataset_version"];

						system_monitoring_interface::global_mon->log_job_section_start("DataSet", dataset_name + " Start", start_dataset, __FILE__, __LINE__);

						bool script_run = (bool)script_definition["run_on_change"];
						json existing_script = get_dataset(dataset_name, dataset_version, sys_perm);
						bool run_script = false;
						if (existing_script.empty() or script_run)
							run_script = true;

						if (existing_script.empty())
						{
							// in corona, creating an object doesn't actually persist anything 
							// but a change in identifier.  It's a clean way of just getting the 
							// "new chumpy" item for ya.  Or you can just shove it in there.
							json put_script_request = create_system_request(script_definition);
							json created_object = put_script_request[data_field];
							json save_result = put_object(put_script_request);
							if (not save_result[success_field]) {
								system_monitoring_interface::global_mon->log_warning(save_result[message_field]);
								system_monitoring_interface::global_mon->log_json<json>(save_result);
								existing_script = save_result[data_field];
							}
							else
								system_monitoring_interface::global_mon->log_information(save_result[message_field]);
						}

						if (run_script and script_definition.has_member("import"))
						{
							json import_spec = script_definition["import"];
							std::vector<std::string> missing;

							if (not import_spec.has_members(missing, { "target_class", "type" })) {
								system_monitoring_interface::global_mon->log_warning("Import missing:");
								std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
									system_monitoring_interface::global_mon->log_warning(s);
									});
								system_monitoring_interface::global_mon->log_information("the source json is:");
								system_monitoring_interface::global_mon->log_json<json>(import_spec, 2);
								continue;
							}

							std::string target_class = import_spec["target_class"];
							std::string import_type = import_spec["type"];

							if (import_type == "csv") {

								if (not import_spec.has_members(missing, { "filename", "delimiter" })) {
									system_monitoring_interface::global_mon->log_warning("Import CSV missing:");
									std::for_each(missing.begin(), missing.end(), [](const std::string& s) {
										system_monitoring_interface::global_mon->log_warning(s);
										});
									system_monitoring_interface::global_mon->log_information("the source json is:");
									system_monitoring_interface::global_mon->log_json<json>(import_spec, 2);
									continue;
								}

								std::string filename = import_spec["filename"];
								std::string delimiter = import_spec["delimiter"];
								if (filename.empty() or delimiter.empty()) {
									system_monitoring_interface::global_mon->log_warning("filename and delimiter can't be blank.");
								}

								json column_map = import_spec["column_map"];

								FILE* fp = nullptr;
								int error_code = fopen_s(&fp, filename.c_str(), "rS");

								if (fp) {
									// Buffer to store each line of the file.
									char line[8182];
									json datomatic = jp.create_array();

									// create template object
									json codata = jp.create_object();
									codata.put_member(class_name_field, target_class);
									json cor = create_system_request(codata);
									json new_object_response =  create_object(cor);

									if (new_object_response[success_field]) {
										json new_object_template = new_object_response[data_field];

										// Read each line from the file and store it in the 'line' buffer.
										int64_t total_row_count = 0;
										while (fgets(line, sizeof(line), fp)) {
											// Print each line to the standard output.
											json new_object = new_object_template.clone();
											new_object.erase_member(object_id_field);
											jp.parse_delimited_string(new_object, column_map, line, delimiter[0]);
											datomatic.push_back(new_object);
											if (datomatic.size() > 1000) {
												timer tx;
												json cor = create_system_request(datomatic);
												json put_result =  put_object(cor);
												if (put_result[success_field]) {
													double e = tx.get_elapsed_seconds();
													total_row_count += datomatic.size();
													std::string msg = std::format("import {0} rows / sec, {1} rows total", datomatic.size() / e, total_row_count);
													system_monitoring_interface::global_mon->log_activity(msg, e, __FILE__, __LINE__);
													datomatic = jp.create_array();
												}
												else {
													log_error_array(put_result);
													break;
												}
											}
										}

										if (datomatic.size() > 0) {
											timer tx;
											json cor = create_system_request(datomatic);
											json put_result = put_object(cor);
											if (put_result[success_field]) {
												double e = tx.get_elapsed_seconds();
												total_row_count += datomatic.size();
												std::string msg = std::format("final import {0} rows / sec, {1} rows total", datomatic.size() / e, total_row_count);
												system_monitoring_interface::global_mon->log_activity(msg, e, __FILE__, __LINE__);
												datomatic = jp.create_array();
											}
											else {
												log_error_array(put_result);
												break;
											}
										}

									}

									// Close the file stream once all lines have been read.
									fclose(fp);
								}
								else {
									char error_buffer[256] = {};
									strerror_s(
										error_buffer,
										std::size(error_buffer),
										error_code
									);
									std::string msg = std::format("could not open file {0}:{1}", filename, error_buffer);
									system_monitoring_interface::global_mon->log_warning(msg, __FILE__, __LINE__);
									char directory_name[MAX_PATH] = {};
									char *result = _getcwd(directory_name, std::size(directory_name));
									if (result) {
										msg = std::format("cwd is {0}", result);
										system_monitoring_interface::global_mon->log_information(msg, __FILE__, __LINE__);
									}
								}
							}
						}

						if (run_script and script_definition.has_member("objects")) {
							json object_list = script_definition["objects"];
							if (object_list.array()) {
								for (int j = 0; j < object_list.size(); j++) {
									json object_definition = object_list.get_element(j);
									json put_object_request = create_system_request(object_definition);
									// in corona, creating an object doesn't actually persist anything 
									// but a change in identifier.  It's a clean way of just getting the 
									// "new chumpy" item for ya.  
									json create_result =  create_object(put_object_request);
									if (create_result[success_field]) {
										json created_object = create_result[data_field];
										json save_result =  put_object(put_object_request);
										if (not save_result[success_field]) {
											system_monitoring_interface::global_mon->log_warning(save_result[message_field]);
											system_monitoring_interface::global_mon->log_json<json>(save_result);
										}
										else {
											std::string new_class_name = object_definition[class_name_field];
											int64_t object_id = object_definition[object_id_field];
											std::string object_created = std::format("object {0} ({1})", new_class_name, object_id);
											system_monitoring_interface::global_mon->log_information(object_created);
										}
									}
									else 
									{
										system_monitoring_interface::global_mon->log_warning(create_result[message_field], __FILE__, __LINE__);
										system_monitoring_interface::global_mon->log_json<json>(create_result);
									}
								}
							}

							date_time completed_date = date_time::now();
							script_definition.put_member("completed", completed_date);
							json put_script_request = create_system_request(script_definition);
							json save_script_result =  put_object(put_script_request);
							if (not save_script_result[success_field]) {
								system_monitoring_interface::global_mon->log_warning(save_script_result[message_field]);
								system_monitoring_interface::global_mon->log_json<json>(save_script_result);
							}
							else
								system_monitoring_interface::global_mon->log_information(save_script_result[message_field]);
						}

						system_monitoring_interface::global_mon->log_job_section_stop("DataSet", dataset_name + " Finished", txs.get_elapsed_seconds(), __FILE__, __LINE__);
					}
					system_monitoring_interface::global_mon->log_job_section_stop("DataSets", "", txsect.get_elapsed_seconds(), __FILE__, __LINE__);
				}
			}

			_schema.put_member(class_name_field, "sys_schema"sv);

			json put_schema_request = create_system_request(_schema);
			// in corona, creating an object doesn't actually persist anything 
			// but a change in identifier.  It's a clean way of just getting the 
			// "new chumpy" item for ya.  
			json create_schema_result =  create_object(put_schema_request);
			if (create_schema_result[success_field]) {
				json created_object = put_schema_request[data_field];
				json save_schema_result =  put_object(put_schema_request);
				if (not save_schema_result[success_field]) {
					system_monitoring_interface::global_mon->log_warning(save_schema_result[message_field]);
					system_monitoring_interface::global_mon->log_json<json>(save_schema_result);
				}
				else
					system_monitoring_interface::global_mon->log_information(save_schema_result[message_field]);
			}
			else 
			{
				system_monitoring_interface::global_mon->log_warning(create_schema_result[message_field], __FILE__, __LINE__);
				system_monitoring_interface::global_mon->log_json<json>(create_schema_result);
			}

			system_monitoring_interface::global_mon->log_job_stop("apply_schema", "schema applied", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			json temp = R"({ "success" : true, "message" : "Everything Ok, situation normal."})"_jobject;

			return temp;
		}

		virtual relative_ptr_type open_database(relative_ptr_type _header_location)
		{
			write_scope_lock my_lock(database_lock);

			timer method_timer;
			date_time start_schema = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_job_start("open_database", "Open database", start_schema, __FILE__, __LINE__);

			cache = std::make_unique<xblock_cache>(static_cast<file_block*>(this), maximum_record_cache_size_bytes );
			relative_ptr_type header_location =  header.read(this, _header_location);

			classes_header = std::make_shared<json_table_header>();
			classes_header->open(this, header.data.classes_location);
			std::vector<std::string> class_keys = { class_name_field };
			classes = std::make_shared<json_table>(classes_header, 0, this, class_keys);

			system_monitoring_interface::global_mon->log_job_stop("open_database", "Open database", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return header_location;
		}

		class password_metrics
		{
		public:
			int punctuation_count = 0;
			int digit_count = 0;
			int alpha_count = 0;
			int char_count = 0;

			password_metrics(const std::string& _check)
			{
				for (auto c : _check)
				{
					if (std::ispunct(c) or c == ' ') {
						punctuation_count++;
					}
					else if (std::isdigit(c)) {
						digit_count++;
					}
					else if (std::isalpha(c))
					{
						alpha_count++;
					}

					char_count++;
				}

			}

			bool is_stupid()
			{
				return char_count < 10 or (punctuation_count == 0 or digit_count == 0 or alpha_count == 0);
			}
		};


		virtual json create_user(json create_user_request, bool _system_user = false)
		{
			timer method_timer;
			json_parser jp;
			json response;

			read_scope_lock my_lock(database_lock);

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("create_user", "start", start_time, __FILE__, __LINE__);

			json data = create_user_request[data_field];

			std::string user_name = data[user_name_field];
			std::string user_email = data[user_email_field];
			std::string user_password1 = data["password1"];
			std::string user_password2 = data["password2"];
			std::string user_class = "sys_user";		

			if (user_password1 != user_password2)
			{
				std::vector<validation_error> errors;
				validation_error err;
                err.class_name = "sys_user";
				err.field_name = "password2";
				err.filename = __FILE__;
                err.line_number = __LINE__;
                err.message = "Passwords don't match";
				errors.push_back(err);
				system_monitoring_interface::global_mon->log_function_stop("create_user", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				response = create_user_response(create_user_request, false, "Passwords don't match", data, errors, method_timer.get_elapsed_seconds());
				return response;
			}
			// password complexity check

			password_metrics pm1(user_password1);

			if (pm1.is_stupid())
			{
				std::vector<validation_error> errors;
				validation_error err;
				err.class_name = "sys_user";
				err.field_name = "password1";
				err.filename = __FILE__;
				err.line_number = __LINE__;
				err.message = "Password too simple";
				errors.push_back(err);
				err.class_name = "sys_user";
				err.field_name = "password2";
				err.filename = __FILE__;
				err.line_number = __LINE__;
				err.message = "Password too simple";
				errors.push_back(err);

				system_monitoring_interface::global_mon->log_function_stop("create_user", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				response = create_user_response(create_user_request, false, std::format("Password '{4}' does not meet complexity requirements.  Password must be at least 10 characters long, have letters, numbers, and punctuation marks.  You have length = {0}, letters = {1}, numbers = {2}, punctuation = {3}.", pm1.char_count, pm1.alpha_count, pm1.digit_count, pm1.punctuation_count, user_password1), data, errors, method_timer.get_elapsed_seconds());
				return response;
			}

			bool user_exists = true;
			int attempt_count = 0;

			auto sys_perm = get_system_permission();

			do 
			{
				json existing_user_link = get_user(user_name, sys_perm);

				if (existing_user_link.object()) 
				{
					attempt_count++;
					char buff[128];
					buff[0] = ('0' + rand() % 10);
					buff[1] = 0;
					user_name = user_name + buff;
				}
				else 
				{
					user_exists = false;
				}

			} while (user_exists);

			std::string hashed_pw = crypter.hash(user_password1);

			json create_user_params = jp.create_object();
			create_user_params.put_member(class_name_field, user_class);
			create_user_params.put_member(user_name_field, user_name);
			create_user_params.put_member(user_password_field, hashed_pw);
			create_user_params.copy_member(user_first_name_field, data);
			create_user_params.copy_member(user_last_name_field, data);
			create_user_params.copy_member(user_email_field, data);
			create_user_params.copy_member(user_mobile_field, data);
			create_user_params.copy_member(user_street1_field, data);
			create_user_params.copy_member(user_street2_field, data);
			create_user_params.copy_member(user_state_field, data);
			create_user_params.copy_member(user_zip_field, data);

			// TODO: SYSTEM USER CONFIRMATION POLICY
			// the default system user doesn't need a confirmation process
			// this may change or at least be allowed as an option 
			// if in the future you really wanted to lock this down.

			bool is_system_user = _system_user;
			if (is_system_user) {
				is_system_user = user_name == user_name;
			}

			create_user_params.put_member("confirmed_code", is_system_user ? 1 : 0);

			std::string new_code = get_random_code();
			create_user_params.put_member("validation_code", new_code);

			json create_object_request = create_system_request(create_user_params);
			json user_result =  put_object(create_object_request);
			json jerrors = user_result["errors"];
			if (user_result[success_field]) {
				json new_user_wrapper = user_result[data_field]["sys_user"].get_element(0);
				new_user_wrapper = new_user_wrapper[data_field];
				if (not is_system_user)
				{
					send_user_confirmation(new_user_wrapper);
				}
				new_user_wrapper.erase_member("password");
				new_user_wrapper.erase_member("confirmed_code");
				new_user_wrapper.erase_member("validation_code");
				response = create_user_response(create_user_request, true, "User created", new_user_wrapper, jerrors, method_timer.get_elapsed_seconds());
			}
			else
			{
				response = create_user_response(create_user_request, false, "User not created", user_result, jerrors, method_timer.get_elapsed_seconds());
			}

			save();

			system_monitoring_interface::global_mon->log_function_stop("create_user", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return response;
		}


		virtual json send_user_confirm_code(json validation_code_request) override
		{
			timer method_timer;
			json_parser jp;
			json response;

			date_time start_time = date_time::now();
			timer tx;

			auto sys_perm = get_system_permission();

			system_monitoring_interface::global_mon->log_function_start("send_validation_code", "start", start_time, __FILE__, __LINE__);

			json data = validation_code_request[data_field];

			std::string user_name = data[user_name_field];

			json user_info = get_user(user_name, sys_perm);

			std::string message = "Code not sent.";

			if (user_info.object()) {
				message = "Code sent";
				send_user_confirmation(user_info);
			}

			json errors = jp.create_array();
			response = create_user_response(validation_code_request, true, message, data, errors, tx.get_elapsed_seconds());

			system_monitoring_interface::global_mon->log_function_stop("send_validation_code", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return response;
		}


		// this allows a user to login
		virtual json user_confirm_user_code(json _confirm_request)
		{
			timer method_timer;
			json_parser jp;
			json response;

			date_time start_time = date_time::now();
			timer tx;

			read_scope_lock my_lock(database_lock);
			std::vector<validation_error> errors;

			system_monitoring_interface::global_mon->log_function_start("confirm", "start", start_time, __FILE__, __LINE__);

			json data = _confirm_request;
			std::string user_name = data[user_name_field];
			std::string user_code = data["validation_code"];

			auto sys_perm = get_system_permission();

			json user = get_user(user_name, sys_perm);

			if (user.empty()) {

				std::vector<validation_error> errors;
				validation_error err;
				err.class_name = "sys_user";
				err.field_name = "user_name";
				err.filename = __FILE__;
				err.line_number = __LINE__;
				err.message = "username not found";
				errors.push_back(err);

				response = create_user_response(_confirm_request, false, "user not found", data, errors, method_timer.get_elapsed_seconds());
				return response;
			}

			std::string sys_code = user["validation_code"];

			if (sys_code == user_code)
			{
				user.put_member("confirmed_code", 1);
				std::string email = user[user_email_field];
				std::vector<std::string> email_components = split(email, '@');

				if (email_components.size() > 1) {
					std::string domain = email_components[1];
					json team = get_team_by_domain(domain, sys_perm);

					if (team.empty()) 
					{
						team = get_team_by_domain("*", sys_perm);
					}

					if (not team.empty())
					{
						user.put_member("team_name", (std::string)team["team_name"]);
						json workflow_objects = jp.create_array();
						json workflow_classes = team["workflow_classes"];
						workflow_objects = user["workflow_objects"];
						if (not workflow_objects.object())
						{
							workflow_objects = jp.create_object();
						}
						if (workflow_classes.array()) {
							for (auto wf_class : workflow_classes) {
								std::string class_name = (std::string)wf_class;
								if (workflow_objects.has_member(class_name)) {
									continue;
								}
								json create_req = jp.create_object();
								create_req.put_member(class_name_field, class_name);
								json sys_create_req = create_system_request(create_req);
								json result = create_object(sys_create_req);
								if (result[success_field]) {
									json new_object = result[data_field];
									int64_t object_id = new_object[object_id_field];
									json sys_create_req = create_system_request(new_object);
									workflow_objects.put_member_i64(class_name, object_id);
									put_object_nl(sys_create_req);
								}
							}
						}
						user.put_member("workflow_objects", workflow_objects);
					}
					else
					{
						user.put_member("team_name", std::string(""));
					}
				}

				put_user(user, sys_perm);

				response = create_response(user_name, auth_general, true, "Ok", data, errors, method_timer.get_elapsed_seconds());
			}
			else
			{
				std::vector<validation_error> errors;
				validation_error err;
				err.class_name = "sys_user";
				err.field_name = "validation_code";
				err.filename = __FILE__;
				err.line_number = __LINE__;
				err.message = "Incorrect validation code.";
				errors.push_back(err);

				response = create_user_response(_confirm_request, false, "Incorrect validation code.  Check your e-mail or send another one.", jp.create_object(), errors, method_timer.get_elapsed_seconds());
			}

			system_monitoring_interface::global_mon->log_function_stop("confirm", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			save();

			return response;
		}

		// and user set password
		virtual json set_user_password(json _password_request)
		{
			timer method_timer;
			json_parser jp;
			json response;

			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("confirm", "start", start_time, __FILE__, __LINE__);

			read_scope_lock my_lock(database_lock);
			std::vector<validation_error> errors;

			json data = _password_request[data_field];
			std::string user_name;
			std::string user_code = data["validation_code"];
			std::string requested_user_name = data[user_name_field];

			auto sys_perm = get_system_permission();

			json user = get_user(requested_user_name, sys_perm);

			if (user.empty()) {

				std::vector<validation_error> errors;
				validation_error err;
				err.class_name = "sys_user";
				err.field_name = "user_name";
				err.filename = __FILE__;
				err.line_number = __LINE__;
				err.message = "Incorrect user_name.";
				errors.push_back(err);

				response = create_user_response(_password_request, false, "user not found", data, errors, method_timer.get_elapsed_seconds());
				return response;
			}

			std::string sys_code = user["validation_code"];

			if (user_code.size()>0 and sys_code.size()>0 and sys_code == user_code)
			{
				user.put_member("validation_code", std::string(""));
				user_name = requested_user_name;

			}
			else if (not check_message(_password_request, { auth_general }, user_name))
			{
				std::vector<validation_error> errors;
				validation_error err;
				err.class_name = "sys_user";
				err.field_name = "user_name";
				err.filename = __FILE__;
				err.line_number = __LINE__;
				err.message = "Denied.";
				errors.push_back(err);

				response = create_user_response(_password_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("confirm", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			system_monitoring_interface::global_mon->log_function_start("confirm", "start", start_time, __FILE__, __LINE__);

			if (requested_user_name != user_name)
			{
				response = create_response(_password_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("confirm", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			std::string user_password1 = data["password1"];
			std::string user_password2 = data["password2"];

			if (user_password1 != user_password2)
			{
				std::vector<validation_error> errors;
				validation_error err;
				err.class_name = "sys_user";
				err.field_name = "password2";
				err.filename = __FILE__;
				err.line_number = __LINE__;
				err.message = "Passwords don't match.";
				errors.push_back(err);

				response = create_user_response(_password_request, false, "Passwords do not match.", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("confirm", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			password_metrics pm1(user_password1);

			if (pm1.is_stupid())
			{
				std::vector<validation_error> errors;
				validation_error err;
				err.class_name = "sys_user";
				err.field_name = "password1";
				err.filename = __FILE__;
				err.line_number = __LINE__;
				err.message = "Password too simple.";
				errors.push_back(err);

				response = create_user_response(_password_request, false, "Password fails complexity test.", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("confirm", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			std::string encrypted_password = crypter.hash(user_password1);
			user.put_member("password", encrypted_password);

			put_user(user, sys_perm);
			json jerrors;
			response = create_user_response(_password_request, true, "Ok", data, jerrors, method_timer.get_elapsed_seconds());

			system_monitoring_interface::global_mon->log_function_stop("confirm", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			save();

			return response;
		}


		// this starts a login attempt
		virtual json login_user(json _login_request)
		{
			timer method_timer;
			json_parser jp;
			json response;

			std::vector<validation_error> errors;

			date_time start_time = date_time::now();
			timer tx;

			read_scope_lock my_lock(database_lock);

			system_monitoring_interface::global_mon->log_function_start("login_user", "start", start_time, __FILE__, __LINE__);

			json data = _login_request;
			std::string user_name = data[user_name_field];
			std::string user_password = data["password"];
			std::string hashed_user_password;

			std::string hashed_pw = crypter.hash(user_password);
			auto sys_perm = get_system_permission();

			json user = get_user(user_name, sys_perm);
			std::string pw = user["password"];

			if (pw == hashed_pw)
			{
				bool confirm = (bool)user["confirmed_code"];

				json workflow = user["workflow_objects"];
				json navigation_options = jp.create_object();

				if (user_name == default_user and default_user.size() > 0) 
				{					
					if (workflow.object()) {
						auto members = workflow.get_members();
						for (auto member : members) {
							std::string class_name = member.first;
							int64_t object_id = (int64_t)member.second;
							json key = jp.create_object();
							key.put_member_i64(object_id_field, object_id);
							key.put_member(class_name_field, class_name);
							json obj = select_single_object(key, false, sys_perm);
							navigation_options.put_member(class_name, obj);
						}
					}

					json result = user.clone();
					result.put_member("navigation", navigation_options);
					result.erase_member("password");
					result.erase_member("confirmed_code");
					result.erase_member("validation_code");

					response = create_response(user_name, auth_system, true, "Ok", result, errors, method_timer.get_elapsed_seconds());
				}
				else if (confirm)
				{
					if (workflow.object()) {
						auto members = workflow.get_members();
						for (auto member : members) {
							std::string class_name = member.first;
							int64_t object_id = (int64_t)member.second;
							json key = jp.create_object();
							key.put_member_i64(object_id_field, object_id);
							key.put_member(class_name_field, class_name);
							json obj = select_single_object(key, false, sys_perm);
							navigation_options.put_member(class_name, obj);
						}
					}

					json result = user.clone();
					result.put_member("navigation", navigation_options);
					result.erase_member("password");
					result.erase_member("confirmed_code");
					result.erase_member("validation_code");

					response = create_response(user_name, auth_general, true, "Ok", result, errors, method_timer.get_elapsed_seconds());
				}
				else
				{
					response = create_response(user_name, auth_general, false, "Need confirmation", data, errors, method_timer.get_elapsed_seconds());
				}
			}
			else
			{
				response = create_response(_login_request, false, "Failed", jp.create_object(), errors, method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("login_user", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			save();

			return response;
		}

		virtual json run_object(json _run_object_request)
		{
			std::string user_name;
			json_parser jp;
			json response;

			date_time start_time = date_time::now();
			timer tx;
			std::vector<validation_error> errors;

			if (not check_message(_run_object_request, { auth_general }, user_name))
			{
				response = create_response(_run_object_request, false, "Denied", jp.create_object(), errors, tx.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("edit_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			json data = _run_object_request[data_field];
			json result = put_object(_run_object_request);
			if (result.error())
			{
				system_monitoring_interface::global_mon->log_function_stop("edit_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}
			json key = jp.create_object();
			key.copy_member(class_name_field, data);
			key.copy_member(object_id_field, data);
			json get_object_request = create_request(user_name, auth_general, key);
			get_object_request.put_member("include_children", true);
			response = get_object(get_object_request);
			return response;
		}

		virtual json edit_object(json _edit_object_request) override
		{
			timer method_timer;
			json_parser jp;
			json result;

			read_scope_lock my_lock(database_lock);

			date_time start_time = date_time::now();
			timer tx;
			std::string user_name;

			bool include_children = (bool)_edit_object_request["include_children"];
			std::vector<validation_error> errors;

			if (not check_message(_edit_object_request, { auth_general }, user_name))
			{
				result = create_response(_edit_object_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("edit_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			system_monitoring_interface::global_mon->log_function_start("edit_object", "start", start_time, __FILE__, __LINE__);

			json token = _edit_object_request[token_field];
			json key = _edit_object_request.extract({ class_name_field, object_id_field });
			int64_t object_id = (int64_t)key[object_id_field];
			std::string class_name = key[class_name_field];

			class_permissions perms = get_class_permission(user_name, class_name);

			if (perms.get_grant == class_grants::grant_none)
			{
				result = create_response(_edit_object_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("edit_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			auto edit_class = write_lock_class(class_name);
			if (edit_class) 
			{
				edit_class->init_validation(this, perms);
				json jedit_object = select_single_object(key, include_children, perms);
				if (not jedit_object.empty() and include_children) 
				{
					std::string token = _edit_object_request[token_field];
					edit_class->run_queries(this, token, class_name, jedit_object);
				}
				json jedit_class = jp.create_object();
				edit_class->get_json(jedit_class);
				result = jp.create_object();
				result.put_member("class", jedit_class);
				result.put_member("object", jedit_object);
			}
			else 
			{
				system_monitoring_interface::global_mon->log_function_stop("edit_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return create_response(_edit_object_request, false, "Invalid class.", jp.create_object(), errors, method_timer.get_elapsed_seconds());
			}

			system_monitoring_interface::global_mon->log_function_stop("edit_object", "success", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return create_response(_edit_object_request, true, "Ok", result, errors, method_timer.get_elapsed_seconds());
		}

		virtual json get_classes(json get_classes_request)
		{
			timer method_timer;
			json_parser jp;

			json result;
			json result_list;
			
			date_time start_time = date_time::now();
			timer tx;

			read_scope_lock my_lock(database_lock);
			std::vector<validation_error> errors;

			system_monitoring_interface::global_mon->log_function_start("get_classes", "start", start_time, __FILE__, __LINE__);

			std::string user_name;

			if (not check_message(get_classes_request, { auth_general }, user_name))
			{
				result = create_response(get_classes_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				return result;
			}

			result_list =  classes->select([this, user_name](int _index, json& _item) {
				auto permission = get_class_permission(user_name, _item[class_name_field]);
				json_parser jp;

				if (permission.get_grant != class_grants::grant_none) 
				{
					return _item;
				}
				else 
				{
					json empty = jp.create_object("Skip", "this");
					return empty;
				}
			});

			json data = jp.create_object();
			data.put_member("class", result_list);

			system_monitoring_interface::global_mon->log_function_stop("get_classes", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			result = create_response(get_classes_request, true, "Ok", result_list, errors, method_timer.get_elapsed_seconds());

			return result;
		}

		virtual json get_class(json get_class_request)
		{
			timer method_timer;
			json_parser jp;
			json result;

			date_time start_time = date_time::now();
			timer tx;

			read_scope_lock my_lock(database_lock);

			system_monitoring_interface::global_mon->log_function_start("get_class", "start", start_time, __FILE__, __LINE__);

			std::vector<std::string> missing_elements;
			std::vector<validation_error> errors;


			if (not get_class_request.has_members(missing_elements, { token_field, class_name_field })) {
				std::string error_message;
				error_message = "get_class missing elements:";
				std::string comma = "";
				for (auto m : missing_elements) {
					error_message.append(comma);
					error_message.append(m);
				}
				json response = create_response(get_class_request, false, error_message, jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			std::string user_name;

			if (not check_message(get_class_request, { auth_general }, user_name))
			{
				result = create_response(get_class_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			std::string class_name = get_class_request[class_name_field];

			auto permission = get_class_permission( user_name, class_name);

			json key = jp.create_object(class_name_field, class_name);
			key.set_natural_order();

			if (permission.get_grant != class_grants::grant_none) {

				json class_definition = classes->get(key);

				auto classd = read_lock_class(class_name);
				if (classd) {
					json class_info = classd->get_info(this);

					result = jp.create_object();
					result.put_member("class", class_definition);
					result.put_member("info", class_info);

					result = create_response(get_class_request, true, "Ok", result, errors, method_timer.get_elapsed_seconds());
					system_monitoring_interface::global_mon->log_function_stop("get_class", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
				else {
					result = create_response(get_class_request, false, "Missing Class", key, errors, method_timer.get_elapsed_seconds());
					system_monitoring_interface::global_mon->log_function_stop("get_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
			}
			else {
				result = create_response(get_class_request, false, "denied", key, errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_class", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return result;
		}

		virtual json put_class(json put_class_request)
		{
			timer method_timer;
			json result;
			json_parser jp;

			read_scope_lock my_lock(database_lock);
			std::vector<validation_error> errors;

			date_time start_time = date_time::now();
			timer tx;

			std::string pc_name = "put_class";
			std::string pc_msg = (std::string)put_class_request[data_field][class_name_field];
			std::string pc_start = pc_msg + " start";
			std::string pc_stop = pc_msg + " stop";
			std::string pc_failed = pc_msg + " failed";

			system_monitoring_interface::global_mon->log_function_start(pc_name, pc_start, start_time, __FILE__, __LINE__);

			std::vector<std::string> missing_elements;
			if (not put_class_request.has_members(missing_elements, { token_field })) {
				std::string error_message;
				error_message = "create_class missing elements:";
				std::string comma = "";
				for (auto m : missing_elements) {
					error_message.append(comma);
					error_message.append(m);
				}
				json response = create_response(put_class_request, false, error_message, jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop(pc_name, pc_failed, tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			std::string user_name;

			if (not check_message(put_class_request, { auth_general }, user_name))
			{
				result = create_response(put_class_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop(pc_name, pc_failed, tx.get_elapsed_seconds(), __FILE__, __LINE__);

				return result;
			}

			json token = put_class_request[token_field];
			json jclass_definition = put_class_request[data_field];
			std::string class_name = jclass_definition[class_name_field];

			if (jclass_definition.error())
			{
				result = create_response(put_class_request, false, "Invalid class", jclass_definition, errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop(pc_name, pc_failed, tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			if (class_name.empty()) {
				result = create_response(put_class_request, false, "No class name", jclass_definition, errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop(pc_name, pc_failed, tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			auto permission = get_class_permission(
				user_name,
				class_name);

			if (permission.put_grant != class_grants::grant_any) {
				result = create_response(put_class_request, false, "Denied", jclass_definition, errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop(pc_name, pc_failed, tx.get_elapsed_seconds(), __FILE__, __LINE__);

				return result;
			}

			activity update_activity;

			update_activity.db = this;

			auto class_to_modify = update_activity.create_class(class_name);

			bool success = class_to_modify->update(&update_activity, jclass_definition);

			if (success) 
			{				
				save_class(class_to_modify);
				result = create_response(put_class_request, success, "Ok", jclass_definition, update_activity.errors, method_timer.get_elapsed_seconds());
			}
			else 
			{
				result = create_response(put_class_request, success, "errors", jclass_definition, update_activity.errors, method_timer.get_elapsed_seconds());
			}

			save();
			system_monitoring_interface::global_mon->log_function_stop(pc_name, pc_stop, tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return result;
		}


		private:

		json query_class(std::string _user_name, json query_details, json update_json)
		{
			timer method_timer;
			json_parser jp;
			json response;

			date_time start_time = date_time::now();
			timer tx;
			std::vector<validation_error> errors;

			system_monitoring_interface::global_mon->log_function_start("update", "start", start_time, __FILE__, __LINE__);

			bool include_children = (bool)query_details["include_children"];

			json base_class_name = query_details[class_name_field];
			if (base_class_name.empty()) {
				response = create_response(_user_name, auth_general, false, "class_name not specified", query_details, errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("update", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			auto permission =  get_class_permission(_user_name, base_class_name);
			if (permission.get_grant == class_grants::grant_none)
			{
				response = create_response(_user_name, auth_general, false, "denied", query_details, errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("update", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			auto class_def = read_lock_class(base_class_name);

			json object_list = jp.create_array();
			if (not class_def) {
				response = create_response(_user_name, auth_general, false, "class not found", query_details, errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("update", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			json filter = query_details["filter"];
			if (filter.empty()) {
				filter = jp.create_object();
			}

			for (auto class_pair : class_def->get_descendants())
			{
				read_class_sp classd = read_lock_class(class_pair.first);
				if (classd) {
					json objects = classd->get_objects(this, filter, include_children, permission);

					if (objects.array()) {
						for (auto obj : objects) {
							if (permission.get_grant == class_grants::grant_any or
								(permission.get_grant == class_grants::grant_own 
									and (std::string)obj["created_by"] == _user_name)) {
								object_list.push_back(obj);
							}
						}
					}
				}
			}

			response = create_response(_user_name, auth_general, true, "completed", object_list, errors, method_timer.get_elapsed_seconds());

			system_monitoring_interface::global_mon->log_function_stop("update", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			return response;
		}

		public:

		virtual json query(json query_request)
		{
			timer tx;
			read_scope_lock my_lock(database_lock);
			json_parser jp;
			json jx; // not creating an object, leaving it empty.  should work with empty objects
			// or with an object that has no members.
			json response;

			std::string user_name;
			std::vector<validation_error> errors;

			if (not check_message(query_request, { auth_general }, user_name))
			{
				response = create_response(query_request, false, "Denied", jp.create_object(), errors, tx.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("query", "denied", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			query_context context;

			context.froms_preloaded = true;

			json from_classes = query_request["from"];
			if (from_classes.array())
			{
				json stages = query_request["stages"];

				if (not stages.array() or stages.size() == 0)
				{
					response = create_response(query_request, false, "query has no stages", jp.create_object(), errors, tx.get_elapsed_seconds());
					system_monitoring_interface::global_mon->log_function_stop("query", "query has no stages", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					return response;
				}

				for (auto from_class : from_classes)
				{
					std::string from_class_name = from_class[class_name_field];
					std::string from_name = from_class["name"];
					if (from_name.empty())
					{
						response = create_response(query_request, false, "from with no name", jp.create_object(), errors, tx.get_elapsed_seconds());
						system_monitoring_interface::global_mon->log_function_stop("query", "from with no name", tx.get_elapsed_seconds(), __FILE__, __LINE__);
						return response;
					}
					if (from_class_name.empty())
					{
						response = create_response(query_request, false, "from with no class", jp.create_object(), errors, tx.get_elapsed_seconds());
						system_monitoring_interface::global_mon->log_function_stop("query", "from with no class", tx.get_elapsed_seconds(), __FILE__, __LINE__);
						return response;
					}
					auto cd = read_lock_class(from_class_name);
					if (not cd) {
						std::string message = std::format("from class '{0}' not found", from_class_name);
						response = create_response(query_request, false, message, jp.create_object(), errors, tx.get_elapsed_seconds());
						system_monitoring_interface::global_mon->log_function_stop("query", "from class not found", tx.get_elapsed_seconds(), __FILE__, __LINE__);
						return response;
					}
				}

				from_join fj;

				// these run in order to allow for dependencies
				for (auto from_class : from_classes)
				{
					std::string from_class_name = from_class[class_name_field];
					std::string filter_source_name = from_class["name"];
					std::string filter_class_name = from_class[class_name_field];
					json data = from_class[data_field];
					json objects;

					// allow the query to have inline objects
					if (data.object())
					{
						objects = jp.create_array();
						objects.push_back(data);
						fj.add_data(from_class, objects);
					}
					else if (data.array())
					{
						objects = data;
						fj.add_data(from_class, objects);
					}
					else if (data.empty()) {
						objects = jp.create_array();
						json from_classes = jp.create_array();
						json class_filter = from_class["filter"];
						auto filter_class = read_lock_class(filter_class_name);

						if (class_filter.object()) {
							auto members = class_filter.get_members();
							for (auto member : members)
							{
								auto fld = filter_class->get_field(member.first);
								if (not fld) {
									context.add_error(filter_source_name, member.first, "Invalid field for filter", __FILE__, __LINE__);
								}
								if (member.second.is_string())
								{
									std::string value = (std::string)member.second;
									if (value.starts_with("$"))
									{
										std::string path(value.substr(1));
										std::vector<std::string> split_path = split(path, '.');
										if (split_path.size() == 2) {
											std::string& source_from_name = split_path[0];
											std::string& source_from_member = split_path[1];
											fj.add_join(source_from_name, source_from_member, filter_source_name, member.first);
										}
										else
										{
											response = create_response(query_request, false, "Bad $ reference in query.", class_filter, errors, tx.get_elapsed_seconds());
											system_monitoring_interface::global_mon->log_function_stop("query", "bad query data", tx.get_elapsed_seconds(), __FILE__, __LINE__);
											return response;
										}
									}
								}
							}
						}
						json filters = fj.get_filters(from_class);
						if (filters.size() > 0) {
							for (auto from_filter : filters)
							{
								json query_class_response = query_class(user_name, from_filter, jx);
								json temp_objects = query_class_response[data_field];
								if (temp_objects.array()) {
									bool include_children = (bool)from_class["include_children"];
									if (include_children)
									{
										auto edit_class = read_lock_class(filter_class_name);
										if (edit_class) {
											std::string token = query_request[token_field];
											for (auto obj : temp_objects) {
												edit_class->run_queries(this, token, filter_class_name, obj);
											}
										}
									}
									for (auto obj : temp_objects)
									{

										objects.push_back(obj);
									}
									fj.add_data(from_filter, objects);
								}
							}							
						}
						else
						{
							std::string message = std::format("Missing filters for {0}", filter_class_name);
							response = create_response(query_request, false, message, jp.create_object(), errors, tx.get_elapsed_seconds());
							system_monitoring_interface::global_mon->log_function_stop("query", "filter error", tx.get_elapsed_seconds(), __FILE__, __LINE__);
							return response;
						}
					}
					else {
						response = create_response(query_request, true, "query data is not an object or an array", jp.create_object(), errors, tx.get_elapsed_seconds());
						system_monitoring_interface::global_mon->log_function_stop("query", "bad query data",  tx.get_elapsed_seconds(), __FILE__, __LINE__);
						return response;
					}
				}

				for (auto from_class : from_classes)
				{
					std::string from_name = from_class["name"];
					json data = fj.get_data(from_name);
					context.set_data_source(from_name, data);
				}

				if (context.is_error()) 
				{
					json query_errors = context.get_errors();
					response = create_response(query_request, false, "errors", query_errors, errors, tx.get_elapsed_seconds());
					system_monitoring_interface::global_mon->log_function_stop("query", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					return response;
				}

				// so now, we've loaded up our context, we can extract the stages
				context.put_json(query_request);

				// and, we can run the thing.
				json query_results = context.run();
				if (context.is_error()) {
					json query_errors = context.get_errors();
					response = create_response(query_request, false, "errors", query_errors, errors, tx.get_elapsed_seconds());
					system_monitoring_interface::global_mon->log_function_stop("query", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
				else 
				{
					json query_errors = context.get_errors();
					response = create_response(query_request, true, "completed", query_results, errors, tx.get_elapsed_seconds());
					system_monitoring_interface::global_mon->log_function_stop("query", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				}
			}
			else 
			{
				response = create_response(query_request, true, "query has no froms", jp.create_object(), errors, tx.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("query", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			return response;
		}


		virtual json create_object(json create_object_request)
		{
			timer method_timer;
			json_parser jp;
			using namespace std::literals;
			date_time start_time = date_time::now();
			timer tx;
			read_scope_lock my_lock(database_lock);
			system_monitoring_interface::global_mon->log_function_start("create_object", "start", start_time, __FILE__, __LINE__);

			json token = create_object_request[token_field];
			json data = create_object_request[data_field];
			std::string class_name = data[class_name_field];
			json response;

			std::string user_name;
			std::vector<validation_error> errors;

			if (not check_message(create_object_request, { auth_general }, user_name))
			{
				response = create_response(create_object_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("create_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			auto permission =  get_class_permission(user_name, class_name);
			if (permission.put_grant == class_grants::grant_none) {
				json result = create_response(create_object_request, false, "Denied", data, errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("create_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			auto class_def = read_lock_class(class_name);

			if (class_def) {

				json new_object = jp.create_object();
				new_object.put_member(class_name_field, class_name);

				auto fields = class_def->get_fields();

				for (auto& field : fields)
				{
					std::string field_name = field->get_field_name();

					if (field_name == "created_by" or field_name == "created" or field_name == "updated_by" or field_name == "updated")
						continue;

					if (data.has_member(field_name))
					{
						new_object.copy_member(field_name, data);
						continue;
					}

					if (field_name == class_name_field) {
						new_object.put_member(field_name, class_name);
					}
					else if (field->get_field_type() == field_types::ft_object)
					{
						new_object.put_member_object(field_name);
					}
					else if (field->get_field_type() == field_types::ft_array or field->get_field_type() == field_types::ft_query)
					{
						new_object.put_member_array(field_name);
					}
					else if (field->get_field_type() == field_types::ft_double)
					{
						new_object.put_member(field_name, 0.0);
					}
					else if (field->get_field_type() == field_types::ft_string)
					{
						new_object.put_member(field_name, ""sv);
					}
					else if (field->get_field_type() == field_types::ft_int64)
					{
						new_object.put_member_i64(field_name, 0);
					}
					else if (field->get_field_type() == field_types::ft_datetime)
					{
						date_time dt;
						new_object.put_member(field_name, dt);
					}
					else if (field->get_field_type() == field_types::ft_function)
					{
						auto key = std::make_tuple(class_name, field_name);
						if (functions.contains(key)) {
							new_object.put_member_function(field_name, functions[key]);
						}
						else 
						{
							std::string err_message = std::format("function {0} {1} not defined", class_name, field_name);
							new_object.put_member(field_name, err_message);
						}
					}
				}
				int64_t new_id = get_next_object_id();
				new_object.put_member_i64("object_id", new_id);
				response = create_response(create_object_request, true, "Object created", new_object, errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("create_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else {
				std::string msg = std::format("create_object failed because the class '{0}' was never found.", class_name);
				system_monitoring_interface::global_mon->log_warning(msg);
				response = create_response(create_object_request, false, "Couldn't find class", create_object_request, errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("create_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			save();

			return response;
			
		}

		virtual json put_object_nl(json put_object_request)
		{
			timer method_timer;
			json_parser jp;
			json token;
			json object_definition;
			json result;

			date_time start_time = date_time::now();
			timer tx;
			std::vector<validation_error> errors;

			system_monitoring_interface::global_mon->log_function_start("put_object", "start", start_time, __FILE__, __LINE__);

			object_definition = put_object_request[data_field];
			std::string user_name;

			if (not check_message(put_object_request, { auth_general }, user_name))
			{
				result = create_response(put_object_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("put_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			result = check_object(object_definition, user_name, errors);

			json grouped_by_class_name = result[data_field];

			if (result[success_field])
			{
				auto classes_and_data = grouped_by_class_name.get_members();

				json child_objects = jp.create_array();

				for (auto class_pair : classes_and_data)
				{
					auto cd = read_lock_class(class_pair.first);
					if (cd) {

						// now that we have our class, we can go ahead and open the storage for it

						json data_list = class_pair.second.map([](std::string _member, int _index, json& _item) -> json {
							return _item[data_field];
							});

						auto perms = get_class_permission(user_name, class_pair.first);

						cd->put_objects(this, child_objects, data_list, perms);
					}
				}

				header.write(this);

				if (child_objects.size() == 0) {
					save();
				}
				else
				{
					put_object_request.put_member(data_field, child_objects);
					put_object(put_object_request);
				}

				result = create_response(put_object_request, true, "Object(s) created", grouped_by_class_name, errors, method_timer.get_elapsed_seconds());
			}
			else
			{
				result = create_response(put_object_request, false, result[message_field], grouped_by_class_name, errors, method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("put_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return result;
		}

		virtual json put_object(json put_object_request)
		{
			read_scope_lock my_lock(database_lock);

			return put_object_nl(put_object_request);
		}

		virtual json get_object(json get_object_request)
		{
			timer method_timer;
			json_parser jp;
			json result;
			read_scope_lock my_lock(database_lock);
			date_time start_time = date_time::now();
			timer tx;

			system_monitoring_interface::global_mon->log_function_start("get_object", "start", start_time, __FILE__, __LINE__);

			std::string user_name;
			json object_key = get_object_request.extract({ class_name_field, object_id_field });

			std::vector<validation_error> errors;
			if (not check_message(get_object_request, { auth_general }, user_name))
			{
				result = create_response(get_object_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			std::string class_name = object_key[class_name_field];

			auto permission =  get_class_permission(user_name, class_name);
			if (permission.get_grant == class_grants::grant_none) {
				json result = create_response(get_object_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return result;
			}

			bool include_children = (bool)get_object_request["include_children"];
			
			json obj =  select_single_object(object_key, include_children, permission);
			if (obj.object()) 
			{
				if (include_children)
				{
					auto edit_class = read_lock_class(class_name);
					if (edit_class) {
						std::string token = get_object_request[token_field];
						edit_class->run_queries(this, token, class_name, obj);
					}
				}

				result = create_response(get_object_request, true, "Ok", obj, errors, method_timer.get_elapsed_seconds());
			}
			else 
			{
				result = create_response(get_object_request, false, "Not found", object_key, errors, method_timer.get_elapsed_seconds());
			}
			system_monitoring_interface::global_mon->log_function_stop("get_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

			return result;
		}


		virtual json delete_object(json delete_object_request)
		{
			timer method_timer;
			json response;
			json_parser jp;
			read_scope_lock my_lock(database_lock);
			date_time start_time = date_time::now();			
			timer tx;

			std::vector<validation_error> errors;

			system_monitoring_interface::global_mon->log_function_start("delete_object", "start", start_time, __FILE__, __LINE__);

			std::string user_name;
			json object_key = delete_object_request.extract({ class_name_field, object_id_field });

			if (not check_message(delete_object_request, { auth_general }, user_name))
			{
				response = create_response(delete_object_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("delete_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			std::string class_name = object_key[class_name_field];

			auto permission = get_class_permission(user_name, class_name);
			if (permission.delete_grant == class_grants::grant_none) {
				response = create_response(delete_object_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("get_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			auto cd = read_lock_class(class_name);
			if (cd) {

				cd->delete_objects(this, object_key, true, permission);

				response = create_response(delete_object_request, true, "Ok", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("delete_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}
			else 
			{
				response = create_response(delete_object_request, true, "class not found", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("delete_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			}

			save();

			return response;
		}

		virtual json copy_object(json copy_request)
		{
			timer method_timer;
			json_parser jp;
			read_scope_lock my_lock(database_lock);
			json response;

			date_time start_time = date_time::now();
			timer tx;
			system_monitoring_interface::global_mon->log_function_start("copy_object", "start", start_time, __FILE__, __LINE__);

			std::vector<validation_error> errors;
			std::string user_name;

			if (not check_message(copy_request, { auth_general }, user_name))
			{
				response = create_response(copy_request, false, "Denied", jp.create_object(), errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("copy_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			json source_spec = copy_request["from"];
			std::string source_class = source_spec[class_name_field];			
			std::string source_path = source_spec["path"];
			json source_key = source_spec.extract({ class_name_field, object_id_field });

			json dest_spec = copy_request["to"];
			std::string dest_class = dest_spec[class_name_field];
			std::string dest_path = dest_spec["path"];
			json dest_key = dest_spec.extract({ class_name_field, object_id_field });

			json transform_spec = copy_request["transform"];
			std::string transform_class = transform_spec[class_name_field];

			class_permissions src_permission, dest_permission, trans_permission;

			src_permission = get_class_permission(user_name, source_class);
			if (src_permission.get_grant == class_grants::grant_none) {
				response = create_response(copy_request, false, "Denied", source_spec, errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("copy_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			dest_permission = get_class_permission(user_name, dest_class);
			if (dest_permission.put_grant == class_grants::grant_none) {
				response = create_response(copy_request, false, "Denied", dest_spec, errors, method_timer.get_elapsed_seconds());
				system_monitoring_interface::global_mon->log_function_stop("copy_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
				return response;
			}

			if (not transform_class.empty()) {
				trans_permission = get_class_permission(user_name, transform_class);
				if (trans_permission.get_grant == class_grants::grant_none) {
					response = create_response(copy_request, false, "Denied", transform_spec, errors, method_timer.get_elapsed_seconds());
					system_monitoring_interface::global_mon->log_function_stop("copy_object", "failed", tx.get_elapsed_seconds(), __FILE__, __LINE__);
					return response;
				}
			}

			// load the complete object
			json object_source = select_object(source_key, true, src_permission);

			json object_to_copy_result;
			json source_object;

			if (not object_source.empty())
			{
				// if the source object is not empty, then, walk the source path to extract the child object we want
				object_to_copy_result = object_source.query(source_path);
				source_object = object_to_copy_result["value"];

				// if that worked, then let's go for the copy
				if (source_object.object()) 
				{
					json new_object;
					// we transform the object into a new class while copying, if there is a 
					// transform specification

					if (not transform_class.empty()) {
						// so we create a new object of the class
						json transform_key = transform_spec.extract({ class_name_field });
						json cor = create_request(user_name, auth_general, transform_spec);
						json new_object_result = create_object(cor);

						// then we can copy the fields over
						if (new_object_result[success_field]) {
							new_object = new_object_result[data_field];
							auto new_object_fields = new_object.get_members();
							for (auto nof : new_object_fields) {
								if (source_object.has_member(nof.first)) {
									new_object.copy_member(nof.first, source_object);
								}
							}
						}
					}
					else 
					{
						new_object = source_object.clone();
						int64_t new_object_id = get_next_object_id();
						new_object.put_member_i64(object_id_field, new_object_id);
					}

					//
					json object_dest = select_single_object(dest_key, true, dest_permission);

					// now we are going to put this object in our destination
					json object_dest_result = object_dest.query(dest_path);

					if (object_dest_result.object()) {
						json update_obj;
						json target = object_dest_result["target"];
						std::string name = object_dest_result["name"];
						update_obj = object_dest_result["object"];
						if (target.array())
						{
							target.push_back(new_object);
						}
						else if (target.object())
						{
							target.put_member(name, new_object);
						}
						json por = create_request(user_name, auth_general, update_obj);
						response = put_object(por);
					}
					else {
						response = create_response(copy_request, false, "could not find dest object", object_dest, errors, method_timer.get_elapsed_seconds());;
					}
				}
				else {
					response = create_response(copy_request, false, "could not find source object", object_source, errors, method_timer.get_elapsed_seconds());;
				}
			}
			else {
				response = create_response(copy_request, false, "source object not specified", object_source, errors, method_timer.get_elapsed_seconds());;;
			}

			system_monitoring_interface::global_mon->log_function_stop("copy_object", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);
			save();
			return response;
		}

		private:

		json create_system_request(json _data)
		{
			json_parser jp;

			json payload;

			payload = jp.create_object();

			json token = jp.create_object();
			token.put_member(user_name_field, default_user);
			token.put_member(authorization_field, auth_system);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member(token_expires_field, expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member(signature_field, cipher_text);

			std::string token_string = token.to_json_typed();
			std::string base64_token_string = base64_encode(token_string);

			payload.put_member(token_field, base64_token_string);

			payload.put_member(data_field, _data);

			return payload;
		}


		json create_request(std::string _user_name, std::string _authorization, json _data)
		{
			json_parser jp;

			json payload;

			if (_data.object())
			{
				payload = jp.create_object();
			}

			json token = jp.create_object();
			token.put_member(user_name_field, _user_name);
			token.put_member(authorization_field, _authorization);
			date_time expiration = date_time::utc_now() + this->token_life;
			token.put_member(token_expires_field, expiration);
			std::string cipher_text = crypter.encrypt(token, get_pass_phrase(), get_iv());
			token.put_member(signature_field, cipher_text);

			std::string token_string = token.to_json_typed();
			std::string base64_token_string = base64_encode(token_string);

			payload.put_member(token_field, base64_token_string);
			payload.put_member(data_field, _data);

			return payload;
		}

	};

	////

	class_interface* activity::get_class(std::string _class_name)
	{
		auto fi = classes.find(_class_name);

		if (fi != classes.end()) {
			return fi->second.get();
		}

		auto impl = db->write_lock_class(_class_name);
		classes.emplace(std::pair<std::string, write_class_sp>(_class_name, std::move(impl)));
		return get_class(_class_name);
	}

	class_interface* activity::create_class(std::string _class_name)
	{
		auto fi = classes.find(_class_name);

		if (fi != classes.end()) {
			return fi->second.get();
		}

		auto impl = db->create_lock_class(_class_name);
		classes.emplace(std::pair<std::string, write_class_sp>(_class_name, std::move(impl)));
		return get_class(_class_name);
	}


	bool test_database_engine(json& _proof, std::shared_ptr<application> _app)
	{
		bool success = true;
		std::shared_ptr<file> dtest = std::make_shared<file>();
		using namespace std::literals;
		date_time st = date_time::now();
		timer tx;
		system_monitoring_interface::global_mon->log_function_start("table proof", "start", st, __FILE__, __LINE__);

		std::shared_ptr<file> f = _app->create_file_ptr(FOLDERID_Documents, "corona_database.cdb");

		json_parser jp;
		json proof_assertion = jp.create_object();

		proof_assertion.put_member("test_name", "database"sv);

		json dependencies = jp.parse_object(R"( 
{ 
	"create" : [  "table.get", "table.put", "table.create", "table.select" ],
	"login" : [  "table.get", "table.put",  "table.select" ],
    "class" : [ "table.get", "table.put", "table.create", "table.select" ],
	"class_index" : [ "table.get", "table.put", "table.create", "table.select" ],
	"create" : [ "table.get", "table.put", "table.select" ],
	"put" : [ "table.get", "table.put", "table.select" ],
	"put_index" : [ "table.get", "table.put", "table.select" ],
	"get" : [ "table.get" ],
	"get_index" : [ "table.get", "table.select" ],
	"delete" : [ "table.erase" ],
	"team_class" : [ "table.get", "table.put", "table.create", "table.select" ],
	"team_get" : [ "table.get", "table.select" ],
	"team_put" : [ "table.get", "table.put", "table.select" ],
	"team_delete" : [ "table.get", "table.put", "table.create", "table.select" ]
}
)");

		bool login_success;

		date_time start_schema = date_time::now();
		system_monitoring_interface::global_mon->log_job_start("test_database_engine", "start", start_schema, __FILE__, __LINE__);

		corona_database db(dtest);

		proof_assertion.put_member("dependencies", dependencies);
		json db_config = jp.create_object();
		json server_config = jp.create_object();
		server_config.put_member(sys_user_name_field, "todd"sv);
		server_config.put_member(sys_user_password_field, "randomite"sv);
		server_config.put_member(sys_user_email_field, "todd.bandrowsky@gmail.com"sv);
		server_config.put_member(sys_default_team_field, "GuestTeam"sv);
		db_config.put_member("Server", server_config);

		db.apply_config(db_config);

		relative_ptr_type database_location = db.create_database();

		login_success = true;

		json login_positive_request = R"(
{
	"user_name" : "todd",
	"password" : "randomite"
}
)"_jobject;

		json login_negative_request = R"(
{
	"user_name" : "todd",
	"password" : "reachio"
}
)"_jobject;

		json login_result;

		login_result = db.login_user(login_negative_request);

		if (login_result[success_field]) {
			login_success = false;
			system_monitoring_interface::global_mon->log_warning("able to login with bad account", __FILE__, __LINE__);
		}

		login_result = db.login_user(login_positive_request);

		if (not login_result[success_field]) {
			login_success = false;
			system_monitoring_interface::global_mon->log_warning("can't with good account", __FILE__, __LINE__);
		}


		system_monitoring_interface::global_mon->log_job_stop("test_database_engine", "complete", tx.get_elapsed_seconds(), __FILE__, __LINE__);

		return success;
	}

	bool collection_tests()
	{

		try {

			application app;

			std::shared_ptr<dynamic_box> box = std::make_shared<dynamic_box>();
			box->init(1 << 21);

			jschema schema;
			relative_ptr_type schema_id;

			schema = jschema::create_schema(box, 20, true, schema_id);

			relative_ptr_type quantity_field_id = null_row;
			relative_ptr_type last_name_field_id = null_row;
			relative_ptr_type first_name_field_id = null_row;
			relative_ptr_type birthday_field_id = null_row;
			relative_ptr_type count_field_id = null_row;
			relative_ptr_type title_field_id = null_row;
			relative_ptr_type institution_field_id = null_row;

			schema.bind_field("quantity", quantity_field_id);
			schema.bind_field("lastName", last_name_field_id);
			schema.bind_field("firstName", first_name_field_id);
			schema.bind_field("birthday", birthday_field_id);
			schema.bind_field("count", count_field_id);
			schema.bind_field("title", title_field_id);
			schema.bind_field("institutionName", institution_field_id);

			jcollection_ref ref;
			ref.data = std::make_shared<dynamic_box>(1 << 20);
			ref.max_objects = 50;
			ref.collection_size_bytes = 1 << 19;

			init_collection_id(ref.collection_id);

			jcollection people = schema.create_collection(&ref);

			put_class_request person;

			person.class_name = "person";
			person.class_description = "a person";
			person.member_fields = { last_name_field_id, first_name_field_id, birthday_field_id, count_field_id, quantity_field_id };
			relative_ptr_type person_class_id = schema.put_class(person);

			if (person_class_id == null_row)
			{
				std::cout << __LINE__ << ":class create failed failed" << std::endl;
				return false;
			}

			int birthdaystart = 1941;
			int countstart = 12;
			double quantitystart = 10.22;
			int increment = 5;

			relative_ptr_type people_object_id;

			auto sl = people.create_object(person_class_id, people_object_id);
			auto last_name = sl.get_string(0);
			auto first_name = sl.get_string(1);
			auto birthday = sl.get_time(2);
			auto count = sl.get_int64(3);
			auto qty = sl.get_double(4);
			last_name = "last 1";
			first_name = "first 1";
			birthday = birthdaystart + increment * 0;
			count = countstart + increment * 0;
			qty = quantitystart + increment * 0;

			sl = people.create_object(person_class_id, people_object_id);
			last_name = sl.get_string(0);
			first_name = sl.get_string(1);
			birthday = sl.get_time(2);
			count = sl.get_int64(3);
			qty = sl.get_double(4);
			last_name = "last 2";
			first_name = "first 2";
			birthday = birthdaystart + increment * 1;
			count = countstart + increment * 1;
			qty = quantitystart + increment * 1;

			sl = people.create_object(person_class_id, people_object_id);
			last_name = sl.get_string(0);
			first_name = sl.get_string(1);
			birthday = sl.get_time(2);
			count = sl.get_int64(3);
			qty = sl.get_double(4);
			last_name = "last 3";
			first_name = "first 3";
			birthday = birthdaystart + increment * 2;
			count = countstart + increment * 2;
			qty = quantitystart + increment * 2;

			sl = people.create_object(person_class_id, people_object_id);
			last_name = sl.get_string(0);
			first_name = sl.get_string(1);
			birthday = sl.get_time(2);
			count = sl.get_int64(3);
			qty = sl.get_double(4);
			last_name = "last 4";
			first_name = "first 4";
			birthday = birthdaystart + increment * 3;
			count = countstart + increment * 3;
			qty = quantitystart + increment * 3;

			sl = people.create_object(person_class_id, people_object_id);
			last_name = sl.get_string(0);
			first_name = sl.get_string(1);
			birthday = sl.get_time(2);
			count = sl.get_int64(3);
			qty = sl.get_double(4);
			last_name = "last 5 really long test 12345 abcde 67890 fghij 12345 klmno 67890 pqrst";
			first_name = "first 5 really long test 12345 abcde 67890 fghij 12345 klmno 67890 pqrst";
			birthday = birthdaystart + increment * 4;
			count = countstart + increment * 4;
			qty = quantitystart + increment * 4;

			int inc_count = 0;

			for (auto sl : people)
			{
				last_name = sl.item.get_string(0);
				if (not last_name.starts_with("last")) {
					std::cout << __LINE__ << ":last name failed" << std::endl;
					return false;
				}
				first_name = sl.item.get_string(1);
				if (not first_name.starts_with("first")) {
					std::cout << __LINE__ << ":first name failed" << std::endl;
					return false;
				}
				birthday = sl.item.get_time(2);
				count = sl.item.get_int64(3);
				qty = sl.item.get_double(4);

				if (birthday != birthdaystart + increment * inc_count) {
					std::cout << __LINE__ << ":birthday failed" << std::endl;
					return false;
				}

				if (count != countstart + increment * inc_count) {
					std::cout << __LINE__ << ":count failed" << std::endl;
					return false;
				}

				if (qty != quantitystart + increment * inc_count) {

					std::cout << __LINE__ << ":qty failed" << std::endl;
					return false;
				}

				inc_count++;
			}

			return true;
		}
		catch (std::exception exc)
		{
			system_monitoring_interface::global_mon->log_exception(exc);
			return false;
		}

	}

	bool array_tests()
	{

		try {
			std::shared_ptr<dynamic_box> box = std::make_shared<dynamic_box>();
			box->init(1 << 21);
			application app;

			jschema schema;
			relative_ptr_type schema_id;

			schema = jschema::create_schema(box, 50, true, schema_id);

			put_class_request sprite_frame_request;

			sprite_frame_request.class_name = "spriteframe";
			sprite_frame_request.class_description = "sprite frame";
			sprite_frame_request.member_fields = { "shortName", "object_x", "object_y", "object_w", "object_h", "color" };
			relative_ptr_type sprite_frame_class_id = schema.put_class(sprite_frame_request);

			if (sprite_frame_class_id == null_row) {
				std::cout << __LINE__ << ":class create failed failed" << std::endl;
				return false;
			}

			put_class_request sprite_class_request;
			sprite_class_request.class_name = "sprite";
			sprite_class_request.class_description = "sprite";
			sprite_class_request.member_fields = { "shortName", "object_x", "object_y", "object_w", "object_h" , member_field(sprite_frame_class_id) };
			relative_ptr_type sprite_class_id = schema.put_class(sprite_class_request);

			if (sprite_class_id == null_row) {
				std::cout << __LINE__ << ":class create failed failed" << std::endl;
				return false;
			}

			collection_id_type colid;

			init_collection_id(colid);

			relative_ptr_type classesb[2] = { sprite_class_id, null_row };

			jcollection_ref ref;
			ref.data = std::make_shared<dynamic_box>(1 << 20);
			ref.max_objects = 50;
			ref.collection_size_bytes = 1 << 19;

			init_collection_id(ref.collection_id);

			jcollection sprites = schema.create_collection(&ref);

			for (int i = 0; i < 10; i++) {

				relative_ptr_type new_sprite_id;
				auto slice = sprites.create_object(sprite_class_id, new_sprite_id);
				auto image_name = slice.get_string(0);


				auto bx = slice.get_int32("object_x");
				auto by = slice.get_int32("object_y");
				auto bw = slice.get_int32("object_w");
				auto bh = slice.get_int32("object_h");

				rectangle image_rect;

				image_name = std::format("{} #{}", "image", i);
				bx = 0;
				by = 0;
				bw = 1000;
				bh = 1000;

#if _DETAIL
				std::cout << "before:" << image_name << std::endl;
				std::cout << image_rect->w << " " << image_rect->h << " " << image_rect->x << " " << image_rect->y << std::endl;
#endif

			}

			int scount = 0;

			for (auto slice : sprites)
			{
				auto image_name = slice.item.get_string(0);
				auto bx = slice.item.get_int32("object_x");
				auto by = slice.item.get_int32("object_y");
				auto bw = slice.item.get_int32("object_w");
				auto bh = slice.item.get_int32("object_h");

#if _DETAIL
				std::cout << image_name << std::endl;
				std::cout << image_rect->w << " " << image_rect->h << " " << image_rect->x << " " << image_rect->y << std::endl;
#endif

				if (bw != 1000 or bh != 1000) {

					std::cout << __LINE__ << ":array failed" << std::endl;
					return false;
				}

			}

			return true;
		}
		catch (std::exception& exc)
		{
			system_monitoring_interface::global_mon->log_exception(exc);
			return false;
		}
	}

}

#endif
