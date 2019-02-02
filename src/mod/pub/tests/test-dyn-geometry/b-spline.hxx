#pragma once

#include "mod-test-dyn-geometry.hxx"

#ifdef MOD_TEST_DYN_GEOMETRY

#include "pfm.hxx"
#include "control-curve.hxx"
#include <vector>


class Bspline : public ControlCurve
{
public:
	static const int STEPS = 12;

	// the basis function for a cubic B spline
	float b(int i, float t);

	//evaluate a point on the B spline
	mws_sp<control_curve_ns::Point> p(int i, float t);

	void paint();
};

#endif

