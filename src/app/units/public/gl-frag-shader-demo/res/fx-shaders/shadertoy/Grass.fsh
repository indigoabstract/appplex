// https://www.shadertoy.com/view/XsX3R2

// iChannel0: t11

// Attempt at raymarching grass using distance fields

const float PI = 3.14159265359;

vec3 ldir = normalize(vec3(0.1,0.5,-1.5));

vec3 rotateX(vec3 p, float theta)
{
	return vec3(p.x, cos(theta)*p.y-sin(theta)*p.z,cos(theta)*p.z+sin(theta)*p.y);
}

vec3 rotateY(vec3 p, float theta)
{
	return vec3(cos(theta)*p.x-sin(theta)*p.z,p.y,cos(theta)*p.z+sin(theta)*p.x);
}

float grass(vec3 p)
{
	vec3 d = abs(p) - vec3(0.025,1.0,0.025);
	return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,vec3(0.0)));
}

vec3 skycolour(vec3 dir)
{
	float sun = clamp(dot(ldir,dir), 0.0, 1.0);
	vec3 sky = vec3(0.6,0.71,1.75) - dir.y*0.2*vec3(1.0,0.5,1.0) + 0.075;
	sky += 0.2*vec3(1.0,0.6,0.1)*pow(sun, 8.0);
	sky *= 0.95;
	sky += 0.1*vec3(1.0,0.4,0.2)*pow(sun, 3.0);
	return sky;
}

float dist(vec3 p)
{	
	vec3 d = texture2D(iChannel0,(p.xz / 32.0)).rgb;
	p.y += sin(p.x*2.0)*cos(p.z*2.0)*0.85*d.x;
	vec3 p0 = p + d*vec3(0.5,0.4,0.5);
	vec3 p1 = p - d*vec3(0.5,0.4,0.5);
	p0.xz = mod(p0.xz,vec2(0.5)) - vec2(0.25);
	p1.xz = mod(p1.xz,vec2(0.4)) - vec2(0.20);

	vec2 w = vec2(cos(iGlobalTime+d.x*1.0),sin(iGlobalTime+d.y*1.0)) * (p.y+2.0) * 0.05;
	p0.xz += w;
	p1.xz -= w;
	
	p0 = rotateX(p0,w.x*PI*0.450*p.y*cos(iGlobalTime));
	p1 = rotateX(p1,w.y*PI*0.250*p.y*cos(iGlobalTime));
	float ground = dot(p,vec3(0.0,1.0,0.0)) + 2.0;
	return min(grass(p0),min(ground,grass(p1)));
}

vec3 calc_normal(vec3 p)
{
	vec3 n;
	n.x = dist(p + vec3(0.01,0.0,0.0)) - dist(p - vec3(0.01,0.0,0.0));
	n.y = dist(p + vec3(0.0,0.01,0.0)) - dist(p - vec3(0.0,0.01,0.0));
	n.z = dist(p + vec3(0.0,0.0,0.01)) - dist(p - vec3(0.0,0.0,0.01));
	return normalize(n);
}

float calc_ao(vec3 p, vec3 n)
{

	float r = 0.0;
	float w = 1.0;
	for (float i=1.0; i<=5.0; i++)
	{
		float d0 = (i / 5.0) * 1.25;
		r += w * (d0 - dist(p + n * d0));
		w *= 0.5;
	}
	float ao = 1.0 - clamp(r,0.0,1.0);
	return ao;
}

vec4 march(vec3 ro, vec3 rd)
{
	float t = 0.0;
	for (int i=0; i<256; i++)
	{
		vec3 p = ro + rd * t;
		float d = dist(p);
		if (abs(d) < 0.01)
			return vec4(p,1.0);
		t += d;
		if (t >= 100.0)
			break;
	}
	return vec4(ro + rd * t, 0.0);
}

vec3 schlick(vec3 f0, vec3 h, vec3 v)
{
	return f0 + (1.0 - f0) * pow(1.0-max(dot(v,h),0.0),5.0);
}

vec3 shade(vec3 pos, vec3 v)
{
	const float spec_power = 4.0;
	const float normalization = (spec_power + 2.0) / 8.0;
	const vec3 f0 = vec3(0.057,0.057,0.037);
	
	vec3 n = calc_normal(pos);
	
	float g = clamp(1.0 - exp(-pos.y*1.0), 0.0, 1.0);
	vec3 albedo = mix(vec3(0.2,0.4,0.03),vec3(0.65,1.0,0.15),g);
	
	float ao = calc_ao(pos,n);
	
	vec3 h = normalize(v+ldir);
	vec3 fresnel = schlick(f0,h,v);
	
	float ndl = dot(n,ldir) * 0.5 + 0.5;
	float ndh = max(dot(n,h),0.0);
	float ndv = max(dot(n,v),0.0);
	
	float alpha = 1.0 / sqrt((PI/4.0) * spec_power + (PI/2.0));
	float vis = (ndl * (1.0 - alpha) + alpha) * (ndv * (1.0 - alpha) + alpha); 
	vis = 1.0 / vis;
	
	vec3 spec = vec3(pow(ndh, spec_power)) * normalization * vis * ndl;
	return albedo * (spec * fresnel + ndl * (1.0 - fresnel) + 
					 ao * skycolour(reflect(-v,n)) * 1.5);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec3 rd = vec3(-1.0 + 2.0 * uv.xy, 1.0);
	rd.x *= iResolution.x / iResolution.y;
	rd = normalize(rd);
	
	vec2 p = 128.0*vec2(cos(0.75+0.0047*iGlobalTime), sin(1.5+0.0021*iGlobalTime));
	
	vec3 ro = vec3(p.x,7.0,p.y);
	rd = rotateX(rd, (sin(iGlobalTime/7.0) * (PI / 8.0)) + (PI / 10.0));
	rd = rotateY(rd, cos(iGlobalTime/14.0) * PI / 4.0);
	
	gl_FragColor.w = 1.0;
	vec4 res = march(ro,rd);
	if (res.w == 1.0)
	{
		gl_FragColor.rgb = shade(res.xyz,-rd);
	}
	else
	{
		vec3 sky = skycolour(rd);
		gl_FragColor.rgb = sky;
	}
}
