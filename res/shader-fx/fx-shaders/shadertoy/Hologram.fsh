// https://www.shadertoy.com/view/Xss3RH

// iChannel0: t4

// @eddbiddulph

#define ITERS 64

vec3 rotateX(float a, vec3 v)
{
	return vec3(v.x, cos(a) * v.y + sin(a) * v.z, cos(a) * v.z - sin(a) * v.y);
}


vec3 rotateY(float a, vec3 v)
{
	return vec3(cos(a) * v.x + sin(a) * v.z, v.y, cos(a) * v.z - sin(a) * v.x);
	
}

// Returns the entry and exit points along the given ray with
// a cube of edge length 2 centered at the origin.
vec2 cubeInterval(vec3 ro, vec3 rd)
{
	vec3 slabs0 = (vec3(+1.0) - ro) / rd;
	vec3 slabs1 = (vec3(-1.0) - ro) / rd;
	
	vec3 mins = min(slabs0, slabs1);
	vec3 maxs = max(slabs0, slabs1);
	
	return vec2(max(max(mins.x, mins.y), mins.z),				
				min(min(maxs.x, maxs.y), maxs.z));
	
}

vec2 hologramInterval(vec3 ro, vec3 rd)
{
	vec3 scale = vec3(1.0, 1.0, 0.1);   
	return cubeInterval(ro / scale, rd / scale);
}

vec2 powerboxInterval(vec3 ro, vec3 rd)
{
	vec3 scale = vec3(0.1, 0.2, 0.08);
	vec3 trans = vec3(-1.1, 0.8, 0.0);
	return cubeInterval((ro - trans) / scale, rd / scale);
}

float hologramBrightness(vec2 p)
{
	return dot(texture2D(iChannel0, p).rgb, vec3(1.0 / 3.0));
}

float flicker(float x)
{
	x = fract(x);
	return smoothstep(0.0, 0.1, x) - smoothstep(0.1, 0.2, x);
}

float flickers()
{
	return 1.0 + flicker(iGlobalTime) + flicker(iGlobalTime * 1.2);
}

vec3 hologramImage(vec2 p)
{
	vec2 tc = (p * -1.0 + vec2(1.0)) * 0.5;

	float d = 1e-3;
	
	float b0 = hologramBrightness(tc);
	float b1 = (hologramBrightness(tc + vec2(d, 0.0)) - b0) / d;
	float b2 = (hologramBrightness(tc + vec2(0.0, d)) - b0) / d;
	
	float f = flickers();
	float sharp = pow(length(vec2(b1, b2)) * 0.1, 5.0) * 0.02;
	
	return (vec3(sharp + b0) * 3.0) * vec3(0.5, 0.7, 1.0) *
				mix(0.5, 0.9, pow(0.5 + 0.5 * cos(p.y * 80.0 + iGlobalTime * 70.0), 4.0)) * f *
		(2.0 - tc.y * 2.0 + (1.0 - smoothstep(0.0, 0.1, tc.y)) * 10.0);
}

vec3 floorTex(vec3 ro, vec3 rd)
{
	float t = (1.0 - ro.y) / rd.y;
	float hit = step(t, 0.0);
	vec2 tc = ro.xz + rd.xz * t;
	vec3 glow = vec3(0.6, 0.8, 1.0) * 1.0 / length(tc * vec2(0.3, 1.0)) * 0.2 * mix(1.0, flickers(), 0.25);
	float w = abs(cos(tc.x) * 0.3 * min(1.0, tc.x * 0.5) - tc.y);
	float wires = max(step(-1.0, tc.x), smoothstep(0.02, 0.03, w));
	return mix(vec3(0.0), glow * wires, hit);
}

void main(void)
{
	vec2 uv = (gl_FragCoord.xy / iResolution.xy - vec2(0.5)) * 2.0;
	
	uv.x *= iResolution.x / iResolution.y;
	
	vec3 ct = vec3(0.0, 0.0, 0.0);
	vec3 cp = rotateY(iGlobalTime * 0.5 * 0.5, vec3(0.0, -0.5 + sin(iGlobalTime * 0.7 * 0.5) * 0.7, 1.2));
	vec3 cw = normalize(ct - cp);
	vec3 cu = normalize(cross(cw, vec3(0.0, 1.0, 0.0)));
	vec3 cv = normalize(cross(cu, cw));
	
	mat3 rm = mat3(cu, cv, cw);

	vec3 ro = cp, rd = rm * vec3(uv, -1.0);
	
	vec2 io = hologramInterval(ro, rd);
	vec2 io_pb = powerboxInterval(ro, rd);
	
	float cov = step(io.x, io.y);
	float cov_pb = step(io_pb.x, io_pb.y);
		
	float len = abs(io.x - io.y);
	
	vec3 accum = vec3(0.0);	

	vec3 fl = floorTex(ro, rd);
	
	io.x = min(0.0, io.x);

	ro += rd * io.x;
	rd *= (io.y - io.x) / float(ITERS);
	
	for(int i = 0; i < ITERS; i += 1)
	{
		vec3 rp = ro + rd * float(i);
		accum += hologramImage(rp.xy) * sqrt(abs(1.0 - abs(rp.z) * 10.0));
	}
	
	vec3 fcol = mix(fl, vec3(0.0), cov_pb);
	
	fcol = mix(fcol, fcol + accum * cov * (len / float(ITERS)) * 2.0,
			   	max(1.0 - cov_pb, step(io_pb.x, io.x)));
	
	gl_FragColor.rgb = fcol;
	gl_FragColor.a = 1.0;
}
