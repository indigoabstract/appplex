#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST

#include "mod.hxx"


class mod_test : public mws_mod
{
public:
	static mws_sp<mod_test> nwi();

    virtual void init();
	virtual void init_mws();
	virtual void load();

private:
	mod_test();

    bool texInit;
    gfx_uint* create_texture(int iw, int ih);
    gfx_uint textureId[1];
    gfx_uint texId[1];
    gfx_float *circleVertices;
    gfx_int circlePoints;
};

#endif
