#pragma once

#include "pfm.hpp"
#include <glm/fwd.hpp>


class gfx_color;
class mws_font_impl;
class font_cache;
class font_db_impl;


class mws_font : public enable_shared_from_this < mws_font >
{
public:
   static shared_ptr<mws_font> new_inst(std::shared_ptr<mws_font> i_fnt);
   static shared_ptr<mws_font> new_inst(float isize, const std::string& ifont_path = "");
	shared_ptr<mws_font> get_inst();
	const std::string& get_file_name()const;
	const std::string& get_full_path()const;
	std::string get_font_name()const;
	std::string get_file_extension()const;
	const std::string& get_root_directory()const;
	float get_size()const;
	float get_ascender();
	float get_descender();
	float get_height();
	glm::vec2 get_text_dim(const std::string& itext);
	float get_text_width(const std::string& itext);
	float get_text_height(const std::string& itext);
	const gfx_color& get_color()const;
	void set_color(const gfx_color& icolor);

private:
	friend class font_db_impl;
	mws_font();

	// cache font data for quick access
	weak_ptr<font_cache> fnt_cache;
	shared_ptr<mws_font_impl> p;
};