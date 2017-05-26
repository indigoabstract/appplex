#pragma once

#include "unit-test-dyn-geometry.hpp"

#ifdef UNIT_TEST_DYN_GEOMETRY

#include "pfm.hpp"
#include "control-curve.hpp"
#include <vector>


class Bspline : public ControlCurve
{
public:
	static const int STEPS = 12;

	// the basis function for a cubic B spline
	float b(int i, float t);

	//evaluate a point on the B spline
	shared_ptr<control_curve_ns::Point> p(int i, float t);

	void paint();
};

#endif

