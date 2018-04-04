#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_ABSTRACT_RACING

#include "unit.hpp"

class unit_abstract_racing : public unit
{
public:
	static shared_ptr<unit_abstract_racing> new_instance();

    virtual void init();
	virtual void init_mws();
    virtual void load();

private:
	unit_abstract_racing();
};

#endif
