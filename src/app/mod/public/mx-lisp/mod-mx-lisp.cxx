#include "stdafx.hxx"

#include "mod-mx-lisp.hxx"

#ifdef MOD_MX_LISP


#include "mws-mod-ctrl.hxx"


void run_mx_lisp_repl();

namespace mod_mx_lisp_pref
{
	class mod_preferences_detail : public mws_mod_preferences
	{
	public:
		virtual bool requires_gfx(){return false;}
	};
}




class long_op_run_mx_lisp
{
public:
	void run()
	{
		run_mx_lisp_repl();
	}
};


mod_mx_lisp::mod_mx_lisp() : mws_mod(mws_stringify(MOD_MX_LISP))
{
	prefs = mws_sp<mws_mod_preferences>(new mod_mx_lisp_pref::mod_preferences_detail());
}

mws_sp<mod_mx_lisp> mod_mx_lisp::nwi()
{
	return mws_sp<mod_mx_lisp>(new mod_mx_lisp());
}

bool mod_mx_lisp::update()
{
	long_op_run_mx_lisp repl;
	repl.run();
	mws_mod_ctrl::inst()->set_app_exit_on_next_run(true);

	return true;
}


#endif
