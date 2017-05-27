// https://www.shadertoy.com/view/lsl3zn

//credits 2 polygonize
//http://tinyurl.com/b9go98e

float time;
float delta    = 0.20015;
float PI       = 3.1415;
int colorIndex = 0;
int material   = 0;

const vec3 lightPosition  = vec3(3.5,3.5,-1.0);
const vec3 lightDirection = vec3(-0.5,0.5,-1.0);

float displace(vec3 p) {
return ((cos(4.*p.x)*sin(4.*p.y)*sin(4.*p.z))*cos(30.1))*sin(time);
}
vec3 rotateX(vec3 pos, float alpha) {
mat4 trans= mat4(1.0, 0.0, 0.0, 0.0, 0.0, cos(alpha), -sin(alpha), 0.0, 0.0, sin(alpha), cos(alpha), 0.0, 0.0, 0.0, 0.0, 1.0);
return vec3(trans * vec4(pos, 1.0));
}


vec3 rotateY(vec3 pos, float alpha) {
mat4 trans2= mat4(cos(alpha), 0.0, sin(alpha), 0.0, 0.0, 1.0, 0.0, 0.0,-sin(alpha), 0.0, cos(alpha), 0.0, 0.0, 0.0, 0.0, 1.0);
return vec3(trans2 * vec4(pos, 1.0));
}

float rBox( vec3 p, vec3 b, float r ){
	return length(max(abs(p)-b,0.0))-r;
}


float minBox( float d1, float d2 ){
return min(d1,d2);
}


	
float f(vec3 position) {
	
	float d, a, b, c, m, n, q, dist;
	d = displace(position);
	b = rBox(rotateY(rotateX(position+vec3(0.5,0.0,-6.0),time*3.0),time*3.0), vec3(0.7,0.7,0.7), 0.4);
	c = rBox(position+vec3(0,0,-16), vec3(25.6,15.6,0.6), 0.2 );
	b = b + d;
	if (c < b) material = 1;
	else material = 0;
	return minBox(c,b);
	
}


vec3 ray(vec3 start, vec3 direction, float t) {
	
	return start + t * direction;
	
}



vec3 gradient(vec3 position) {

	return vec3(f(position + vec3(delta, 0.0, 0.0)) - f(position - vec3(delta, 0.0, 0.0)),f(position + vec3(0.0,delta, 0.0)) - f(position - vec3(0.0, delta, 0.0)),f(position + vec3(0.0, 0.0, delta)) - f(position - vec3(0.0, 0.0, delta)));

}
	
void main(void)
{
	vec2 uv  = gl_FragCoord.xy / iResolution.xy;
	vec3 cam = vec3( -0.20, -.5, -3.4 );
	float aspect = iResolution.x/iResolution.y;
	vec3 near = vec3((gl_FragCoord.x - 0.5 * iResolution.x) / iResolution.x * 2.0  * aspect,(gl_FragCoord.y - 0.5 * iResolution.y) / iResolution.y * 2.0,0.0);

	time = iGlobalTime;
	
	vec3 vd = normalize(near - cam);
	vd.x -= .1;
	vd.z -= .0001;
	vd.y -= .08;
	
	float t = 0.0;
	float dst;
	vec3 pos;
	vec4 color = vec4(vec3(1.0),1.0);
	vec3 normal;
	vec3 up = normalize(vec3(-0.0, 1.0,0.0));
	

	for(int i=0; i < 64; i++) {
	
		pos = ray(cam,	vd, t);
		dst = f(pos);
	
		if( abs(dst) < 0.008 ) {
			
			normal = normalize(gradient(pos));
			
			vec4 color1 = vec4(0.15, 0.19, 0.5,1.0);
			vec4 color2 = vec4(.10, 0.1, 0.11,1.0);
			
			vec4 color3 = mix(color2, color1, (1.0+dot(up, normal))/2.0);
			color = color3 * max(dot(normal, normalize(lightDirection)),0.0) +vec4(0.1,0.1,0.1,1.0);
			
			vec3 E = normalize(cam - pos);
			vec3 R = reflect(-normalize(lightDirection), normal);
			float specular = pow( max(dot(R, E), 0.0), 8.0);
			color +=vec4(1.6, 1.4,0.4,0.0)*specular;
			if(material==1) color = vec4(0.0,0.0,0.0,1.0);
			color += vec4(vec3(0.5, 1.0,0.5)*pow(float(i)/128.0*2.1, 2.0) *1.0,1.0);
			break;
			
			
		}
	
		t = t + dst * 1.0;
	
	}	
	
	gl_FragColor = color;
		
}
