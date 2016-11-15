// https://www.shadertoy.com/view/MslXW8
// imperative-style sdf raymarching toolkit each SDF function provides an analytical gradient on top of that, color is shaded by curvature (http://www.cs.utah.edu/~gk/papers/vis03/)
// v0.1

//#define HEROKU
#ifdef HEROKU
#ifdef GL_ES
precision mediump float;
#endif
// compatibility for glsl.heroku.com 
uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;
#define iGlobalTime time
#define iResolution resolution
#endif

#define EPS 1e-5

vec2 aspect = vec2(iResolution.x / iResolution.y, 1.0);
vec2 uv = gl_FragCoord.xy / iResolution.xy;
vec2 position = (uv*2.0-1.0)*aspect;
vec2 nmouse = iMouse.xy / iResolution.xy;
float ScreenH = min(iResolution.x,iResolution.y);
float AA = ScreenH*0.4;
float AAINV = 1.0 / AA;

////////////////////////////////////////////////////////////////////

void scene();

////////////////////////////////////////////////////////////////////

struct Trace {
	float d;
	vec3 grad;
};
	
struct Context {
	vec3 position;
	vec3 world_normal;
	vec3 world_position;
	vec3 albedo;
	vec3 color;
	Trace trace;
	mat4 mtx_view;
	bool hit;
} _ctx = Context(
	vec3(0.0), vec3(0.0), vec3(0.0),
	vec3(1.0),
	vec3(0.0,0.0,0.0),
	Trace(1e+20, vec3(0.0)),
	mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0),
	false
);

float run_scene(vec3 p) {
	_ctx.trace.d = 1e+20;
	_ctx.position = p;
	scene();
	return _ctx.trace.d;
}

vec3 calc_gradient(vec3 p) {
	_ctx.trace.d = 1e+20;
	_ctx.position = p;
	scene();
	return _ctx.trace.grad;
}

float raymarch(in vec3 ro, in vec3 rd, in float maxt) {
    float precis = 0.001;
    float h = precis * 2.0;
    float t = 0.0;
    for(int i = 0; i < 60; i++) {
        if(abs(h) < precis || t > maxt) continue;
		h = run_scene(ro + rd*t);
        t += h;
    }
    return t;
}

float softshadow(vec3 ro, vec3 rd, float mint, float maxt, float k) {
    float sh = 1.0;
    float t = mint;
    float h = 0.0;
    for(int i = 0; i < 30; i++) {
        if(t > maxt) continue;
		h = run_scene(ro + rd*t);
        sh = min(sh, k * h / t);
        t += h;
    }
    return sh;
}

void look_at(vec3 eye, vec3 center) {
	vec3 f = normalize(center - eye);
	vec3 u = vec3(0.0,0.0,1.0);
	vec3 s = normalize(cross(f,u));
	u = cross(s,f);

	_ctx.mtx_view = mat4( 
		  s, 0.0,
          u, 0.0,
          f, 0.0,
          eye, 1.0);
}

void orbit() {
	float a = radians(180.0*(nmouse.x*2.0-1.0));
	vec3 p = vec3(vec2(sin(a), -cos(a))*3.0, nmouse.y*5.0);
	
	look_at(p, vec3(0.0));
}

void trace() {
	vec3 ro = (_ctx.mtx_view * vec4(position.xy,0.0, 1.0)).xyz;
	vec3 rd = mat3(_ctx.mtx_view) * vec3(0.0,0.0,1.0);
	float t = raymarch(ro, rd, 10.0);
	_ctx.hit = (t < 10.0);
	
	_ctx.world_normal = -_ctx.trace.grad;
	_ctx.world_position = ro+rd*t;//_ctx.position;
}

mat3 transpose(mat3 m) {
	return mat3(
		m[0].x, m[1].x, m[2].x,
		m[0].y, m[1].y, m[2].y, 
		m[0].z, m[1].z, m[2].z
	);
}

const float H_ERR = 1e-4;
mat3 hessian(vec3 p) {
	vec2 d = vec2(0.0, H_ERR);
	
	vec3 gx = calc_gradient(p + d.yxx) - calc_gradient(p - d.yxx);
	vec3 gy = calc_gradient(p + d.xyx) - calc_gradient(p - d.xyx);
	vec3 gz = calc_gradient(p + d.xxy) - calc_gradient(p - d.xxy);
	
	return transpose(mat3(gx,gy,gz)) / (2.0*H_ERR);
}

const mat3 identity = mat3(
	1.0, 0.0, 0.0,
	0.0, 1.0, 0.0,
	0.0, 0.0, 1.0
);
vec4 curvature(vec3 p) {

	vec3 g = calc_gradient(p);
	float gl = length(g);
	vec3 n = -g / gl; // -g / |g|
	
	// I - n*n^T
	mat3 P = identity - mat3(
		n * n.x,
		n * n.y,
		n * n.z
	);	
	
	// hessian
	mat3 H = hessian(p);
	
	// G = -PHP/|g|
	mat3 G = -(P*H*P)*(1.0/gl);
	// trace of G
	float T = G[0][0]*G[0][0] + G[1][1]*G[1][1] + G[2][2]*G[2][2];
	// squared frobenius norm F of G
	float FF = dot(G[0],G[0]) + dot(G[1],G[1]) + dot(G[2],G[2]);
	// principal curvatures
	float TT = T*T;
	float QQ = abs(2.0*FF - TT);
	float Q = sqrt(QQ);
	float k1 = (T + Q)*0.5;
	float k2 = (T - Q)*0.5;	
	
	// principals, mean, gaussian
	return vec4(k1,k2,T*0.5,0.25*(TT-QQ));
}


vec3 filmic_gamma3(vec3 linear) {
    vec3 x = max(vec3(0.0), linear-0.004);
    return (x*(x*6.2+0.5))/(x*(x*6.2+1.7)+0.06);
}

void tonemap(float exposure) {
	_ctx.color = clamp(filmic_gamma3(_ctx.color * exposure), 0.0, 1.0);
}

void shade_normals() {
	if(!_ctx.hit) return;
	_ctx.color = _ctx.world_normal*0.5+0.5;
}

vec3 physhue2rgb(float hue, float ratio) {
    return smoothstep(
        vec3(0.0),vec3(1.0),
        abs(mod(hue + vec3(0.0,1.0,2.0)*(1.0/ratio),1.0)*2.0-1.0));
}

vec3 orthogonal(vec3 v) {
    return abs(v.x) > abs(v.z) ? vec3(-v.y, v.x, 0.0)
                               : vec3(0.0, -v.z, v.y);
}

void albedo_curvature() {
	vec4 curv = curvature(_ctx.world_position);
	
	vec3 c = physhue2rgb(clamp(curv.x*0.01,-1.0,1.0)*0.5+0.4, 3.0);
	c.z = 0.5;
	c = mix(c, vec3(1.0), 0.25);
	c *= c;
	_ctx.albedo = c;
}

void shade_ao(vec3 c) {
	const float st = 0.2;
	float total = 0.0;
	float weight = 0.5;
	for (int i = 1; i <= 5; ++i)
	{
		float d1 = st * float(i);
		float d2 = run_scene(_ctx.world_position + _ctx.world_normal * d1);
		total += weight * (d1 - d2);
		weight *= 0.5;
	}
	
	_ctx.color += _ctx.albedo * c * clamp(1.0 - 2.0 * total, 0.0, 1.0);
}

void shade_light_direct(vec3 c, vec3 l) {
	if(!_ctx.hit) return;
	
	float shadow = clamp(softshadow(_ctx.world_position, l, 0.1, 20.0, 7.0),0.0,1.0);
	
	float cos_Ol = max(0.0, dot(_ctx.world_normal, l));
	_ctx.color += _ctx.albedo * c * (shadow * cos_Ol);
}

void blit() {
	gl_FragColor = vec4(_ctx.color,1.0);
}

Trace pop() {
	Trace t = _ctx.trace;
	_ctx.trace.d = 1e+20;
	return t;
}

void add(Trace t) {
	_ctx.trace = (t.d < _ctx.trace.d)?t:_ctx.trace;
}

void add(float d, vec3 grad) {
	add(Trace(d, grad));
}

void intersect(Trace t) {
	_ctx.trace = (t.d >= _ctx.trace.d)?t:_ctx.trace;
}

void subtract(Trace t) {
	t.d = -t.d;
	t.grad = -t.grad;
	intersect(t);
}

// k = 0.1
void smooth_union_poly(Trace b, float k) {
    float h = 0.5+0.5*(b.d-_ctx.trace.d)/k;
    h = clamp(h, 0.0, 1.0);
    _ctx.trace = Trace(
		b.d + (_ctx.trace.d-b.d)*h - k*h*(1.0-h),
		b.grad + (_ctx.trace.grad-b.grad)*h - k*h*(1.0-h));
}

void plane(vec3 p, vec3 n) {
	p -= _ctx.position;
	float s = -dot(p, n);	
	add(s, -n);
}

void sphere(vec3 p, float r) {
	p -= _ctx.position;
	float s = length(p);
	add(s-r,p / s);
}

void round_box(vec3 p, vec3 b, float r) {
	p -= _ctx.position;
	vec3 ap = abs(p);
	vec3 n = (ap / p) * max(ap-b,0.0);
  	float d = length(n);
	add(d-r, n/d);
}

void torus(vec3 p, vec2 t) {
	p -= _ctx.position;
	
	float ln = length(p.xz);
	vec2 q = vec2(ln-t.x,p.y);
	float d = length(q);
	
	vec3 n = p - (vec3(p.x, 0.0, p.z) / ln)*t.x;
	
	add(d - t.y, n / d);
}

void outline(float w) {
	float d = abs(_ctx.trace.d);
	_ctx.trace.grad *= d / _ctx.trace.d;
	_ctx.trace.d = d - w;
}

////////////////////////////////////////////////////////////////////

void scene() {
	
	float c = sin(iGlobalTime)*0.5+0.5;

	torus(vec3(0.5+c,0.0,0.0), mix(vec2(0.45,0.05),vec2(0.25,0.25), c));
	Trace t = pop();
	
	sphere(vec3(-1.0,0.0,0.0), 0.3);
	
	round_box(vec3(0.0), mix(vec3(0.45),vec3(0.3),c), mix(0.05,0.2,c));
	
	plane(vec3(0.0,0.0,-0.5), vec3(0.0,0.0,1.0));
	smooth_union_poly(t, 0.1);
}

void main(void)
{
	float t = iGlobalTime * 0.1;
	
	vec3 l = normalize(vec3(sin(t),-cos(t),0.8));
	
	orbit();
	trace();
	albedo_curvature();
	shade_light_direct(vec3(1.0,0.8,0.5), l);
	shade_ao(vec3(0.2,0.5,1.0)*0.2);
	tonemap(1.5);
	//shade_normals();
	blit();
}
