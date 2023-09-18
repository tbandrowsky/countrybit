#pragma once

namespace corona
{
    namespace win32
    {

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
            presentation_style& get_style();
        };
    }
}
