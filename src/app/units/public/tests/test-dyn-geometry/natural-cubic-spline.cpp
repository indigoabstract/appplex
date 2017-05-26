#include "stdafx.h"

#include "unit-test-dyn-geometry.hpp"

#ifdef UNIT_TEST_DYN_GEOMETRY

#include "natural-cubic-spline.hpp"


// http://www.cse.unsw.edu.au/~lambert/splines/
void NatCubic::calcFinalPoints()
{
	shared_ptr<std::vector<shared_ptr<Cubic> > > vx = calcNaturalCubic(pts.npoints - 1, pts.xpoints);
	shared_ptr<std::vector<shared_ptr<Cubic> > > vy = calcNaturalCubic(pts.npoints - 1, pts.ypoints);
	glm::vec3 p0((*vx)[0]->eval(0), (*vy)[0]->eval(0), 0);

	final_point_list.push_back(p0);

	for (int i = 0; i < vx->size(); i++)
	{
		for (int j = 1; j <= STEPS; j++)
		{
			float u = j / (float) STEPS;
			glm::vec3 p((*vx)[i]->eval(u), (*vy)[i]->eval(u), 0);

			final_point_list.push_back(p);
		}
	}
}

shared_ptr<std::vector<shared_ptr<Cubic> > > NatCubic::calcNaturalCubic(int n, std::vector<float>& x)
{
	std::vector<float> gamma(n + 1);
	std::vector<float> delta(n + 1);
	std::vector<float> D(n + 1);
	int i;

	/* We solve the equation
	[2 1       ] [D[0]]   [3(x[1] - x[0])  ]
	|1 4 1     | |D[1]|   |3(x[2] - x[0])  |
	|  1 4 1   | | .  | = |      .         |
	|    ..... | | .  |   |      .         |
	|     1 4 1| | .  |   |3(x[n] - x[n-2])|
	[       1 2] [D[n]]   [3(x[n] - x[n-1])]

	by using row operations to convert the matrix to upper triangular
	and then back sustitution.  The D[i] are the derivatives at the knots.
	*/

	gamma[0] = 1.0f / 2.0f;

	for(i = 1; i < n; i++)
	{
		gamma[i] = 1 / (4 - gamma[i - 1]);
	}

	gamma[n] = 1 / (2 - gamma[n-1]);

	delta[0] = 3.f * (x[1] - x[0]) * gamma[0];

	for(i = 1; i < n; i++)
	{
		delta[i] = (3*(x[i+1]-x[i-1])-delta[i-1])*gamma[i];
	}

	delta[n] = (3*(x[n]-x[n-1])-delta[n-1])*gamma[n];

	D[n] = delta[n];

	for(i = n-1; i >= 0; i--)
	{
		D[i] = delta[i] - gamma[i]*D[i+1];
	}

	/* now compute the coefficients of the cubics */
	shared_ptr<std::vector<shared_ptr<Cubic> > > C(new std::vector<shared_ptr<Cubic> >(n));

	for(i = 0; i < n; i++)
	{
		(*C)[i] = shared_ptr<Cubic>(new Cubic((float)x[i], D[i], 3*(x[i+1] - x[i]) - 2*D[i] - D[i+1], 2*(x[i] - x[i+1]) + D[i] + D[i+1]));
	}

	return C;
}

shared_ptr<std::vector<shared_ptr<Cubic> > > NatCubic::calcClosedNaturalCubic(int n, std::vector<float>& x)
{
	std::vector<float> w(n + 1);
	std::vector<float> v(n + 1);
	std::vector<float> y(n + 1);
	std::vector<float> D(n + 1);
	float z, F, G, H;
	int k;

	/* We solve the equation
	[4 1      1] [D[0]]   [3(x[1] - x[n])  ]
	|1 4 1     | |D[1]|   |3(x[2] - x[0])  |
	|  1 4 1   | | .  | = |      .         |
	|    ..... | | .  |   |      .         |
	|     1 4 1| | .  |   |3(x[n] - x[n-2])|
	[1      1 4] [D[n]]   [3(x[0] - x[n-1])]

	by decomposing the matrix into upper triangular and lower matrices
	and then back sustitution.  See Spath "Spline Algorithms for Curves
	and Surfaces" pp 19--21. The D[i] are the derivatives at the knots.
	*/
	w[1] = v[1] = z = 1.0f/4.0f;
	y[0] = z * 3 * (x[1] - x[n]);
	H = 4;
	F = 3 * (x[0] - x[n-1]);
	G = 1;

	for ( k = 1; k < n; k++)
	{
		v[k+1] = z = 1/(4 - v[k]);
		w[k+1] = -z * w[k];
		y[k] = z * (3*(x[k+1]-x[k-1]) - y[k-1]);
		H = H - G * w[k];
		F = F - G * y[k-1];
		G = -v[k] * G;
	}

	H = H - (G+1)*(v[n]+w[n]);
	y[n] = F - (G+1)*y[n-1];

	D[n] = y[n]/H;
	D[n-1] = y[n-1] - (v[n]+w[n])*D[n]; /* This equation is WRONG! in my copy of Spath */

	for ( k = n-2; k >= 0; k--)
	{
		D[k] = y[k] - v[k+1]*D[k+1] - w[k+1]*D[n];
	}


	shared_ptr<std::vector<shared_ptr<Cubic> > > C(new std::vector<shared_ptr<Cubic> >(n + 1));

	/* now compute the coefficients of the cubics */
	for ( k = 0; k < n; k++)
	{
		(*C)[k] = shared_ptr<Cubic>(new Cubic((float)x[k], D[k], 3*(x[k+1] - x[k]) - 2*D[k] - D[k+1], 2*(x[k] - x[k+1]) + D[k] + D[k+1]));
	}

	(*C)[n] = shared_ptr<Cubic>(new Cubic((float)x[n], D[n], 3*(x[0] - x[n]) - 2*D[n] - D[0], 2*(x[n] - x[0]) + D[n] + D[0]));

	return C;
}

/* draw a cubic spline */
void NatCubic::paint()
{
	//if (pts.npoints >= 2)
	//{
	//	Cubic[] X = calcNaturalCubic(pts.npoints-1, pts.xpoints);
	//	Cubic[] Y = calcNaturalCubic(pts.npoints-1, pts.ypoints);

	//	/* very crude technique - just break each segment up into steps lines */
	//	Polygon p = new Polygon();
	//	p.addPoint((int) Math.round(X[0].eval(0)),
	//		(int) Math.round(Y[0].eval(0)));
	//	for (int i = 0; i < X.length; i++) {
	//		for (int j = 1; j <= STEPS; j++) {
	//			float u = j / (float) STEPS;
	//			p.addPoint(Math.round(X[i].eval(u)),
	//				Math.round(Y[i].eval(u)));
	//		}
	//	}
	//	g.drawPolyline(p.xpoints, p.ypoints, p.npoints);
	//}
}

#endif
