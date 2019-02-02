#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_TEST_KUBE_SPHERE

#include "unit.hxx"


class unit_test_kube_sphere_impl;

class unit_test_kube_sphere : public unit
{
public:
   static mws_sp<unit_test_kube_sphere> nwi();

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   unit_test_kube_sphere();
   virtual void receive(mws_sp<mws_dp> idp);

   mws_sp<unit_test_kube_sphere_impl> p;
};

#endif
