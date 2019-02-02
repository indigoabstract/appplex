#pragma once

#include "unit-test-dyn-geometry.hxx"

#ifdef UNIT_TEST_DYN_GEOMETRY

#include "pfm.hxx"
#include <string>
#include <vector>
#include <glm/inc.hpp>


namespace control_curve_ns
{
	class Point
	{
	public:
		Point()
		{
			x = y = 0.f;
		}

		Point(float ix, float iy)
		{
			x = ix;
			y = iy;
		}

		float x;
		float y;
	};


	class Polygon
	{
	public:
		Polygon()
		{
			reset();
		}

		void addPoint(float x, float y)
		{
			if (xpoints.size() <= npoints)
			{
				int new_size = npoints + 1;

				xpoints.resize(new_size);
				ypoints.resize(new_size);
				zpoints.resize(new_size);
			}

			xpoints[npoints] = x;
			ypoints[npoints] = y;
			zpoints[npoints] = 0;
			npoints++;
		}

		void reset()
		{
			xpoints.clear();
			ypoints.clear();
			zpoints.clear();
			npoints = 0;
		}

		std::vector<float> xpoints;
		std::vector<float> ypoints;
		std::vector<float> zpoints;
		int npoints;
	};
}


/** this class represents a cubic polynomial */
class Cubic
{
public:
	float a, b, c, d;         /* a + b*u + c*u^2 +d*u^3 */

	Cubic(float ia, float ib, float ic, float id)
	{
		a = ia;
		b = ib;
		c = ic;
		d = id;
	}

	/** evaluate cubic */
	float eval(float u)
	{
		return (((d*u) + c)*u + b)*u + a;
	}
};


class ControlCurve
{
public:
	static const int EPSILON = 36;  /* square of distance for picking */

	ControlCurve();

	void paint();
	/** return index of control point near to (x,y) or -1 if nothing near */
	int selectPoint(float x, float y);
	// square of an int
	static float sqr(float x);
	/** add a control point, return index of new control point */
	int addPoint(float x, float y);
	/** set selected control point */
	void setPoint(float x, float y);
	/** remove selected control point */
	void removePoint();

	virtual void reset();

	virtual void calcFinalPoints() = 0;

	std::string toString();

	control_curve_ns::Polygon pts;
	std::vector<glm::vec3> final_point_list;

protected:
	int selection;
};

#endif

