#include "stdafx.hxx"

#include "mod-keyboard.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "mws/mws-text-box.hxx"
#include "fonts/mws-font.hxx"
#include "fonts/mws-font-db.hxx"


mod_keyboard::mod_keyboard() : mws_mod(mws_stringify(MOD_KEYBOARD)) {}

mws_sp<mod_keyboard> mod_keyboard::nwi()
{
   return mws_sp<mod_keyboard>(new mod_keyboard());
}

void mod_keyboard::init()
{
}


namespace mod_keyboard_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init() override
      {
         mws_page::init();

         // text-field
         {

            text_field = mws_text_field::nwi();
            text_field->set_position(glm::vec2(10, 10));
            text_field->set_size(glm::vec2(10, 10));
            text_field->set_editable(true);
            attach(text_field);

            text_field->on_click = [text_field = text_field, type_cmd_txt = type_cmd_txt]()
            {
               std::string txt = text_field->get_text();

               if (txt == type_cmd_txt)
               {
                  text_field->set_text("");
               }
            };
            text_field->on_gained_focus = text_field->on_click;
            text_field->on_lost_focus = [text_field = text_field, type_cmd_txt = type_cmd_txt]()
            {
               std::string txt = text_field->get_text();

               if (txt.empty())
               {
                  text_field->set_text(type_cmd_txt);
               }
            };
            text_field->on_action = [text_field = text_field]()
            {
               std::string txt = text_field->get_text();

               if (!txt.empty())
               {
                  text_field->set_text("");
               }
            };
            text_field->set_text(type_cmd_txt);

            if (!mws::input::has_touchscreen())
            {
               text_field->set_focus();
            }
         }
      }

      virtual void receive(mws_sp<mws_dp> i_dp) override
      {
         if (i_dp->is_type(mws_key_evt::key_evt_type))
         {
            mws_sp<mws_key_evt> ke = mws_key_evt::as_key_evt(i_dp);

            if (ke->get_type() == mws_key_evt::ke_released)
            {
               //mws_println("key [ %d ]", ke->get_key());
            }
         }

         if (!i_dp->is_processed())
         {
            mws_page::receive(i_dp);
         }
      }

      virtual void update_state() override
      {
         mws_page::update_state();
      }

      virtual void update_view(mws_sp<mws_camera> i_g) override
      {
         mws_sp<mws_mod> mod = get_mod();
         bool alt_held = mod->key_ctrl_inst->key_is_held(mws_key_alt);
         bool ctrl_held = mod->key_ctrl_inst->key_is_held(mws_key_control);
         bool shift_held = mod->key_ctrl_inst->key_is_held(mws_key_shift);
         float x_off = 30.f;
         float y_off = 0.f;
         float y_add = 35.f;
         std::string keys_held;

         mws_page::update_view(i_g);

         i_g->drawText("keyboard-test", x_off, y_off);
         y_off += y_add;

         if (alt_held)
         {
            i_g->drawText("alt held", x_off, y_off);
         }
         y_off += y_add;

         if (ctrl_held)
         {
            i_g->drawText("ctrl held", x_off, y_off);
         }
         y_off += y_add;

         if (shift_held)
         {
            i_g->drawText("shift held", x_off, y_off);
         }
         y_off += y_add;

         for (uint32 k = mws_key_invalid + 1; k < mws_key_count; k++)
         {
            mws_key_types key = (mws_key_types)k;

            if (mod->key_ctrl_inst->key_is_held(key))
            {
               keys_held += std::to_string(key) + ", ";
            }
         }

         if (!keys_held.empty())
         {
            i_g->drawText("keys held: " + keys_held, x_off, y_off);
         }
         y_off += y_add;
      }

      virtual void on_resize() override
      {
         mws_page::on_resize();
         // mws
         float width = mws_r.w;
         float height = mws_r.h;
         float w = 100;
         float h = 100;
         float w2 = w / 2.f;
         float h2 = h / 2.f;
         mws_rect ui_rect;

         float tf_height = 50.f;
         text_field->set_position(glm::vec2(20, height - 2 * tf_height + 20 * 2));
         text_field->set_size(glm::vec2(width - 20 * 2.f, tf_height));
      }

      mws_sp<mws_text_field> text_field;
      std::string type_cmd_txt = "click here to type";
      mws_sp<mws_font> main_font;
   };
}


void mod_keyboard::init_mws()
{
   mws_root->new_page<mod_keyboard_ns::main_page>();
   mws_cam->clear_color = true;
}

void mod_keyboard::load()
{
}
