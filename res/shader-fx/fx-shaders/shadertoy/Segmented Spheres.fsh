// https://www.shadertoy.com/view/4dsGW8
// Part from our Revision 2013 entry Intrinsic

#ifdef GL_ES
precision mediump float;
#endif

vec3 rotatex(in vec3 p, float ang)
{
	return vec3(p.x,p.y*cos(ang)-p.z*sin(ang),p.y*sin(ang)+p.z*cos(ang)); 
}
vec3 rotatey(in vec3 p, float ang)
{
	return vec3(p.x*cos(ang)-p.z*sin(ang),p.y, p.x*sin(ang)+p.z*cos(ang)); 
}

float scene(in vec3 p)	
{
	float r = 0.4; 
	float ox = float(int(p.x*50.0+50.0*r)); 
	float oy = float(int(p.y*50.0+50.0*r)); 
	
	p.x = mod(p.x+r, 2.0*r) - r; 
	p.y = mod(p.y+r, 2.0*r) - r; 
	p = rotatex(p, oy*0.1); 
	p = rotatey(p, ox*0.1); 
	p.z += sin(ox*ox*0.01+oy*oy*0.00+iGlobalTime)*0.01; 
	return length(p) - r; 
}
vec3 get_normal(in vec3 p)
{
	vec3 eps = vec3(0.001, 0, 0); 
	float nx = scene(p + eps.xyy) - scene(p - eps.xyy); 
	float ny = scene(p + eps.yxy) - scene(p - eps.yxy); 
	float nz = scene(p + eps.yyx) - scene(p - eps.yyx); 
	return normalize(vec3(nx,ny,nz)); 
}

float rm2(in vec3 ro, in vec3 rd)
{
	vec3 pos = ro; 
	float dist = 1.0; 
	float d; 
	for (int i = 0; i < 5; i++) {
		d = scene(pos); 
		pos += rd*d;
		dist -= d; 
	}
	return dist; 
}

vec3 rm3(in vec3 ro, in vec3 rd)
{
	vec3 color = vec3(0); 
	vec3 pos = ro; 
	float dist = 1.0; 
	float d; 
	for (int i = 0; i < 32; i++) {
		d = scene(pos); 
		pos += rd*d;
		dist -= d; 
	}
	if (dist < 10.0 && abs(d) < 0.1) {
		vec3 l = normalize(vec3(1,1,1)); 
		vec3 n = get_normal(pos); 
		vec3 r = reflect(rd, n); 
		float fres = clamp(dot(n, -rd),0.0, 1.0);  
		float diff = clamp(dot(n, l), 0.0, 1.0); 
		float spec = pow(clamp(dot(r, l), 0.0, 1.0), 40.0);  
		float shade = 1.0; //rm2(pos+0.05*n, n); 
		color = mix(vec3(1,1,1)*0.8, vec3(1,1,1)*0.0, fres); 
		color += 0.1*vec3(1,1,1)*diff*clamp(n.y, 0.0, 1.0);  
		color += vec3(1,1,1)*spec; 
		color += 0.0*shade; 
		//color /= dist; 
	}
	return color; 
}


void main( void ) {

	vec2 p = 2.0*( gl_FragCoord.xy / iResolution.xy )-1.0;
	p.x *= iResolution.x/iResolution.y; 

	vec3 color = vec3(0.1); 
	
	vec3 ro = vec3(0,-iGlobalTime*0.025,1.0);
	vec3 rd = normalize(vec3(p.x,p.y,-2.0)); 
	rd = rotatex(rd, 1.2); 
	vec3 pos = ro; 
	float dist = 0.0; 
	float d; 
	for (int i = 0; i < 96; i++) {
		d = scene(pos)*0.75; 
		pos += rd*d;
		dist += d; 
	}
	if (dist < 10.0 && abs(d) < 0.1) {
		vec3 l = normalize(vec3(0,1,0.5)); 
		vec3 n = get_normal(pos); 
		vec3 r = reflect(rd, n); 
		float fres = clamp(dot(n, -rd),0.0, 1.0);  
		float diff = clamp(dot(n, l), 0.0, 1.0); 
		float spec = pow(clamp(dot(r, l), 0.0, 1.0), 40.0);  
		float shade = rm2(pos+0.05*n, 1.0*n); 
		vec3 refl = rm3(pos+0.01*n, r); 
		color = mix(vec3(1,0.99,0.98)*0.8, vec3(1,1,1)*0.0, fres); 
		color += 0.1*vec3(1,1,1)*diff*clamp(n.y, 0.0, 1.0);  
		color += vec3(1,1,1)*spec; 
		color *= 1.0*(1.0-1.0*shade);  
		color += 0.5*refl*(1.0-fres);  
		color /= dist; 
	}
	
	// light "glow"
	color += vec3(0.97,0.99,1.0)*clamp((1.0-length(p*vec2(0.5,1.0)-vec2(0.0,0.75))), 0.0, 1.0)*(0.5); 
	
	gl_FragColor = vec4(color, 1.0); 
}
