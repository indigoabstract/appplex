#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_KUBE_SPHERE

#include "unit.hpp"


class unit_test_kube_sphere_impl;

class unit_test_kube_sphere : public unit
{
public:
	static shared_ptr<unit_test_kube_sphere> new_instance();

    virtual void init();
    virtual void load();
	virtual bool update();

private:
	unit_test_kube_sphere();
	virtual void receive(shared_ptr<iadp> idp);

	shared_ptr<unit_test_kube_sphere_impl> p;
};

#endif
