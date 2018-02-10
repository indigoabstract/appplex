#pragma once

// mws - multi-widget-set (as in, opposed to, say, a one-widget-set :) )
#include "pfm.hpp"
#include "com/util/util.hpp"
#include "com/unit/input-ctrl.hpp"
#include <unordered_map>
#include <exception>
#include <string>
#include <vector>


class unitctrl;
class linear_transition;
class ms_linear_transition;
class unit;
class unit;
class ux;
class ux_page_tab;
class ux_page;
class ux_page_item;
class ux_button;
class ux_list;
class ux_tree;
class ux_font;
class ux_camera;


const std::string UX_EVT_MODEL_UPDATE					= "ux-model-update";
const std::string UX_EVT_PAGE_TRANSITION				= "ux-page-transition";


slide_scrolling::scroll_dir get_scroll_dir(touch_sym_evt::touch_sym_evt_types swipe_type);


class ux_rect
{
public:
	ux_rect()
	{
		set(0, 0, 0, 0);
	}

	ux_rect(float ix, float iy, float iw, float ih)
	{
		set(ix, iy, iw, ih);
	}

	void set(float ix, float iy, float iw, float ih)
	{
		x = ix;
		y = iy;
		w = iw;
		h = ih;
	}

	float x, y, w, h;
};


class ux_model : public enable_shared_from_this<ux_model>, public ia_node
{
public:
	virtual ~ux_model(){}
	shared_ptr<ux_model> get_instance();

	virtual void receive(shared_ptr<iadp> idp);
	virtual void notify_update();
	void set_view(shared_ptr<ux> iview);
	shared_ptr<ux> get_view();

protected:
	ux_model(){}

	shared_ptr<ia_sender> sender_inst();

	weak_ptr<ux> view;
};


class ux : public enable_shared_from_this<ux>, public ia_node
{
public:
	virtual void init(){}
	virtual void on_destroy(){}
	virtual ~ux(){}
	shared_ptr<ux> get_instance();

	void set_visible(bool iis_visible);
	bool is_visible()const;
	void set_id(std::string iid);
	const std::string& get_id();
	virtual shared_ptr<ux> find_by_id(const std::string& iid);
	virtual shared_ptr<ux> contains_id(const std::string& iid);
	virtual bool contains_ux(const shared_ptr<ux> iux);
	shared_ptr<ux> get_parent();
	shared_ptr<ux_page_tab> get_root();
	virtual shared_ptr<unit> get_unit();

	virtual void receive(shared_ptr<iadp> idp);
	virtual void update_state();
	virtual void update_view(shared_ptr<ux_camera> g);
	ux_rect get_pos();
	virtual bool is_hit(float x, float y);

protected:
	ux();
	ux(shared_ptr<ux> iparent);

	bool enabled;
	bool is_opaque;
	ux_rect uxr;
	weak_ptr<ux> parent;
	weak_ptr<ux_page_tab> root;

private:
	virtual shared_ptr<ia_sender> sender_inst();

	std::string id;
};


class ux_page_transition : public enable_shared_from_this<ux_page_transition>, public iadp
{
public:
	enum page_transition_types
	{
		REPLACE_CURRENT_PAGE,
		PUSH_CURRENT_PAGE,
		POP_CURRENT_PAGE,
		CLEAR_PAGE_STACK,
	};

	enum page_jump_types
	{
		HISTORY_ADD_PAGE,
		HISTORY_REWIND_TO_PAGE,
		HISTORY_IGNORE_PAGE,
	};

	virtual ~ux_page_transition(){}
	static shared_ptr<ux_page_transition> new_instance(shared_ptr<ux_page> ipage);
	static shared_ptr<ux_page_transition> new_instance(shared_ptr<ux_page_tab> iuxroot, std::string iid);

	shared_ptr<ux_page> get_target_page();
	slide_scrolling::scroll_dir get_scroll_dir();
	page_transition_types get_transition_type();
	page_jump_types get_jump_type();

	shared_ptr<ux_page_transition> set_scroll_dir(slide_scrolling::scroll_dir idir);
	shared_ptr<ux_page_transition> set_transition_type(page_transition_types iptType);
	shared_ptr<ux_page_transition> set_jump_type(page_jump_types ipjType);

protected:
	ux_page_transition(shared_ptr<ux_page> ipage);
	ux_page_transition(shared_ptr<ux_page_tab> iuxroot, std::string iid = "");

	shared_ptr<ux_page_transition> get_instance();

	weak_ptr<ux_page> page;
	weak_ptr<ux_page_tab> uxroot;
	std::string id;
	slide_scrolling::scroll_dir dir;
	page_transition_types pt_type;
	page_jump_types pj_type;
};


class ux_transition_ctrl
{
public:
};


class ux_page_tab : public ux
{
public:
	static const std::string VKEYBOARD_MAIN_PAGE;
	static const std::string VKEYBOARD_UP_PAGE;
	static const std::string VKEYBOARD_RIGHT_PAGE;
	static const std::string VKEYBOARD_DOWN_PAGE;

	virtual ~ux_page_tab(){}
	static shared_ptr<ux_page_tab> new_instance(shared_ptr<unit> iu);
	static shared_ptr<ux_page_tab> new_shared_instance(ux_page_tab* inew_page_tab_class_instance);
	virtual void init();
	virtual void on_destroy();

	virtual shared_ptr<ux> contains_id(const std::string& iid);
	virtual bool contains_ux(const shared_ptr<ux> iux);
	shared_ptr<ux_page_tab> get_ux_page_tab_instance();
	virtual shared_ptr<unit> get_unit();
	bool is_empty();

	virtual void receive(shared_ptr<iadp> idp);
	virtual void update_state();
	virtual void update_view(shared_ptr<ux_camera> g);
	shared_ptr<ux_page> get_page_at(int idx);
	void set_first_page(shared_ptr<ux_page> up);
	void show_vkeyboard();
	virtual void on_resize();

protected:
	ux_page_tab(shared_ptr<unit> iu);

private:
	friend class unitctrl;
	friend class ux_page;

	void add(shared_ptr<ux_page> p);
	int get_page_index(shared_ptr<ux_page> p);
	void new_instance_helper();

	shared_ptr<ux_page_transition> current_transition;
	std::vector<shared_ptr<ux_page> > page_stack;
	std::vector<shared_ptr<ux_page> > pages;
	shared_ptr<ux_page> current_page;
	shared_ptr<ux_page> last_page;
	std::vector<shared_ptr<ux_page> > page_history;
	slide_scrolling ss;
	weak_ptr<unit> u;
};


class ux_page : public ux
{
public:
	static const shared_ptr<ux_page> PAGE_NONE;
	static const shared_ptr<ux_page> PREV_PAGE;
	static const shared_ptr<ux_page> NEXT_PAGE;

	virtual ~ux_page(){}
	static shared_ptr<ux_page> new_instance(shared_ptr<ux_page_tab> iparent);
	static shared_ptr<ux_page> new_shared_instance(ux_page* inew_page_class_instance);
	virtual void init();
	virtual void on_destroy();

	virtual shared_ptr<ux> contains_id(const std::string& iid);
	virtual bool contains_ux(const shared_ptr<ux> iux);
	shared_ptr<ux_page> get_ux_page_instance();
	shared_ptr<ux_page_tab> get_ux_page_parent();

	virtual void on_visibility_changed(bool iis_visible);
	virtual void on_show_transition(const shared_ptr<linear_transition> itransition);
	virtual void on_hide_transition(const shared_ptr<linear_transition> itransition);

	virtual void receive(shared_ptr<iadp> idp);
	virtual void update_input_subux(shared_ptr<iadp> idp);
	virtual void update_input_std_behaviour(shared_ptr<iadp> idp);
	virtual void update_state();
	virtual void update_view(shared_ptr<ux_camera> g);
	shared_ptr<ux> get_ux_at(int idx);

	std::unordered_map<int, std::shared_ptr<ux_page_transition> > tmap;

protected:
	ux_page();
	ux_page(shared_ptr<ux_page_tab> iparent);
	virtual void on_resize();

private:
	friend class ux_page_tab;
	friend class ux_page_item;

	static shared_ptr<ux_page> new_standalone_instance();
	void add(shared_ptr<ux_page_item> b);

	std::vector<shared_ptr<ux_page_item> > mlist;
	kinetic_scrolling ks;
};


class ux_page_item : public ux
{
public:
	virtual ~ux_page_item(){}

	shared_ptr<ux_page> get_ux_page_item_parent();

protected:
	ux_page_item(shared_ptr<ux_page> iparent);

	void add_to_page();
};
