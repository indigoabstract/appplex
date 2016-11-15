#include "stdafx.h"

#include "perlinnoise.hpp"
#include "pfm.hpp"
#include <rng/rng.hpp>


Palette Perlin::pal;
PerlinSettings Perlin::settings;
float** Perlin::noisev;
int Perlin::crtSeed;

float Perlin::cur_freq;
int Perlin::width;
int Perlin::twidth;
int Perlin::theight;

int Perlin::format;
bool Perlin::flat;
bool Perlin::tileable;
bool Perlin::sphere_mapped;

Palette Perlin::palTab[100];
int Perlin::palCount = 0;
PerlinSettings Perlin::settingsTab[100];
int Perlin::settingsCount = 0;

void Perlin::initPalette()
{
	Perlin::pal.name = "-";
}

void Perlin::initSettings()
{
	float freq = 0.75f;
	float amp = 1.5f;
	float lacunarity = 1.f;
	float persistence = 0.5f;
	int octaves = 2;
	int seed = pfm::time::get_time_millis() & 0x7fffffff;
	setParams(freq, amp, lacunarity, persistence, octaves, seed);

	Perlin::settings.smoothFn = SMOOTHING_ON;
	Perlin::settings.interpFn = LINEAR_INTERPOLATION;
	Perlin::settings.filterFn = ABS_BOUND;
	Perlin::settings.noiseFn = 1;
	Perlin::format = FLAT;

	Perlin::pal.clearColors();
	Perlin::pal.setColor(0.f, 51, 51, 51);
	Perlin::pal.setColor(0.05f, 51, 51, 51);
	Perlin::pal.setColor(.5f, 255, 255, 255);
	Perlin::pal.setColor(1.f, 255, 255, 255);
}

void Perlin::addPalette(std::string name)
{
	palTab[palCount] = Palette();
	pal = palTab[palCount++];
	// Init palette so that we are in a valid state.
	initPalette();
	pal.name = name;
}

Palette Perlin::getCurrentPalette()
{
	return pal;
}

Palette Perlin::getPalette(std::string name)
{
	Palette palette;

	//    for (int k = 0; k < palCount; k++)
	//    {
	//        if (name.compare(palTab[k].name) == 0)
	//        {
	//            palette = palTab[k];
	//            break;
	//        }
	//    }

	return palette;
}

void Perlin::setCurrentPalette(int idx)
{
	pal = palTab[idx];
}

void Perlin::setCurrentPalette(std::string name)
{
	for (int k = 0; k < palCount; k++)
	{
		if (name.compare(palTab[k].name) == 0)
		{
			pal = palTab[k];
			break;
		}
	}
}

void Perlin::addSettings(std::string name)
{
	settingsTab[settingsCount] = PerlinSettings();
	settings = settingsTab[settingsCount++];
	// Init settings so that we are in a valid state.
	initSettings();
	//settings.name = name;
}

PerlinSettings Perlin::getCurrentSettings()
{
	return settings;
}

PerlinSettings Perlin::getSettings(std::string name)
{
	PerlinSettings ps;

	//    for (int k = 0; k < settingsCount; k++)
	//    {
	//        if (name.compare(settingsTab[k].name) == 0)
	//        {
	//            ps = settingsTab[k];
	//            break;
	//        }
	//    }

	return ps;
}

void Perlin::setCurrentSettings(int idx)
{
	settings = settingsTab[idx];
}

void Perlin::setCurrentSettings(std::string name)
{
	//    for (int k = 0; k < settingsCount; k++)
	//    {
	//        if (name.compare(settingsTab[k].name) == 0)
	//        {
	//            settings = settingsTab[k];
	//            break;
	//        }
	//    }
}

float Perlin::smoothFn(int a, int b)
{
	switch (Perlin::settings.smoothFn)
	{
	case SMOOTHING_OFF:
		return noise2D(a, b);

	case SMOOTHING_ON:
		return smoothedNoise2D(a, b);
	}

	return 0;
}

float Perlin::interpFn(float a1, float a2, float a3)
{
	switch (Perlin::settings.interpFn)
	{
	case LINEAR_INTERPOLATION:
		return linearInterpolation(a1, a2, a3);

	case COSINE_INTERPOLATION:
		return cosineInterpolation(a1, a2, a3);

	case CUBIC_INTERPOLATION:
		return cubicInterpolation(a1, a2, a3);
		//return 0;
	}

	return 0;
}

float Perlin::filterFn(float a1)
{
	switch (Perlin::settings.filterFn)
	{
	case NORMALIZED_TRUNCATED_BOUND:
		return normalizedTruncatedBound(a1);

	case ABS_BOUND:
		return absBound(a1);

	case TRUNCATED_BOUND:
		return truncatedBound(a1);
	}

	return 0;
}

float Perlin::smoothedNoise2D(int x,int y)
{
	float corners = ( noise2D(x-1, y-1)+noise2D(x+1, y-1)+noise2D(x-1, y+1)+noise2D(x+1, y+1) ) / 16;
	float sides   = ( noise2D(x-1, y)  +noise2D(x+1, y)  +noise2D(x, y-1)  +noise2D(x, y+1) ) /  8;
	float center  =  noise2D(x, y) / 4;
	return corners + sides + center;
}

float Perlin::absBound(float n)
{
	if(n < 0.0f) n = -n;
	if(n > 1.0f) n = 1.0f;
	return n;
}

float Perlin::truncatedBound(float n)
{
	if(n < 0.0f) n = 0.0f;
	if(n > 1.0f) n = 1.0f;
	return n;
}

float Perlin::normalizedTruncatedBound(float n)
{
	if(n < -1.0f) n = -1.0f;
	if(n > 1.0f) n = 1.0f;
	return (n*.5f) + .5f;
}

void Perlin::genTxt(int *argb, int width, int height, int scanLength)
{
	int w = width / 2;
	int h = height / 4;
	//game.RNG rng = new game.RNG();
	float p;
	int kf = 1;

	for(int y = 0, l = 0; y < h; y++)
	{
		int offset = l = kf * y * scanLength;

		for(int x = 0; x < w; x++, l++)
		{
			//argb[l] = rng.nextInt() & 0xffffff;
		}
	}

	for(int y = 0, l = 0; y < h; y++)
	{
		int offset = l = kf * y * scanLength;

		for(int x = 0; x < w; x++, l++)
		{
			//argb[l] = RGBBuff.average(x, y) + (rng.nextInt() % 0xffffff)/2;
			//argb[l] = RGBBuff.average(x, y);
		}
	}

	for(int y = 0, l = 0; y < h; y++)
	{
		int offset = l = kf * y * scanLength;

		for(int x = 0; x < w; x++, l++)
		{
			//argb[l] = rng.nextInt();
			//p = noise2D(x + y * 8997587);
			//p = (1.0f - (rng.nextInt() & 0x7fffffff) / 1073741824.f);
			p = argb[l] / (float)0xffffff;
			argb[l] = Perlin::pal.getColor(p);
		}

		//        System.arraycopy(argb, offset, argb, l, w);
		//        //System.arraycopy(argb, offset, argb, l + w, 2 * w);
		//        //System.arraycopy(argb, offset, argb, offset + 2 * h * scanLength, scanLength);
		//        System.arraycopy(argb, offset, argb, -offset + 2 * (2 * h - 1) * scanLength, scanLength);
	}
}

void Perlin::genPerlinTile(int *tileBounds, int tileBoundsLength, int *argb, int tileWidth)
{
	float p;
	int tileHeight = tileBoundsLength / 2;
	int xl, xr;
	int adr = 0;
	//System.out.println("th " + tileHeight);

	for(int y = 0; y < tileHeight; y++, adr += tileWidth)
	{
		xl = tileBounds[y] + 1;
		xr = tileBounds[y + tileHeight];
		//System.out.println("xl " + xl + " xr " + xr);

		for(int x = xl, off = adr + xl; x <= xr; x++, off++)
		{
			p = getNoise2D(x, y, settings.noiseFn);
			p = Perlin::filterFn(p);
			argb[off] = Perlin::pal.getColor(p);
		}
	}
}

void Perlin::genPerlinTexture(int *argb, int twidth0, int theight0)
{
	//setParams(.1f, iamp, persistence, octaves, random_seed);
	twidth = twidth0;
	theight = theight0;
	float *hm = new float[twidth0 * theight0];
	genHeightMap(hm, twidth0, theight0);
	genColorMap(argb, hm, twidth0, theight0);
	//paintTexture();
	delete hm;
}

void Perlin::genHeightMap(float *hm, int twidth0, int theight0)
{
	float p;
	int w = twidth;
	int h = theight;

	Perlin::width = w;

	for(int y = 0, l = 0; y < h; y++)
	{
		for(int x = 0; x < w; x++, l++)
		{
			p = getNoise2D(x, y, settings.noiseFn);
			//p = Cellular(x,y,twidth,20, crtSeed);
			p = Perlin::filterFn(p);
			hm[l] = p;
		}
	}
}

void Perlin::genColorMap(int *argb, float *hm, int twidth0, int theight0)
{
	for(int y = 0, l = 0; y < theight0; y++)
	{
		for(int x = 0; x < twidth0; x++, l++)
		{
			argb[l] = Perlin::pal.getColor(hm[l]);
		}
	}
}

void Perlin::setParams(float init_freq, float init_amp, float lacunarity0, float persistence0, int octaves, int random_seed)
{
	Perlin::settings.ifreq = init_freq;
	Perlin::settings.iamp = init_amp;
	Perlin::settings.lacunarity = lacunarity0;
	Perlin::settings.persistence = persistence0;
	Perlin::settings.octaves = octaves;
	Perlin::settings.seed = crtSeed = random_seed;
}

float Perlin::noise1D(int x)
{
	x += Perlin::settings.seed;
	x = (x<<13) ^ x;
	return ( 1.0f - ( (x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
	//return ( 1.0f - ((x * 15731 + 789221) & 0x7fffffff) / 1073741824.0f);
}

int Perlin::rndNoise(int x, int y)
{
	//System.out.println("c " + crtSeed);
	//if((crtSeed = A * (crtSeed % Q) - R * (crtSeed / Q)) <= 0)crtSeed += M;
	//crtSeed += x-y;
	//crtSeed = A * (crtSeed % Q) - R * (crtSeed / Q);
	crtSeed = (crtSeed + x) * 15731;
	crtSeed = (crtSeed + y) * 789221;
	crtSeed &= 0x7fffffff;
	return crtSeed;
}

float Perlin::noise2D(int x, int y)
{
	/*
	if(Perlin::format == TILEABLE)
	{
	// round f to nearest int s
	float f = Perlin::cur_freq*Perlin::width;
	int s = (int)f;
	if(f - s > .5f) s++;

	if (s > 0)
	{
	x %= s;
	y %= s;
	}
	if (x < 0) x += s;
	if (y < 0) y += s;
	}
	*/

	//float f = 1.f - rndNoise(x, y) / 1073741824.f;
	//float f = rndNoise(x, y);
	//System.out.println("f " + f);
	//return f;
	// all the mysterious large numbers in these functions are prime.
	float fl= noise1D(x + y * 8997587);//57);
	//return Noise((y ^ 0x562a56fa ) * ( x ^ y ));
	//return Noise((y ^ 0x386a6c5e ) * ( x ^ y ));
	/*
	x = x+y*8997587+Perlin::random_seed;
	//x += Perlin::random_seed;
	x = (x<<13) ^ x;
	return ( 1.0f - ( (x * (x * x * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
	*/
	//if(y==17&&x==17)
	{
		//trace("seed %d %f" , Perlin::settings.seed, fl);
	}
	return fl;
}

float Perlin::linearInterpolation(float a,float b,float x)
{
	return a*(1-x) + b*x;
}

float Perlin::cosineInterpolation(float a, float b, float x)
{
	// accelerate this with a cos lookup table?
	float ft = x * 3.1415927f;
	float f = (1 - (float)cos(ft)) * .5f;
	return a*(1-f)+b*f;
}

float Perlin::cubicInterpolation(float a, float b, float x)
{
	float fac1 = 3 * (float)pow(1-x, 2) - 2 * (float)pow(1-x,3);
	float fac2 = 3 * (float)pow(x, 2) - 2 * (float)pow(x, 3);

	return a * fac1 + b * fac2; //add the weighted factors
}

float Perlin::perlinNoise2D(float x, float y)
{
	float total = 0;

	float freq = Perlin::settings.ifreq;
	float amp = Perlin::settings.iamp;

	for(int i = 0; i < Perlin::settings.octaves; i++)
	{
		Perlin::cur_freq = freq;
		total += interpolatedNoise2D(x * freq, y * freq) * amp;
		freq *= Perlin::settings.lacunarity;
		amp *= Perlin::settings.persistence;
	}

	return total;
}

float Perlin::interpolatedNoise2D(float x, float y)
{
	//if (true)return ImprovedNoise.noise3D(x, y, 0);
	//if (true)return OriginalNoise.noise2(x, y);
	int a = (int)x;
	int b = (int)y;
	float frac_a = x - a;
	float frac_b = y - b;

	float v1, v2, v3, v4;

	v1 = smoothFn(a, b);
	v2 = smoothFn(a + 1, b);
	v3 = smoothFn(a, b + 1);
	v4 = smoothFn(a + 1, b + 1);
	/*
	v1 = smoothedNoise2D(a,b);
	v2 = smoothedNoise2D(a + 1, b);
	v3 = smoothedNoise2D(a,b+1);
	v4 = smoothedNoise2D(a+1,b+1);
	*/
	/*
	v1 = noisev[b][a];
	v2 = noisev[b][a + 1];
	v3 = noisev[b + 1][a];
	v4 = noisev[b + 1][a + 1];
	*/
	float i1 = Perlin::interpFn(v1 , v2 , frac_a);
	float i2 = Perlin::interpFn(v3 , v4 , frac_a);

	return Perlin::interpFn(i1 , i2 , frac_b);
}

float Perlin::perlinNoise3D(float x, float y, float z)
{
	float total = 0;

	float freq = Perlin::settings.ifreq;
	float amp = Perlin::settings.iamp;

	for(int i = 0; i < Perlin::settings.octaves; i++)
	{
		total += interpolatedNoise3D(x * freq, y * freq, z * freq) * amp;
		freq *= 2;
		amp *= Perlin::settings.persistence;
	}

	return total;
}

float Perlin::noise3D(int x, int y, int z)
{
	return noise1D(x + (y * 89213) + (z * 8997587));
}

float Perlin::interpolatedNoise3D(float x, float y, float z)
{
	int a = (int)x;
	int b = (int)y;
	int c = (int)z;
	float frac_a = x - a;
	float frac_b = y - b;
	float frac_c = z - c;

	float v1 = noise3D(a,b,c);
	float v2 = noise3D(a+1,b,c);
	float v3 = noise3D(a,b+1,c);
	float v4 = noise3D(a+1,b+1,c);

	float i1 = Perlin::interpFn(v1 , v2 , frac_a);
	float i2 = Perlin::interpFn(v3 , v4 , frac_a);
	float i3 = Perlin::interpFn(i1 , i2 , frac_b);


	float v5 = noise3D(a,b,c+1);
	float v6 = noise3D(a+1,b,c+1);
	float v7 = noise3D(a,b+1,c+1);
	float v8 = noise3D(a+1,b+1,c+1);

	float j1 = Perlin::interpFn(v5 , v6 , frac_a);
	float j2 = Perlin::interpFn(v7 , v8 , frac_a);
	float j3 = Perlin::interpFn(j1 , j2 , frac_b);

	return Perlin::interpFn(i3 , j3 , frac_c);

}

void Perlin::genSphereTiledTexture(int *argb,int w, int h, int scanLength)
{
	float p, u, v, r, s, a, b, c;

	for(int x = 0; x < w; x++)
	{
		for(int y = 0; y < h; y++)
		{

			u = (float)x/(float)w;
			v = (float)y/(float)h;

			r = u * 2.0f * 3.14156f;
			s = (v - .5f) * 3.14156f;

			a = (float)(cos(r)*cos(s));
			b = (float)(sin(r)*cos(s));
			c = (float)(sin(s));

			a+=1.0f;
			b+=1.0f;
			c+=1.0f;

			a*=w;
			b*=w;
			c*=w;

			p = perlinNoise3D(a,b,c);
			p = Perlin::filterFn(p);

			argb[(y * scanLength) + x] = Perlin::pal.getColor(p);
		}
	}
}

float Perlin::Cellular(float x,float y,int width,int tam_cas, int seed)
{
	double primero=2*tam_cas, segundo=2*tam_cas,tercero=2*tam_cas,dist_aux;
	int casilla_pto;
	double xpunto, ypunto;
	int n_casillas=(int)(width/tam_cas)+1;
	int casillax=(int)(x/tam_cas);
	int casillay=(int)(y/tam_cas);
	int casilla=n_casillas*casillay+casillax;
	for (int j=-1;j<2;j++)
	{
		for (int i=-1;i<2;i++)
		{
			casilla_pto=casilla+i+j*n_casillas;
			xpunto=(casillax+i)*tam_cas+noise1D(casilla_pto+seed)*tam_cas;
			ypunto=(casillay+j)*tam_cas+noise1D(casilla_pto+10+seed)*tam_cas;
			dist_aux=sqrt((x-xpunto)*(x-xpunto)+(y-ypunto)*(y-ypunto));

			if (primero>dist_aux)
			{
				tercero=segundo;
				segundo=primero;
				primero=dist_aux;
			}
			else
			{
				if (segundo>dist_aux)
				{
					tercero= segundo;
					segundo=dist_aux;}
				else
				{if (tercero>dist_aux)
				{tercero=dist_aux;}}
			}
		}}
	return (float)(primero*primero/(segundo*tercero));
}

float Perlin::getNoise2D(int x, int y, int id)
{
	switch(id)
	{
	case 0:
		return Perlin::perlinNoise2D(x, y);

	case 1:
		return (Perlin::noise2D(x,y) + 2 * Perlin::perlinNoise2D(x,y)) / 6;
	}

	return 0;
}






float grad[12][3] = {
	{1.0,1.0,0.0},{-1.0,1.0,0.0},{1.0,-1.0,0.0},{-1.0,-1.0,0.0},
	{1.0,0.0,1.0},{-1.0,0.0,1.0},{1.0,0.0,-1.0},{-1.0,0.0,-1.0},
	{0.0,1.0,1.0},{0.0,-1.0,1.0},{0.0,1.0,-1.0},{0.0,-1.0,-1.0}
};

int perm_init[512] = {151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180, 151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180};
int perm[512];

void set_seed(int seed)
{ 
	for (int i = 0; i < 256; i++) { 
		int shift = (i + seed) % 256; 
		perm[i] = perm_init[shift & 0xff]; 
		perm[i + 256] = perm_init[shift & 0xff]; 
	}
}

float dot(float x, float y, float z, float* g)
{
	return x*g[0] + y*g[1] + z*g[2];
}

float noise(float xin, float yin, float zin){
	float F3, G3, t, X0, Y0, Z0, x0, y0, z0, s, x1, y1, z1, x2, y2, z2, x3, y3, z3, t0, t1, t2, t3, n0, n1, n2, n3;
	int i, j, k, ii, jj, kk, i1, j1, k1, i2, j2, k2, gi0, gi1, gi2, gi3;

	F3 = 1.0/3.0;
	s = (xin+yin+zin)*F3;
	i = xin+s;
	j = yin+s;
	k = zin+s;
	G3 = 1.0/6.0;
	t = (i+j+k)*G3;
	X0 = i-t;
	Y0 = j-t;
	Z0 = k-t;
	x0 = xin-X0;
	y0 = yin-Y0;
	z0 = zin-Z0;

	if(x0 >= y0){
		if(y0 >= z0){
			i1=1; j1=0; k1=0; i2=1; j2=1; k2=0;
		}
		else if(x0 >= z0){
			i1=1; j1=0; k1=0; i2=1; j2=0; k2=1;
		}
		else{
			i1=0; j1=0; k1=1; i2=1; j2=0; k2=1;
		}
	}
	else{
		if(y0 < z0){
			i1=0; j1=0; k1=1; i2=0; j2=1; k2=1;
		}
		else if(x0 < z0){ 
			i1=0; j1=1; k1=0; i2=0; j2=1; k2=1;
		}
		else{
			i1=0; j1=1; k1=0; i2=1; j2=1; k2=0;
		}
	}

	x1 = x0 - i1 + G3;
	y1 = y0 - j1 + G3;
	z1 = z0 - k1 + G3;
	x2 = x0 - i2 + 2.0*G3;
	y2 = y0 - j2 + 2.0*G3;
	z2 = z0 - k2 + 2.0*G3;
	x3 = x0 - 1.0 + 3.0*G3;
	y3 = y0 - 1.0 + 3.0*G3;
	z3 = z0 - 1.0 + 3.0*G3;

	ii = i & 255;
	jj = j & 255;
	kk = k & 255;

	gi0 = perm[ii+perm[jj+perm[kk]]] % 12;
	gi1 = perm[ii+i1+perm[jj+j1+perm[kk+k1]]] % 12;
	gi2 = perm[ii+i2+perm[jj+j2+perm[kk+k2]]] % 12;
	gi3 = perm[ii+1+perm[jj+1+perm[kk+1]]] % 12;

	t0 = 0.6 - x0*x0 - y0*y0 - z0*z0;
	if(t0<0){
		n0 = 0.0;
	}
	else{
		t0 *= t0;
		n0 = t0 * t0 * dot(x0, y0, z0, grad[gi0]);
	}

	t1 = 0.6 - x1*x1 - y1*y1 - z1*z1;
	if(t1<0){
		n1 = 0.0;
	}
	else{
		t1 *= t1;
		n1 = t1 * t1 * dot(x1, y1, z1, grad[gi1]);
	}

	t2 = 0.6 - x2*x2 - y2*y2 - z2*z2;
	if(t2<0){
		n2 = 0.0;
	}
	else{
		t2 *= t2;
		n2 = t2 * t2 * dot(x2, y2, z2, grad[gi2]);
	}

	t3 = 0.6 - x3*x3 - y3*y3 - z3*z3;
	if(t3<0){
		n3 = 0.0;
	}
	else{
		t3 *= t3;
		n3 = t3 * t3 * dot(x3, y3, z3, grad[gi3]);
	}

	return 16.0*(n0 + n1 + n2 + n3)+1.0;
}

float simplex_noise(int octaves, float x, float y, float z){
	float value = 0.0;
	int i;
	for(i=0; i<octaves; i++){
		value += noise(
			x*powf(2, i),
			y*powf(2, i),
			z*powf(2, i)
			);
	}
	return value;
}


// SimplexNoise1234
// Copyright 2003-2011, Stefan Gustavson

#define FASTFLOOR(x) ( ((x)>0) ? ((int)x) : (((int)x)-1) )

//---------------------------------------------------------------------
// Static data

/*
* Permutation table. This is just a random jumble of all numbers 0-255,
* repeated twice to avoid wrapping the index at 255 for each lookup.
* This needs to be exactly the same for all instances on all platforms,
* so it's easiest to just keep it as static explicit data.
* This also removes the need for any initialisation of this class.
*
* Note that making this an int[] instead of a char[] might make the
* code run faster on platforms with a high penalty for unaligned single
* byte addressing. Intel x86 is generally single-byte-friendly, but
* some other CPUs are faster with 4-aligned reads.
* However, a char[] is smaller, which avoids cache trashing, and that
* is probably the most important aspect on most architectures.
* This array is accessed a *lot* by the noise functions.
* A vector-valued noise over 3D accesses it 96 times, and a
* float-valued 4D noise 64 times. We want this to fit in the cache!
*/
unsigned char SimplexNoise1234::perm[512] =
{
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
	151,160,137,91,90,15,
	131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
	190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
	88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
	77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
	102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
	135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
	5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
	223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
	129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
	251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
	49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
	138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180 
};

//---------------------------------------------------------------------

/*
* Helper functions to compute gradients-dot-residualvectors (1D to 4D)
* Note that these generate gradients of more than unit length. To make
* a close match with the value range of classic Perlin noise, the final
* noise values need to be rescaled to fit nicely within [-1,1].
* (The simplex noise functions as such also have different scaling.)
* Note also that these noise functions are the most practical and useful
* signed version of Perlin noise. To return values according to the
* RenderMan specification from the SL noise() and pnoise() functions,
* the noise values need to be scaled and offset to [0,1], like this:
* float SLnoise = (SimplexNoise1234::noise(x,y,z) + 1.0) * 0.5;
*/

float  SimplexNoise1234::grad( int hash, float x )
{
	int h = hash & 15;
	float grad = 1.0f + (h & 7);   // Gradient value 1.0, 2.0, ..., 8.0
	if (h&8) grad = -grad;         // Set a random sign for the gradient
	return ( grad * x );           // Multiply the gradient with the distance
}

float  SimplexNoise1234::grad( int hash, float x, float y )
{
	int h = hash & 7;      // Convert low 3 bits of hash code
	float u = h<4 ? x : y;  // into 8 simple gradient directions,
	float v = h<4 ? y : x;  // and compute the dot product with (x,y).
	return ((h&1)? -u : u) + ((h&2)? -2.0f*v : 2.0f*v);
}

float  SimplexNoise1234::grad( int hash, float x, float y , float z )
{
	int h = hash & 15;     // Convert low 4 bits of hash code into 12 simple
	float u = h<8 ? x : y; // gradient directions, and compute dot product.
	float v = h<4 ? y : h==12||h==14 ? x : z; // Fix repeats at h = 12 to 15
	return ((h&1)? -u : u) + ((h&2)? -v : v);
}

float  SimplexNoise1234::grad( int hash, float x, float y, float z, float t )
{
	int h = hash & 31;      // Convert low 5 bits of hash code into 32 simple
	float u = h<24 ? x : y; // gradient directions, and compute dot product.
	float v = h<16 ? y : z;
	float w = h<8 ? z : t;
	return ((h&1)? -u : u) + ((h&2)? -v : v) + ((h&4)? -w : w);
}

// A lookup table to traverse the simplex around a given point in 4D.
// Details can be found where this table is used, in the 4D noise method.
/* TODO: This should not be required, backport it from Bill's GLSL code! */
static unsigned char simplex[64][4] =
{
	{0,1,2,3},{0,1,3,2},{0,0,0,0},{0,2,3,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,2,3,0},
	{0,2,1,3},{0,0,0,0},{0,3,1,2},{0,3,2,1},{0,0,0,0},{0,0,0,0},{0,0,0,0},{1,3,2,0},
	{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
	{1,2,0,3},{0,0,0,0},{1,3,0,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,3,0,1},{2,3,1,0},
	{1,0,2,3},{1,0,3,2},{0,0,0,0},{0,0,0,0},{0,0,0,0},{2,0,3,1},{0,0,0,0},{2,1,3,0},
	{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
	{2,0,1,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,0,1,2},{3,0,2,1},{0,0,0,0},{3,1,2,0},
	{2,1,0,3},{0,0,0,0},{0,0,0,0},{0,0,0,0},{3,1,0,2},{0,0,0,0},{3,2,0,1},{3,2,1,0}
};

// 1D simplex noise
float SimplexNoise1234::noise(float x)
{

	int i0 = FASTFLOOR(x);
	int i1 = i0 + 1;
	float x0 = x - i0;
	float x1 = x0 - 1.0f;

	float n0, n1;

	float t0 = 1.0f - x0*x0;
	//  if(t0 < 0.0f) t0 = 0.0f;
	t0 *= t0;
	n0 = t0 * t0 * grad(perm[i0 & 0xff], x0);

	float t1 = 1.0f - x1*x1;
	//  if(t1 < 0.0f) t1 = 0.0f;
	t1 *= t1;
	n1 = t1 * t1 * grad(perm[i1 & 0xff], x1);
	// The maximum value of this noise is 8*(3/4)^4 = 2.53125
	// A factor of 0.395 would scale to fit exactly within [-1,1], but
	// we want to match PRMan's 1D noise, so we scale it down some more.
	return 0.25f * (n0 + n1);

}

// 2D simplex noise
float SimplexNoise1234::noise(float x, float y)
{

#define F2 0.366025403 // F2 = 0.5*(sqrt(3.0)-1.0)
#define G2 0.211324865 // G2 = (3.0-Math.sqrt(3.0))/6.0

	float n0, n1, n2; // Noise contributions from the three corners

	// Skew the input space to determine which simplex cell we're in
	float s = (x+y)*F2; // Hairy factor for 2D
	float xs = x + s;
	float ys = y + s;
	int i = FASTFLOOR(xs);
	int j = FASTFLOOR(ys);

	float t = (float)(i+j)*G2;
	float X0 = i-t; // Unskew the cell origin back to (x,y) space
	float Y0 = j-t;
	float x0 = x-X0; // The x,y distances from the cell origin
	float y0 = y-Y0;

	// For the 2D case, the simplex shape is an equilateral triangle.
	// Determine which simplex we are in.
	int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
	if(x0>y0) {i1=1; j1=0;} // lower triangle, XY order: (0,0)->(1,0)->(1,1)
	else {i1=0; j1=1;}      // upper triangle, YX order: (0,0)->(0,1)->(1,1)

	// A step of (1,0) in (i,j) means a step of (1-c,-c) in (x,y), and
	// a step of (0,1) in (i,j) means a step of (-c,1-c) in (x,y), where
	// c = (3-sqrt(3))/6

	float x1 = x0 - i1 + G2; // Offsets for middle corner in (x,y) unskewed coords
	float y1 = y0 - j1 + G2;
	float x2 = x0 - 1.0f + 2.0f * G2; // Offsets for last corner in (x,y) unskewed coords
	float y2 = y0 - 1.0f + 2.0f * G2;

	// Wrap the integer indices at 256, to avoid indexing perm[] out of bounds
	int ii = i & 0xff;
	int jj = j & 0xff;

	// Calculate the contribution from the three corners
	float t0 = 0.5f - x0*x0-y0*y0;
	if(t0 < 0.0f) n0 = 0.0f;
	else {
		t0 *= t0;
		n0 = t0 * t0 * grad(perm[ii+perm[jj]], x0, y0); 
	}

	float t1 = 0.5f - x1*x1-y1*y1;
	if(t1 < 0.0f) n1 = 0.0f;
	else {
		t1 *= t1;
		n1 = t1 * t1 * grad(perm[ii+i1+perm[jj+j1]], x1, y1);
	}

	float t2 = 0.5f - x2*x2-y2*y2;
	if(t2 < 0.0f) n2 = 0.0f;
	else {
		t2 *= t2;
		n2 = t2 * t2 * grad(perm[ii+1+perm[jj+1]], x2, y2);
	}

	// Add contributions from each corner to get the final noise value.
	// The result is scaled to return values in the interval [-1,1].
	return 40.0f * (n0 + n1 + n2); // TODO: The scale factor is preliminary!
}

// 3D simplex noise
float SimplexNoise1234::noise(float x, float y, float z)
{

	// Simple skewing factors for the 3D case
#define F3 0.333333333
#define G3 0.166666667

	float n0, n1, n2, n3; // Noise contributions from the four corners

	// Skew the input space to determine which simplex cell we're in
	float s = (x+y+z)*F3; // Very nice and simple skew factor for 3D
	float xs = x+s;
	float ys = y+s;
	float zs = z+s;
	int i = FASTFLOOR(xs);
	int j = FASTFLOOR(ys);
	int k = FASTFLOOR(zs);

	float t = (float)(i+j+k)*G3; 
	float X0 = i-t; // Unskew the cell origin back to (x,y,z) space
	float Y0 = j-t;
	float Z0 = k-t;
	float x0 = x-X0; // The x,y,z distances from the cell origin
	float y0 = y-Y0;
	float z0 = z-Z0;

	// For the 3D case, the simplex shape is a slightly irregular tetrahedron.
	// Determine which simplex we are in.
	int i1, j1, k1; // Offsets for second corner of simplex in (i,j,k) coords
	int i2, j2, k2; // Offsets for third corner of simplex in (i,j,k) coords

	/* This code would benefit from a backport from the GLSL version! */
	if(x0>=y0) {
		if(y0>=z0)
		{ i1=1; j1=0; k1=0; i2=1; j2=1; k2=0; } // X Y Z order
		else if(x0>=z0) { i1=1; j1=0; k1=0; i2=1; j2=0; k2=1; } // X Z Y order
		else { i1=0; j1=0; k1=1; i2=1; j2=0; k2=1; } // Z X Y order
	}
	else { // x0<y0
		if(y0<z0) { i1=0; j1=0; k1=1; i2=0; j2=1; k2=1; } // Z Y X order
		else if(x0<z0) { i1=0; j1=1; k1=0; i2=0; j2=1; k2=1; } // Y Z X order
		else { i1=0; j1=1; k1=0; i2=1; j2=1; k2=0; } // Y X Z order
	}

	// A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
	// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
	// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
	// c = 1/6.

	float x1 = x0 - i1 + G3; // Offsets for second corner in (x,y,z) coords
	float y1 = y0 - j1 + G3;
	float z1 = z0 - k1 + G3;
	float x2 = x0 - i2 + 2.0f*G3; // Offsets for third corner in (x,y,z) coords
	float y2 = y0 - j2 + 2.0f*G3;
	float z2 = z0 - k2 + 2.0f*G3;
	float x3 = x0 - 1.0f + 3.0f*G3; // Offsets for last corner in (x,y,z) coords
	float y3 = y0 - 1.0f + 3.0f*G3;
	float z3 = z0 - 1.0f + 3.0f*G3;

	// Wrap the integer indices at 256, to avoid indexing perm[] out of bounds
	int ii = i & 0xff;
	int jj = j & 0xff;
	int kk = k & 0xff;

	// Calculate the contribution from the four corners
	float t0 = 0.6f - x0*x0 - y0*y0 - z0*z0;
	if(t0 < 0.0f) n0 = 0.0f;
	else {
		t0 *= t0;
		n0 = t0 * t0 * grad(perm[ii+perm[jj+perm[kk]]], x0, y0, z0);
	}

	float t1 = 0.6f - x1*x1 - y1*y1 - z1*z1;
	if(t1 < 0.0f) n1 = 0.0f;
	else {
		t1 *= t1;
		n1 = t1 * t1 * grad(perm[ii+i1+perm[jj+j1+perm[kk+k1]]], x1, y1, z1);
	}

	float t2 = 0.6f - x2*x2 - y2*y2 - z2*z2;
	if(t2 < 0.0f) n2 = 0.0f;
	else {
		t2 *= t2;
		n2 = t2 * t2 * grad(perm[ii+i2+perm[jj+j2+perm[kk+k2]]], x2, y2, z2);
	}

	float t3 = 0.6f - x3*x3 - y3*y3 - z3*z3;
	if(t3<0.0f) n3 = 0.0f;
	else {
		t3 *= t3;
		n3 = t3 * t3 * grad(perm[ii+1+perm[jj+1+perm[kk+1]]], x3, y3, z3);
	}

	// Add contributions from each corner to get the final noise value.
	// The result is scaled to stay just inside [-1,1]
	return 32.0f * (n0 + n1 + n2 + n3); // TODO: The scale factor is preliminary!
}


// 4D simplex noise
float SimplexNoise1234::noise(float x, float y, float z, float w)
{

	// The skewing and unskewing factors are hairy again for the 4D case
#define F4 0.309016994 // F4 = (Math.sqrt(5.0)-1.0)/4.0
#define G4 0.138196601 // G4 = (5.0-Math.sqrt(5.0))/20.0

	float n0, n1, n2, n3, n4; // Noise contributions from the five corners

	// Skew the (x,y,z,w) space to determine which cell of 24 simplices we're in
	float s = (x + y + z + w) * F4; // Factor for 4D skewing
	float xs = x + s;
	float ys = y + s;
	float zs = z + s;
	float ws = w + s;
	int i = FASTFLOOR(xs);
	int j = FASTFLOOR(ys);
	int k = FASTFLOOR(zs);
	int l = FASTFLOOR(ws);

	float t = (i + j + k + l) * G4; // Factor for 4D unskewing
	float X0 = i - t; // Unskew the cell origin back to (x,y,z,w) space
	float Y0 = j - t;
	float Z0 = k - t;
	float W0 = l - t;

	float x0 = x - X0;  // The x,y,z,w distances from the cell origin
	float y0 = y - Y0;
	float z0 = z - Z0;
	float w0 = w - W0;

	// For the 4D case, the simplex is a 4D shape I won't even try to describe.
	// To find out which of the 24 possible simplices we're in, we need to
	// determine the magnitude ordering of x0, y0, z0 and w0.
	// The method below is a good way of finding the ordering of x,y,z,w and
	// then find the correct traversal order for the simplex we?re in.
	// First, six pair-wise comparisons are performed between each possible pair
	// of the four coordinates, and the results are used to add up binary bits
	// for an integer index.
	int c1 = (x0 > y0) ? 32 : 0;
	int c2 = (x0 > z0) ? 16 : 0;
	int c3 = (y0 > z0) ? 8 : 0;
	int c4 = (x0 > w0) ? 4 : 0;
	int c5 = (y0 > w0) ? 2 : 0;
	int c6 = (z0 > w0) ? 1 : 0;
	int c = c1 + c2 + c3 + c4 + c5 + c6;

	int i1, j1, k1, l1; // The integer offsets for the second simplex corner
	int i2, j2, k2, l2; // The integer offsets for the third simplex corner
	int i3, j3, k3, l3; // The integer offsets for the fourth simplex corner

	// simplex[c] is a 4-vector with the numbers 0, 1, 2 and 3 in some order.
	// Many values of c will never occur, since e.g. x>y>z>w makes x<z, y<w and x<w
	// impossible. Only the 24 indices which have non-zero entries make any sense.
	// We use a thresholding to set the coordinates in turn from the largest magnitude.
	// The number 3 in the "simplex" array is at the position of the largest coordinate.
	i1 = simplex[c][0]>=3 ? 1 : 0;
	j1 = simplex[c][1]>=3 ? 1 : 0;
	k1 = simplex[c][2]>=3 ? 1 : 0;
	l1 = simplex[c][3]>=3 ? 1 : 0;
	// The number 2 in the "simplex" array is at the second largest coordinate.
	i2 = simplex[c][0]>=2 ? 1 : 0;
	j2 = simplex[c][1]>=2 ? 1 : 0;
	k2 = simplex[c][2]>=2 ? 1 : 0;
	l2 = simplex[c][3]>=2 ? 1 : 0;
	// The number 1 in the "simplex" array is at the second smallest coordinate.
	i3 = simplex[c][0]>=1 ? 1 : 0;
	j3 = simplex[c][1]>=1 ? 1 : 0;
	k3 = simplex[c][2]>=1 ? 1 : 0;
	l3 = simplex[c][3]>=1 ? 1 : 0;
	// The fifth corner has all coordinate offsets = 1, so no need to look that up.

	float x1 = x0 - i1 + G4; // Offsets for second corner in (x,y,z,w) coords
	float y1 = y0 - j1 + G4;
	float z1 = z0 - k1 + G4;
	float w1 = w0 - l1 + G4;
	float x2 = x0 - i2 + 2.0f*G4; // Offsets for third corner in (x,y,z,w) coords
	float y2 = y0 - j2 + 2.0f*G4;
	float z2 = z0 - k2 + 2.0f*G4;
	float w2 = w0 - l2 + 2.0f*G4;
	float x3 = x0 - i3 + 3.0f*G4; // Offsets for fourth corner in (x,y,z,w) coords
	float y3 = y0 - j3 + 3.0f*G4;
	float z3 = z0 - k3 + 3.0f*G4;
	float w3 = w0 - l3 + 3.0f*G4;
	float x4 = x0 - 1.0f + 4.0f*G4; // Offsets for last corner in (x,y,z,w) coords
	float y4 = y0 - 1.0f + 4.0f*G4;
	float z4 = z0 - 1.0f + 4.0f*G4;
	float w4 = w0 - 1.0f + 4.0f*G4;

	// Wrap the integer indices at 256, to avoid indexing perm[] out of bounds
	int ii = i & 0xff;
	int jj = j & 0xff;
	int kk = k & 0xff;
	int ll = l & 0xff;

	// Calculate the contribution from the five corners
	float t0 = 0.6f - x0*x0 - y0*y0 - z0*z0 - w0*w0;
	if(t0 < 0.0f) n0 = 0.0f;
	else {
		t0 *= t0;
		n0 = t0 * t0 * grad(perm[ii+perm[jj+perm[kk+perm[ll]]]], x0, y0, z0, w0);
	}

	float t1 = 0.6f - x1*x1 - y1*y1 - z1*z1 - w1*w1;
	if(t1 < 0.0f) n1 = 0.0f;
	else {
		t1 *= t1;
		n1 = t1 * t1 * grad(perm[ii+i1+perm[jj+j1+perm[kk+k1+perm[ll+l1]]]], x1, y1, z1, w1);
	}

	float t2 = 0.6f - x2*x2 - y2*y2 - z2*z2 - w2*w2;
	if(t2 < 0.0f) n2 = 0.0f;
	else {
		t2 *= t2;
		n2 = t2 * t2 * grad(perm[ii+i2+perm[jj+j2+perm[kk+k2+perm[ll+l2]]]], x2, y2, z2, w2);
	}

	float t3 = 0.6f - x3*x3 - y3*y3 - z3*z3 - w3*w3;
	if(t3 < 0.0f) n3 = 0.0f;
	else {
		t3 *= t3;
		n3 = t3 * t3 * grad(perm[ii+i3+perm[jj+j3+perm[kk+k3+perm[ll+l3]]]], x3, y3, z3, w3);
	}

	float t4 = 0.6f - x4*x4 - y4*y4 - z4*z4 - w4*w4;
	if(t4 < 0.0f) n4 = 0.0f;
	else {
		t4 *= t4;
		n4 = t4 * t4 * grad(perm[ii+1+perm[jj+1+perm[kk+1+perm[ll+1]]]], x4, y4, z4, w4);
	}

	// Sum up and scale the result to cover the range [-1,1]
	return 27.0f * (n0 + n1 + n2 + n3 + n4); // TODO: The scale factor is preliminary!
}
