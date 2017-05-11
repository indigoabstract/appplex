#include "stdafx.h"

#include "ux-com.hpp"
#include "ux-camera.hpp"
#include "ux-font.hpp"
#include "unit.hpp"
#include "com/util/util.hpp"
#include "com/unit/transitions.hpp"
#include "glm/vec2.hpp"
#include <algorithm>

using std::string;
using std::vector;


ux_button::ux_button(shared_ptr<ux_page> iparent) : ux_page_item(iparent)
{
	color = gfx_color::colors::black;
	font = ux_font::new_inst(get_unit()->get_width() / 50.f);
	font->set_color(gfx_color::colors::white);
	set_text("n/a");
}

shared_ptr<ux_button> ux_button::new_instance(shared_ptr<ux_page> iparent)
{
	shared_ptr<ux_button> u(new ux_button(iparent));
	u->add_to_page();
	return u;
}

shared_ptr<ux_button> ux_button::new_shared_instance(ux_button* newButtonClassInstance)
{
	shared_ptr<ux_button> u(newButtonClassInstance);
	u->add_to_page();
	return u;
}

void ux_button::init(ux_rect iuxr, int iColor, string iText)
{
	uxr = iuxr;
	color = gfx_color(iColor);

	set_text(iText);
}

void ux_button::receive(shared_ptr<iadp> idp)
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

void ux_button::on_click(){}
void ux_button::update_state(){}

void ux_button::update_view(shared_ptr<ux_camera> g)
{
	if(enabled && uxr.w > 0 && uxr.h > 0)
	{
		g->setColor(color.argb());
		g->fillRect(uxr.x, uxr.y, uxr.w, uxr.h);
		//draw_bar(g, uxr.x, uxr.y, uxr.w, uxr.h, color);
		g->drawText(text, uxr.x + 10, uxr.y + uxr.h / 2 - 10, font);
	}
}

void ux_button::set_color(const gfx_color& icolor)
{
	color = icolor;
}

void ux_button::set_text(string iText)
{
	text = iText;
}


ux_list_model::ux_list_model()
{
	selected_elem = 0;
}

int ux_list_model::get_selected_elem()
{
	return selected_elem;
}

void ux_list_model::set_selected_elem(int iselectedElem)
{
	selected_elem = iselectedElem;
}


ux_list::ux_list(shared_ptr<ux_page> iparent) : ux_page_item(iparent)
{
	uxr.set(0, 0, iparent->get_pos().w, iparent->get_pos().h);
	item_height = 150;
	vertical_space = 55;
	item_w = uxr.w * 80 / 100;
	item_x = (uxr.w - item_w) / 2;
}

shared_ptr<ux_list> ux_list::new_instance(shared_ptr<ux_page> iparent)
{
	shared_ptr<ux_list> u(new ux_list(iparent));
	u->add_to_page();
	return u;
}

shared_ptr<ux_list> ux_list::new_shared_instance(ux_list* newListClassInstance)
{
	shared_ptr<ux_list> u(newListClassInstance);
	u->add_to_page();
	return u;
}

void ux_list::init(){}

void ux_list::receive(shared_ptr<iadp> idp)
{
	if(idp->is_type(UX_EVT_MODEL_UPDATE))
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

		uxr.h = listheight;
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

void ux_list::update_state()
{
}

void ux_list::update_view(shared_ptr<ux_camera> g)
{
	if(!model)
	{
		return;
	}
	
	ux_rect pr = get_parent()->get_pos();
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

void ux_list::set_model(shared_ptr<ux_list_model> imodel)
{
	model = imodel;
	model->set_view(get_instance());
}

shared_ptr<ux_list_model> ux_list::get_model()
{
	return model;
}

int ux_list::element_at(float x, float y)
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


ux_tree_model::ux_tree_model()
{
	length = 0;
}

void ux_tree_model::set_length(int ilength)
{
	length = ilength;
}

int ux_tree_model::get_length()
{
	return length;
}

void ux_tree_model::set_root_node(shared_ptr<ux_tree_model_node> iroot)
{
	root = iroot;
}

shared_ptr<ux_tree_model_node> ux_tree_model::get_root_node()
{
	return root;
}

ux_tree::ux_tree(shared_ptr<ux_page> iparent) : ux_page_item(iparent)
{
}

shared_ptr<ux_tree> ux_tree::new_instance(shared_ptr<ux_page> iparent)
{
	shared_ptr<ux_tree> u(new ux_tree(iparent));
	u->add_to_page();
	return u;
}

shared_ptr<ux_tree> ux_tree::new_shared_instance(ux_tree* newTreeClassInstance)
{
	shared_ptr<ux_tree> u(newTreeClassInstance);
	u->add_to_page();
	return u;
}

void ux_tree::init()
{
}

void ux_tree::receive(shared_ptr<iadp> idp)
{
	if(idp->is_type(UX_EVT_MODEL_UPDATE))
	{
		float h = 25 + model->get_length() * 20;
		float w = 0;

		if(model->get_root_node())
		{
			//shared_ptr<ux_font> f = gfx_openvg::get_instance()->getFont();
			//get_max_width(f, model->get_root_node(), 0, w);
		}

		uxr.h = h;
		uxr.w = w / 2;
	}
}

void ux_tree::update_state()
{
}

void ux_tree::update_view(shared_ptr<ux_camera> g)
{
	shared_ptr<ux_tree_model_node> node = model->get_root_node();

	if(node->nodes.size() > 0)
	{
		int elemIdx = 0;

		draw_tree_elem(g, node, 0, elemIdx);
	}
}

void ux_tree::set_model(shared_ptr<ux_tree_model> imodel)
{
	model = imodel;
	model->set_view(get_instance());
}

shared_ptr<ux_tree_model> ux_tree::get_model()
{
	return model;
}

void ux_tree::get_max_width(shared_ptr<ux_font> f, const shared_ptr<ux_tree_model_node> node, int level, float& maxWidth)
{
	int size = node->nodes.size();

	for (int k = 0; k < size; k++)
	{
		shared_ptr<ux_tree_model_node> kv = node->nodes[k];

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

void ux_tree::draw_tree_elem(shared_ptr<ux_camera> g, const shared_ptr<ux_tree_model_node> node, int level, int& elemIdx)
{
	int size = node->nodes.size();
	ux_rect r = get_parent()->get_pos();

	for (int k = 0; k < size; k++)
	{
		shared_ptr<ux_tree_model_node> kv = node->nodes[k];
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
