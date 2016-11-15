#include "stdafx.h"

#include "unit-texxed.hpp"

#ifdef UNIT_TEXXED

#include "gap-buffer.hpp"
#include "com/ux/ux-camera.hpp"
#include "com/ux/ux-com.hpp"
#include "com/ux/ux-font.hpp"
#include "com/ux/font-db.hpp"
#include "com/ux/text-vxo.hpp"
#include "min.hpp"


namespace unit_texxed_ns
{
	class text_area_impl : public ux_page_item
	{
	public:
		static shared_ptr<text_area_impl> new_inst(shared_ptr<ux_page> iparent)
		{
			shared_ptr<text_area_impl> inst(new text_area_impl(iparent));
			inst->add_to_page();
			return inst;
		}

		virtual void set_text(const std::string& itext)
		{
			tx_src = std::make_shared<GapBuffer>(itext);
			tx_vxo->clear_text();
			//tx_vxo->add_text(itext, glm::vec2(), font);
			scroll_text(glm::vec2());
			select_char_at(glm::vec2());
		}

		virtual void scroll_text(const glm::vec2& ioff)
		{
			//tx_vxo->position += glm::vec3(ioff, 0);
			glm::vec2 prev_off = text_offset;
			text_offset -= ioff;
			text_offset = glm::max(text_offset, glm::vec2());
			tx_vxo->clear_text();
			int line_idx = text_offset.y / font->get_height();

			if (line_idx + text_rows > tx_src->get_line_count())
			{
				text_offset = prev_off;
				line_idx = text_offset.y / font->get_height();
				//line_idx = tx_src->get_line_count() - text_rows;
			}

			tx_rows = tx_src->get_lines_at(line_idx, text_rows);
			float rem = glm::mod(text_offset.y, font->get_height());
			tx_vxo->position = glm::vec3(pos.x - text_offset.x, pos.y - rem, 0);

			for (int k = 0; k < tx_rows.size(); k++)
			{
				tx_vxo->add_text(tx_rows[k], glm::vec2(0, k * font->get_height()), font);
			}
		}

		virtual void set_position(const glm::vec2& ipos)
		{
			pos = ipos;
			tx_vxo->position = glm::vec3(ipos, 0);
			(*tx_vxo)[MP_SCISSOR_AREA] = glm::vec4(pos, dim);
		}

		virtual void set_dimension(const glm::vec2& idim)
		{
			dim = idim;
			(*tx_vxo)[MP_SCISSOR_AREA] = glm::vec4(pos, dim);
			text_rows = dim.y / font->get_height() + 1;
			uxr = ux_rect(pos.x, pos.y, dim.x, dim.y);
		}

		virtual void select_char_at(const glm::vec2& ipos)
		{
			float rem = glm::mod(text_offset.y, font->get_height());
			int row_idx = (ipos.y + rem) / font->get_height();
			std::string& text = tx_rows[row_idx];
			auto& glyphs = font_db::inst()->get_glyph_vect(font->get_inst(), text);
			float x_off = -text_offset.x;
			int k = 0;

			for (k = 0; k < text.length(); k++)
			{
				font_glyph glyph = glyphs[k];

				if (glyph.is_valid())
				{
					float span = 0;
					char ch = text[k];
					if (ch == '\t')
					{
						span = 2 * font->get_height();
					}
					else
					{
						float kerning = 0.0f;
						if (k > 0)
						{
							kerning = glyph.get_kerning(text[k - 1]);
						}
						span = kerning + glyph.get_advance_x();
					}

					if (x_off + span > ipos.x)
					{
						break;
					}
					else
					{
						x_off += span;
					}
				}
			}

			if (k < text.length())
			{
				char c = text[k];
				vprint("%c %d %d\n", c, row_idx, k);
				select_char_rect.x = x_off;
				select_char_rect.y = row_idx * font->get_height() - rem;
				select_char_rect.z = (glyphs.size() > k) ? glyphs[k].get_advance_x() : 0;
				select_char_rect.w = font->get_height();
			}
			else
			{
				select_char_rect = glm::vec4();
			}
		}

		virtual void update_state()
		{
			if (ks.is_active())
			{
				point2d scroll_pos = ks.update();
				scroll_text(glm::vec2(scroll_pos.x, scroll_pos.y));
			}
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			g->drawRect(pos.x, pos.y, dim.x, dim.y);
			g->drawRect(select_char_rect.x + pos.x, select_char_rect.y + pos.y, select_char_rect.z, select_char_rect.w);
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			if (idp->is_processed())
			{
				return;
			}

			if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
			{
				shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

				float x = ts->crt_state.pos.x - uxr.x;
				float y = ts->crt_state.pos.y - uxr.y;

				switch (ts->get_type())
				{
				case touch_sym_evt::TS_PRESSED:
				{
					ks.grab(x, y);
					select_char_at(glm::vec2(x, y));
					ts->process();
					break;
				}

				case touch_sym_evt::TS_RELEASED:
				{
					ts->process();
					break;
				}

				case touch_sym_evt::TS_PRESS_AND_DRAG:
				{
					float dx = ts->crt_state.pos.x - ts->prev_state.pos.x;
					float dy = ts->crt_state.pos.y - ts->prev_state.pos.y;

					if (ts->is_finished)
					{
						ks.start_slowdown();
					}
					else
					{
						ks.begin(x, y);
					}
					scroll_text(glm::vec2(dx, dy));

					ts->process();
					break;
				}
				}
			}
			else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
			{
				shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);
				float off = 51.175;

				if (ke->get_type() != key_evt::KE_RELEASED)
				{
					if (ke->get_type() == key_evt::KE_PRESSED)
					{
						off = 21.175;
					}

					switch (ke->get_key())
					{
					case KEY_UP:
					{
						scroll_text(glm::vec2(0, off));
						break;
					}

					case KEY_RIGHT:
					{
						scroll_text(glm::vec2(-off, 0));
						break;
					}

					case KEY_DOWN:
					{
						scroll_text(glm::vec2(0, -off));
						break;
					}

					case KEY_LEFT:
					{
						scroll_text(glm::vec2(off, 0));
						break;
					}
					}
				}
			}

			//ux_page::receive(idp);
		}

	protected:
		text_area_impl(shared_ptr<ux_page> iparent) : ux_page_item(iparent)
		{
			tx_vxo = text_vxo::new_inst();
			iparent->get_unit()->gfx_scene_inst->attach(tx_vxo);
			tx_vxo->camera_id_list.push_back(get_unit()->ux_cam->camera_id);
			(*tx_vxo)[MP_SCISSOR_ENABLED] = true;
			font = ux_font::new_inst(48);
		}

		shared_ptr<text_area_model> tx_src;
		shared_ptr<text_vxo> tx_vxo;
		shared_ptr<ux_font> font;
		glm::vec2 pos;
		glm::vec2 dim;
		glm::vec2 text_offset;
		int text_rows;
		kinetic_scrolling ks;
		std::vector<std::string> tx_rows;
		glm::vec4 select_char_rect;
	};


	class main_page : public ux_page
	{
	public:
		main_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			ux_page::init();

			int w = get_unit()->get_width();
			int h = get_unit()->get_height();

			tx_res = pfm::filesystem::load_res_as_string("kappaxx-src.sbmd");

			if (!tx_res)
			{
				trx("error: kappaxx-src.sbmd not found.");

				return;
			}

			ta = text_area_impl::new_inst(get_ux_page_instance());
			ta->set_position(glm::vec2(50, 70));
			ta->set_dimension(glm::vec2(w - 80, h - 130));
			ta->set_text(*tx_res);

			shared_ptr<ux_button> b;
			b = ux_button::new_instance(get_ux_page_instance());
			b->init(ux_rect(10, h - 50, 50, 20), 0x8200b4, "btn");
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			if (idp->is_processed())
			{
				return;
			}
			
			if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
			{
				ta->receive(idp);
			}

			ux_page::receive(idp);
		}

		virtual void update_state()
		{
			ux_page::update_state();
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			ux_page::update_view(g);

			std::string text = "texxed : texxt-editor";

			g->drawText(text, 10, 20);
		}

		shared_ptr<text_area_impl> ta;
		shared_ptr<std::string > tx_res;
	};
}


unit_texxed::unit_texxed()
{
	set_name("texxed");
}

shared_ptr<unit_texxed> unit_texxed::new_instance()
{
	return shared_ptr<unit_texxed>(new unit_texxed());
}

void unit_texxed::init()
{
}

void unit_texxed::init_ux()
{
	ux_page::new_shared_instance(new unit_texxed_ns::main_page(uxroot));
	ux_cam->clear_color = true;
	//ux_cam->clear_color_value = gfx_color::colors::white;
}

void unit_texxed::load()
{
}

#endif
