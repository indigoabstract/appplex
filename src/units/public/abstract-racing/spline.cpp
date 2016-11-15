#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_ABSTRACT_RACING

#include "spline.hpp"
#include <vector>


spline::spline(float a, float b, float c, float d)
{
	this->a = a;
	this->b = b;
	this->c = c;
	this->d = d;
}

float spline::eval(float u)
{
	return (((d*u) + c)*u + b)*u + a;
}

spline** spline::calculateClosedCubic(int n, float x[])
{
	std::vector<float> w(n+1);
	std::vector<float> v(n + 1);
	std::vector<float> y(n + 1);
	std::vector<float> D(n + 1);
	float z, F, G, H;
	int k;

	w[1] = v[1] = z = 1.0f/4.0f;
	y[0] = z * 3 * (x[1] - x[n]);
	H = 4;
	F = 3 * (x[0] - x[n-1]);
	G = 1;
	for (k = 1; k < n; k++)
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
	D[n-1] = y[n-1] - (v[n]+w[n])*D[n];
	for (k = n-2; k >= 0; k--)
		D[k] = y[k] - v[k+1]*D[k+1] - w[k+1]*D[n];

	// compute the coefficients of the cubics
	spline** C = new spline*[n+1];
	for (k = 0; k < n; k++)
		C[k] = new spline((float)x[k], D[k], 3*(x[k+1] - x[k]) - 2*D[k] - D[k+1], 2*(x[k] - x[k+1]) + D[k] + D[k+1]);
	C[n] = new spline((float)x[n], D[n], 3*(x[0] - x[n]) - 2*D[n] - D[0], 2*(x[n] - x[0]) + D[n] + D[0]);

	return C;
}

spline** spline::calculateNaturalCubic(int n, float x[])
{
	std::vector<float> gamma(n + 1);
	std::vector<float> delta(n + 1);
	std::vector<float> D(n + 1);
	int i;

	gamma[0] = 1.0f/2.0f;
	for (i = 1; i < n; i++)
		gamma[i] = 1/(4-gamma[i-1]);
	gamma[n] = 1/(2-gamma[n-1]);

	delta[0] = 3*(x[1]-x[0])*gamma[0];
	for (i = 1; i < n; i++)
		delta[i] = (3*(x[i+1]-x[i-1])-delta[i-1])*gamma[i];
	delta[n] = (3*(x[n]-x[n-1])-delta[n-1])*gamma[n];

	D[n] = delta[n];
	for (i = n-1; i >= 0; i--)
		D[i] = delta[i] - gamma[i]*D[i+1];

	//compute the coefficients of the cubics
	spline **C = new spline*[n];
	for (i = 0; i < n; i++)
		C[i] = new spline((float)x[i], D[i], 3*(x[i+1] - x[i]) - 2*D[i] - D[i+1], 2*(x[i] - x[i+1]) + D[i] + D[i+1]);

	return C;
}

#endif
