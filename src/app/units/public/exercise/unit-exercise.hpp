#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_EXERCISE

#include "unit.hpp"

class long_operation;


class unit_exercise : public unit
{
public:
	static mws_sp<unit_exercise> nwi();

	virtual bool update();
	virtual void init();
	virtual void init_mws();
	virtual void load();
	virtual void unload();

private:
	unit_exercise();

	mws_sp<long_operation> lop;
};

#endif
