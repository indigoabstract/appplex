#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_TEST

#include "unit.hxx"


class unit_test : public unit
{
public:
	static mws_sp<unit_test> nwi();

    virtual void init();
	virtual void init_mws();
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
