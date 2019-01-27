#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_MX_LISP

#include "unit.hpp"


class unit_mx_lisp : public unit
{
public:
	static mws_sp<unit_mx_lisp> nwi();

    virtual bool update();

private:
	unit_mx_lisp();
};

#endif
