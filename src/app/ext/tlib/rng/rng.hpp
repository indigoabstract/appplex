#pragma once

#include "pfm.hpp"

// 7 ^ 5
const int A = 16807;
// 2 ^ 31 - 1
const int M = 2147483647;
// m0 / a0 == 127773
const int Q = (M / A);
// m0 % a0 == 2836
const int R = (M % A);

class RNG
{
public:
	RNG()
	{
		long t = pfm::time::get_time_millis();
		setSeed((int)(t & 0x7fffffff));
		//System.out.println("t " + t + " is " + initSeed);
	}

	RNG(int seed0)
	{
		setSeed(seed0);
	}

	int getInitSeed()
	{
		return initSeed;
	}

	int getCrtSeed()
	{
		return crtSeed;
	}

	// 0 < seed0 < m0
	void setSeed(int seed0)
	{
		initSeed = crtSeed = seed0;
		z = gauss_next = 0.f;
	}

	int nextInt()
	{
		return nextIntV1();
	}

	int nextIntV1()
	{
		if ((crtSeed = A * (crtSeed % Q) - R * (crtSeed / Q)) <= 0)
		{
			return crtSeed += M;
		}
		// This is equivalent, but quite slower though:
		// crtSeed = (crtSeed & 0x7FFFFFFF) + (crtSeed >> 31);
		return crtSeed;
	}

	int nextIntV2()
	{

		crtSeed = (214013 * crtSeed + 2531011);

		return (crtSeed >> 16) & 0x7FFF;
	}

	/**
	 * Generates a random long integer X where O<=X<m0.
	 * The integer X falls along a uniform distribution.
	 * For example, if m0 is 100, you get "percentile dice"
	 */
	int nextInt(int n)
	{
		return nextInt() % n;
	}

	/**
	 * Generate a random long integer X where 1<=X<=m0
	 * Note: this correctly handles m0 <= 1
	 */
	int nextOne(int m0)
	{
		return (nextInt(m0) + 1);
	}

	/**
	 * Generates a random long integer X where a0<=X<=b0
	 * The integer X falls along a uniform distribution.
	 * Note: range(0,N-1) == rand_int(N)
	 */
	int range(int a0, int b0)
	{
		return ((a0)+(nextInt(1 + (b0)-(a0))));
	}

	// between [0, 1)
	float random_float()
	{
		float r = nextInt() / (float)0x7fffffff;

		if (r < 0.)
		{
			r += 1.f;
		}

		return r;
	}

	float range_float(float a0, float b0)
	{
		float scale = 100.f;
		int ia0 = 0;
		int ib0 = (b0 - a0) * scale;

		return a0 + range(ia0, ib0) / scale;
	}

	float random_gauss(float mu, float sigma);

	/**
	 * Generate a random long integer X where a0-d0<=X<=a0+d0
	 * The integer X falls along a uniform distribution.
	 * Note: spread(a0,d0) == range(a0-d0,a0+d0)
	 */
	int spread(int a0, int d0)
	{
		return ((a0)+(nextInt(1 + (d0)+(d0))) - (d0));
	}

	/**
	 * Evaluate to TRUE "p0" percent of the time
	 */
	bool percent(int p0)
	{
		return (nextInt(100) < (p0));
	}

	/**
	 * Evaluate to TRUE 1 in p0 times
	 * Not recommended for low p0's (like 2, 3, 4, 5), use percent instead
	 */
	bool oneIn(int p0)
	{
		return (nextInt(p0) == 0);
	}

	/**
	 * Generate a random sign.
	 * @return either 1 or -1.
	 */
	int sign()
	{
		// x % 256 = x & 255
		return (nextInt() & 255) > 127 ? 1 : -1;
	}

	int nextInt1();

private:
	int crtSeed;
	int initSeed;
	float z;
	float gauss_next;
};
