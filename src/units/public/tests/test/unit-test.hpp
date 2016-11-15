#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST

#include "unit.hpp"


class unit_test : public unit
{
public:
	static shared_ptr<unit_test> new_instance();

    virtual void init();
	virtual void init_ux();
	virtual void load();

private:
	unit_test();

    bool texInit;
    gfx_uint* create_texture(int iw, int ih);
    gfx_uint textureId[1];
    gfx_uint texId[1];
    gfx_float *circleVertices;
    gfx_int circlePoints;
};

#endif
