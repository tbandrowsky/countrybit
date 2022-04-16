#pragma once

#include "pobject.h"
#include <extractor.h>

namespace countrybit
{
	namespace system
	{
		void pvalue::set_value(database::string_box& dest) const
		{
			switch (pvalue_type) {
			case pvalue_types::string_value:
				dest = string_value;
				break;
			case pvalue_types::double_value:
				dest = double_value;
				break;
			}
		}

		void pvalue::set_value(database::double_box& dest) const
		{
			switch (pvalue_type) {
			case pvalue_types::double_value:
				dest = double_value;
				break;
			default:
				dest = 0.0;
				break;
			}
		}

		void pvalue::set_value(database::float_box& dest) const
		{
			switch (pvalue_type) {
			case pvalue_types::double_value:
				dest = double_value;
				break;
			default:
				dest = 0;
				break;
			}
		}

		void pvalue::set_value(database::int8_box& dest) const
		{
			switch (pvalue_type) {
			case pvalue_types::double_value:
				dest = (int8_t)double_value;
				break;
			default:
				dest = 0;
				break;
			}
		}

		void pvalue::set_value(database::int16_box& dest) const
		{
			switch (pvalue_type) {
			case pvalue_types::double_value:
				dest = (int16_t)double_value;
				break;
			default:
				dest = 0;
				break;
			}
		}

		void pvalue::set_value(database::int32_box& dest) const
		{
			switch (pvalue_type) {
			case pvalue_types::double_value:
				dest = (int32_t)double_value;
				break;
			default:
				dest = 0;
				break;
			}
		}

		void pvalue::set_value(database::int64_box& dest) const
		{
			switch (pvalue_type) {
			case pvalue_types::double_value:
				dest = (int64_t)double_value;
				break;
			}
		}

		void pvalue::set_value(database::time_box& dest) const
		{
			switch (pvalue_type) {
			case pvalue_types::time_value:
				dest = (int64_t)time_value;
				break;
			}
		}

		void pvalue::set_value(database::object_id_box& dest) const
		{
			if (pvalue_type == pvalue_types::string_value)
			{
			}
		}

		void pvalue::set_value(database::collection_id_box& dest) const
		{
			if (pvalue_type == pvalue_types::string_value)
			{
			}
		}

		void pvalue::set_value(database::color_box& dest) const
		{
			if (pvalue_type == pvalue_types::string_value)
			{
				string_extractor extractor(string_value, strlen(this->string_value), 100, "");
				auto result = extractor.get_color();
				if (result.success) {
					database::color c;
					c.red = result.red / 256.0;
					c.green = result.green / 256.0;
					c.blue = result.blue / 256.0;
					c.alpha = result.alpha / 256.0;
					dest = c;
				}
			}
		}

		void pvalue::set_value(database::point_box& dest) const
		{
			if (pvalue_type == pvalue_types::string_value)
			{
				string_extractor extractor(string_value, strlen(this->string_value), 100, "");
				auto result = extractor.get_point();
				if (result.success) {
					database::point c;
					c.x = result.x;
					c.y = result.y;
					c.z = result.z;
					dest = c;
				}
			}
		}

		void pvalue::set_value(database::rectangle_box& dest) const
		{
			if (pvalue_type == pvalue_types::string_value)
			{
				string_extractor extractor(string_value, strlen(this->string_value), 100, "");
				auto result = extractor.get_rectangle();
				if (result.success) {
					database::rectangle c;
					c.corner.x = result.x;
					c.corner.y = result.y;
					c.corner.z = 0.0;
					c.size.x = result.w;
					c.size.y = result.h;
					c.size.z = 0.0;
					dest = c;
				}
			}
		}

		void pvalue::set_value(database::image_box& dest) const
		{
			if (pvalue_type == pvalue_types::string_value)
			{
				string_extractor extractor(string_value, strlen(this->string_value), 100, "");
				auto result = extractor.get_rectangle();
				if (result.success) {
					database::rectangle c;
					c.corner.x = result.x;
					c.corner.y = result.y;
					c.corner.z = 0.0;
					c.size.x = result.w;
					c.size.y = result.h;
					c.size.z = 0.0;
					dest.get_data_ref().source = c;
				}
			}
		}

		void pvalue::set_value(database::wave_box& dest) const
		{
			if (pvalue_type == pvalue_types::string_value)
			{
				string_extractor extractor(string_value, strlen(this->string_value), 100, "");
				auto result = extractor.get_audio();
				if (result.success) {
					database::wave_instance  c;
					c.start_seconds = result.start_seconds;
					c.stop_seconds = result.stop_seconds;
					c.pitch_adjust = result.pitch_adjust;
					c.volume_adjust = result.volume_adjust;
					c.playing = result.playing;
					dest = c;
				}
			}
		}

		void pvalue::set_value(database::midi_box& dest) const
		{
			if (pvalue_type == pvalue_types::string_value)
			{
				string_extractor extractor(string_value, strlen(this->string_value), 100, "");
				auto result = extractor.get_audio();
				if (result.success) {
					database::midi_instance  c;
					c.start_seconds = result.start_seconds;
					c.stop_seconds = result.stop_seconds;
					c.pitch_adjust = result.pitch_adjust;
					c.volume_adjust = result.volume_adjust;
					c.playing = result.playing;
				}
			}
		}

	}
}
