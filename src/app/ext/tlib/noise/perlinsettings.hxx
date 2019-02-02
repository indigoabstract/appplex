#pragma once

#include "pfm.hxx"

#define SMOOTHING        0
#define INTERPOLATION    1
#define FILTER           2
#define SCALE            3
#define AMPLITUDE        4
#define LACUNARITY       5
#define PERSISTENCE      6
#define OCTAVES          7
#define SEED             8
#define NOISE            9

#define SMOOTHING_OFF    0
#define SMOOTHING_ON     1

#define LINEAR_INTERPOLATION    0
#define COSINE_INTERPOLATION    1
#define CUBIC_INTERPOLATION     2

#define NTB_FILTER       0
#define ABS_FILTER       1
#define TRB_FILTER       2

#define NORMALIZED_TRUNCATED_BOUND  0
#define ABS_BOUND                   1
#define TRUNCATED_BOUND             2

#define FLAT              0
#define TILEABLE          1
#define SPHERE_TILED      2

class PerlinSettings
{
public:
    PerlinSettings();
    void setParam(int name, float value1, int value2);

    float cur_freq;
    float ifreq;
    float iamp;
    float lacunarity;
    float persistence;
    int octaves;
    int seed;
    int smoothFn;
    int interpFn;
    int filterFn;
    int noiseFn;
    //string name;

};
