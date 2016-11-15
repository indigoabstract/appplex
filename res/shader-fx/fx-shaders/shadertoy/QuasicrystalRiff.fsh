// https://www.shadertoy.com/view/lslGzn
// Available as Mac Screensaver.  Code/Downloads here:
// https://bitbucket.org/rallen/quasicrystalscreensaver/wiki/Home

const float PI = 3.1415926535897931;
const float ra = 0.0 / 3.0 * 2.0 * PI;
const float ga = 1.0 / 3.0 * 2.0 * PI;
const float ba = 2.0 / 3.0 * 2.0 * PI;

// all of these can be played with
const float scale = 25.0;
const float tscale = 7.0;
const float xpixels = 3.0;
const float ypixels = 3.0;
const int symmetry = 7;

float adj(float n, float m)
{
    return scale * (n / m);
}

vec2 point(vec2 src)
{
    return vec2(adj(src.x,ypixels), adj(src.y,ypixels));
}

vec2 wave( vec2 p, float th )
{
    float t = fract( iGlobalTime / tscale );
    t *= 2.0 * PI;
    vec2 d = vec2( cos( th ), sin( th ) );
    d = d * cos( dot( d, p ) + t );
    d = (d + vec2(1.0,1.0))*0.5;
    return d;
}

vec3 combine( vec2 p )
{
    vec2 sum = vec2( 0.0 );
    vec2 rdir = vec2( cos( ra ), sin( ra ) );
    vec2 gdir = vec2( cos( ga ), sin( ga ) );
    vec2 bdir = vec2( cos( ba ), sin( ba ) );
    for (int i = 0; i < symmetry; ++i)
    {
        sum += wave( point( p ), float( i ) * PI / float( symmetry ) );
    }
    sum.x = mod(floor(sum.x), 2.0) == 0.0 ? fract(sum.x) : 1.0 - fract(sum.x);
    sum.y = mod(floor(sum.y), 2.0) == 0.0 ? fract(sum.y) : 1.0 - fract(sum.y);
    sum = 2.0*sum - vec2(1.0,1.0);
    return vec3( dot( rdir, sum ), dot( gdir, sum ), dot( bdir, sum ) );
}

void main(void)
{
    vec2 vUV = 2.0*(gl_FragCoord.xy / iResolution.xy);
    gl_FragColor = vec4( combine( vec2( vUV.x * xpixels, vUV.y * ypixels ) ), 1.0 );
}
