
#ifdef WINDESKTOP_GUI

namespace corona
{
	namespace win32
	{

		template<typename Out>
		void split(const std::string& s, char delim, Out result) {
			std::stringstream ss;
			ss.str(s);
			std::string item;
			while (std::getline(ss, item, delim)) {
				*(result++) = item;
			}
		}

		std::vector<std::string> split(const std::string& s, char delim);

	}
}

#endif

