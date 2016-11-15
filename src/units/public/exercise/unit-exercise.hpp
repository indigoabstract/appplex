#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_EXERCISE

#include "unit.hpp"

class long_operation;


class unit_exercise : public unit
{
public:
	static shared_ptr<unit_exercise> new_instance();

	virtual bool update();
	virtual void init();
	virtual void init_ux();
	virtual void load();
	virtual void unload();

private:
	unit_exercise();

	shared_ptr<long_operation> lop;
};

#endif
