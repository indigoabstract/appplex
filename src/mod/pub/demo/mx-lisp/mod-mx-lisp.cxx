#include "stdafx.hxx"

#include "mod-mx-lisp.hxx"
#include "mws-mod-ctrl.hxx"


extern "C"
{
   void run_mx_lisp_repl();
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
	settings_v.requires_gfx = false;
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
