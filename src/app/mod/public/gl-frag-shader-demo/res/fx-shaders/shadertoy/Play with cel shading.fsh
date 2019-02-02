// https://www.shadertoy.com/view/4sfXzS
const float epsilon = 1e-6;
const float PI = 3.14159265358979323846;
const float pov = PI/10.0;
const float maxdistance = 5.0;
const vec3 light = vec3(0.0, 0.0, 1.0);
const float celShades = 5.0;
const vec3 lightColor = vec3(1.0, 0.0, 0.0);
const vec3 eye = vec3(0.0, 0.0, 4.0);

// for calculating normal
const float delta = 0.0001;
const vec3 dx = vec3(delta, 0.0, 0.0);
const vec3 dy = vec3(0.0, delta, 0.0);
const vec3 dz = vec3(0.0, 0.0, delta);

// for edge detection
const float delta2 = 0.01;
const vec3 dx2 = vec3(delta2, 0.0, 0.0);
const vec3 dy2 = vec3(0.0, delta2, 0.0);
const vec3 dz2 = vec3(0.0, 0.0, delta2);

float c = cos(iGlobalTime);
float s = sin(iGlobalTime);

float smin( float a, float b)
{
	float k = 0.2;
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}

float sdTorus(vec3 p, vec2 t)
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}

float scene(vec3 p)
{
    mat3 m1 = mat3(c, 0, -s,
                  0, 1, 0,
                  s, 0, c);
    mat3 m2 = mat3(c, -s, 0,
                  s, c, 0,
                  0, 0, 1);
    p = p*m1*m2;
    float size = (abs(c) + abs(s))*0.5;
    //return sdTorus(p, vec2(size, size/2.0));
    //return sdBox(p, vec3(size));
    return smin(sdTorus(p, vec2(size, size/2.0)),
                sdBox(p, vec3(0.6)));
}

vec3 march(vec3 ray)
{
    vec3 color = vec3(0.3);
    float t = 0.0;
    float mindistance = maxdistance;
    for (int i  = 0; i < 256; ++i)
    {
        vec3 p = eye + ray * t;
        float d = scene(p);
        if (d < mindistance)
            mindistance = d;
        if(abs(d) < epsilon)
        {
            vec3 n1 = normalize(vec3(scene(p+dx2)-d, scene(p+dy2)-d, scene(p+dz2)-d));
            vec3 n2 = normalize(vec3(d-scene(p-dx2), d-scene(p-dy2), d-scene(p-dz2)));

            if (abs(dot(n1,n2)) > 0.99)
            {
                vec3 normal = normalize(vec3(
                    scene(p+dx)-scene(p-dx),
                    scene(p+dy)-scene(p-dy),
                    scene(p+dz)-scene(p-dz)));
                float intensity = dot(normal, light);
                intensity = ceil(intensity * celShades) / celShades;
                color = lightColor * intensity;
            }
            else
                color = vec3(0.0);
            break;
        }
        t += d;
        if (t > maxdistance)
        {
            if (mindistance < 0.02)
                color = vec3(0.0);
            break;
        }
    }
    return color;
}

void main(void)
{
    float ar = iResolution.x / iResolution.y;
    vec2 uv = (gl_FragCoord.xy) / iResolution.yy - vec2(0.5 * ar, 0.5);
    vec3 ray = normalize(vec3(uv, eye.z - 0.5/atan(pov)) - eye);
    gl_FragColor = vec4(march(ray), 1.0);
}
