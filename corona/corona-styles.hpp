#ifndef CORONA_STYLES_H
#define CORONA_STYLES_H

#include "corona-visual.hpp"

#include <string>
#include <vector>
#include <map>

namespace corona {

	class presentation_style
	{
	public:
		std::string PrimaryFont;

		std::vector<std::string> MasterBrandColors;
		std::vector<std::string> MasterBrandTextColors;

		std::vector<std::string> SecondaryBrandColors;
		std::vector<std::string> SecondaryBrandTextColors;

		std::vector<std::string> SupportingBrandColors;
		std::vector<std::string> SupportingBrandTextColors;

		std::map<std::string, std::vector<std::string>> DataColors;
		std::map<std::string, std::vector<std::string>> ChartColors;

		std::string HeaderBackgroundColor;
		std::string HeaderTextColor;

		std::string PageBackgroundColor;
		std::string PageBorderColor;
		std::string PageTextColor;
		std::string PageTextFont;

		std::string TitleBackgroundColor;
		std::string TitleTextColor;
		std::string TitleTextFont;

		std::string SubtitleBackgroundColor;
		std::string SubtitleTextColor;
		std::string SubtitleTextFont;

		std::string ChapterTitleBackgroundColor;
		std::string ChapterTitleTextColor;
		std::string ChapterTextFont;

		std::string SubchapterTitleBackgroundColor;
		std::string SubchapterTitleTextColor;
		std::string SubchapterTextFont;

		std::string ParagraphBackgroundColor;
		std::string ParagraphTextColor;
		std::string ParagraphTextFont;

		std::string CodeBackgroundColor;
		std::string CodeTextColor;
		std::string CodeTextFont;

		std::string BackgroundColor;
		std::string TextColor;

		std::string FormBackgroundColor;

		std::string Section1BackgroundColor;
		std::string Section1TextColor;

		std::string Section2BackgroundColor;
		std::string Section2TextColor;

		std::string Section3BackgroundColor;
		std::string Section3TextColor;

		std::string Section4BackgroundColor;
		std::string Section4TextColor;

		std::string FooterBackgroundColor;
		std::string FooterTextColor;

		visual_alignment PrevailingAlignment;
	};

	class presentation_style_factory
	{
		std::string current_style;
	public:
		std::map<std::string, presentation_style> styles;

		presentation_style_factory();
		presentation_style* get_style();
	};

	presentation_style_factory::presentation_style_factory()
	{
		presentation_style woodruff;

		woodruff.PrimaryFont = "Open Sans,Arial";

		woodruff.MasterBrandColors = { "319B42", "78BE20", "1F2A44", "D0D3D4", "FFFFFF" };
		woodruff.SecondaryBrandColors = { "6399AE", "34657F", "A0D1CA", "6BBBAE", "621244", "C1C6C8", "A2AAAD", "7C878E", "5B6770", "081F2C" };
		woodruff.SupportingBrandColors = { "5CAA7F", "EED484", "B04A5A", "893B67", "7E5475", "C1C6C8", "A2AAAD", "7C878E", "5B6770", "007396" };

		woodruff.MasterBrandTextColors = { "FFFFFF", "000000", "FFFFFF", "000000", "000000" };
		woodruff.SecondaryBrandTextColors = { "000000", "FFFFFF", "000000", "000000", "FFFFFF", "000000", "000000", "000000", "FFFFFF", "FFFFFF" };
		woodruff.SupportingBrandTextColors = { "000000", "000000", "000000", "000000", "000000", "000000", "000000", "000000", "FFFFFF", "000000" };

		woodruff.DataColors = {
			{ "Blue", {"34657F", "D4E1E8", "AACDDA", "6399AE", "34657F", "081F2C" } },
			{ "Gray", { "7C878E", "DEE1E1", "C1C6C8", "A2AAAD", "7C878E", "5B6770" } },
			{ "Green", { "4BA24D", "C8DFBC", "AFD29E", "6DB658", "4BA24D", "008A41" } },
			{ "Plum", { "621244", "D7D3DB", "C1B2C3", "996F8D", "862663", "621244"} },
			{ "Khaki", { "B6B7A2", "DEDDD3", "CBCBBC", "B6B7A2", "888F6E", "67724C" } },
			{ "Khaki", { "B6B7A2", "DEDDD3", "CBCBBC", "B6B7A2", "888F6E", "67724C" } },
		};
		woodruff.ChartColors = {
			{ "Deductible", { "34657F" } },
			{ "Lavender", { "E0D4D9", "CDB9C2", "B898A7" } },
			{ "Perriwinkle", { "C8D9E2", "B0C9D7", "98BBCC" } },
			{ "Grass", { "C9DEBC", "B2D2A2", "A2C490" } },
			{ "Tulips", { "F8E0A4", "ECCF8F", "EAC189" } },
			{ "Earth", { "E1DACE", "D8CEBE", "C4B69D" } },
			{ "Stone", { "DEE1E1", "C1C6C8", "A2AAAD" } }
		};


		woodruff.PageBackgroundColor = "FFFFFF";
		woodruff.PageTextColor = "000000";
		woodruff.PageBorderColor = "#203020";

		woodruff.HeaderBackgroundColor = "1F241F";
		woodruff.HeaderTextColor = woodruff.MasterBrandColors[0];

		woodruff.TitleBackgroundColor = "#00000000";
		woodruff.TitleTextColor = woodruff.MasterBrandColors[0];

		woodruff.SubtitleBackgroundColor = "#00000000";
		woodruff.SubtitleTextColor = woodruff.MasterBrandColors[0];

		woodruff.FormBackgroundColor = "#EEEFEE";

		woodruff.ChapterTitleBackgroundColor = "#COC2CO";
		woodruff.ChapterTitleTextColor = "111111";

		woodruff.SubchapterTitleBackgroundColor = "FFFFFF";
		woodruff.SubchapterTitleTextColor = "111111";

		woodruff.ParagraphBackgroundColor = "#00000000";
		woodruff.ParagraphTextColor = "111111";

		woodruff.CodeBackgroundColor = "00000000";
		woodruff.CodeTextColor = "F8E0A4";

		woodruff.Section1BackgroundColor = "DEE1E1";
		woodruff.Section1TextColor = woodruff.ParagraphTextColor;

		woodruff.Section2BackgroundColor = "D4E1E8";
		woodruff.Section2TextColor = woodruff.ParagraphTextColor;

		woodruff.Section3BackgroundColor = "D7D3DB";
		woodruff.Section3TextColor = woodruff.ParagraphTextColor;

		woodruff.Section4BackgroundColor = "DEDDD3";
		woodruff.Section4TextColor = woodruff.ParagraphTextColor;

		woodruff.FooterBackgroundColor = woodruff.MasterBrandColors[0];
		woodruff.FooterTextColor = woodruff.MasterBrandTextColors[0];

		styles["Woodruff"] = woodruff;
		current_style = "Woodruff";
	}

	presentation_style *presentation_style_factory::get_style()
	{
		return &styles[current_style];
	}

}

#endif
