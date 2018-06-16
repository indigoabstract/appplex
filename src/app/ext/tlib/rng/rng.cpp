#include "stdafx.h"

#include "rng.hpp"
#include "pfm-def.h"
#include <glm/inc.hpp>


int RNG::nextInt1()
{
    crtSeed = A * (crtSeed % Q) - R * (crtSeed / Q);

    if (crtSeed <= 0)
    {
        crtSeed += M;
    }

    return crtSeed;
}

float RNG::random_gauss(float mu, float sigma)
{
	z = gauss_next;
	gauss_next = 0.f;

	if (z == 0.f)
	{
		float x2pi = random_float() * 2.f *  glm::pi<float>();
		float g2rad = sqrtf(-2.0 * logf(1.0 - random_float()));
		z = cosf(x2pi) * g2rad;
		gauss_next = sinf(x2pi) * g2rad;
	}

	return mu + z*sigma;
}
