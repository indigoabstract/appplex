#pragma once

#include "unit-test-dyn-geometry.hpp"

#ifdef UNIT_TEST_DYN_GEOMETRY

#include "pfm.hpp"
#include "b-spline.hpp"
#include <vector>


class CatmullRom : public Bspline
{
	// Catmull-Rom spline is just like a B spline, only with a different basis
	float b(int i, float t);
};

#endif
