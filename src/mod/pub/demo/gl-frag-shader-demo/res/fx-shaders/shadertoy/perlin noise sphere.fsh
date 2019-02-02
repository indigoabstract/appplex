// https://www.shadertoy.com/view/Msf3DH

// iChannel0: t14

// Perlin Noise Sphere by Johannes Diemke 2012.
//
// description:
// very simple and still impressive. the signed distance field is basically
// sdf(p) := sdSphere(p) - scale * pnoise(p)
// GLSL perlin noise implementation from taken from
// https://github.com/ashima/webgl-noise
//
// UPDATE: uses now iqs fast 3d noise
//
// video:
// http://www.youtube.com/watch?v=ntYFwDKEj4o
//
// contact:
// johannes.diemke@uni-oldenburg.de

float time	 = iGlobalTime * 0.2;
float width  = iResolution.x;
float height = iResolution.y;

const float DELTA  = 0.02;
const float PI	   = 3.14159265;

const vec3 lightPosition  = vec3(0.1, 0.1, -1.0);
const vec3 cameraPosition = vec3(0.0, 0.0, -1.6);
float aspect			  = width / height;

float sphere(vec3 position, float r) {
	return length(position) - r;
}

vec3 rotateX(vec3 pos, float alpha) {				
	return vec3(pos.x,
				pos.y * cos(alpha) + pos.z * -sin(alpha),
				pos.y * sin(alpha) + pos.z * cos(alpha));
}

vec3 rotateY(vec3 pos, float alpha) {
	return vec3(pos.x * cos(alpha) + pos.z * sin(alpha),
				pos.y,
				pos.x * -sin(alpha) + pos.z * cos(alpha));
}

vec3 translate(vec3 position, vec3 translation) {
	return position - translation;
}

float udRoundBox( vec3 p, vec3 b, float r ) {
  return length(max(abs(p)-b,0.0))-r;
}

float sdSphere( vec3 p, float s ) {
  return length(p)-s;
}

float opU( float d1, float d2 ) {
    return min(d1,d2);
}

float snoise(vec3 x) {
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D( iChannel0, (uv+ 0.5)/256.0, -100.0 ).yx;
	return mix( rg.x, rg.y, f.z );
}

int colorIndex = 0;

float function(vec3 position) {
	
	vec3 pos = rotateY(rotateX(position, time*2.0),time*1.5);
	
	return opU(sphere(pos, 0.58)-0.38*snoise(vec3(pos*4.5+time*1.5)),
			   udRoundBox( position-vec3(0.0,0.0,4.0), vec3(8.0,4.0,1.0), 0.2 ));
}

vec3 ray(vec3 start, vec3 direction, float t) {
	return start + t * direction;
}

vec3 gradient(vec3 position) {

	return vec3(function(position + vec3(DELTA, 0.0, 0.0)) - function(position - vec3(DELTA, 0.0, 0.0)),
	function(position + vec3(0.0,DELTA, 0.0)) - function(position - vec3(0.0, DELTA, 0.0)),
	function(position + vec3(0.0, 0.0, DELTA)) - function(position - vec3(0.0, 0.0, DELTA)));

	
}

void main() {	
	
	vec3 nearPlanePosition = vec3((gl_FragCoord.x - 0.5 * width) / width * 2.0  * aspect,
							      (gl_FragCoord.y - 0.5 * height) / height * 2.0,
							       0.0);
							  
	vec3 viewDirection = normalize(nearPlanePosition - cameraPosition);
	
	float t = 0.0;
	float distance;
	vec3 position;
	vec4 color = vec4(vec3(1.,1.,0.4),1.0);// plasma();//vec4(0.0,0.2,0.0,1);
	vec3 normal;
	vec3 up = normalize(vec3(-0.0, 1.0,0.0));
	
	for(int i=0; i < 40; i++) {
		position = ray(cameraPosition,	viewDirection, t);
		distance = function(position);
		
	
		
		if(distance < 0.002) {
			
				
			normal = normalize(gradient(position));
			
			vec4 color1 = vec4(0.5, 0.9, 0.5,1.0);
			vec4 color2 = vec4(1.0, 0.1, 0.1,1.0);
			
			vec4 color3 = mix(color2, color1, (1.0+dot(up, normal))/2.0);
			
			//if(colorIndex == 1) color3 = vec4(float(0xAD)/255.0,float(0xFF)/255.0,float(0x2F)/255.0,1.0);
			//if(colorIndex == 2) color3 = plasma();
			color = color3*0.7 * max(dot(normal, normalize(lightPosition-position)),0.0) ;//+vec4(0.1,0.1,0.1,1.0);

			//specular
			vec3 E = normalize(cameraPosition - position);
			vec3 R = reflect(-normalize(lightPosition-position), normal);

			
			float specular = pow( max(dot(R, E), 0.0), 
		                 39.0);
			
			float alpha = 1.0-clamp( pow(length(position-vec3(0.0,0.0,1.0)),3.0)*0.0018,0.0, 1.0);
			
			float ao = 1.0;//computeAO(position, normal);
			//color = vec4(color.xyz * ao, 1.0);

			//sss
			float sss = 1.0 ;//-computeAO(position,viewDirection)*3.0;

			float shad = 1.0;//computeShadow(position+normalize(lightPosition-position)*0.1);

//			if(dot(normalize(lightPosition-position), normal) < 0.0)
//				shad =0.3;
			float shadow =0.3+ 0.7 * shad;

			  color = vec4(color.xyz *shad+ color3.xyz*0.4*ao, 1.0);


			color +=vec4(0.6, 0.4,0.4,0.0)*specular *shad;
color =mix( color,vec4(1.0, 0.5,0.1,1.0),(1.0-sss));
			
			// interation glow
			color += vec4(vec3(0.5, 0.8,0.1)*0.8*pow(float(i)/32.0*1.0, 2.0),1.0);
			
			break;
		}
		
			t = t + distance * 0.68;
	}
								  						  
	gl_FragColor = color;								  
}
