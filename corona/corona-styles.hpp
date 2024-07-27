/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved

About this File

Notes

For Future Consideration
*/


#ifndef CORONA_STYLES_H
#define CORONA_STYLES_H

namespace corona 
{

	void get_json(json& _dest, std::map<std::string, std::vector<std::string>>& _src)
	{
		json_parser jp;
		for (auto& pair : _src)
		{
			json color_array = jp.create_array();
			for (auto& color : pair.second)
			{
				color_array.append_element(color);
			}
			_dest.put_member(pair.first, color_array);
		}
	}

	void put_json(std::map<std::string, std::vector<std::string>>& _dest, json& _src)
	{
		json_parser jp;

		auto members = _src.get_members();

		for (auto m : members)
		{
			if (!_dest.contains(m.first)) {
				std::vector<std::string> tcolor_array;
				_dest.insert_or_assign(m.first, tcolor_array);
			}

			std::vector<std::string>& color_array = _dest[m.first];
			
			json colors = m.second;
			if (colors.array()) {
				for (int i = 0; i < colors.size(); i++)
				{
					std::string colori = colors.get_element(i);
					color_array.push_back(colori);
				}
			}
		}
	}

	class presentation_style
	{
	public:
		std::string PrimaryFont;
		visual_alignment PrimaryAlignment;

		std::map<std::string, std::vector<std::string>> Colors;

		std::shared_ptr<viewStyleRequest> PageStyle;
		std::shared_ptr<viewStyleRequest> CaptionStyle;
		std::shared_ptr<viewStyleRequest> TitleStyle;
		std::shared_ptr<viewStyleRequest> SubtitleStyle;
		std::shared_ptr<viewStyleRequest> ChapterTitleStyle;
		std::shared_ptr<viewStyleRequest> ChapterSubTitleStyle;
		std::shared_ptr<viewStyleRequest> AuthorsCreditStyle;
		std::shared_ptr<viewStyleRequest> ParagraphStyle;
		std::shared_ptr<viewStyleRequest> FormStyle;
		std::shared_ptr<viewStyleRequest> LabelStyle;
		std::shared_ptr<viewStyleRequest> PlaceholderStyle;
		std::shared_ptr<viewStyleRequest> ErrorStyle;
		std::shared_ptr<viewStyleRequest> SuccessStyle;
		std::shared_ptr<viewStyleRequest> CodeStyle;
	};

	void get_json(json& _dest, presentation_style& _src)
	{
		json_parser jp;

		json colors = jp.create_object();
		json page_style = jp.create_object();
		json caption_style = jp.create_object();
		json title_style = jp.create_object();
		json subtitle_style = jp.create_object();
		json chapter_title_style = jp.create_object();
		json chapter_subtitle_style = jp.create_object();
		json paragraph_style = jp.create_object();
		json form_style = jp.create_object();
		json label_style = jp.create_object();
		json placeholder_style = jp.create_object();
		json error_style = jp.create_object();
		json success_style = jp.create_object();
		json code_style = jp.create_object();

		_dest.put_member("primary_font", _src.PrimaryFont);
		get_json(_dest, "primary_alignment", _src.PrimaryAlignment);

		get_json(colors, _src.Colors);
		_dest.put_member("colors", colors);

		if (_src.PageStyle) {
			get_json(page_style, *_src.PageStyle.get());
			_dest.put_member("page_style", page_style);			
		}
		if (_src.CaptionStyle) {
			get_json(caption_style, *_src.CaptionStyle.get());
			_dest.put_member("caption_style", caption_style);
		}
		if (_src.TitleStyle) {
			get_json(title_style, *_src.TitleStyle.get());
			_dest.put_member("title_style", title_style);
		}
		if (_src.SubtitleStyle) {
			get_json(subtitle_style, *_src.SubtitleStyle.get());
			_dest.put_member("subtitle_style", subtitle_style);
		}
		if (_src.ChapterTitleStyle) {
			get_json(chapter_title_style, *_src.ChapterTitleStyle.get());
			_dest.put_member("chapter_title_style", chapter_title_style);
		}
		if (_src.ChapterSubTitleStyle) {
			get_json(chapter_subtitle_style, *_src.ChapterSubTitleStyle.get());
			_dest.put_member("chapter_subtitle_style", chapter_subtitle_style);
		}
		if (_src.ParagraphStyle) {
			get_json(paragraph_style, *_src.ParagraphStyle.get());
			_dest.put_member("paragraph_style", paragraph_style);
		}
		if (_src.FormStyle) {
			get_json(form_style, *_src.FormStyle.get());
			_dest.put_member("form_style", form_style);
		}
		if (_src.PlaceholderStyle) {
			get_json(placeholder_style, *_src.PlaceholderStyle.get());
			_dest.put_member("placeholder_style", placeholder_style);
		}
		if (_src.ErrorStyle) {
			get_json(error_style, *_src.ErrorStyle.get());
			_dest.put_member("error_style", error_style);
		}
		if (_src.SuccessStyle) {
			get_json(success_style, *_src.SuccessStyle.get());
			_dest.put_member("success_style", success_style);
		}
		if (_src.CodeStyle) {
			get_json(code_style, *_src.CodeStyle.get());
			_dest.put_member("code_style", code_style);
		}
	}

	void put_json(presentation_style& _dest, json& _src)
	{
		std::vector<std::string> missing;
		if (!_src.has_members(missing, { "colors", "page_style", "caption_style",
			"title_style", "subtitle_style", 
			"chapter_title_style", "chapter_subtitle_style", 
			"paragraph_style", "form_style", "label_style",
			"error_style", "success_style", "code_style"})) {
			system_monitoring_interface::global_mon->log_warning("style sheet is missing");
			std::for_each(missing.begin(), missing.end(), [](const std::string& s) { 
				std::cout << s << std::endl; 
				});
			return;
		}

		json colors = _src["colors"];

		_dest.Colors.clear();
		put_json(_dest.Colors, _src);

		json page_style = _src["page_style"];
		if (page_style.object()) {
			if (!_dest.PageStyle) {
				_dest.PageStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.PageStyle, page_style);
			_dest.PageStyle->set_default_name( "page_style" );
		}

		json caption_style = _src["caption_style"];
		if (caption_style.object()) {
			if (!_dest.CaptionStyle) {
				_dest.CaptionStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.CaptionStyle, caption_style);
			_dest.CaptionStyle->set_default_name("caption_style");
		}

		json title_style = _src["title_style"];
		if (title_style.object()) {
			if (!_dest.TitleStyle) {
				_dest.TitleStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.TitleStyle, title_style);
			_dest.TitleStyle->set_default_name("title_style");
		}

		json subtitle_style = _src["subtitle_style"];
		if (subtitle_style.object()) {
			if (!_dest.SubtitleStyle) {
				_dest.SubtitleStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.SubtitleStyle, subtitle_style);
			_dest.SubtitleStyle->set_default_name("subtitle_style");
		}

		json chapter_title_style = _src["chapter_title_style"];
		if (chapter_title_style.object()) {
			if (!_dest.ChapterTitleStyle) {
				_dest.ChapterTitleStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.ChapterTitleStyle, chapter_title_style);
			_dest.ChapterTitleStyle->set_default_name("chapter_title_style");
		}

		json chapter_subtitle_style = _src["chapter_subtitle_style"];
		if (chapter_subtitle_style.object()) {
			if (!_dest.ChapterSubTitleStyle) {
				_dest.ChapterSubTitleStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.ChapterSubTitleStyle, chapter_subtitle_style);
			_dest.ChapterSubTitleStyle->set_default_name("chapter_subtitle_style");
		}

		json paragraph_style = _src["paragraph_style"];
		if (paragraph_style.object()) {
			if (!_dest.ParagraphStyle) {
				_dest.ParagraphStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.ParagraphStyle, paragraph_style);
			_dest.ParagraphStyle->set_default_name("paragraph_style");
		}

		json form_style = _src["form_style"];
		if (form_style.object()) {
			if (!_dest.FormStyle) {
				_dest.FormStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.FormStyle, form_style);
			_dest.FormStyle->set_default_name("form_style");
		}

		json label_style = _src["label_style"];
		if (label_style.object()) {
			if (!_dest.LabelStyle) {
				_dest.LabelStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.LabelStyle, label_style);
			_dest.LabelStyle->set_default_name("label_style");
		}

		json placeholder_style = _src["placeholder_style"];
		if (placeholder_style.object()) {
			if (!_dest.PlaceholderStyle) {
				_dest.PlaceholderStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.PlaceholderStyle, placeholder_style);
			_dest.PlaceholderStyle->set_default_name("placeholder_style");
		}

		json error_style = _src["error_style"];
		if (error_style.object()) {
			if (!_dest.ErrorStyle) {
				_dest.ErrorStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.ErrorStyle, error_style);
			_dest.ErrorStyle->set_default_name("error_style");
		}

		json success_style = _src["success_style"];
		if (success_style.object()) {
			if (!_dest.SuccessStyle) {
				_dest.SuccessStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.SuccessStyle, success_style);
			_dest.SuccessStyle->set_default_name("success_style");
		}

		json code_style = _src["code_style"];
		if (code_style.object()) {
			if (!_dest.CodeStyle) {
				_dest.CodeStyle = std::make_shared<viewStyleRequest>();
			}
			put_json(_dest.CodeStyle, code_style);
			_dest.CodeStyle->set_default_name("code_style");
		}
	}


	class presentation_style_factory
	{
		std::string current_style;
		std::map<std::string, std::shared_ptr<presentation_style>> style_sheets;

		void create_default_styles();

		static presentation_style_factory* current;

		presentation_style_factory();

	public:

		static presentation_style_factory* get_current()
		{
			if (current == nullptr) {
				current = new presentation_style_factory();
			}
			return current;
		}

		presentation_style* get_style();
		void load_style_sheet(json& _src);
	};

	presentation_style_factory *presentation_style_factory::current = nullptr;

	// 4D6DFF

	presentation_style_factory::presentation_style_factory()
	{
		create_default_styles();
	}

	void presentation_style_factory::create_default_styles()
	{
		std::shared_ptr<presentation_style> countrybit = std::make_shared<presentation_style>();

		std::string bigFont = "TW Cent MT, Century Gothic, Arial";
		std::string textFont = "Tahoma, Arial";

		countrybit->PrimaryFont = textFont;

		countrybit->Colors = {
			{ "Blue", {"34657F", "D4E1E8", "AACDDA", "6399AE", "34657F", "081F2C" } },
			{ "Gray", { "7C878E", "DEE1E1", "C1C6C8", "A2AAAD", "7C878E", "5B6770" } },
			{ "Green", { "4BA24D", "C8DFBC", "AFD29E", "6DB658", "4BA24D", "008A41" } },
			{ "Plum", { "621244", "D7D3DB", "C1B2C3", "996F8D", "862663", "621244"} },
			{ "Khaki", { "B6B7A2", "DEDDD3", "CBCBBC", "B6B7A2", "888F6E", "67724C" } },
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

		countrybit->PageStyle = std::make_shared<viewStyleRequest>();
		countrybit->PageStyle->name = "Page";
		countrybit->PageStyle->box_fill_brush = generalBrushRequest(lgbr);
		countrybit->PageStyle->box_fill_brush.set_name("PageBackground");
		countrybit->PageStyle->box_border_brush = generalBrushRequest();
		countrybit->PageStyle->box_border_brush.setColor("#C0C0C0");
		countrybit->PageStyle->box_border_brush.set_name("PageBorder");
		countrybit->PageStyle->shape_fill_brush = generalBrushRequest();
		countrybit->PageStyle->shape_fill_brush.setColor("#000000");
		countrybit->PageStyle->shape_fill_brush.set_name("PageText");
		countrybit->PageStyle->text_style = text_style;
		countrybit->PageStyle->text_style.fontName = textFont;
		countrybit->PageStyle->text_style.fontSize = 14;
		countrybit->PageStyle->text_style.name = "PageFont";

		countrybit->PlaceholderStyle = std::make_shared<viewStyleRequest>();
		countrybit->PlaceholderStyle->name = "Placeholder";
		countrybit->PlaceholderStyle->box_fill_brush = generalBrushRequest(lgbr);
		countrybit->PlaceholderStyle->box_fill_brush.set_name("PlaceholderBackground");
		countrybit->PlaceholderStyle->box_border_brush = generalBrushRequest();
		countrybit->PlaceholderStyle->box_border_brush.setColor("#C0C0C0");
		countrybit->PlaceholderStyle->box_border_brush.set_name("PlaceholderBorder");
		countrybit->PlaceholderStyle->shape_fill_brush = generalBrushRequest();
		countrybit->PlaceholderStyle->shape_fill_brush.setColor("#000000");
		countrybit->PlaceholderStyle->shape_fill_brush.set_name("PlaceholderText");
		countrybit->PlaceholderStyle->text_style = text_style;
		countrybit->PlaceholderStyle->text_style.fontName = textFont;
		countrybit->PlaceholderStyle->text_style.fontSize = 23;
		countrybit->PlaceholderStyle->text_style.name = "PlaceholderFont";

		lgbr.gradientStops = {
{ toColor("#600000"), 0.0 },
{ toColor("#600000"), 0.13 },
{ toColor("#200000"), 1.0 }
		};

		countrybit->ErrorStyle = std::make_shared<viewStyleRequest>();
		countrybit->ErrorStyle->name = "Error";
		countrybit->ErrorStyle->box_fill_brush = generalBrushRequest(lgbr);
		countrybit->ErrorStyle->box_fill_brush.set_name("ErrorBackground");
		countrybit->ErrorStyle->box_border_brush = generalBrushRequest();
		countrybit->ErrorStyle->box_border_brush.setColor("#C0C0C0");
		countrybit->ErrorStyle->box_border_brush.set_name("ErrorBorder");
		countrybit->ErrorStyle->shape_fill_brush = generalBrushRequest();
		countrybit->ErrorStyle->shape_fill_brush.setColor("#CC0000");
		countrybit->ErrorStyle->shape_fill_brush.set_name("ErrorText");
		countrybit->ErrorStyle->text_style = text_style;
		countrybit->ErrorStyle->text_style.fontName = "Arial";
		countrybit->ErrorStyle->text_style.fontSize = 14;
		countrybit->ErrorStyle->text_style.name = "ErrorFont";
		countrybit->ErrorStyle->text_style.vertical_align = visual_alignment::align_center;
		countrybit->ErrorStyle->text_style.horizontal_align = visual_alignment::align_near;

		lgbr.gradientStops = {
{ toColor("#000010"), 0.0 },
{ toColor("#000010"), 0.13 },
{ toColor("#000020"), 1.0 }
		};

		countrybit->SuccessStyle = std::make_shared<viewStyleRequest>();
		countrybit->SuccessStyle->name = "Success";
		countrybit->SuccessStyle->box_fill_brush = generalBrushRequest(lgbr);
		countrybit->SuccessStyle->box_fill_brush.set_name("SuccessBackground");
		countrybit->SuccessStyle->box_border_brush = generalBrushRequest();
		countrybit->SuccessStyle->box_border_brush.setColor("#C0C0C0");
		countrybit->SuccessStyle->box_border_brush.set_name("SuccessBorder");
		countrybit->SuccessStyle->shape_fill_brush = generalBrushRequest();
		countrybit->SuccessStyle->shape_fill_brush.setColor("#CC0000");
		countrybit->SuccessStyle->shape_fill_brush.set_name("SuccessText");
		countrybit->SuccessStyle->text_style = text_style;
		countrybit->SuccessStyle->text_style.fontName = textFont;
		countrybit->SuccessStyle->text_style.fontSize = 16;
		countrybit->SuccessStyle->text_style.name = "SuccessFont";
		countrybit->SuccessStyle->text_style.vertical_align = visual_alignment::align_center;
		countrybit->SuccessStyle->text_style.horizontal_align = visual_alignment::align_near;

		lgbr.start.x = .5;
		lgbr.start.y = 0;
		lgbr.stop.x = .5;
		lgbr.stop.y = 1;
		lgbr.gradientStops = {
	{ toColor("#313150"), 0.0 },
	{ toColor("#313150"), 0.13 },
	{ toColor("#101020"), 1.0 },
		};

		countrybit->CaptionStyle = std::make_shared<viewStyleRequest>();
		countrybit->CaptionStyle->name = "Caption";
		countrybit->CaptionStyle->box_fill_brush = generalBrushRequest(lgbr);
		countrybit->CaptionStyle->box_fill_brush.set_name("CaptionBackground");
		countrybit->CaptionStyle->box_border_brush = generalBrushRequest();
		countrybit->CaptionStyle->box_border_brush.setColor("#C0C0C0");
		countrybit->CaptionStyle->box_border_brush.set_name("CaptionBorder");
		countrybit->CaptionStyle->shape_fill_brush = generalBrushRequest();
		countrybit->CaptionStyle->shape_fill_brush.setColor("#FFFFFF");
		countrybit->CaptionStyle->shape_fill_brush.set_name("CaptionText");
		countrybit->CaptionStyle->text_style = text_style;
		countrybit->CaptionStyle->text_style.fontName = bigFont;
		countrybit->CaptionStyle->text_style.fontSize = 24;
		countrybit->CaptionStyle->text_style.name = "CaptionFont";
		countrybit->CaptionStyle->text_style.vertical_align = visual_alignment::align_center;
		countrybit->CaptionStyle->text_style.horizontal_align = visual_alignment::align_near;

		countrybit->TitleStyle = std::make_shared<viewStyleRequest>();
		countrybit->TitleStyle->name = "Title";
		countrybit->TitleStyle->box_fill_brush = generalBrushRequest(lgbr);
		countrybit->TitleStyle->box_fill_brush.set_name("TitleBackground");
		countrybit->TitleStyle->box_border_brush = generalBrushRequest();
		countrybit->TitleStyle->box_border_brush.setColor("#FFFFFF");
		countrybit->TitleStyle->box_border_brush.set_name("TitleBorder");
		countrybit->TitleStyle->shape_fill_brush = generalBrushRequest();
		countrybit->TitleStyle->shape_fill_brush.setColor("#FFFFFF");
		countrybit->TitleStyle->shape_fill_brush.set_name("TitleText");
		countrybit->TitleStyle->text_style = text_style;
		countrybit->TitleStyle->text_style.fontName = bigFont;
		countrybit->TitleStyle->text_style.fontSize = 38;
		countrybit->TitleStyle->text_style.name = "TitleFont";
		countrybit->TitleStyle->text_style.vertical_align = visual_alignment::align_center;
		countrybit->TitleStyle->text_style.horizontal_align = visual_alignment::align_near;

		countrybit->SubtitleStyle = std::make_shared<viewStyleRequest>();
		countrybit->SubtitleStyle->name = "Subtitle";
		countrybit->SubtitleStyle->box_fill_brush = generalBrushRequest(lgbr);
		countrybit->SubtitleStyle->box_fill_brush.set_name("SubtitleBackground");
		countrybit->SubtitleStyle->box_border_brush = generalBrushRequest();
		countrybit->SubtitleStyle->box_border_brush.setColor("#FFFFFF");
		countrybit->SubtitleStyle->box_border_brush.set_name("SubtitleBorder");
		countrybit->SubtitleStyle->shape_fill_brush = generalBrushRequest();
		countrybit->SubtitleStyle->shape_fill_brush.setColor("#FFFFFF");
		countrybit->SubtitleStyle->shape_fill_brush.set_name("SubtitleText");
		countrybit->SubtitleStyle->text_style = text_style;
		countrybit->SubtitleStyle->text_style.fontName = bigFont;
		countrybit->SubtitleStyle->text_style.fontSize = 20;
		countrybit->SubtitleStyle->text_style.name = "SubtitleFont";
		countrybit->SubtitleStyle->text_style.vertical_align = visual_alignment::align_center;
		countrybit->SubtitleStyle->text_style.horizontal_align = visual_alignment::align_near;

		countrybit->AuthorsCreditStyle = std::make_shared<viewStyleRequest>();
		countrybit->AuthorsCreditStyle->name = "AuthorsCredit";
		countrybit->AuthorsCreditStyle->box_fill_brush = generalBrushRequest(lgbr);
		countrybit->AuthorsCreditStyle->box_fill_brush.set_name("AuthorsCreditBackground");
		countrybit->AuthorsCreditStyle->box_border_brush = generalBrushRequest();
		countrybit->AuthorsCreditStyle->box_border_brush.setColor("#FFFFFF");
		countrybit->AuthorsCreditStyle->box_border_brush.set_name("AuthorsCreditBorder");
		countrybit->AuthorsCreditStyle->shape_fill_brush = generalBrushRequest();
		countrybit->AuthorsCreditStyle->shape_fill_brush.setColor("#FFFFFF");
		countrybit->AuthorsCreditStyle->shape_fill_brush.set_name("AuthorsCreditText");
		countrybit->AuthorsCreditStyle->text_style = text_style;
		countrybit->AuthorsCreditStyle->text_style.fontName = bigFont;
		countrybit->AuthorsCreditStyle->text_style.fontSize = 20;
		countrybit->AuthorsCreditStyle->text_style.name = "AuthorsCreditFont";
		countrybit->AuthorsCreditStyle->text_style.vertical_align = visual_alignment::align_center;
		countrybit->AuthorsCreditStyle->text_style.horizontal_align = visual_alignment::align_near;

		countrybit->ChapterTitleStyle = std::make_shared<viewStyleRequest>();
		countrybit->ChapterTitleStyle->name = "ChapterTitle";
		countrybit->ChapterTitleStyle->box_fill_brush = generalBrushRequest(lgbr);
		countrybit->ChapterTitleStyle->box_fill_brush.set_name("ChapterTitleBackground");
		countrybit->ChapterTitleStyle->box_border_brush = generalBrushRequest();
		countrybit->ChapterTitleStyle->box_border_brush.setColor("#FFFFFF");
		countrybit->ChapterTitleStyle->box_border_brush.set_name("ChapterTitleBorder");
		countrybit->ChapterTitleStyle->shape_fill_brush = generalBrushRequest();
		countrybit->ChapterTitleStyle->shape_fill_brush.setColor("#000000");
		countrybit->ChapterTitleStyle->shape_fill_brush.set_name("ChapterTitleText");
		countrybit->ChapterTitleStyle->text_style = text_style;
		countrybit->ChapterTitleStyle->text_style.fontName = bigFont;
		countrybit->ChapterTitleStyle->text_style.fontSize = 20;
		countrybit->ChapterTitleStyle->text_style.name = "ChapterTitleFont";
		countrybit->ChapterTitleStyle->text_style.vertical_align = visual_alignment::align_center;
		countrybit->ChapterTitleStyle->text_style.horizontal_align = visual_alignment::align_near;


		countrybit->ChapterSubTitleStyle = std::make_shared<viewStyleRequest>();
		countrybit->ChapterSubTitleStyle->name = "ChapterSubTitle";
		countrybit->ChapterSubTitleStyle->box_fill_brush = generalBrushRequest(lgbr);
		countrybit->ChapterSubTitleStyle->box_fill_brush.set_name("ChapterSubTitleBackground");
		countrybit->ChapterSubTitleStyle->box_border_brush = generalBrushRequest();
		countrybit->ChapterSubTitleStyle->box_border_brush.setColor("#FFFFFF");
		countrybit->ChapterSubTitleStyle->box_border_brush.set_name("ChapterSubTitleBorder");
		countrybit->ChapterSubTitleStyle->shape_fill_brush = generalBrushRequest();
		countrybit->ChapterSubTitleStyle->shape_fill_brush.setColor("#FFFFFF");
		countrybit->ChapterSubTitleStyle->shape_fill_brush.set_name("ChapterSubTitleText");
		countrybit->ChapterSubTitleStyle->text_style = text_style;
		countrybit->ChapterSubTitleStyle->text_style.fontName = textFont;
		countrybit->ChapterSubTitleStyle->text_style.fontSize = 14;
		countrybit->ChapterSubTitleStyle->text_style.underline = 1;
		countrybit->ChapterSubTitleStyle->text_style.name = "ChapterSubTitleFont";
		countrybit->ChapterSubTitleStyle->text_style.vertical_align = visual_alignment::align_center;
		countrybit->ChapterSubTitleStyle->text_style.horizontal_align = visual_alignment::align_near;

		countrybit->CodeStyle = std::make_shared<viewStyleRequest>();
		countrybit->CodeStyle->name = "Code";
		countrybit->CodeStyle->box_fill_brush = generalBrushRequest("CodeBackground", "#111111");
		countrybit->CodeStyle->box_border_brush = generalBrushRequest("CodeBorder", "#FFFFFF");
		countrybit->CodeStyle->shape_fill_brush = generalBrushRequest("CodeText", "#FFFFFF");
		countrybit->CodeStyle->text_style = text_style;
		countrybit->CodeStyle->text_style.fontName = textFont;
		countrybit->CodeStyle->text_style.fontSize = 14;
		countrybit->CodeStyle->text_style.underline = 1;
		countrybit->CodeStyle->text_style.name = "CodeFont";
		countrybit->CodeStyle->text_style.vertical_align = visual_alignment::align_near;
		countrybit->CodeStyle->text_style.horizontal_align = visual_alignment::align_near;

		countrybit->ParagraphStyle = std::make_shared<viewStyleRequest>();
		countrybit->ParagraphStyle->name = "Paragraph";
		countrybit->ParagraphStyle->box_fill_brush = generalBrushRequest("ParagraphBackground", "#111111");
		countrybit->ParagraphStyle->box_border_brush = generalBrushRequest("ParagraphBorder", "#FFFFFF");
		countrybit->ParagraphStyle->shape_fill_brush = generalBrushRequest("ParagraphText", "#000000");
		countrybit->ParagraphStyle->text_style = text_style;
		countrybit->ParagraphStyle->text_style.fontName = textFont;
		countrybit->ParagraphStyle->text_style.fontSize = 14;
		countrybit->ParagraphStyle->text_style.underline = 0;
		countrybit->ParagraphStyle->text_style.name = "ParagraphFont";

		countrybit->LabelStyle = std::make_shared<viewStyleRequest>();
		countrybit->LabelStyle->name = "Label";
		countrybit->LabelStyle->box_fill_brush = generalBrushRequest("LabelBackground", "#FFFFFF");
		countrybit->LabelStyle->box_border_brush = generalBrushRequest("LabelBorder", "#FFFFFF");
		countrybit->LabelStyle->shape_fill_brush = generalBrushRequest("LabelText", "#000000");
		countrybit->LabelStyle->text_style = text_style;
		countrybit->LabelStyle->text_style.fontName = textFont;
		countrybit->LabelStyle->text_style.fontSize = 14;
		countrybit->LabelStyle->text_style.underline = 0;
		countrybit->LabelStyle->text_style.name = "LabelFont";

		countrybit->FormStyle = std::make_shared<viewStyleRequest>();
		countrybit->FormStyle->name = "Form";
		countrybit->FormStyle->box_fill_brush = generalBrushRequest("FormBackground", "#FFFFFF");
		countrybit->FormStyle->box_border_brush = generalBrushRequest("FormBorder", "#FFFFFF");
		countrybit->FormStyle->shape_fill_brush = generalBrushRequest("FormText", "#000000");
		countrybit->FormStyle->text_style = text_style;
		countrybit->FormStyle->text_style.fontName = textFont;
		countrybit->FormStyle->text_style.fontSize = 14;
		countrybit->FormStyle->text_style.underline = 0;
		countrybit->FormStyle->text_style.name = "FormFont";

		style_sheets["default"] = countrybit;
		current_style = "default";
	}

	presentation_style *presentation_style_factory::get_style()
	{
		return style_sheets[current_style].get();
	}

	void presentation_style_factory::load_style_sheet(json& _src)
	{
		lockable me;

		{
			scope_lock lock(me);

			std::string style_sheet_name = _src["style_sheet_name"];
			if (style_sheet_name.empty()) {
				system_monitoring_interface::global_mon->log_warning("style_sheet_name must not be empty");
			}
			std::shared_ptr<presentation_style> update_style;

			if (!style_sheet_name.empty()) {

				if (style_sheets.contains(style_sheet_name)) {
					update_style = style_sheets[style_sheet_name];
					presentation_style* st = update_style.get();
					put_json(*st, _src);
				}
				else 
				{
					update_style = std::make_shared<presentation_style>();
					presentation_style* st = update_style.get();
					put_json(*st, _src);
					style_sheets.insert_or_assign(style_sheet_name, update_style);
				}

				current_style = style_sheet_name;
			}
		}
	}
}

#endif
