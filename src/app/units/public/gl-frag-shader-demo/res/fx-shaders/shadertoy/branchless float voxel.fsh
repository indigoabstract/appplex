//https://www.shadertoy.com/view/MssXR8
//The raycasting code is somewhat based around a 2D raycasting toutorial found here: 
//http://lodev.org/cgtutor/raycasting.html
#define COMPARE
const int MAX_RAY_STEPS = 80;
float sdSphere(vec3 p, float d) { return length(p) - d; } 
 
struct AABB {
   vec3 Min;
   vec3 Max;
};
AABB aabb =AABB(vec3(-22.0),vec3(22.0));

float sdBox( vec3 p, vec3 b )
{
  	vec3 d = abs(p) - b;
	return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

vec2 rotate2d(vec2 v, float a) {
	float sinA = sin(a);
	float cosA = cos(a);
	return vec2(v.x * cosA - v.y * sinA, v.y * cosA + v.x * sinA);	
}	
	float hash( float n ) { return fract(sin(n)*43758.5453123); }
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
    f = f*f*(3.0-2.0*f);
	
    float n = p.x + p.y*157.0 + 113.0*p.z;
    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                   mix( hash(n+157.0), hash(n+158.0),f.x),f.y),
               mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                   mix( hash(n+270.0), hash(n+271.0),f.x),f.y),f.z);
}
	
bool getVoxel(vec3 c) {
	
	vec3 p = c+0.5 ;
	
	
	p = p+mod(p,aabb.Max.x*2.0)-aabb.Max.x;
	//if (noise(p)>0.5){
	return ( noise(p)*min(max(-sdSphere(p, aabb.Max.x+5.0), sdBox(p, aabb.Max)), -sdSphere(p, 50.0)) <0.0);
	//}	else {
	//	return false;
	//}
}

bool raystep(inout vec3 sideDist, inout vec3 mapPos, inout vec3 mask, const vec3 deltaDist, const vec3 rayStep){
	#ifndef COMPARE
	bvec3 b1 = lessThan(sideDist.xyz, sideDist.yzx);
	mask = vec3(lessThanEqual(sideDist.xyz, vec3(b1)*sideDist.zxy));
	#else
	vec3 cp = step( sideDist, sideDist.yzx );
	mask = (cp * ( vec3(1.0 ) - cp.zxy ));
	#endif
	
	sideDist += mask * deltaDist;
	mapPos += mask * rayStep;
	return getVoxel(mapPos);
}
vec3 gamma(vec3 col){
	return pow(col, vec3(1.0 / 1.2));
}
void main(void)
{
	float t = iGlobalTime;
	vec2 screenPos = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;
	vec3 rayDir = vec3(0.0, 0.001, 1.0) + screenPos.x * vec3(1.0, 0.0, 0.0) + screenPos.y * vec3(0.0, 1.0, 0.0)* iResolution.y / iResolution.x;
	vec3 rayPos = vec3(0.0, 13.0, -13.0);
	rayDir.xz = rotate2d(rayDir.xz, t);
	rayPos.xz = rotate2d(rayPos.xz, t);
	vec3 mapPos = floor(rayPos);
	vec3 deltaDist = abs(length(rayDir)/rayDir);
	vec3 rayStep = sign(rayDir);
	vec3 sideDist = ((rayStep*((mapPos) - rayPos) + rayStep * 0.5) + 0.5) * deltaDist; 
	vec3 mask=vec3(0.0);
	bool hit=false;
	
	for (int i = 0; i < MAX_RAY_STEPS; i++) {
		hit = ( hit == false ) ? raystep(sideDist, mapPos, mask, deltaDist, rayStep):true;
		if (hit) continue; // should be break really ;)
	}
	gl_FragColor.rgb = gamma(mask/(mapPos-rotate2d(rayPos.xz, t).xyy+20.5));
}
