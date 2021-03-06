#pragma once

#include "mod-dyn-geometry.hxx"

#ifdef MOD_DYN_GEOMETRY

#include "pfm.hxx"
#include "b-spline.hxx"
#include <vector>


class CatmullRom : public Bspline
{
	// Catmull-Rom spline is just like a B spline, only with a different basis
	float b(int i, float t);
};

#endif
