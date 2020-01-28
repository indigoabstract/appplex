#include "stdafx.hxx"

#include "mod-test-dyn-geometry.hxx"
#include "b-spline.hxx"
#include <glm/inc.hpp>


// http://www.cse.unsw.edu.au/~lambert/splines/
// the basis function for a cubic B spline
float Bspline::b(int i, float t)
{
	switch (i)
	{
	case -2:
		return (((-t+3)*t-3)*t+1)/6;
	case -1:
		return (((3*t-6)*t)*t+4)/6;
	case 0:
		return (((-3*t+3)*t+3)*t+1)/6;
	case 1:
		return (t*t*t)/6;
	}

	return 0; //we only get her;e if an invalid i is specified
}

//evaluate a point on the B spline
mws_sp<control_curve_ns::Point> Bspline::p(int i, float t)
{
	float px = 0;
	float py = 0;

	for (int j = -2; j<=1; j++)
	{
		px += b(j,t) * pts.xpoints[i+j];
		py += b(j,t) * pts.ypoints[i+j];
	}

	//return mws_sp<Point>(new Point((int)glm::round(px),(int)glm::round(py)));
	return mws_sp<control_curve_ns::Point>(new control_curve_ns::Point(px, py));
}

void Bspline::paint()
{
	//Polygon pol = new Polygon ();
	//Point q = p(2,0);
	//pol.addPoint(q.x,q.y);

	//for (int i = 2; i < pts.npoints-1; i++)
	//{
	//	for (int j = 1; j <= STEPS; j++)
	//	{
	//		q = p(i,j/(float)STEPS);
	//		pol.addPoint(q.x,q.y);
	//	}
	//}
	//g.drawPolyline(pol.xpoints, pol.ypoints, pol.npoints);
}

