#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_EXERCISE

#include "mws-mod.hxx"

class long_operation;


class mod_exercise : public mws_mod
{
public:
	static mws_sp<mod_exercise> nwi();

	virtual bool update();
	virtual void init();
	virtual void init_mws();
	virtual void load();
	virtual void unload();

private:
	mod_exercise();

	mws_sp<long_operation> lop;
};

#endif
