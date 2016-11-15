// https://www.shadertoy.com/view/4dsXWj
#define PLANE 1000.0
#define PI 3.141593
#define LOOPS 128.0
#define LIGHTLOOPS 128.0
#define L1 1
#define L2 2
#define SPEED 5.0

struct light 
{
    vec3 pos;
    vec3 color;
    float intensity;
    float dist;
};
    
struct ball
{
    vec3 pos;
    vec3 color;
    float r;
};
    
struct hitInfo
{
    vec4 info;
    int type;
};
    
light light1 = light(vec3(0,0,-20.0), vec3(1,1,1), 200.0, 1.0);
light light2 = light(vec3(0,-20,40.0), vec3(1,1,1), 1.0, 5.0);
ball ball1 = ball(vec3(0,10,0), vec3(1,0,0), 10.0);
    
vec4 Rball(vec3 point, vec3 pos, float r, vec3 color) {
    return vec4(1,1,0,distance(point, pos)-r);
}

vec4 Rplane(vec3 point) {
    return vec4(0,sin(point.z)+sin(point.x),0, point.y+6.0);
}

float map(in vec3 p) {
    float d = Rplane(p).w;
    d = min(d, Rball(p,p - vec3(0.0, 0.25, 0.0), 0.25, vec3(1,0,0)).w);
    return d;
}

vec3 calcNormal(vec3 p) {
	vec3 e = vec3(0.001, 0.0, 0.0);
    vec3 nor = vec3(
        map(p + e.xyy) - map(p - e.xyy),
        map(p + e.yxy) - map(p - e.yxy),
        map(p + e.yyx) - map(p - e.yyx)
    );
    return normalize(nor);
}

hitInfo DE(vec3 point) { //distance estimation
    //return vec4(1,0,0,distance(point, vec3(0,0,3))-20.0);
    vec4 ball1d = Rball(point, ball1.pos, ball1.r, ball1.color);
    vec4 plane1d = Rplane(point);
    if(plane1d.w<ball1d.w){return hitInfo(plane1d, 1);}
    return hitInfo(ball1d, 2);
}


vec3 getNormal(vec3 point, int type) { //calculating normal every step is expensive, so...
    if(type == 1) {return vec3(0,1,0);}
    if(type == 2) {return normalize(point - ball1.pos);}
    return vec3(0,0,0);
}

light closestLight(vec3 point) {
    return light1;
    if(distance(point, light1.pos)<distance(point, light2.pos)){return light1;}
    return light2;
}

float traceToLight(vec3 pos, vec3 normal, light l) {
    vec3 point = pos;
    float dis = 0.1;
    for(float i = 0.0; i<LIGHTLOOPS; ++i) {
        point += normalize(l.pos-point)*dis;
        hitInfo castInfo = DE(point);
        dis = castInfo.info.w;
        if(dis<0.0001){return 0.0;}
    }
    float light = (dot(normal,normalize(l.pos-pos)));
    return clamp(light/(pow(distance(l.pos, pos), 2.0))*light1.intensity, 0.,1.);
}

vec3 createCast(vec3 origo, vec3 tgt, vec3 up, vec2 plane, float fov) {
    vec3 az = normalize(tgt - origo);
    vec3 ax = normalize(cross(az, up));
    vec3 ay = cross(ax, az);
    
    vec3 point = tgt + fov * length(tgt-origo) * (plane.x * ax + plane.y * ay);
    return normalize(point-origo);
}

vec4 casty(vec3 start, vec3 dir) {
    vec3 point = start;
    dir = normalize(dir);
    for(float i = 0.0; i<LOOPS; ++i) {
        hitInfo castInfo = DE(point);
        float dis = castInfo.info.w;
        if(dis<0.1) {
            float light = traceToLight(point, getNormal(point, castInfo.type), closestLight(point));
            return vec4(castInfo.info.xyz*light+castInfo.info.xyz*0.0,1);
        }
        point += dir*dis;
    }
    return vec4(0,0,0,0);
}

void main(void)
{
    vec3 campos = vec3(sin(iGlobalTime)*50.,sin(iGlobalTime)*10.+10.,-50.*cos(iGlobalTime));
    light1.pos = vec3(-5.,40., -10.);
    light2.pos = vec3(20.*cos(iGlobalTime*SPEED-3.0), 30, 20);
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec4 color = casty(campos, createCast(campos, vec3(0,0,0), vec3(0,1,0), (gl_FragCoord.xy - 0.5 * iResolution.xy) / iResolution.y, 1.0));
	gl_FragColor = color+vec4(0.,0.0,0.0,0.0);
}
