#pragma once

#include <chrono>


// 7 ^ 5
const int A = 16807;
// 2 ^ 31 - 1
const int M = 2147483647;
// m0 / a0 == 127773
const int Q = (M / A);
// m0 % a0 == 2836
const int R = (M % A);

struct rng_state
{
   int crt_seed;
   int init_seed;
   float z;
   float gauss_next;
};


class RNG
{
public:
   RNG::RNG()
   {
      auto now_tp = std::chrono::system_clock::now();
      uint64 count = now_tp.time_since_epoch().count();
      uint32 t = count & 0x7fffffff;// pfm::time::get_time_millis();
      setSeed(t);
      //System.out.println("t " + t + " is " + init_seed);
   }

	RNG(int seed0)
	{
		setSeed(seed0);
	}

   const rng_state& get_state() const { return st; }

   void set_state(const rng_state& i_state)
   {
      st = i_state;
   }

	int getInitSeed()
	{
		return st.init_seed;
	}

	int getCrtSeed()
	{
		return st.crt_seed;
	}

	// 0 < seed0 < m0
	void setSeed(int seed0)
	{
      st.init_seed = st.crt_seed = seed0;
      st.z = st.gauss_next = 0.f;
	}

	int nextInt()
	{
		return nextIntV1();
	}

	int nextIntV1()
	{
		if ((st.crt_seed = A * (st.crt_seed % Q) - R * (st.crt_seed / Q)) <= 0)
		{
			return st.crt_seed += M;
		}
		// This is equivalent, but quite slower though:
		// crt_seed = (crt_seed & 0x7FFFFFFF) + (crt_seed >> 31);
		return st.crt_seed;
	}

	int nextIntV2()
	{

      st.crt_seed = (214013 * st.crt_seed + 2531011);

		return (st.crt_seed >> 16) & 0x7FFF;
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
		int ib0 = int((b0 - a0) * scale);

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
   rng_state st;
};
