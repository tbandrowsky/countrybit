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

		std::shared_ptr<generalBrushRequest> PageBackgroundBrush;
		std::shared_ptr<generalBrushRequest> PageBorderBrush;
		std::shared_ptr<generalBrushRequest> PageTextBrush;
		std::shared_ptr<textStyleRequest> PageFont;
		int	PageBorderWidth;

		std::shared_ptr<generalBrushRequest> CaptionBackgroundBrush;
		std::shared_ptr<generalBrushRequest> CaptionBorderBrush;
		std::shared_ptr<generalBrushRequest> CaptionTextBrush;
		std::shared_ptr<textStyleRequest> CaptionFont;
		int	CaptionBorderWidth;

		std::shared_ptr<generalBrushRequest> TitleBackgroundBrush;
		std::shared_ptr<generalBrushRequest> TitleBorderBrush;
		std::shared_ptr<generalBrushRequest> TitleTextBrush;
		std::shared_ptr<textStyleRequest> TitleFont;
		int	TitleBorderWidth;

		std::shared_ptr<generalBrushRequest> SubtitleBackgroundBrush;
		std::shared_ptr<generalBrushRequest> SubtitleBorderBrush;
		std::shared_ptr<generalBrushRequest> SubtitleTextBrush;
		std::shared_ptr<textStyleRequest> SubtitleFont;
		int	SubtitleBorderWidth;

		std::shared_ptr<generalBrushRequest> ChapterTitleBackgroundBrush;
		std::shared_ptr<generalBrushRequest> ChapterTitleBorderBrush;
		std::shared_ptr<generalBrushRequest> ChapterTitleTextBrush;
		std::shared_ptr<textStyleRequest>	ChatperTitleFont;
		int	ChatperTitleBorderWidth;

		std::shared_ptr<generalBrushRequest> ChapterSubTitleBackgroundBrush;
		std::shared_ptr<generalBrushRequest> ChapterSubTitleBorderBrush;
		std::shared_ptr<generalBrushRequest> ChapterSubTitleTextBrush;
		std::shared_ptr<textStyleRequest>	ChapterSubTitleFont;
		int	ChapterSubTitleBorderWidth;

		std::shared_ptr<generalBrushRequest> AuthorsCreditBackgroundBrush;
		std::shared_ptr<generalBrushRequest> AuthorsCreditBorderBrush;
		std::shared_ptr<generalBrushRequest> AuthorsCreditTextBrush;
		std::shared_ptr<textStyleRequest>	AuthorsCreditFont;
		int	AuthorsCreditBorderWidth;

		std::shared_ptr<generalBrushRequest> ParagraphBackgroundBrush;
		std::shared_ptr<generalBrushRequest> ParagraphBorderBrush;
		std::shared_ptr<generalBrushRequest> ParagraphTextBrush;
		std::shared_ptr<textStyleRequest>	ParagraphFont;
		int	ParagraphBorderWidth;

		std::shared_ptr<generalBrushRequest> FormBackgroundBrush;
		std::shared_ptr<generalBrushRequest> FormBorderBrush;
		std::shared_ptr<generalBrushRequest> FormTextBrush;
		std::shared_ptr<textStyleRequest>	FormFont;
		int	FormBorderWidth;

		std::shared_ptr<generalBrushRequest> LabelBackgroundBrush;
		std::shared_ptr<generalBrushRequest> LabelBorderBrush;
		std::shared_ptr<generalBrushRequest> LabelTextBrush;
		std::shared_ptr<textStyleRequest>	LabelFont;
		int	LabelBorderWidth;

		std::shared_ptr<generalBrushRequest> PlaceholderBackgroundBrush;
		std::shared_ptr<generalBrushRequest> PlaceholderBorderBrush;
		std::shared_ptr<generalBrushRequest> PlaceholderTextBrush;
		std::shared_ptr<textStyleRequest>	PlaceholderFont;
		int	PlaceholderBorderWidth;

		std::shared_ptr<generalBrushRequest> ErrorBackgroundBrush;
		std::shared_ptr<generalBrushRequest> ErrorBorderBrush;
		std::shared_ptr<generalBrushRequest> ErrorTextBrush;
		std::shared_ptr<textStyleRequest>	ErrorFont;
		int	ErrorBorderWidth;

		std::shared_ptr<generalBrushRequest> SuccessBackgroundBrush;
		std::shared_ptr<generalBrushRequest> SuccessBorderBrush;
		std::shared_ptr<generalBrushRequest> SuccessTextBrush;
		std::shared_ptr<textStyleRequest>	SuccessFont;
		int	SuccessBorderWidth;

		std::shared_ptr<generalBrushRequest> CodeBackgroundBrush;
		std::shared_ptr<generalBrushRequest> CodeBorderBrush;
		std::shared_ptr<generalBrushRequest> CodeTextBrush;
		std::shared_ptr<textStyleRequest>	CodeFont;
		int	CodeBorderWidth;

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

		std::string bigFont = "TW Cent MT, Century Gothic, Arial";
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
		text_style.line_spacing = 0;
		text_style.horizontal_align = visual_alignment::align_near;
		text_style.vertical_align = visual_alignment::align_near;
		text_style.wrap_text = true;
		text_style.font_stretch = DWRITE_FONT_STRETCH_NORMAL;
		text_style.character_spacing = 0;

		linearGradientBrushRequest lgbr;

		lgbr.start.x = .5;
		lgbr.start.y = 0;
		lgbr.stop.x = .5;
		lgbr.stop.y = 1;

		lgbr.gradientStops = {
	{ toColor("#100030"), 0.0 },
	{ toColor("#000010"), 1.0 }
		};

		countrybit.PageBackgroundBrush = std::make_shared<generalBrushRequest>(lgbr);
		countrybit.PageBackgroundBrush->set_name("PageBackground");

		countrybit.PageBorderBrush = std::make_shared<generalBrushRequest>();
		countrybit.PageBorderWidth = 0;

		countrybit.PageTextBrush = std::make_shared<generalBrushRequest>();

		countrybit.PageBorderBrush->set_name("PageBorder");
		countrybit.PageBorderBrush->setColor("#C0C0C0");
		countrybit.PageTextBrush->set_name("PageText");
		countrybit.PageTextBrush->setColor("#000000");
		countrybit.PageFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.PageFont->fontName = textFont;
		countrybit.PageFont->fontSize = 14;
		countrybit.PageFont->name = "PageFont";

		countrybit.PlaceholderBackgroundBrush = std::make_shared<generalBrushRequest>();
		countrybit.PlaceholderBorderBrush = std::make_shared<generalBrushRequest>();
		countrybit.PlaceholderBorderWidth = 0;
		countrybit.PlaceholderTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.PlaceholderTextBrush->setColor("#000000");
		countrybit.PlaceholderFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.PlaceholderFont->fontName = textFont;
		countrybit.PlaceholderFont->fontSize = 24;
		countrybit.PlaceholderBackgroundBrush->set_name("PlaceholderBackground");
		countrybit.PlaceholderBorderBrush->set_name("PlaceholderBorder");
		countrybit.PlaceholderTextBrush->set_name("PlaceholderText");
		countrybit.PlaceholderFont->name = "PlaceholderFont";

		lgbr.gradientStops = {
{ toColor("#600000"), 0.0 },
{ toColor("#600000"), 0.13 },
{ toColor("#200000"), 1.0 }
		};

		countrybit.ErrorBackgroundBrush = std::make_shared<generalBrushRequest>(lgbr);
		countrybit.ErrorBackgroundBrush->set_name("ErrorBackground");

		lgbr.gradientStops = {
{ toColor("#600000"), 0.0 },
{ toColor("#600000"), 0.13 },
{ toColor("#100000"), 1.0 }
		};

		countrybit.ErrorBorderBrush = std::make_shared<generalBrushRequest>(lgbr);
		countrybit.ErrorBorderBrush->set_name("ErrorBorder");
		countrybit.ErrorBorderWidth = 8;

		countrybit.ErrorTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.ErrorTextBrush->setColor("#FFFFFF");
		countrybit.ErrorTextBrush->set_name("ErrorText");

		countrybit.ErrorFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.ErrorFont->fontName = "Arial";
		countrybit.ErrorFont->fontSize = 14;
		countrybit.ErrorFont->vertical_align = visual_alignment::align_center;
		countrybit.ErrorFont->horizontal_align = visual_alignment::align_near;
		countrybit.ErrorFont->name = "ErrorFont";

		lgbr.gradientStops = {
{ toColor("#000040"), 0.0 },
{ toColor("#000040"), 0.85 },
{ toColor("#000020"), 1.0 }
		};

		countrybit.SuccessBackgroundBrush = std::make_shared<generalBrushRequest>(lgbr);
		countrybit.SuccessBackgroundBrush->set_name("SuccessBackground");

		lgbr.gradientStops = {
{ toColor("#000010"), 0.0 },
{ toColor("#000010"), 0.13 },
{ toColor("#000020"), 1.0 }
		};

		countrybit.SuccessBorderBrush = std::make_shared<generalBrushRequest>(lgbr);
		countrybit.SuccessBorderBrush->set_name("SuccessBorder");
		countrybit.SuccessBorderWidth = 4;
		countrybit.SuccessTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.SuccessTextBrush->setColor("#000000");
		countrybit.SuccessTextBrush->set_name("SuccessText");
		countrybit.SuccessFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.SuccessFont->fontName = textFont;
		countrybit.SuccessFont->fontSize = 16;
		countrybit.SuccessFont->name = "SuccessFont";

		lgbr.start.x = .5;
		lgbr.start.y = 0;
		lgbr.stop.x = .5;
		lgbr.stop.y = 1;
		lgbr.gradientStops = {
	{ toColor("#313150"), 0.0 },
	{ toColor("#313150"), 0.13 },
	{ toColor("#101020"), 1.0 },
		};

		countrybit.CaptionBackgroundBrush = std::make_shared<generalBrushRequest>(lgbr);
		countrybit.CaptionBackgroundBrush->set_name("CaptionBackground");

		countrybit.CaptionBorderBrush = std::make_shared<generalBrushRequest>();
		countrybit.CaptionBorderBrush->set_name("CaptionBorder");
		countrybit.CaptionBorderWidth = 0;

		countrybit.CaptionTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.CaptionTextBrush->setColor("#FFFFFF");
		countrybit.CaptionTextBrush->set_name("CaptionText");
		countrybit.CaptionFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.CaptionFont->fontName = bigFont;
		countrybit.CaptionFont->fontSize = 24;
		countrybit.CaptionFont->name = "CaptionFont";


		countrybit.TitleBackgroundBrush = std::make_shared<generalBrushRequest>();
//		countrybit.TitleBackgroundBrush->set_name("TitleBackground");
	//	countrybit.TitleBackgroundBrush->setColor("#00000000");
		countrybit.TitleBorderBrush = std::make_shared<generalBrushRequest>();
		countrybit.TitleBorderBrush->setColor("#FFFFFF");
		countrybit.TitleBorderBrush->set_name("TitleBorder");
		countrybit.TitleBorderWidth = 0;
		countrybit.TitleTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.TitleTextBrush->setColor("#FFFFFF");
		countrybit.TitleTextBrush->set_name("TitleText");
		countrybit.TitleFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.TitleFont->fontName = bigFont;
		countrybit.TitleFont->fontSize = 48;
		countrybit.TitleFont->vertical_align = visual_alignment::align_center;
		countrybit.TitleFont->name = "TitleFont";


		countrybit.SubtitleBackgroundBrush = std::make_shared<generalBrushRequest>();
		countrybit.SubtitleBackgroundBrush->set_name("SubtitleBackground");
		countrybit.SubtitleBorderBrush = std::make_shared<generalBrushRequest>();
		countrybit.SubtitleBorderBrush->setColor("#FFFFFF");
		countrybit.SubtitleBorderBrush->set_name("SubtitleBorder");
		countrybit.SubtitleBorderWidth = 4;
		countrybit.SubtitleTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.SubtitleTextBrush->setColor("#FFFFFF");
		countrybit.SubtitleTextBrush->set_name("SubtitleText");
		countrybit.SubtitleFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.SubtitleFont->fontName = bigFont;
		countrybit.SubtitleFont->fontSize = 24;
		countrybit.SubtitleFont->name = "SubtitleFont";

		countrybit.AuthorsCreditBackgroundBrush = std::make_shared<generalBrushRequest>();
		countrybit.AuthorsCreditBackgroundBrush->set_name("AuthorsCreditBackground");
		countrybit.AuthorsCreditBorderBrush = std::make_shared<generalBrushRequest>();
		countrybit.AuthorsCreditBorderBrush->setColor("#CCCCDD");
		countrybit.AuthorsCreditBorderBrush->set_name("AuthorsCreditBorder");
		countrybit.AuthorsCreditBorderWidth = 0;
		countrybit.AuthorsCreditTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.AuthorsCreditTextBrush->setColor("#CCCCDD");
		countrybit.AuthorsCreditTextBrush->set_name("AuthorsCreditText");
		countrybit.AuthorsCreditFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.AuthorsCreditFont->fontName = "Old Newspaper font,Georgia,Courier New";
		countrybit.AuthorsCreditFont->fontSize = 16;
		countrybit.AuthorsCreditFont->bold = false;
		countrybit.AuthorsCreditFont->horizontal_align = visual_alignment::align_center;
		countrybit.AuthorsCreditFont->name = "AuthorsCreditFont";


		countrybit.ChapterTitleBackgroundBrush = std::make_shared<generalBrushRequest>();
		countrybit.ChapterTitleBorderBrush = std::make_shared<generalBrushRequest>();
		countrybit.ChapterTitleTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.ChapterTitleTextBrush->setColor("#000000");
		countrybit.ChatperTitleBorderWidth = 4;
		countrybit.ChatperTitleFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.ChatperTitleFont->fontName = textFont;
		countrybit.ChatperTitleFont->fontSize = 24;
		countrybit.ChatperTitleFont->bold = true;
		countrybit.ChapterTitleBackgroundBrush->set_name("ChapterTitleBackground");
		countrybit.ChapterTitleBorderBrush->set_name("ChapterTitleBorder");
		countrybit.ChapterTitleTextBrush->set_name("ChapterTitleText");
		countrybit.ChatperTitleFont->name = "ChapterTitleFont";

		
		countrybit.ChapterSubTitleBackgroundBrush = std::make_shared<generalBrushRequest>();
		countrybit.ChapterSubTitleBorderBrush = std::make_shared<generalBrushRequest>();
		countrybit.ChapterSubTitleBorderWidth = 0;
		countrybit.ChapterSubTitleTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.ChapterSubTitleTextBrush->setColor("#000000");
		countrybit.ChapterSubTitleFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.ChapterSubTitleFont->fontName = textFont;
		countrybit.ChapterSubTitleFont->fontSize = 14;
		countrybit.ChapterSubTitleFont->underline = 1;
		countrybit.ChapterSubTitleBackgroundBrush->set_name("ChapterSubTitleBackground");
		countrybit.ChapterSubTitleBorderBrush->set_name("ChapterSubTitleBorder");
		countrybit.ChapterSubTitleTextBrush->set_name("ChapterSubTitleText");
		countrybit.ChapterSubTitleFont->name = "ChapterSubTitleFont";


		countrybit.CodeBackgroundBrush = std::make_shared<generalBrushRequest>();
		countrybit.CodeBorderBrush = std::make_shared<generalBrushRequest>();
		countrybit.CodeTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.CodeBorderWidth = 0;
		countrybit.CodeBackgroundBrush->setColor("#111111");
		countrybit.CodeBorderBrush->setColor("#000000");
		countrybit.CodeTextBrush->setColor("#33FF33");
		countrybit.CodeFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.CodeFont->fontName = "Consolas,Arial";
		countrybit.CodeFont->fontSize = 14;
		countrybit.CodeFont->underline = 0;
		countrybit.CodeBackgroundBrush->set_name("CodeBackground");
		countrybit.CodeBorderBrush->set_name("CodeBorder");
		countrybit.CodeTextBrush->set_name("CodeText");
		countrybit.CodeFont->name = "CodeFont";


		countrybit.ParagraphBackgroundBrush = std::make_shared<generalBrushRequest>();
		countrybit.ParagraphBorderBrush = std::make_shared<generalBrushRequest>();
		countrybit.ParagraphBorderWidth = 0;
		countrybit.ParagraphTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.ParagraphTextBrush->setColor("#000000");
		countrybit.ParagraphFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.ParagraphFont->fontName = textFont;
		countrybit.ParagraphFont->fontSize = 14;
		countrybit.ParagraphFont->underline = 0;
		countrybit.ParagraphBackgroundBrush->set_name("ParagraphBackground");
		countrybit.ParagraphBorderBrush->set_name("ParagraphBorder");
		countrybit.ParagraphTextBrush->set_name("ParagraphText");
		countrybit.ParagraphFont->name = "ParagraphFont";

		countrybit.LabelBackgroundBrush = std::make_shared<generalBrushRequest>();
		countrybit.LabelBorderBrush = std::make_shared<generalBrushRequest>();
		countrybit.LabelBorderWidth = 0;
		countrybit.LabelTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.LabelTextBrush->setColor("#000000");
		countrybit.LabelFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.LabelFont->fontName = textFont;
		countrybit.LabelFont->fontSize = 14;
		countrybit.LabelFont->underline = 0;
		countrybit.LabelFont->vertical_align = visual_alignment::align_far;
		countrybit.LabelBackgroundBrush->set_name("LabelBackground");
		countrybit.LabelBorderBrush->set_name("LabelBorder");
		countrybit.LabelTextBrush->set_name("LabelText");
		countrybit.LabelFont->name = "LabelFont";

		countrybit.FormBackgroundBrush = std::make_shared<generalBrushRequest>();
		countrybit.FormBorderBrush = std::make_shared<generalBrushRequest>();
		countrybit.FormBorderWidth = 0;
		countrybit.FormTextBrush = std::make_shared<generalBrushRequest>();
		countrybit.FormTextBrush->setColor("#000000");
		countrybit.FormFont = std::make_shared<textStyleRequest>(text_style);
		countrybit.FormFont->fontName = textFont;
		countrybit.FormFont->fontSize = 14;
		countrybit.FormFont->underline = 0;
		countrybit.FormBackgroundBrush->set_name("FormBackground");
		countrybit.FormBorderBrush->set_name("FormBorder");
		countrybit.FormTextBrush->set_name("FormText");
		countrybit.FormFont->name = "FormFont";

		styles["Countrybit"] = countrybit;
		current_style = "Countrybit";
	}

	presentation_style *presentation_style_factory::get_style()
	{
		return &styles[current_style];
	}

}

#endif
