
#ifndef CORONA_TEST_HPP
#define CORONA_TEST_HPP

namespace corona
{
	class test_result
	{
	public:
		std::string				 test_name;
		bool					 success;
		std::string				 file;
		int						 line;

		test_result() = default;
		test_result(const test_result&) = default;
		test_result(test_result&&) = default;
		test_result& operator=(test_result&&) = default;
		test_result& operator=(const test_result&) = default;

		test_result(std::string _test_name, bool _success, std::string _file, int _line)
		{
			test_name = _test_name;
			success = _success;
			file = _file;
			line = _line;
		}
	};

	class test_set
	{
	protected:

		std::string									test_set_name;
		std::vector<std::shared_ptr<test_set>>		dependencies;
		std::vector<std::shared_ptr<test_result>>	tests;

	public:

		test_set( std::string _name, std::vector<std::shared_ptr<test_set>>&& _dependencies )
		{
			test_set_name = _name;
			dependencies = std::move(_dependencies);
		}

		virtual void test(test_result _test)
		{
			std::shared_ptr<test_result> tr = std::make_shared<test_result>(_test);
			tests.push_back(tr);
		}

		virtual bool prove( int _indent )
		{
			std::string indent(_indent, ' ');
			bool is_true = true;

			for (auto test : tests)
			{
				if (not test->success) 
				{
					std::string test_result = std::format("{0}.{1} failed", test_set_name, test->test_name);
					system_monitoring_interface::global_mon->log_warning(test_result, test->file.c_str(), test->line);
					is_true = false;
				}
			}

			for (auto set : dependencies) 
			{
				if (not set->prove(_indent + 2)) 
				{
					is_true = false;
				}
			}

			return is_true;
		}
	};

	class test_master
	{
	protected:
		std::map<std::string, std::shared_ptr<test_set>> test_sets;

	public:
		std::shared_ptr<test_set> create_test_set(std::string set_name, std::vector<std::string> _dependencies)
		{
			std::vector<std::shared_ptr<test_set>> dependencies;
			for (auto name : _dependencies) 
			{
				auto iter = test_sets.find(name);
				if (iter == std::end(test_sets)) {
					std::string error = "Missing test set '" + name + "'";
					throw std::logic_error(error);
				}
				dependencies.push_back(iter->second);
			}
			std::shared_ptr<test_set> new_set = std::make_shared<test_set>(set_name, std::move(dependencies));
		}

		bool prove(std::string test_name)
		{
			return test_sets[test_name]->prove(0);
		}
	};
}

#endif
