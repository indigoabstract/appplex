#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_KEMX

#include "unit.hpp"
#include <string>
#include <vector>


class mws_tree_model_node;
class mws_page_tab;
class node_info;


class unit_kemx : public unit
{
public:
	static shared_ptr<unit_kemx> new_instance();

	virtual void init();
	virtual void init_mws();
	virtual void load();
	virtual void unload();

private:
	unit_kemx();

	static void create_mws_tree_model(const std::vector<shared_ptr<node_info> >& list, shared_ptr<mws_tree_model_node> node, int& length);

	std::string data;
	std::vector<shared_ptr<node_info> > node_list;
	bool is_valid_expression;
};

#endif
