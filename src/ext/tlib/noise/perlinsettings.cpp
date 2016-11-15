#include "stdafx.h"

#include "perlinsettings.hpp"

PerlinSettings::PerlinSettings()
{
    noiseFn = 0;
}

void PerlinSettings::setParam(int name, float value1, int value2)
{
    switch (name)
    {
    case SMOOTHING:
        smoothFn = value2;
        break;

    case INTERPOLATION:
        interpFn = value2;
        break;

    case FILTER:
        filterFn = value2;
        break;

    case SCALE:
        ifreq = value1;
        break;

    case AMPLITUDE:
        iamp = value1;
        break;

    case LACUNARITY:
        lacunarity = value1;
        break;

    case PERSISTENCE:
        persistence = value1;
        break;

    case OCTAVES:
        octaves = value2;
        break;

    case SEED:
        seed = value2;
        break;

    case NOISE:
        noiseFn = value1;
        break;
    }
}
