
/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File
This is the application root class.  It can be used for both 
console and desktop applications.  It manages a threadpool
that is used pervasively by Corona.  It also provides the entry
to open and work with files asynchronously.

This is a singleton.  

Notes

For Future Consideration
This is a singleton.  having a public constructor is wrong.
*/

#ifndef CORONA_APPLICATION_H
#define CORONA_APPLICATION_H

namespace corona
{

	class application
	{
	public:

		std::string application_name;
		std::string application_folder_name;

		application()
		{
			global_job_queue = std::make_unique<job_queue>();
			global_job_queue->start(0);
		}

		~application()
		{
			global_job_queue->shutDown();
		}

		std::string get_config_filename(std::string _filename)
		{
			char buff[MAX_PATH * 2];
			memset(buff, 0, sizeof(buff) / sizeof(char));
			::GetModuleFileName(nullptr, buff, sizeof(buff) / sizeof(char));
			::PathRemoveFileSpec(buff);
			std::string temp = buff;
			temp += "\\";
			temp += _filename;
			return temp;
		}

		std::string get_data_filename(std::string _filename)
		{
			char path_name[ MAX_PATH * 4 ];
			wchar_t* wide_path = nullptr;
			std::string result;
			::SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_DEFAULT, NULL, &wide_path);
			if (wide_path)
			{
				istring<2048> temp = wide_path;
				PathAppend(temp.c_str_w(), application_folder_name.c_str());
				if (CreateDirectory(temp.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
					PathAppend(temp.c_str_w(), _filename.c_str());
					result = temp;
				}
				else {
					// Failed to create directory
				}
			}
			return result;
		}

		void wait()
		{
			global_job_queue->waitForEmptyQueue();
		}

		std::string get_user_name()
		{
			std::string result;
			char buffer[UNLEN + 1] = {};
			DWORD max_length = sizeof(buffer) / sizeof(char);
			if (GetUserNameEx(NameUserPrincipal, buffer, &max_length)) {
				result = buffer;
			}
			return result;
		}

		file open_file(file_path filename, file_open_types _file_open_type)
		{
			file f(global_job_queue.get(), filename, _file_open_type);
			return f;
		}

		bool file_exists(std::string filename)
		{
			DWORD       fileAttr;

			fileAttr = GetFileAttributes(filename.c_str());
			if (0xFFFFFFFF == fileAttr && GetLastError() == ERROR_FILE_NOT_FOUND)
				return false;
			return true;
		}

		file open_file(KNOWNFOLDERID folderId, file_path filename, file_open_types _file_open_type)
		{
			file f(global_job_queue.get(), folderId, filename, _file_open_type);
			return f;
		}

		file create_file(KNOWNFOLDERID folderId, file_path filename)
		{
			return file(global_job_queue.get(), folderId, filename, file_open_types::create_always);
		}

		file create_file(file_path filename)
		{
			return file(global_job_queue.get(), filename, file_open_types::create_always);
		}

		std::shared_ptr<file> open_file_ptr(KNOWNFOLDERID folderId, file_path filename, file_open_types _file_open_type)
		{
			std::shared_ptr<file> f = std::make_shared<file>(global_job_queue.get(), folderId, filename, _file_open_type);
			return f;
		}

		std::shared_ptr<file> open_file_ptr(file_path filename, file_open_types _file_open_type)
		{
			std::shared_ptr<file> f = std::make_shared<file>(global_job_queue.get(), filename, _file_open_type);
			return f;
		}

		std::shared_ptr<file> create_file_ptr(KNOWNFOLDERID folderId, file_path filename)
		{
			std::shared_ptr<file> f = std::make_shared<file>(global_job_queue.get(), folderId, filename, file_open_types::create_always);
			return f;
		}

		std::shared_ptr<file> create_file_ptr(file_path filename)
		{
			std::shared_ptr<file> f = std::make_shared<file>(global_job_queue.get(), filename, file_open_types::create_always);
			return f;
		}

		void add_job(job* _job)
		{
			global_job_queue->add_job(_job);
		}

		virtual void log(const char* _msg)
		{
			std::cout << _msg << std::endl;
		}

	};

	file_batch application_tests()
	{
		try {
			application app;

			auto my_file = app.create_file("file.txt");

			my_file.add(32);

			char buffer[10] = { 0 };

			strcpy_s(buffer, "test1");
			co_await my_file.write( 0, buffer, 5);
			strcpy_s(buffer, "test2");
			co_await my_file.write( 5, buffer, 5);
			co_await my_file.read( 0, buffer, 5);

			if (strcmp(buffer, "test1") != 0)
			{
				std::cout << "read / write test failed" << std::endl;
			}

			co_return true;
		}
		catch (std::exception exc)
		{
			std::cout << __LINE__ << " " << exc.what() << std::endl;
			co_return false;
		}
	}
}

#endif
