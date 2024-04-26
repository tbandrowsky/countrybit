#ifndef CORONA_STYLES_H
#define CORONA_STYLES_H

namespace corona 
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

		generalBrushRequest PageBackgroundBrush;
		generalBrushRequest PageBorderBrush;
		generalBrushRequest PageTextBrush;
		textStyleRequest	PageFont;

		generalBrushRequest CaptionBackgroundBrush;
		generalBrushRequest CaptionBorderBrush;
		generalBrushRequest CaptionTextBrush;
		textStyleRequest	CaptionFont;

		generalBrushRequest TitleBackgroundBrush;
		generalBrushRequest TitleBorderBrush;
		generalBrushRequest TitleTextBrush;
		textStyleRequest	TitleFont;

		generalBrushRequest SubtitleBackgroundBrush;
		generalBrushRequest SubtitleBorderBrush;
		generalBrushRequest SubtitleTextBrush;
		textStyleRequest	SubtitleFont;

		generalBrushRequest ChapterTitleBackgroundBrush;
		generalBrushRequest ChapterTitleBorderBrush;
		generalBrushRequest ChapterTitleTextBrush;
		textStyleRequest	ChatperTitleFont;

		generalBrushRequest ChapterSubTitleBackgroundBrush;
		generalBrushRequest ChapterSubTitleBorderBrush;
		generalBrushRequest ChapterSubTitleTextBrush;
		textStyleRequest	ChapterSubTitleFont;

		generalBrushRequest ParagraphBackgroundBrush;
		generalBrushRequest ParagraphBorderBrush;
		generalBrushRequest ParagraphTextBrush;
		textStyleRequest	ParagraphFont;

		generalBrushRequest FormBackgroundBrush;
		generalBrushRequest FormBorderBrush;
		generalBrushRequest FormTextBrush;
		textStyleRequest	FormFont;

		generalBrushRequest LabelBackgroundBrush;
		generalBrushRequest LabelBorderBrush;
		generalBrushRequest LabelTextBrush;
		textStyleRequest	LabelFont;

		generalBrushRequest PlaceholderBackgroundBrush;
		generalBrushRequest PlaceholderBorderBrush;
		generalBrushRequest PlaceholderTextBrush;
		textStyleRequest	PlaceholderFont;

		generalBrushRequest ErrorBackgroundBrush;
		generalBrushRequest ErrorBorderBrush;
		generalBrushRequest ErrorTextBrush;
		textStyleRequest	ErrorFont;

		generalBrushRequest CodeBackgroundBrush;
		generalBrushRequest CodeBorderBrush;
		generalBrushRequest CodeTextBrush;
		textStyleRequest	CodeFont;

		visual_alignment PrevailingAlignment;
	};

	class presentation_style_factory
	{
		std::string current_style;

		void create_countrybit_styles();

	public:
		std::map<std::string, presentation_style> styles;

		presentation_style_factory();
		presentation_style* get_style();
	};

	// 4D6DFF

	presentation_style_factory::presentation_style_factory()
	{
		create_countrybit_styles();
	}

	void presentation_style_factory::create_countrybit_styles()
	{
		presentation_style countrybit;

		std::string bigFont = "Century Gothic,Arial";
		std::string textFont = "Tahoma,Arial";

		countrybit.PrimaryFont = textFont;

		countrybit.MasterBrandColors = { "353535", "ffffff", "d2d7df", "bdbbb0", "8a897c", };
		countrybit.SecondaryBrandColors = { "94d1be", "3b413c", "9db5b2", "daf0ee", "94d1be", "79726e", "7c7977", "808080", "5B6770", "081F2C" };
		countrybit.SupportingBrandColors = {  };

		countrybit.MasterBrandTextColors = { "FFFFFF", "000000", "FFFFFF", "000000", "000000" };
		countrybit.SecondaryBrandTextColors = { "000000", "FFFFFF", "000000", "000000", "FFFFFF", "000000", "000000", "000000", "FFFFFF", "FFFFFF" };
		countrybit.SupportingBrandTextColors = {  };

		countrybit.DataColors = {
			{ "Blue", {"34657F", "D4E1E8", "AACDDA", "6399AE", "34657F", "081F2C" } },
			{ "Gray", { "7C878E", "DEE1E1", "C1C6C8", "A2AAAD", "7C878E", "5B6770" } },
			{ "Green", { "4BA24D", "C8DFBC", "AFD29E", "6DB658", "4BA24D", "008A41" } },
			{ "Plum", { "621244", "D7D3DB", "C1B2C3", "996F8D", "862663", "621244"} },
			{ "Khaki", { "B6B7A2", "DEDDD3", "CBCBBC", "B6B7A2", "888F6E", "67724C" } },
			{ "Khaki", { "B6B7A2", "DEDDD3", "CBCBBC", "B6B7A2", "888F6E", "67724C" } },
		};

		countrybit.ChartColors = {
			{ "Deductible", { "34657F" } },
			{ "Lavender", { "E0D4D9", "CDB9C2", "B898A7" } },
			{ "Perriwinkle", { "C8D9E2", "B0C9D7", "98BBCC" } },
			{ "Grass", { "C9DEBC", "B2D2A2", "A2C490" } },
			{ "Tulips", { "F8E0A4", "ECCF8F", "EAC189" } },
			{ "Earth", { "E1DACE", "D8CEBE", "C4B69D" } },
			{ "Stone", { "DEE1E1", "C1C6C8", "A2AAAD" } }
		};

		textStyleRequest text_style;

		text_style.name = "text_style";
		text_style.fontName = bigFont;
		text_style.fontSize = 72;
		text_style.bold = false;
		text_style.italics = false;
		text_style.underline = false;
		text_style.strike_through = false;
		text_style.horizontal_align = visual_alignment::align_near;
		text_style.vertical_align = visual_alignment::align_near;
		text_style.wrap_text = true;
		text_style.font_stretch = DWRITE_FONT_STRETCH_NORMAL;
		text_style.character_spacing = 0;

		linearGradientBrushRequest lgbr;
		
		lgbr.gradientStops = {
	{ toColor("#FFFFFF"), 0.0 },
	{ toColor("#CFCFCF"), 1.0 }
		};

		countrybit.PageBackgroundBrush = lgbr;
		countrybit.PageBorderBrush.setColor("#c0c0c0");
		countrybit.PageTextBrush.setColor("#000000");		
		countrybit.PageFont = text_style;
		countrybit.PageFont.fontName = "Century Gothic,Arial";
		countrybit.PageFont.fontSize = 24;

		lgbr.gradientStops = {
{ toColor("#000000"), 0.0 },
{ toColor("#010101"), 1.0 }
		};

		countrybit.PlaceholderBackgroundBrush = lgbr;
		countrybit.PlaceholderTextBrush.setColor("#FFFFFF");
		countrybit.PlaceholderFont = text_style;
		countrybit.PlaceholderFont.fontName = "Century Gothic,Arial";
		countrybit.PlaceholderFont.fontSize = 24;

		countrybit.CaptionBackgroundBrush = lgbr;
		countrybit.CaptionTextBrush.setColor("#FFFFFF");
		countrybit.CaptionFont = text_style;
		countrybit.CaptionFont.fontName = "Century Gothic,Arial";
		countrybit.CaptionFont.fontSize = 48;

		countrybit.TitleBackgroundBrush.clear();
		countrybit.TitleBorderBrush.clear();
		countrybit.TitleTextBrush.setColor("#000000");
		countrybit.TitleFont = text_style;
		countrybit.TitleFont.fontName = "Century Gothic,Arial";
		countrybit.TitleFont.fontSize = 32;

		countrybit.SubtitleBackgroundBrush.clear();
		countrybit.SubtitleBorderBrush.clear();
		countrybit.SubtitleTextBrush.setColor("#000000");
		countrybit.SubtitleFont = text_style;
		countrybit.SubtitleFont.fontName = "Century Gothic,Arial";
		countrybit.SubtitleFont.fontSize = 28;

		countrybit.ChapterTitleBackgroundBrush.clear();
		countrybit.ChapterTitleBorderBrush.clear();
		countrybit.ChapterTitleTextBrush.setColor("#000000");
		countrybit.ChatperTitleFont = text_style;
		countrybit.ChatperTitleFont.fontName = "Century Gothic,Arial";
		countrybit.ChatperTitleFont.fontSize = 20;
		
		countrybit.ChapterSubTitleBackgroundBrush.clear();
		countrybit.ChapterSubTitleBorderBrush.clear();
		countrybit.ChapterSubTitleTextBrush.setColor("#000000");
		countrybit.ChapterSubTitleFont = text_style;
		countrybit.ChapterSubTitleFont.fontName = "Century Gothic,Arial";
		countrybit.ChapterSubTitleFont.fontSize = 18;
		countrybit.ChapterSubTitleFont.underline = 1;

		countrybit.CodeBackgroundBrush.setColor("#111111");
		countrybit.CodeBorderBrush.setColor("#000000");
		countrybit.CodeTextBrush.setColor("#33FF33");
		countrybit.CodeFont = text_style;
		countrybit.CodeFont.fontName = "Consolas,Arial";
		countrybit.CodeFont.fontSize = 14;
		countrybit.CodeFont.underline = 0;

		countrybit.ParagraphBackgroundBrush.clear();
		countrybit.ParagraphBorderBrush.clear();
		countrybit.ParagraphTextBrush.setColor("#000000");
		countrybit.ParagraphFont = text_style;
		countrybit.ParagraphFont.fontName = "Century Gothic, Arial";
		countrybit.ParagraphFont.fontSize = 14;
		countrybit.ParagraphFont.underline = 0;

		styles["Countrybit"] = countrybit;
		current_style = "Countrybit";
	}

	presentation_style *presentation_style_factory::get_style()
	{
		return &styles[current_style];
	}

}

#endif
