#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_GEN_TEXTURE

#include "unit.hpp"


class unit_gen_texture : public unit
{
public:
	static shared_ptr<unit_gen_texture> nwi();

    virtual void init();
	virtual void init_mws();
    virtual void load();

private:
	unit_gen_texture();
};

#endif
