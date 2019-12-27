#include "stdafx.hxx"

#include "appplex-conf.hxx"

#if defined MOD_MWS

#include "mws-camera.hxx"
#include "gfx.hxx"
#include "mws-font.hxx"
#include "font-db.hxx"
#include "text-vxo.hxx"


namespace ns_mws_camera
{
   class draw_text_op : public draw_op
   {
   public:
#if defined MOD_VECTOR_FONTS

      draw_text_op()
      {
         tx_vxo = text_vxo::nwi();
      }

      void push_data(mws_sp<rw_sequence> seq, const std::string& text, float ix, float iy, const mws_sp<mws_font> ifont)
      {
         tx = text;
         x = ix;
         y = iy;
         fonts.push_back(ifont);
         seq->w.write_pointer(this);
         write_data(seq);
         font_idx++;
      }

      virtual void read_data(mws_sp<rw_sequence> seq)
      {
         int length = seq->r.read_uint32();
         std::vector<char> txt(length);
         seq->r.read_int8((int8*)begin_ptr(txt), length * sizeof(char), 0);
         tx = std::string(begin_ptr(txt), length);
         x = seq_util::read_float(seq);
         y = seq_util::read_float(seq);
         font_idx = seq->r.read_uint32();
      }

      virtual void write_data(mws_sp<rw_sequence> seq)
      {
         seq->w.write_uint32(tx.length());
         seq->w.write_int8((int8*)tx.c_str(), tx.length() * sizeof(char), 0);
         seq_util::write_float(seq, x);
         seq_util::write_float(seq, y);
         seq->w.write_uint32(font_idx);
      }

      void on_update_start(mws_sp<draw_context> idc)
      {
         tx_vxo->clear_text();
      }

      void draw(mws_sp<draw_context> idc)
      {
         if (!tx.empty())
         {
            tx_vxo->add_text(tx, glm::vec2(x, y), fonts[font_idx]);
         }
      }

      void on_update_end(mws_sp<draw_context> idc)
      {
         tx_vxo->draw_in_sync(idc->get_cam());
         fonts.clear();
         tx.clear();
         font_idx = 0;
      }

      void set_text_blending(const std::string& i_blend_type)
      {
         (*tx_vxo)[MP_BLENDING] = i_blend_type;
      }

      mws_sp<text_vxo> tx_vxo;
      std::string tx;
      float x;
      float y;
      int font_idx = 0;
      // hold refs to the fonts, so they don't get destroyed before they're used for drawing
      std::vector<mws_sp<mws_font> > fonts;

#elif defined MOD_BITMAP_FONTS

      draw_text_op() {}
      void push_data(mws_sp<rw_sequence> seq, const std::string& text, float ix, float iy, const mws_sp<mws_font> ifont) {}
      virtual void read_data(mws_sp<rw_sequence> seq) {}
      virtual void write_data(mws_sp<rw_sequence> seq) {}
      void on_update_start(mws_sp<draw_context> idc) {}
      void draw(mws_sp<draw_context> idc) {}
      void on_update_end(mws_sp<draw_context> idc) {}
      void set_text_blending(const std::string& i_blend_type) {}

#endif
   };
}
using namespace ns_mws_camera;


class mws_camera_impl
{
public:
   mws_camera_impl() {}

   mws_sp<mws_font> get_font()const
   {
      return (font) ? font : font_db::inst()->get_global_font();
   }

   void set_font(mws_sp<mws_font> i_font)
   {
      font = i_font;
   }

   gfx_color color;
   draw_text_op d_text;

private:
   mws_sp<mws_font> font;
};


mws_sp<mws_camera> mws_camera::nwi(mws_sp<gfx> i_gi)
{
   mws_sp<mws_camera> inst(new mws_camera(i_gi));
   inst->load(inst);
   return inst;
}

void mws_camera::drawImage(mws_sp<gfx_tex> img, float x, float y, float width, float height)
{
   draw_image(img, x, y, width, height);
}

void mws_camera::drawLine(float i, float j, float k, float l)
{
   draw_line(glm::vec3(i, j, 0.f), glm::vec3(k, l, 0.f), p->color.to_vec4(), 1.f);
}

void mws_camera::drawRect(float x, float y, float width, float height)
{
   draw_line(glm::vec3(x, y, 0.f), glm::vec3(x + width, y, 0.f), p->color.to_vec4(), 1.f);
   draw_line(glm::vec3(x + width, y, 0.f), glm::vec3(x + width, y + height, 0.f), p->color.to_vec4(), 1.f);
   draw_line(glm::vec3(x + width, y + height, 0.f), glm::vec3(x, y + height, 0.f), p->color.to_vec4(), 1.f);
   draw_line(glm::vec3(x, y + height, 0.f), glm::vec3(x, y, 0.f), p->color.to_vec4(), 1.f);
}

void mws_camera::fillRect(float x, float y, float width, float height)
{
   draw_plane(glm::vec3(x + width * 0.5, y + height * 0.5, 0.f), glm::vec3(0, 0, 1), glm::vec2(width, height), p->color.to_vec4());
   //drawRect(x, y, width, height);
   //draw_line(glm::vec3(x, y, 0.f), glm::vec3(x + width, y, 0.f), color.to_vec4(), height / 2);
   //draw_line(glm::vec3(10, 50, 0.f), glm::vec3(100, 50, 0.f), ia_color::colors::blue.to_vec4(), 1.f);
}

mws_sp<mws_font> mws_camera::get_font()const
{
   return p->get_font();
}

void mws_camera::set_font(mws_sp<mws_font> i_font)
{
   p->set_font(i_font);
}

void mws_camera::drawText(const std::string& text, float x, float y, const mws_sp<mws_font> ifnt)
{
   if (enabled)
   {
      const mws_sp<mws_font> fnt = (ifnt) ? ifnt : p->get_font();
      p->d_text.push_data(draw_ops, text, x, y, fnt);
   }
}

void mws_camera::set_color(const gfx_color& i_color)
{
   p->color = i_color;
}

void mws_camera::push_transform_state()
{

}

void mws_camera::pop_transform_state()
{

}

void mws_camera::rotate(float angle)
{

}

void mws_camera::scale(float sx, float sy)
{

}

void mws_camera::translate(float tx, float ty)
{

}

mws_camera::mws_camera(mws_sp<gfx> i_gi) : gfx_camera(i_gi)
{
   sort_function = z_order_sort_function;
}

void mws_camera::load(mws_sp<gfx_camera> inst)
{
   gfx_camera::load(inst);
   p = mws_sp<mws_camera_impl>(new mws_camera_impl());
   projection_type = gfx_camera::e_orthographic_proj;
   rendering_priority = 0xffff;
}

mws_sp<text_vxo> mws_camera::get_text_vxo()
{
   return p->d_text.tx_vxo;
}

void mws_camera::set_text_blending(const std::string& i_blend_type)
{
   p->d_text.set_text_blending(i_blend_type);
}

void mws_camera::update_camera_state()
{
   p->d_text.on_update_start(draw_ctx);
   gfx_camera::update_camera_state();
   p->d_text.on_update_end(draw_ctx);
}

#endif
