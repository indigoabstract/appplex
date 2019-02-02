#include "stdafx.hxx"

#include "unit-mx-lisp.hxx"

#ifdef UNIT_MX_LISP


#include "unit-ctrl.hxx"


void run_mx_lisp_repl();

namespace unit_mx_lisp_pref
{
	class unit_preferences_detail : public unit_preferences
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


unit_mx_lisp::unit_mx_lisp() : unit(mws_stringify(UNIT_MX_LISP))
{
	prefs = mws_sp<unit_preferences>(new unit_mx_lisp_pref::unit_preferences_detail());
}

mws_sp<unit_mx_lisp> unit_mx_lisp::nwi()
{
	return mws_sp<unit_mx_lisp>(new unit_mx_lisp());
}

bool unit_mx_lisp::update()
{
	long_op_run_mx_lisp repl;
	repl.run();
	unit_ctrl::inst()->set_app_exit_on_next_run(true);

	return true;
}


#endif
