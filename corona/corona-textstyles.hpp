#ifndef CORONA_TEXTSTYLES
#define CORONA_TEXTSTYLES

namespace corona {

	class textStyle : public deviceDependentAssetBase
	{
		IDWriteTextFormat* lpWriteTextFormat;

		std::string fontName;
		float size;
		bool bold;
		bool italic;
		bool underline;
		bool strike_through;
		double line_spacing;
		visual_alignment horizontal_align;
		visual_alignment vertical_align;
		bool wrap_text;
		double character_spacing;
		DWRITE_FONT_STRETCH font_stretch;

	public:



		textStyle(std::string _fontName,
			float _size,
			bool _bold,
			bool _italic,
			bool _underline,
			bool _strike_through,
			double _line_spacing,
			visual_alignment _horizontal_align,
			visual_alignment _vertical_align,
			bool _wrap_text,
			double _character_spacing,
			DWRITE_FONT_STRETCH _font_stretch) :
			fontName(_fontName),
			size(_size),
			bold(_bold),
			italic(_italic),
			underline(_underline),
			strike_through(_strike_through),
			line_spacing(_line_spacing),
			horizontal_align(_horizontal_align),
			vertical_align(_vertical_align),
			character_spacing(_character_spacing),
			font_stretch(_font_stretch),
			lpWriteTextFormat(NULL)
		{
			;
		}

		virtual ~textStyle()
		{
			release();
		}

		std::string get_fontName() { return fontName; };
		float get_size() { return size; }
		bool get_bold() { return bold; }
		bool get_italic() { return italic; }
		bool get_underline() { return underline; }
		bool get_strike_through() { return strike_through; }
		double get_line_spacing() { return line_spacing; }
		double get_character_spacing() { return character_spacing; }
		visual_alignment get_horizontal_align() { return horizontal_align; }
		visual_alignment get_vertical_align() { return vertical_align; }
		bool get_wrap_text() { return wrap_text; }

		bool create(direct2dContextBase* ptarget)
		{
			HRESULT hr = -1;

			if (ptarget)
			{
				if (!ptarget->getDeviceContext())
					return false;

				istring<2048> fontList = fontName;
				istring<2048> fontName;

				int state = 0;
				char* fontExtractedName = fontList.next_token({ ',', ';' }, state);
				lpWriteTextFormat = NULL;

				while (fontExtractedName)
				{
					fontName = fontExtractedName;
					iwstring<2048> wideName = fontName;

					DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL;

					if (italic) {
						fontStyle = DWRITE_FONT_STYLE_ITALIC;
					}

					FLOAT dpiX = 96.0, dpiY = 96.0;
					ptarget->getDeviceContext()->GetDpi(&dpiX, &dpiY);

					if (auto fact = ptarget->getAdapter().lock())
					{
						if (lpWriteTextFormat) {
							lpWriteTextFormat->Release();
							lpWriteTextFormat = nullptr;
						}
						HRESULT hr = fact->getDWriteFactory()->CreateTextFormat(wideName.c_str(),
							NULL,
							bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_REGULAR,
							fontStyle,
							font_stretch,
							size,
							L"en-US",
							&lpWriteTextFormat);

						if (SUCCEEDED(hr) || lpWriteTextFormat != nullptr) {
							break;
						}
						else 
						{
							std::cout << "Font '" << fontName << "' not loaded." << std::endl;
						}
					}
					else {
						std::cout << "Font '" << fontName << "' not created adapter not locked." << std::endl;
					}

					fontExtractedName = fontList.next_token(',', state);
				};

				if (lpWriteTextFormat != nullptr)
				{
					if (line_spacing > 0.0) {
						lpWriteTextFormat->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, line_spacing, line_spacing * .8);
					}

					switch (horizontal_align)
					{
					case visual_alignment::align_near:
						lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
						break;
					case visual_alignment::align_center:
						lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
						break;
					case visual_alignment::align_far:
						lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
						break;
					case visual_alignment::align_justify:
						lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
						break;
					}

					switch (horizontal_align)
					{
					case visual_alignment::align_near:
						lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
						break;
					case visual_alignment::align_center:
						lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
						break;
					case visual_alignment::align_far:
						lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
						break;
					case visual_alignment::align_justify:
						lpWriteTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_JUSTIFIED);
						break;
					}

					switch (vertical_align)
					{
					case visual_alignment::align_near:
						lpWriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
						break;
					case visual_alignment::align_center:
						lpWriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
						break;
					case visual_alignment::align_far:
						lpWriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
						break;
					case visual_alignment::align_justify:
						lpWriteTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
						break;
					}

					if (wrap_text)
					{
						lpWriteTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_EMERGENCY_BREAK);
					}


					return true;
				}
			}
			return false;
		}

		void release()
		{
			if (lpWriteTextFormat)
				lpWriteTextFormat->Release();
			lpWriteTextFormat = NULL;
		}

		IDWriteTextFormat* getFormat()
		{
			return lpWriteTextFormat;
		}

	};
}

#endif
