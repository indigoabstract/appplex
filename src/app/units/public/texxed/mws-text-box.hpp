#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEXXED

#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-com.hpp"
#include "com/mws/mws-font.hpp"
#include "com/mws/font-db.hpp"
#include "com/mws/text-vxo.hpp"
#include "min.hpp"


class text_box : public mws_text_area
{
public:
   static mws_sp<text_box> nwi();
   void setup() override;
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
