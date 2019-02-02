// https://www.shadertoy.com/view/lsf3DH
precision lowp float;

 float time = iGlobalTime*0.4;

 float width  = iResolution.x;
 float height = iResolution.y;
const float delta  = 0.02;
const float PI =  3.14159265;

float sphere(vec3 position, float r)
{
        return length(position) - r ;//+ 0.53*sin(position.y*1.1 + mod(time*0.2, 2.0*PI)-PI)
        //+ 0.17*sin(position.z*2.2 + mod(time*0.02, 2.0*PI)-PI);
}


vec3 rotateX(vec3 pos, float alpha) {
	mat4 trans= mat4(1.0, 0.0, 0.0, 0.0,
				0.0, cos(alpha), -sin(alpha), 0.0,
				0.0, sin(alpha), cos(alpha), 0.0,
				0.0, 0.0, 0.0, 1.0);
				
				
	return vec3(trans * vec4(pos, 1.0));
}

vec3 rotateY(vec3 pos, float alpha) {

				
	mat4 trans2= mat4(cos(alpha), 0.0, sin(alpha), 0.0,
				0.0, 1.0, 0.0, 0.0,
				-sin(alpha), 0.0, cos(alpha), 0.0,
				0.0, 0.0, 0.0, 1.0);
				
	return vec3(trans2 * vec4(pos, 1.0));
}

vec3 translate(vec3 position, vec3 translation) {
	return position - translation;
}


float cube(vec3 pos,float size){
    return max(max(abs(pos.x)-size,abs(pos.y)-size),abs(pos.z)-size) ;//+ 0.17*sin(pos.z*2.2 + mod(time*0.02, 2.0*PI)-PI);
}

float sdTorus( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float udRoundBox( vec3 p, vec3 b, float r )
{
  return length(max(abs(p)-b,0.0))-r;
}

float sdSphere( vec3 p, float s )
{
  return length(p)-s;
}
/*
float function(vec3 position) {
	

	return  sdTorus(
			
			rotateX(
			rotateY(
			
			translate(position.xyz,vec3(0.0,0.0,20.0))
			,
			time* 2.3),position.x*0.8 + time* 0.1)
			
			, vec2(4.0,2.0));
}*/

	


float opS( float d1, float d2 )
{
    return max(-d1,d2);
}

float opRep( vec3 p, vec3 c )
{
    vec3 q = mod(p,c)-0.5*c;
    return sdSphere( q ,0.46);
}

	vec3 opRep2( vec3 p, vec3 c )
{
   return mod(p,c)-0.5*c;

}

float wonderCube(vec3 position) {

vec3 disp = vec3(0.0,0.0,6.0);

vec3 newPos = rotateY(rotateX(translate(position.xyz,disp),1.3*time),1.0*time);
return opS( opRep(newPos, vec3(1.3,1.3,1.3)),udRoundBox(newPos, vec3(8.2,6.2,9.2), 0.5))
;
}

float opCheapBend( vec3 p )
{

	

    float c = cos(0.9*(1.0+sin(time*0.2))+p.y*0.018);
    float s = sin(0.9*(1.0+sin(time*0.2))+p.y*0.018);
    mat2  m = mat2(c,-s,s,c);
    vec3  q = vec3(m*p.xy,p.z);
	float alpha =//(1.0+sin(time*1.0))/4.0;
0.38;//(1.0+sin(time*1.0))/4.0;

vec3 disp = vec3(0.0,0.0,6.0);

vec3 newPos = rotateY(rotateX(translate(q.xyz,disp),1.3*time),1.0*time);
   // newPos = opRep2(newPos, vec3(8.0));
    return wonderCube(q)*(1.0-alpha) + sdTorus(newPos, vec2(0.3,0.001))*alpha;
}

float opU( float d1, float d2 )
{
    return min(d1,d2);
}

float opTwist( vec3 p )
{
    float c = cos(0.90*p.y);
    float s = sin(0.90*p.y);
    mat2  m = mat2(c,-s,s,c);
    vec3  q = vec3(m*p.xz,p.y);
    return wonderCube(q);
}

float opDisplace( vec3 p )
{
    float d1 = sdTorus(p, vec2(4.0,2.0));
    float d2 = (sin(p.x*3.0) + sin(p.y*5.0)) *0.2;
    return d1+d2;
}


float opBlend( vec3 position )
{
vec3 disp = vec3(0.0+4.0*sin(time*2.2+3.0),0.0+2.0*sin(time*1.3),8.0+0.3*sin(time*1.5));

vec3 newPos = rotateY(rotateX(translate(position.xyz,disp),2.3*time),1.8*time);
    float d1 = wonderCube(position);
	vec3 p = position;
    float d2 = sin(0.800*p.x)*sin(0.800*p.y)*sin(0.800*p.z) ;
 
    return d1+d2;
}

//////////
vec3 mod289(vec3 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
  return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec3 fade(vec3 t) {
  return t*t*t*(t*(t*6.0-15.0)+10.0);
}

// Classic Perlin noise
float cnoise(vec3 P)
{
  vec3 Pi0 = floor(P); // Integer part for indexing
  vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
  Pi0 = mod289(Pi0);
  Pi1 = mod289(Pi1);
  vec3 Pf0 = fract(P); // Fractional part for interpolation
  vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
  vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  vec4 iy = vec4(Pi0.yy, Pi1.yy);
  vec4 iz0 = Pi0.zzzz;
  vec4 iz1 = Pi1.zzzz;

  vec4 ixy = permute(permute(ix) + iy);
  vec4 ixy0 = permute(ixy + iz0);
  vec4 ixy1 = permute(ixy + iz1);

  vec4 gx0 = ixy0 * (1.0 / 7.0);
  vec4 gy0 = fract(floor(gx0) * (1.0 / 7.0)) - 0.5;
  gx0 = fract(gx0);
  vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
  vec4 sz0 = step(gz0, vec4(0.0));
  gx0 -= sz0 * (step(0.0, gx0) - 0.5);
  gy0 -= sz0 * (step(0.0, gy0) - 0.5);

  vec4 gx1 = ixy1 * (1.0 / 7.0);
  vec4 gy1 = fract(floor(gx1) * (1.0 / 7.0)) - 0.5;
  gx1 = fract(gx1);
  vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
  vec4 sz1 = step(gz1, vec4(0.0));
  gx1 -= sz1 * (step(0.0, gx1) - 0.5);
  gy1 -= sz1 * (step(0.0, gy1) - 0.5);

  vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
  vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
  vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
  vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
  vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
  vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
  vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
  vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

  vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.x;
  g010 *= norm0.y;
  g100 *= norm0.z;
  g110 *= norm0.w;
  vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.x;
  g011 *= norm1.y;
  g101 *= norm1.z;
  g111 *= norm1.w;

  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
  float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
  float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
  float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
  float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
  float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
  float n111 = dot(g111, Pf1);

  vec3 fade_xyz = fade(Pf0);
  vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
  vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
  float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x); 
  return 2.2 * n_xyz;
}
///////////


int colorIndex = 0;

#define pi 3.14159265
float perlin(vec3 p) {
	vec3 i = floor(p);
	vec4 a = dot(i, vec3(1., 57., 21.)) + vec4(0., 57., 21., 78.);
	vec3 f = cos((p-i)*pi)*(-.5)+.5;
	a = mix(sin(cos(a)*a),sin(cos(1.+a)*(1.+a)), f.x);
	a.xy = mix(a.xz, a.yw, f.y);
	return mix(a.x, a.y, f.z);
}


vec3 n1 = vec3(1.000,0.000,0.000);
vec3 n2 = vec3(0.000,1.000,0.000);
vec3 n3 = vec3(0.000,0.000,1.000);
vec3 n4 = vec3(0.577,0.577,0.577);
vec3 n5 = vec3(-0.577,0.577,0.577);
vec3 n6 = vec3(0.577,-0.577,0.577);
vec3 n7 = vec3(0.577,0.577,-0.577);
vec3 n8 = vec3(0.000,0.357,0.934);
vec3 n9 = vec3(0.000,-0.357,0.934);
vec3 n10 = vec3(0.934,0.000,0.357);
vec3 n11 = vec3(-0.934,0.000,0.357);
vec3 n12 = vec3(0.357,0.934,0.000);
vec3 n13 = vec3(-0.357,0.934,0.000);
vec3 n14 = vec3(0.000,0.851,0.526);
vec3 n15 = vec3(0.000,-0.851,0.526);
vec3 n16 = vec3(0.526,0.000,0.851);
vec3 n17 = vec3(-0.526,0.000,0.851);
vec3 n18 = vec3(0.851,0.526,0.000);
vec3 n19 = vec3(-0.851,0.526,0.000);

float spikeball(vec3 p) {
	p = rotateX(p-vec3(0.0,0.0,6.0),time*3.0);
   vec3 q=p;
   p = normalize(p);
   vec4 b = max(max(max(
      abs(vec4(dot(p,n16), dot(p,n17),dot(p, n18), dot(p,n19))),
      abs(vec4(dot(p,n12), dot(p,n13), dot(p, n14), dot(p,n15)))),
      abs(vec4(dot(p,n8), dot(p,n9), dot(p, n10), dot(p,n11)))),
      abs(vec4(dot(p,n4), dot(p,n5), dot(p, n6), dot(p,n7))));
   b.xy = max(b.xy, b.zw);
   b.x = pow(max(b.x, b.y), 140.);
   return length(q)-1.75*pow(1.5,b.x*(1.-mix(.3, 1., sin(time*2.)*.5+.5)*b.x)) + 0.04* perlin(p*6.0);
}

float function(vec3 position) {
	
	///float a = opCheapBend(position);
		
	float a =udRoundBox(position-vec3(0.0,-4.0,2.2), vec3(15.0,0.01,15.0),0.0);//cube(position+vec3(0.0,6.3,-3.2), 3.0);
	vec3 pos = vec3(0.5*sin(time*3.0)*3.0,2.0,0.5*cos(time*3.0)*3.0);
	float b = //udRoundBox(rotateY(rotateX(position+vec3(0.0,-1.4,-3.4)+pos,time*3.0),time*3.0), vec3(1.4,1.4,1.4), 0.3);
opS( opRep(rotateY(rotateX(position+vec3(0.0,-1.4,-3.4)+pos,time*3.0),time*3.0), vec3(0.95,0.95,0.95)),udRoundBox(rotateY(rotateX(position+vec3(0.0,-1.4,-3.4)+pos,time*3.0),time*3.0), vec3(1.4,1.4,1.4), 0.3));// - 0.03* perlin(position*3.8);
//	float c = udRoundBox(position+vec3(0,0,-16), vec3(25.6,15.6,0.6), 0.2 );

	colorIndex = 0;
//	if(b < a) colorIndex = 1;
//	float dist =opU(a,b);
	if( b <a) colorIndex = 1;

	float dist = opU(a,b);
	return dist;
	

//	if(c < dist) colorIndex =2;

//	return opU(dist,c) ;

//return opBlend(position);

}





vec3 ray(vec3 start, vec3 direction, float t) {
	return start + t * direction;
}

vec3 gradient(vec3 position) {

	return vec3(function(position + vec3(delta, 0.0, 0.0)) - function(position - vec3(delta, 0.0, 0.0)),
	function(position + vec3(0.0,delta, 0.0)) - function(position - vec3(0.0, delta, 0.0)),
	function(position + vec3(0.0, 0.0, delta)) - function(position - vec3(0.0, 0.0, delta)));

	
}

vec4 plasma() {
 vec2 p = -1.0 +2.0 * gl_FragCoord.xy / vec2(640, 360);
float cossin1 = ((cos(p.x * 2.50 +time*2.5) +sin(p.y*3.70-time*4.5) +sin(time*2.5))+3.0)/6.0;
float cossin2 = (cos(p.y * 2.30 +time*3.5) +sin(p.x*2.90-time*1.5) +cos(time)+3.0)/6.0;
float cossin3 = (cos(p.x * 3.10 +time*5.5) +0.5*sin(p.y*2.30-time) +cos(time*3.5)+3.0)/6.0;
return vec4(vec3(cossin1, cossin2, cossin3)*0.6, 1.0);

}

vec3 lightPosition = vec3(3.0*sin(time*0.2),0.5,3.0*cos(time*0.2))-vec3(0.0,-0.5,-2.0);

float aoScale = 0.3; // smaller aoScale = more AO
float computeAO(vec3 position, vec3 normal) {
	
float sum = 0.0;
float stepSize = 0.015;
float t = stepSize;

	for(int i=0; i < 8; i++) {
		position = ray(position, normal, t);
		sum += max(function(position),0.0);
		t+=stepSize;
	}
	return 1.0-clamp(1.0 -(sum * aoScale),0.0, 1.0);
}

float computeShadow(vec3 pos) {

	float t = 0.0;
	float distance;
vec3 position;
float res = 1.0;
float k = 20.0;
	for(int i=0; i < 64; i++) {
		position = ray(pos,normalize(lightPosition-pos) , t);
		distance = function(position);

		res = min(res, k*distance/t);	
		t = t + distance ;
	}

	return res;
}

float computeShadow2(vec3 pos) {

	float t = 0.0;
	float distance;
vec3 position;
	vec3 startPos = lightPosition +normalize(pos-lightPosition)*0.;
float k = 400.0;
	float res = 1.0;
	for(int i=0; i < 64; i++) {

		position = ray(startPos,normalize(pos-lightPosition) , t);
		distance = function(position);

		if(abs(distance) < 0.009) {
			return 0.0;
		}
	res = min(res, k*distance/t);
		t = t + distance;
		if(  t >(length(pos - lightPosition))-0.0) return 1.0;
	}

	return 0.0;
}
vec4 checker(vec2 pos) {

//return vec4((sin(pos.x*10.0)+1.0)/2.0*0.5 +0.5,(sin(pos.y*10.0)+1.0)/2.0*0.5 +0.5,0.0,1.0);
return vec4(vec3(clamp((cos(pos.x) + cos(pos.y)) * 1000., 0.1, 1.)),1.0)+vec4(vec3((1.0+cnoise(vec3(pos*0.8,1.0))))/2.0*0.3,0.0);
}


vec4 computeReflection(vec3 pos, vec3 viewDirection) {
	float t = 0.0;
	float distance;
	vec3 position;
       vec3 cameraPosition = pos;
	vec4  color = vec4(16.0/255.0,25.0/255.0,27.0/255.0,1.0);//vec4(0.0,0.2,0.0,1);
	vec3 normal;
	vec3 up = normalize(vec3(-0.0, 1.0,0.0));
	
	for(int i=0; i < 32; i++) {
		position = ray(cameraPosition,	viewDirection, t);
		distance = function(position);
		
	if(position.z > 7.5) break;
		
		if(distance < 0.002 || i ==31) {
			
				
		normal = normalize(gradient(position));
			
vec4 color3;
		if(colorIndex==0) {
			float alpha = (1.0-min(pow(length((position.xyz -vec3(0.0,-4.2,4.0))*vec3(1.0*0.3,1.0,1.0*0.8))/3.0,2.0),1.0));
			//color =  vec4(color1.xyz*alpha +(1.0-alpha)*color.xyz ,1.0);
			color = checker(position.xz*4.0)*1.0*alpha +(1.0-alpha)*vec4(16.0/255.0,25.0/255.0,27.0/255.0,1.0);
			break;
				}
			if(colorIndex == 1) color3 = vec4(float(121)/255.0,float(132)/255.0,float(125)/255.0,1.0);
			if(colorIndex == 2) color3 = vec4(float(0xAD)/255.0,float(0xFF)/255.0,float(0x2F)/255.0,1.0);


			//float shad = computeShadow(position+normalize(lightPosition-position)*0.009);
			
			color = color3 * max(dot(normal, normalize(lightPosition-position)),0.0);
			color += color3 *0.2 ;

			// SPECULAR HIGHLIGHTS
			vec3 E = normalize(cameraPosition - position);
			vec3 R = reflect(-normalize(lightPosition-position), normal);
			float specular = pow(max(dot(R, E), 0.0), 12.0);

			// compute final color		
			color =vec4(color.xyz , 1.0);

			
			//color = color*0.4+ refl*0.6;
			color +=vec4(1.0, 1.0,1.0,0.0)*specular*1.0;

			// interation glow
			color += vec4(vec3(1.0, 0.5,0.1)*pow(float(i)/32.0*1.2, 2.0) *1.0,1.0);
			break;
		}
		//color += 0.6*vec4(vec3(0.4, 0.9,0.1)*pow(float(i)/64.0*2.6, 2.0) *1.0,1.0);
			t = t + distance * 1.0;
	}
								  
								  
	return color;								  
}



void main()
{	
	vec3 cameraPosition = vec3(0.0, -0.0, -1.0+0.30);
	
	float aspect = width/height;
	vec3 nearPlanePosition = vec3((gl_FragCoord.x - 0.5 * width) / width * 2.0* aspect,
							      (gl_FragCoord.y - 0.5 * height) / height *2.0,
							       -0.2+0.30);

							  
	vec3 viewDirection = normalize(nearPlanePosition - cameraPosition);
	
	float t = 0.0;
	float distance;
	vec3 position;
	// background color
	vec4 color = vec4(16.0/255.0,25.0/255.0,27.0/255.0,1.0);// plasma();//vec4(0.0,0.2,0.0,1);

	vec3 up = normalize(vec3(-0.0, 1.0,0.0));

	// update light
	lightPosition = vec3(0.0,1.4, -1.0);
	
	for(int i=0; i < 64; i++) {

		position = ray(cameraPosition,	viewDirection, t);
		if(position.z > 7.5) break;

		distance = function(position);	
		
		if(abs(distance) < 0.0006 || i ==63) {
			
			vec3 normal;
			normal = normalize(gradient(position));
			
			vec4 color1 = vec4(1.0,1.0,1.0,1.0);
			vec4 color2 = vec4(1.0, 0.1, 0.1,1.0);

			
vec4 color3 ;
	
			//vec4 color3 =
			if(colorIndex==0) {
			float alpha = (1.0-min(pow(length((position.xyz -vec3(0.0,-4.2,4.0))*vec3(1.0*0.3,1.0,1.0*0.8))/3.0,3.0),1.0));
			//color =  vec4(color1.xyz*alpha +(1.0-alpha)*color.xyz ,1.0);
vec4 refl = computeReflection(position+normal *0.004, reflect(viewDirection, normal));
			color = refl*alpha*0.5;
			color += checker(position.xz*4.0)*0.5*alpha +(1.0-alpha)*vec4(16.0/255.0,25.0/255.0,27.0/255.0,1.0);

			break;
				}
			
			if(colorIndex == 1) color3 = vec4(float(121)/255.0,float(132)/255.0,float(125)/255.0,1.0);
			if(colorIndex == 2) color3 = vec4(float(0xAD)/255.0,float(0xFF)/255.0,float(0x2F)/255.0,1.0);


			//float shad = computeShadow(position+normalize(lightPosition-position)*0.009);
			
			color = color3 * max(dot(normal, normalize(lightPosition-position)),0.0);
			color += color3 *0.07 ;

			// SPECULAR HIGHLIGHTS
			vec3 E = normalize(cameraPosition - position);
			vec3 R = reflect(-normalize(lightPosition-position), normal);
			float specular = pow(max(dot(R, E), 0.0), 12.0);

			// compute final color		
			color =vec4(color.xyz , 1.0);

			
//color = color*0.4+ refl*0.6;
			color +=vec4(1.0, 1.0,1.0,0.0)*specular*1.0;

			// interation glow
			color += vec4(vec3(1.0, 0.5,0.1)*pow(float(i)/64.0*1.2, 2.0) *1.0,1.0);
//color = color * alpha + plasma() *(1.0 -alpha);
			

			break;
		}
		
			t = t + distance;
	}
				//color = computeColor(position, viewDirection, cameraPosition);				  
								  
	gl_FragColor = color;								  
	//discard;
}
