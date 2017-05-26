#pragma once

#include "pfm.hpp"
#include <string>
#include "palette.hpp"
#include "perlinsettings.hpp"

class Perlin
{
public:
	static Palette pal;
	static PerlinSettings settings;
	static float **noisev;
	static int crtSeed;

	static float cur_freq;
	static int width;
	static int twidth;
	static int theight;

	static int format;
	static bool flat;
	static bool tileable;
	static bool sphere_mapped;

	static Palette palTab[100];
	static int palCount;
	static PerlinSettings settingsTab[100];
	static int settingsCount;

	static void initPalette();
	static void initSettings();
	static void addPalette(std::string name);
	static Palette getCurrentPalette();
	static Palette getPalette(std::string name);
	static void setCurrentPalette(int idx);
	static void setCurrentPalette(std::string name);
	static void addSettings(std::string name);
	static PerlinSettings getCurrentSettings();
	static PerlinSettings getSettings(std::string name);
	static void setCurrentSettings(int idx);
	static void setCurrentSettings(std::string name);
	static float smoothFn(int a, int b);
	static float interpFn(float a1, float a2, float a3);
	static float filterFn(float a1);
	static float smoothedNoise2D(int x,int y);
	static float absBound(float n);
	static float truncatedBound(float n);
	static float normalizedTruncatedBound(float n);
	static void genTxt(int *argb, int width, int height, int scanLength);
	static void genPerlinTile(int *tileBounds, int tileBoundsLength, int *argb, int tileWidth);
	static void genPerlinTexture(int *argb, int twidth0, int theight0);
	static void genHeightMap(float *hm, int twidth0, int theight0);
	static void genColorMap(int *argb, float *hm, int twidth0, int theight0);
	static void setParams(float init_freq, float init_amp, float lacunarity0, float persistence0, int octaves, int random_seed);
	static float noise1D(int x);
	static int rndNoise(int x, int y);
	static float noise2D(int x, int y);
	static float linearInterpolation(float a,float b,float x);
	static float cosineInterpolation(float a, float b, float x);
	static float cubicInterpolation(float a, float b, float x);
	static float perlinNoise2D(float x, float y);
	static float interpolatedNoise2D(float x, float y);
	static float perlinNoise3D(float x, float y, float z);
	static float noise3D(int x, int y, int z);
	static float interpolatedNoise3D(float x, float y, float z);
	static void genSphereTiledTexture(int *argb,int w, int h, int scanLength);
	static float Cellular(float x,float y,int width,int tam_cas, int seed);
	static float getNoise2D(int x, int y, int id);
};


void set_seed(int seed);
float noise(float xin, float yin, float zin);
float simplex_noise(int octaves, float x, float y, float z);


// SimplexNoise1234
// Copyright 2003-2011, Stefan Gustavson
class SimplexNoise1234
{

public:
	SimplexNoise1234() {}
	~SimplexNoise1234() {}

	/** 1D, 2D, 3D and 4D float Perlin noise
	*/
    static float noise( float x );
    static float noise( float x, float y );
    static float noise( float x, float y, float z );
    static float noise( float x, float y, float z, float w );

	/** 1D, 2D, 3D and 4D float Perlin noise, with a specified integer period
	*/
    static float pnoise( float x, int px );
    static float pnoise( float x, float y, int px, int py );
    static float pnoise( float x, float y, float z, int px, int py, int pz );
    static float pnoise( float x, float y, float z, float w,
		int px, int py, int pz, int pw );

private:
    static unsigned char perm[];
    static float grad( int hash, float x );
    static float grad( int hash, float x, float y );
    static float grad( int hash, float x, float y , float z );
    static float grad( int hash, float x, float y, float z, float t );

};
