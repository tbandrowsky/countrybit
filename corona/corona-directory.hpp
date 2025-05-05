/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT License

About this File
This file is for monitoring for directory changes.

Notes
Needs unit test script

For Future Consideration
*/


#ifndef CORONA_DIRECTORY_H
#define CORONA_DIRECTORY_H

namespace corona
{

	class directory_checker
	{
	public:

		std::map<std::string, std::filesystem::file_time_type> entries;
		std::filesystem::path path;

		directory_checker() = default;
		~directory_checker() = default;

		struct check_options
		{
			std::map<std::string, bool> files_to_ignore;
		};

		bool check_changes( const check_options& _options )
		{
			bool result = false;
			for (const auto& entry : std::filesystem::directory_iterator(path)) {
				auto temp = entry.path().string();
				if (_options.files_to_ignore.contains(temp))
					continue;
				auto foundi = entries.find(temp);
				if (foundi != std::end(entries))
				{
					bool matches = foundi->second != entry.last_write_time();
					if (matches)
					{
						system_monitoring_interface::global_mon->log_information(std::format("file changed {0}", temp));
						result = true;
					}
				}
				else
				{
					result = true;
				}
				entries.insert_or_assign(temp, entry.last_write_time());
			}
			return result;
		}


	};
}

#endif
 