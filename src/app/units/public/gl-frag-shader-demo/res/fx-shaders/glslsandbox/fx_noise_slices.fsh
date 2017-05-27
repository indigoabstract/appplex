#ifdef GL_ES
precision mediump float;
#endif

//noise functions
//top    row : value noise => fractal brownian motion => ridged multifractal
//bottom row : voronoi => voronoi fractal brownian motion => voronoi ridged multifractal
	

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

vec2    toworld(vec2 uv);
float   hash(float v);
vec2    rhash(vec2 uv);
float   noise(in vec2 uv);
float   voronoi(const in vec2 uv);
float   fbm(float a, float f, const in vec2 uv, const int it);
float   rmf(float a, float f, vec2 uv, const in int it);
float   vfbm(float a, float f, const vec2 uv, const in int it);
float   vrmf(float a, float f, vec2 uv, const in int it);

void main( void )
{
    vec2 uv = gl_FragCoord.xy / resolution.xy;
    uv = toworld(uv);
	
    float a = .5;
    float f = 5.;
    const int it = 8;
    
    float n = uv.x > -.66 ? uv.x > .66 ?  rmf(a, f, uv, it) :  fbm(a, f, uv, it) : noise(uv * 8.);
    float v = uv.x > -.66 ? uv.x > .66 ? vrmf(a, f, uv, it) : vfbm(a, f, uv, it) : voronoi(uv*8.);
    float r = uv.y < 0. ? v : n;
    
    gl_FragColor = vec4(r);
}//sphinx

vec2 toworld(vec2 uv){
	uv = uv * 2. - 1.;
	uv.x *= resolution.x/resolution.y;
	return uv;
}

float hash(float v)
{
    return fract(fract(v/1e4)*v-1e6);
}

vec2 rhash(vec2 uv) {
    const mat2 t = mat2(.12121212,.13131313,-.13131313,.12121212);
    const vec2 s = vec2(1e4, 1e6);
    uv *= t;
    uv *= s;
	return  fract(fract(uv/s)*uv);
}

vec2 smooth(vec2 uv)
{
    return uv*uv*(3.-2.*uv);
}

//value noise
float noise(in vec2 uv)
{
    const float k = 257.;
    vec4 l  = vec4(floor(uv),fract(uv));
    float u = l.x + l.y * k;
    vec4 v  = vec4(u, u+1.,u+k, u+k+1.);
    v       = fract(fract(1.23456789*v)*v/.987654321);
    l.zw    = smooth(l.zw);
    l.x     = mix(v.x, v.y, l.z);
    l.y     = mix(v.z, v.w, l.z);
    return    mix(l.x, l.y, l.w);
}

//iq's voronoi
float voronoi(const in vec2 uv)
{
    vec2 p = floor(uv);
    vec2 f = fract(uv);
    float v = 0.;
    for( int j=-1; j<=1; j++ )
        for( int i=-1; i<=1; i++ )
        {
            vec2 b = vec2(i, j);
            vec2 r = b - f + rhash(p + b);
            v += 1./pow(dot(r,r),8.);
        }
    return pow(1./v, 0.0625);
}

//fractal brownian motion
float fbm(float a, float f, const in vec2 uv, const int it)
{
    float n = 0.;
    for(int i = 0; i < 32; i++)
    {
        if(i<it)
        {
            n += noise(uv*f)*a;
            a *= .5;
            f *= 2.;
        }
    }
    return n;
}

//ridged multifractal
float rmf(float a, float f, vec2 uv, const in int it)
{
    float l = 2.;
    float r = 0.;
    for(int i = 0; i < 32; i++)
    {
        if(i<it)
        {
            uv = uv.yx * l;
            float n = noise(uv);     
            n = abs(fract(n-.5)-.5);
            n *= n * a;
            a = clamp(0.,1., n*2.);
            r += n*pow(f, -1.);
            f *= l;
        }
    }
    return r*8.;
}

//voronoi fbm
float vfbm(float a, float f, const in vec2 uv, const in int it)
{
    float n = 0.;
    for(int i = 0; i < 32; i++)
    {
        if(i<it)
        {
            n += voronoi(uv*f)*a;
            f *= 2.;
            a *= .5;
        }
    }
    return n;
}

//ridged multifractal
float vrmf(float a, float f, vec2 uv, const in int it)
{
    float l = 2.;
    float r = 0.;
    for(int i = 0; i < 32; i++)
    {
        if(i<it)
        {
            uv = uv.yx * l;
            float n = voronoi(uv);     
            n = abs(fract(n-.5)-.5);
            n *= n * a;
            a = clamp(0.,1., n*2.);
            r += n*pow(f, -1.);
            f *= l;
        }
    }
    return r*8.;
}
