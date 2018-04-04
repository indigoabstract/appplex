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
class mws;
class mws_page_tab;
class mws_page;
class mws_page_item;
class mws_button;
class mws_list;
class mws_tree;
class mws_font;
class mws_camera;


const std::string MWS_EVT_MODEL_UPDATE					= "mws-model-update";
const std::string MWS_EVT_PAGE_TRANSITION				= "mws-page-transition";


slide_scrolling::scroll_dir get_scroll_dir(touch_sym_evt::touch_sym_evt_types swipe_type);


class mws_rect
{
public:
	mws_rect()
	{
		set(0, 0, 0, 0);
	}

	mws_rect(float ix, float iy, float iw, float ih)
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


class mws_model : public enable_shared_from_this<mws_model>, public ia_node
{
public:
	virtual ~mws_model(){}
	shared_ptr<mws_model> get_instance();

	virtual void receive(shared_ptr<iadp> idp);
	virtual void notify_update();
	void set_view(shared_ptr<mws> iview);
	shared_ptr<mws> get_view();

protected:
	mws_model(){}

	shared_ptr<ia_sender> sender_inst();

	weak_ptr<mws> view;
};


class mws : public enable_shared_from_this<mws>, public ia_node
{
public:
	virtual void init(){}
	virtual void on_destroy(){}
	virtual ~mws(){}
	shared_ptr<mws> get_instance();

	void set_visible(bool iis_visible);
	bool is_visible()const;
	void set_id(std::string iid);
	const std::string& get_id();
	virtual shared_ptr<mws> find_by_id(const std::string& iid);
	virtual shared_ptr<mws> contains_id(const std::string& iid);
	virtual bool contains_mws(const shared_ptr<mws> i_mws);
	shared_ptr<mws> get_parent();
	shared_ptr<mws_page_tab> get_root();
	virtual shared_ptr<unit> get_unit();

	virtual void receive(shared_ptr<iadp> idp);
	virtual void update_state();
	virtual void update_view(shared_ptr<mws_camera> g);
	mws_rect get_pos();
	virtual bool is_hit(float x, float y);

protected:
	mws();
	mws(shared_ptr<mws> iparent);

	bool enabled;
	bool is_opaque;
	mws_rect mws_r;
	weak_ptr<mws> parent;
	weak_ptr<mws_page_tab> root;

private:
	virtual shared_ptr<ia_sender> sender_inst();

	std::string id;
};


class mws_page_transition : public enable_shared_from_this<mws_page_transition>, public iadp
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

	virtual ~mws_page_transition(){}
	static shared_ptr<mws_page_transition> new_instance(shared_ptr<mws_page> ipage);
	static shared_ptr<mws_page_transition> new_instance(shared_ptr<mws_page_tab> imws_root, std::string iid);

	shared_ptr<mws_page> get_target_page();
	slide_scrolling::scroll_dir get_scroll_dir();
	page_transition_types get_transition_type();
	page_jump_types get_jump_type();

	shared_ptr<mws_page_transition> set_scroll_dir(slide_scrolling::scroll_dir idir);
	shared_ptr<mws_page_transition> set_transition_type(page_transition_types iptType);
	shared_ptr<mws_page_transition> set_jump_type(page_jump_types ipjType);

protected:
	mws_page_transition(shared_ptr<mws_page> ipage);
	mws_page_transition(shared_ptr<mws_page_tab> imws_root, std::string iid = "");

	shared_ptr<mws_page_transition> get_instance();

	weak_ptr<mws_page> page;
	weak_ptr<mws_page_tab> mws_root;
	std::string id;
	slide_scrolling::scroll_dir dir;
	page_transition_types pt_type;
	page_jump_types pj_type;
};


class mws_transition_ctrl
{
public:
};


class mws_page_tab : public mws
{
public:
	static const std::string VKEYBOARD_MAIN_PAGE;
	static const std::string VKEYBOARD_UP_PAGE;
	static const std::string VKEYBOARD_RIGHT_PAGE;
	static const std::string VKEYBOARD_DOWN_PAGE;

	virtual ~mws_page_tab(){}
	static shared_ptr<mws_page_tab> new_instance(shared_ptr<unit> iu);
	static shared_ptr<mws_page_tab> new_shared_instance(mws_page_tab* inew_page_tab_class_instance);
	virtual void init();
	virtual void on_destroy();

	virtual shared_ptr<mws> contains_id(const std::string& iid);
	virtual bool contains_mws(const shared_ptr<mws> i_mws);
	shared_ptr<mws_page_tab> get_mws_page_tab_instance();
	virtual shared_ptr<unit> get_unit();
	bool is_empty();

	virtual void receive(shared_ptr<iadp> idp);
	virtual void update_state();
	virtual void update_view(shared_ptr<mws_camera> g);
	shared_ptr<mws_page> get_page_at(int idx);
	void set_first_page(shared_ptr<mws_page> up);
	void show_vkeyboard();
	virtual void on_resize();

protected:
	mws_page_tab(shared_ptr<unit> iu);

private:
	friend class unitctrl;
	friend class mws_page;

	void add(shared_ptr<mws_page> p);
	int get_page_index(shared_ptr<mws_page> p);
	void new_instance_helper();

	shared_ptr<mws_page_transition> current_transition;
	std::vector<shared_ptr<mws_page> > page_stack;
	std::vector<shared_ptr<mws_page> > pages;
	shared_ptr<mws_page> current_page;
	shared_ptr<mws_page> last_page;
	std::vector<shared_ptr<mws_page> > page_history;
	slide_scrolling ss;
	weak_ptr<unit> u;
};


class mws_page : public mws
{
public:
	static const shared_ptr<mws_page> PAGE_NONE;
	static const shared_ptr<mws_page> PREV_PAGE;
	static const shared_ptr<mws_page> NEXT_PAGE;

	virtual ~mws_page(){}
	static shared_ptr<mws_page> new_instance(shared_ptr<mws_page_tab> iparent);
	static shared_ptr<mws_page> new_shared_instance(mws_page* inew_page_class_instance);
	virtual void init();
	virtual void on_destroy();

	virtual shared_ptr<mws> contains_id(const std::string& iid);
	virtual bool contains_mws(const shared_ptr<mws> i_mws);
	shared_ptr<mws_page> get_mws_page_instance();
	shared_ptr<mws_page_tab> get_mws_page_parent();

	virtual void on_visibility_changed(bool iis_visible);
	virtual void on_show_transition(const shared_ptr<linear_transition> itransition);
	virtual void on_hide_transition(const shared_ptr<linear_transition> itransition);

	virtual void receive(shared_ptr<iadp> idp);
	virtual void update_input_sub_mws(shared_ptr<iadp> idp);
	virtual void update_input_std_behaviour(shared_ptr<iadp> idp);
	virtual void update_state();
	virtual void update_view(shared_ptr<mws_camera> g);
	shared_ptr<mws> get_mws_at(int idx);

	std::unordered_map<int, std::shared_ptr<mws_page_transition> > tmap;

protected:
	mws_page();
	mws_page(shared_ptr<mws_page_tab> iparent);
	virtual void on_resize();

private:
	friend class mws_page_tab;
	friend class mws_page_item;

	static shared_ptr<mws_page> new_standalone_instance();
	void add(shared_ptr<mws_page_item> b);

	std::vector<shared_ptr<mws_page_item> > mlist;
	kinetic_scrolling ks;
};


class mws_page_item : public mws
{
public:
	virtual ~mws_page_item(){}

	shared_ptr<mws_page> get_mws_page_item_parent();

protected:
	mws_page_item(shared_ptr<mws_page> iparent);

	void add_to_page();
};
