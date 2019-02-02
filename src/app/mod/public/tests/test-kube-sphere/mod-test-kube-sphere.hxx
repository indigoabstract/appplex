#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_KUBE_SPHERE

#include "mws-mod.hxx"


class mod_test_kube_sphere_impl;

class mod_test_kube_sphere : public mws_mod
{
public:
   static mws_sp<mod_test_kube_sphere> nwi();

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   mod_test_kube_sphere();
   virtual void receive(mws_sp<mws_dp> idp);

   mws_sp<mod_test_kube_sphere_impl> p;
};

#endif
