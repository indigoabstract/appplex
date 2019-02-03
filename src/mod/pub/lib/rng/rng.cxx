#include "stdafx.hxx"

#include "rng.hxx"
#include "pfm.hxx"
#define _USE_MATH_DEFINES
#include <math.h>


RNG::RNG()
{
   long t = pfm::time::get_time_millis();
   setSeed((int)(t & 0x7fffffff));
   //System.out.println("t " + t + " is " + init_seed);
}

int RNG::nextInt1()
{
   st.crt_seed = A * (st.crt_seed % Q) - R * (st.crt_seed / Q);

    if (st.crt_seed <= 0)
    {
       st.crt_seed += M;
    }

    return st.crt_seed;
}

float RNG::random_gauss(float mu, float sigma)
{
   st.z = st.gauss_next;
   st.gauss_next = 0.f;

	if (st.z == 0.f)
	{
		float x2pi = random_float() * 2.f * (float)M_PI;
		float g2rad = sqrtf(-2.f * logf(1.f - random_float()));
      st.z = cosf(x2pi) * g2rad;
      st.gauss_next = sinf(x2pi) * g2rad;
	}

	return mu + st.z * sigma;
}
