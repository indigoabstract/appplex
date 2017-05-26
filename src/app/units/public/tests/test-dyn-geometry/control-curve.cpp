#include "stdafx.h"

#include "unit-test-dyn-geometry.hpp"

#ifdef UNIT_TEST_DYN_GEOMETRY

#include "control-curve.hpp"


ControlCurve::ControlCurve()
{
	selection = -1;
}

void ControlCurve::paint()
{
	//FontMetrics fm = g.getFontMetrics(f);
	//g.setFont(f);
	//int h = fm.getAscent()/2;

	//for(int i = 0; i < pts.npoints; i++)  {
	//	String s = Integer.toString(i);
	//	int w = fm.stringWidth(s)/2;
	//	g.drawString(Integer.toString(i),pts.xpoints[i]-w,pts.ypoints[i]+h);
	//}
}

int ControlCurve::selectPoint(float x, float y)
{
	float mind = (float)0xffffffff;

	selection = -1;

	for (int i = 0; i < pts.npoints; i++)
	{
		float d = sqr(pts.xpoints[i] - x) + sqr(pts.ypoints[i] - y);

		if (d < mind && d < EPSILON)
		{
			mind = d;
			selection = i;
		}
	}
	return selection;
}

float ControlCurve::sqr(float x)
{
	return x*x;
}

int ControlCurve::addPoint(float x, float y)
{
	pts.addPoint(x,y);

	return selection = pts.npoints - 1;
}

void ControlCurve::setPoint(float x, float y)
{
	if (selection >= 0) {
		pts.xpoints[selection] = x;
		pts.ypoints[selection] = y;
	}
}

void ControlCurve::removePoint()
{
	if (selection >= 0)
	{
		pts.npoints--;

		for (int i = selection; i < pts.npoints; i++)
		{
			pts.xpoints[i] = pts.xpoints[i+1];
			pts.ypoints[i] = pts.ypoints[i+1];
		}
	}
}

void ControlCurve::reset()
{
	pts.reset();
	final_point_list.clear();
}

std::string ControlCurve::toString()
{
	//StringBuffer result = new StringBuffer();
	//for (int i = 0; i < pts.npoints; i++) {
	//	result.append(" " + pts.xpoints[i] + " " + pts.ypoints[i]);
	//}
	return std::string();
}

#endif
