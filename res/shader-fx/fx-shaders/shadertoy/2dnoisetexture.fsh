// https://www.shadertoy.com/view/MlS3z1
#define PI 3.14159265
const int iter = 8;

float rand(vec2 uv)
{
    float dt = dot(uv, vec2(12.9898, 78.233));
	return fract(sin(mod(dt, PI / 2.0)) * 43758.5453);
}

float turbulence(vec2 fragCoord, float octave, int id)
{
    float col = 0.0;
    vec2 xy;
    vec2 frac;
    vec2 tmp1;
    vec2 tmp2;
    float i2;
    float amp;
    float maxOct = octave;
    for (int i = 0; i < iter; i++)
    {
        amp = maxOct / octave;
        i2 = float(i);
        xy = id == 1 || id == 4? (fragCoord + 50.0 * float(id) * iGlobalTime / (1.0 + i2)) / octave : fragCoord / octave;
        frac = fract(xy);
        tmp1 = mod(floor(xy) + iResolution.xy, iResolution.xy);
        tmp2 = mod(tmp1 + iResolution.xy - 1.0, iResolution.xy);
        col += frac.x * frac.y * rand(tmp1) / amp;
        col += frac.x * (1.0 - frac.y) * rand(vec2(tmp1.x, tmp2.y)) / amp;
        col += (1.0 - frac.x) * frac.y * rand(vec2(tmp2.x, tmp1.y)) / amp;
        col += (1.0 - frac.x) * (1.0 - frac.y) * rand(tmp2) / amp;
        octave /= 2.0;
    }
    return (col);
}

vec3 clouds(vec2 fragCoord)
{
    float col = turbulence(fragCoord, 128.0, 1) * 0.75;
    return (vec3(0.5 + col / 2.0, 0.667 + col /3.0, 1.0));
}

vec3 marble(vec2 fragCoord)
{
	vec2 period = vec2(3.0, 4.0);
    vec2 turb = vec2(4.0, 64.0);
    float xy = fragCoord.x * period.x / iResolution.y + fragCoord.y * period.y / iResolution.x + turb.x * turbulence(fragCoord, turb.y, 2);
    float col = abs(sin(xy * PI)) * 0.75;
    return (vec3(0.1176 + col, 0.0392 + col, col));
}

vec3 wood(vec2 fragCoord)
{
    vec2 iR = iResolution.xy;
    float period = 3.5;
    vec2 turb = vec2(0.04, 16.0);
    vec2 xy;
    xy.x = (fragCoord.x - iR.x / 2.0) / iR.y;
    xy.y = (fragCoord.y - iR.y / 2.0) / iR.y;
    float dist = length(xy) + turb.x * turbulence(fragCoord, turb.y, 3);
    float col = 0.5 * abs(sin(2.0 * period * dist * PI));
    return (vec3(0.3137 + col, 0.117647 + col, 0.117647));
}

vec3 water(vec2 fragCoord)
{
	vec2 period = vec2(0.0, 0.0);
    vec2 turb = vec2(2.0, 128.0);
    float xy = fragCoord.x * period.x / iResolution.y + fragCoord.y * period.y / iResolution.x + turb.x * turbulence(fragCoord, turb.y, 4);
    float col = abs(sin(xy * PI)) * 0.75;
    return (vec3(1.0 - col, 1.0 - col * 0.7, 1.0 - col / 2.0));
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    if (fragCoord.x < iResolution.x / 2.0)
        fragColor = fragCoord.y < iResolution.y / 2.0 ? vec4(wood(fragCoord * 2.0), 1.0) : vec4(water(fragCoord * 2.0), 1.0);
    else
    	fragColor = fragCoord.y < iResolution.y / 2.0 ? vec4(clouds(fragCoord * 2.0), 1.0) : vec4(marble(fragCoord * 2.0), 1.0);
	//Pick a texture for full screen
    //fragColor = vec4(clouds(fragCoord), 1.0);
    //fragColor = vec4(marble(fragCoord), 1.0);
    //fragColor = vec4(wood(fragCoord), 1.0);
    //fragColor = vec4(water(fragCoord), 1.0);
}
