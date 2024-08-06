/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

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

}

#endif

