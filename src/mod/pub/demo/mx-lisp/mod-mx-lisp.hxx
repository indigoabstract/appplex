#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_MX_LISP

#include "mws-mod.hxx"


class mod_mx_lisp : public mws_mod
{
public:
	static mws_sp<mod_mx_lisp> nwi();

    virtual bool update();

private:
   mod_mx_lisp();
};

#endif