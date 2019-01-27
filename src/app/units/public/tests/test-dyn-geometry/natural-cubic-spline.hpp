#pragma once

#include "unit-test-dyn-geometry.hpp"

#ifdef UNIT_TEST_DYN_GEOMETRY

#include "pfm.hpp"
#include "control-curve.hpp"
#include <vector>


class NatCubic : public ControlCurve
{
public:
	static const int STEPS = 8;

	void calcFinalPoints();

	/* calculates the natural cubic spline that interpolates
	y[0], y[1], ... y[n]
	The first segment is returned as
	C[0].a + C[0].b*u + C[0].c*u^2 + C[0].d*u^3 0<=u <1
	the other segments are in C[1], C[2], ...  C[n-1] */
	mws_sp<std::vector<mws_sp<Cubic> > > calcNaturalCubic(int n, std::vector<float>& x);

	/* calculates the closed natural cubic spline that interpolates
	x[0], x[1], ... x[n]
	The first segment is returned as
	C[0].a + C[0].b*u + C[0].c*u^2 + C[0].d*u^3 0<=u <1
	the other segments are in C[1], C[2], ...  C[n] */
	mws_sp<std::vector<mws_sp<Cubic> > > calcClosedNaturalCubic(int n, std::vector<float>& x);

	/* draw a cubic spline */
	void paint();
};

#endif
