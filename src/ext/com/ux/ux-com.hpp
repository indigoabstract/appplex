#pragma once

#include "ux.hpp"
#include "gfx-color.hpp"

class ux_font;


class ux_button : public ux_page_item
{
public:
	virtual ~ux_button(){}
	static shared_ptr<ux_button> new_instance(shared_ptr<ux_page> iparent);
	static shared_ptr<ux_button> new_shared_instance(ux_button* newButtonClassInstance);
	virtual void init(ux_rect iuxr, int icolor, std::string itext = "n/a");

	virtual void receive(shared_ptr<iadp> idp);

	virtual void on_click();
	virtual void update_state();
	virtual void update_view(shared_ptr<ux_camera> g);
	void set_color(const gfx_color& icolor);
	void set_text(std::string iText);

protected:
	ux_button(shared_ptr<ux_page> iparent);

	std::string text;
	gfx_color color;
	shared_ptr<ux_font> font;
};


class ux_list_model : public ux_model
{
public:
	ux_list_model();
	virtual ~ux_list_model(){}

	virtual int get_length() = 0;
	virtual std::string elem_at(int idx) = 0;
	virtual int get_selected_elem();
	virtual void set_selected_elem(int iselectedElem);
	virtual void on_elem_selected(int idx) = 0;

protected:
	int selected_elem;
};


class ux_list : public ux_page_item
{
public:
	virtual ~ux_list(){}
	static shared_ptr<ux_list> new_instance(shared_ptr<ux_page> iparent);
	static shared_ptr<ux_list> new_shared_instance(ux_list* new_instance);
	virtual void init();

	virtual void receive(shared_ptr<iadp> idp);

	virtual void update_state();
	virtual void update_view(shared_ptr<ux_camera> g);
	void set_model(shared_ptr<ux_list_model> imodel);
	shared_ptr<ux_list_model> get_model();

protected:
	ux_list(shared_ptr<ux_page> iparent);

	int element_at(float x, float y);

	float item_height;
	float vertical_space;
	float item_w;
	float item_x;
	shared_ptr<ux_list_model> model;
};


class ux_tree_model_node
{
public:
	ux_tree_model_node(const std::string& idata) : data(idata){}
	virtual ~ux_tree_model_node(){}

	std::vector<shared_ptr<ux_tree_model_node> > nodes;
	std::string data;
};


class ux_tree_model : public ux_model
{
public:
	ux_tree_model();
	virtual ~ux_tree_model(){}

	virtual void set_length(int ilength);
	virtual int get_length();
	virtual void set_root_node(shared_ptr<ux_tree_model_node> iroot);
	virtual shared_ptr<ux_tree_model_node> get_root_node();

protected:
	int length;
	shared_ptr<ux_tree_model_node> root;
};


class ux_tree : public ux_page_item
{
public:
	virtual ~ux_tree(){}
	static shared_ptr<ux_tree> new_instance(shared_ptr<ux_page> iparent);
	static shared_ptr<ux_tree> new_shared_instance(ux_tree* newTreeClassInstance);
	virtual void init();

	virtual void receive(shared_ptr<iadp> idp);

	virtual void update_state();
	virtual void update_view(shared_ptr<ux_camera> g);
	void set_model(shared_ptr<ux_tree_model> imodel);
	shared_ptr<ux_tree_model> get_model();

protected:
	ux_tree(shared_ptr<ux_page> iparent);

	void get_max_width(shared_ptr<ux_font> f, const shared_ptr<ux_tree_model_node> node, int level, float& maxWidth);
	void draw_tree_elem(shared_ptr<ux_camera> g, const shared_ptr<ux_tree_model_node> node, int level, int& elemIdx);

	shared_ptr<ux_tree_model> model;
};


class text_area_model
{
public:
	virtual int get_line_count() = 0;
	virtual std::string get_line_at(int iidx) = 0;
	virtual std::vector<std::string> get_lines_at(int iidx, int iline_count) = 0;
};
