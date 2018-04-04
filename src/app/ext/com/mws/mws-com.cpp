#include "stdafx.h"

#include "appplex-conf.hpp"

#if defined MOD_MWS

#include "mws-com.hpp"
#include "mws-camera.hpp"
#include "mws-font.hpp"
#include "unit.hpp"
#include "com/util/util.hpp"
#include "com/unit/transitions.hpp"
#include "glm/vec2.hpp"
#include <algorithm>

using std::string;
using std::vector;


mws_button::mws_button(shared_ptr<mws_page> iparent) : mws_page_item(iparent)
{
	color = gfx_color::colors::black;
	font = mws_font::new_inst(get_unit()->get_width() / 50.f);
	font->set_color(gfx_color::colors::white);
	set_text("n/a");
}

shared_ptr<mws_button> mws_button::new_instance(shared_ptr<mws_page> iparent)
{
	shared_ptr<mws_button> u(new mws_button(iparent));
	u->add_to_page();
	return u;
}

shared_ptr<mws_button> mws_button::new_shared_instance(mws_button* newButtonClassInstance)
{
	shared_ptr<mws_button> u(newButtonClassInstance);
	u->add_to_page();
	return u;
}

void mws_button::init(mws_rect i_mwsr, int iColor, string iText)
{
	mws_r = i_mwsr;
	color = gfx_color(iColor);

	set_text(iText);
}

void mws_button::receive(shared_ptr<iadp> idp)
{
	if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
	{
		shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

		switch(ts->get_type())
		{
		case touch_sym_evt::TS_FIRST_TAP:
			if(enabled && is_hit(ts->pressed.te->points[0].x, ts->pressed.te->points[0].y))
			{
				on_click();
				ts->process();
			}

			break;
		}
	}
}

void mws_button::on_click(){}
void mws_button::update_state(){}

void mws_button::update_view(shared_ptr<mws_camera> g)
{
	if(enabled && mws_r.w > 0 && mws_r.h > 0)
	{
		g->setColor(color.argb());
		g->fillRect(mws_r.x, mws_r.y, mws_r.w, mws_r.h);
		//draw_bar(g, mws_r.x, mws_r.y, mws_r.w, mws_r.h, color);
		g->drawText(text, mws_r.x + 10, mws_r.y + mws_r.h / 2 - 10, font);
	}
}

void mws_button::set_color(const gfx_color& icolor)
{
	color = icolor;
}

void mws_button::set_text(string iText)
{
	text = iText;
}


mws_list_model::mws_list_model()
{
	selected_elem = 0;
}

int mws_list_model::get_selected_elem()
{
	return selected_elem;
}

void mws_list_model::set_selected_elem(int iselectedElem)
{
	selected_elem = iselectedElem;
}


mws_list::mws_list(shared_ptr<mws_page> iparent) : mws_page_item(iparent)
{
	mws_r.set(0, 0, iparent->get_pos().w, iparent->get_pos().h);
	item_height = 150;
	vertical_space = 55;
	item_w = mws_r.w * 80 / 100;
	item_x = (mws_r.w - item_w) / 2;
}

shared_ptr<mws_list> mws_list::new_instance(shared_ptr<mws_page> iparent)
{
	shared_ptr<mws_list> u(new mws_list(iparent));
	u->add_to_page();
	return u;
}

shared_ptr<mws_list> mws_list::new_shared_instance(mws_list* newListClassInstance)
{
	shared_ptr<mws_list> u(newListClassInstance);
	u->add_to_page();
	return u;
}

void mws_list::init(){}

void mws_list::receive(shared_ptr<iadp> idp)
{
	if(idp->is_type(MWS_EVT_MODEL_UPDATE))
	{
		float listheight = 0;

		for (int k = 0; k < model->get_length(); k++)
		{
			listheight += item_height + vertical_space;
		}

		if(listheight > 0)
		{
			listheight -= vertical_space;
		}

		mws_r.h = listheight;
	}
	else if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
	{
		shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

		switch(ts->get_type())
		{
		case touch_sym_evt::TS_FIRST_TAP:
			{
				float x = ts->pressed.te->points[0].x;
				float y = ts->pressed.te->points[0].y;

				if(ts->tap_count == 1)
				{
					int idx = element_at(x, y);

					if(idx >= 0)
					{
						model->set_selected_elem(idx);
						model->on_elem_selected(idx);
					}

					ts->process();
				}

				break;
			}
		}
	}
}

void mws_list::update_state()
{
}

void mws_list::update_view(shared_ptr<mws_camera> g)
{
	if(!model)
	{
		return;
	}
	
	mws_rect pr = get_parent()->get_pos();
	int size = model->get_length();
	float vertOffset = pr.y;
	int selectedElem = model->get_selected_elem();

	for (int k = 0; k < size; k++)
	{
		if(k == selectedElem)
		{
			g->setColor(0xff7f00);
		}
		else
		{
			g->setColor(0x5000af);
		}

		g->fillRect(pr.x + item_x, vertOffset, item_w, item_height);
		g->drawText(model->elem_at(k), pr.x + item_x + item_w / 2, vertOffset + item_height / 2 - 10);
		vertOffset += item_height + vertical_space;
	}
}

void mws_list::set_model(shared_ptr<mws_list_model> imodel)
{
	model = imodel;
	model->set_view(get_instance());
}

shared_ptr<mws_list_model> mws_list::get_model()
{
	return model;
}

int mws_list::element_at(float x, float y)
{
	if(!is_hit(x, y))
	{
		return -1;
	}

	float vertOffset = get_parent()->get_pos().y;

	for (int k = 0; k < model->get_length(); k++)
	{
		if(is_inside_box(x, y, item_x, vertOffset, item_w, item_height))
		{
			return k;
		}

		vertOffset += item_height + vertical_space;
	}

	return -1;
}


mws_tree_model::mws_tree_model()
{
	length = 0;
}

void mws_tree_model::set_length(int ilength)
{
	length = ilength;
}

int mws_tree_model::get_length()
{
	return length;
}

void mws_tree_model::set_root_node(shared_ptr<mws_tree_model_node> iroot)
{
	root = iroot;
}

shared_ptr<mws_tree_model_node> mws_tree_model::get_root_node()
{
	return root;
}

mws_tree::mws_tree(shared_ptr<mws_page> iparent) : mws_page_item(iparent)
{
}

shared_ptr<mws_tree> mws_tree::new_instance(shared_ptr<mws_page> iparent)
{
	shared_ptr<mws_tree> u(new mws_tree(iparent));
	u->add_to_page();
	return u;
}

shared_ptr<mws_tree> mws_tree::new_shared_instance(mws_tree* newTreeClassInstance)
{
	shared_ptr<mws_tree> u(newTreeClassInstance);
	u->add_to_page();
	return u;
}

void mws_tree::init()
{
}

void mws_tree::receive(shared_ptr<iadp> idp)
{
	if(idp->is_type(MWS_EVT_MODEL_UPDATE))
	{
		float h = 25.f + model->get_length() * 20.f;
		float w = 0;

		if(model->get_root_node())
		{
			//shared_ptr<mws_font> f = gfx_openvg::get_instance()->getFont();
			//get_max_width(f, model->get_root_node(), 0, w);
		}

		mws_r.h = h;
		mws_r.w = w / 2;
	}
}

void mws_tree::update_state()
{
}

void mws_tree::update_view(shared_ptr<mws_camera> g)
{
	shared_ptr<mws_tree_model_node> node = model->get_root_node();

	if(node->nodes.size() > 0)
	{
		int elemIdx = 0;

		draw_tree_elem(g, node, 0, elemIdx);
	}
}

void mws_tree::set_model(shared_ptr<mws_tree_model> imodel)
{
	model = imodel;
	model->set_view(get_instance());
}

shared_ptr<mws_tree_model> mws_tree::get_model()
{
	return model;
}

void mws_tree::get_max_width(shared_ptr<mws_font> f, const shared_ptr<mws_tree_model_node> node, int level, float& maxWidth)
{
	int size = node->nodes.size();

	for (int k = 0; k < size; k++)
	{
		shared_ptr<mws_tree_model_node> kv = node->nodes[k];

		float textWidth = 0;//get_text_width(f, kv->data);
		float twidth = 25 + level * 20 + textWidth;

		if(twidth > maxWidth)
		{
			maxWidth = twidth;
		}

		if (kv->nodes.size() > 0)
		{
			get_max_width(f, kv, level + 1, maxWidth);
		}
	}
}

void mws_tree::draw_tree_elem(shared_ptr<mws_camera> g, const shared_ptr<mws_tree_model_node> node, int level, int& elemIdx)
{
	int size = node->nodes.size();
	mws_rect r = get_parent()->get_pos();

	for (int k = 0; k < size; k++)
	{
		shared_ptr<mws_tree_model_node> kv = node->nodes[k];
		glm::vec2 dim = g->get_font()->get_text_dim(kv->data);

		g->setColor(0xff00ff);
		g->drawRect(r.x + 20 + level * 20, r.y + 20 + elemIdx * dim.y, dim.x, dim.y);
		g->drawText(kv->data, r.x + 20 + level * 20, r.y + 20 + elemIdx * dim.y);
		elemIdx++;

		if (kv->nodes.size() > 0)
		{
			draw_tree_elem(g, kv, level + 1, elemIdx);
		}
	}
}

#endif
