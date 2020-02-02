#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_LAYERED_CAR_PAINT

#include "mws-mod.hxx"


namespace
{
	class mod_layered_car_paint_impl;
}


class mod_layered_car_paint : public mws_mod
{
public:
	static mws_sp<mod_layered_car_paint> nwi();

	virtual void init();
	virtual void load();
	virtual bool update();

private:
	mod_layered_car_paint();
	virtual void receive(mws_sp<mws_dp> i_dp);

	mws_sp<mod_layered_car_paint_impl> p;
};

#endif
