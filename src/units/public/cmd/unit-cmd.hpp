#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "unit.hpp"

class long_operation;


class unit_cmd : public unit
{
public:
	static shared_ptr<unit_cmd> new_instance();

    virtual bool update();
    virtual void load();

private:
	unit_cmd();

	shared_ptr<long_operation> lop;
};

#endif
