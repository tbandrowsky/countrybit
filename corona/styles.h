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
            std::vector<std::string> SecondaryBrandColors;
            std::vector<std::string> SupportingBrandColors;
            std::map<std::string, std::vector<std::string>> DataColors;
            std::map<std::string, std::vector<std::string>> ChartColors;
        };

        class presentation_style_factory
        {
        public:
            std::map<std::string, presentation_style> styles;

            presentation_style_factory();
        };
    }
}
