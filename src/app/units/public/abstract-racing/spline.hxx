#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_ABSTRACT_RACING

#include "pfm.hxx"


class spline
{
public:
	spline(float a, float b, float c, float d);
	float eval(float u);
	static spline** calculateClosedCubic(int n, float x[]);
	static spline** calculateNaturalCubic(int n, float x[]);

private:
	float a,b,c,d;         /* a + b*u + c*u^2 +d*u^3 */
};

#endif
