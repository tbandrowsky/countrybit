/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

MIT LICENSE

About this File

Notes

For Future Consideration
*/


#ifndef CORONA_UTILITY_H
#define CORONA_UTILITY_H

namespace corona {

	template<typename Out>
	void split(const std::string& s, char delim, Out result) {
		std::stringstream ss;
		ss.str(s);
		std::string item;
		while (std::getline(ss, item, delim)) {
			*(result++) = item;
		}
	}

	std::string join(std::vector<std::string> items, std::string delim);
	std::vector<std::string> split(const std::string& s, char delim);

	std::vector<std::string_view> split(const std::string_view& s, char delim)
	{
		std::vector<std::string_view> elems;
		int start = 0;
		int count = 0;
		for (int i = 0; i < s.size(); i++) 
		{
			if (s[i] == delim) {
				std::string_view temp = s.substr(start, count);
				elems.push_back(temp);
				start = i + 1;
				count = 0;
			}
			else 
			{
				count++;
			}
		}
		if (count) {
			std::string_view temp = s.substr(start, count);
			elems.push_back(temp);
		}

		return elems;
	}


	std::vector<std::string> split(const std::string& s, char delim)
	{
		std::vector<std::string> elems;
		split(s, delim, std::back_inserter(elems));
		return elems;
	}

	std::string join(std::vector<std::string> items, std::string delim)
	{
		std::stringstream ss;
		std::string sep = "";
		for (auto tv : items) {
			ss << tv;
			ss << sep;
			sep = delim;
		}
		return ss.str();
	}

	bool is_number(const std::string& s) {
		for (char c : s) {
			if (not std::isdigit(c)) {
				return false;
			}
		}
		return !s.empty();
	}

	std::string trim(std::string str, size_t width)
	{
		if (width < 3)
			width = 3;
		if (str.length() > width) {
			return str.substr(0, width - 3) + "...";
		}
		return str;
	}
}

#endif

