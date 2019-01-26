#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_MWS

#include "mws-camera.hpp"
#include "mws-com.hpp"
#include "mws-font.hpp"
#include "font-db.hpp"
#include "text-vxo.hpp"
#include "min.hpp"


class text_area_model_ro : public text_area_model
{
public:
   text_area_model_ro();
   virtual bool get_word_wrap() override;
   virtual void set_word_wrap(bool i_word_wrap) override;
   virtual int get_line_count() override;
   virtual std::string get_line_at(int i_idx, bool i_keep_line_break = true) override;
   virtual std::vector<std::string> get_lines_at(int i_idx, int i_line_count, bool i_keep_line_break = true) override;
   virtual std::string get_text() override;
   virtual void push_back(const char* i_text, int i_length) override;
   virtual void push_front(const char* i_text, int i_length) override;
   virtual void insert_at_cursor(const std::string& i_text) override;
   virtual void delete_at_cursor(int32 i_count) override;
   virtual uint32 get_cursor_pos() override;
   virtual void set_cursor_pos(uint32 i_cursor_pos) override;
   virtual void set_text(const std::string& i_text) override;
   virtual void set_text(const char* i_text, int i_length) override;
   virtual void set_size(int i_width, int i_height) override;
   virtual void set_font(mws_sp<mws_font> i_font) override;
   virtual int get_char_at_pixel(float i_x, float i_y) override;

private:
   void update_back_added_line_offsets(const std::string& i_new_text);
   void update_front_added_line_offsets(const std::string& i_new_text);
   void update_line_offsets();

   std::string text;
   bool word_wrap = false;
   std::vector<uint32> line_offsets;
   int width = 0;
   int height = 0;
   mws_sp<mws_font> font;
};


class text_area_model_rw : public text_area_model
{
public:
   text_area_model_rw();
   virtual bool get_word_wrap() override;
   virtual void set_word_wrap(bool i_word_wrap) override;
   virtual int get_line_count() override;
   virtual std::string get_line_at(int i_idx, bool i_keep_line_break = true) override;
   virtual std::vector<std::string> get_lines_at(int i_idx, int i_line_count, bool i_keep_line_break = true) override;
   virtual std::string get_text() override;
   virtual void push_back(const char* i_text, int i_length) override;
   virtual void push_front(const char* i_text, int i_length) override;
   virtual void insert_at_cursor(const std::string& i_text) override;
   virtual void delete_at_cursor(int32 i_count) override;
   virtual uint32 get_cursor_pos() override;
   virtual void set_cursor_pos(uint32 i_cursor_pos) override;
   virtual void set_text(const std::string& i_text) override;
   virtual void set_text(const char* i_text, int i_length) override;
   virtual void set_size(int i_width, int i_height) override;
   virtual void set_font(mws_sp<mws_font> i_font) override;
   virtual int get_char_at_pixel(float i_x, float i_y) override;

private:
   std::string text;
   uint32 cursor_pos = 0;
   bool word_wrap = false;
   mws_sp<mws_font> font;
};


class text_box : public mws_text_area
{
public:
   static mws_sp<text_box> nwi();
   void setup() override;
   virtual bool is_editable() const;
   virtual void set_editable(bool i_is_editable);
   virtual void set_text(const std::string& i_text);
   virtual void push_back_text(const std::string& i_text);
   virtual void push_front_text(const std::string& i_text);
   virtual void insert_at_cursor(const std::string& i_text);
   virtual void delete_at_cursor(int32 i_count);
   virtual void scroll_text(const glm::vec2& ioff);
   virtual void set_position(const glm::vec2& ipos);
   virtual void set_dimension(const glm::vec2& idim);
   virtual void select_char_at(const glm::vec2& ipos);
   virtual void update_state();
   virtual void update_view(mws_sp<mws_camera> g);
   virtual void receive(mws_sp<iadp> idp);

protected:
   text_box();
   mws_sp<text_area_model> new_model();

   bool editable = false;
   mws_sp<text_area_model> tx_src;
   mws_sp<text_vxo> tx_vxo;
   mws_sp<mws_font> font;
   glm::vec2 pos;
   glm::vec2 dim;
   glm::vec2 text_offset;
   int text_rows;
   kinetic_scrolling ks;
   std::vector<std::string> tx_rows;
   glm::vec4 select_char_rect;
   dragging_detector dragging_det;
};

#endif
