// https://www.shadertoy.com/view/XsfGRH
//credits 2 polygonize
//http://tinyurl.com/b9go98e

float tick;
float delta    = 0.20015;
float PI       = 3.1415;
int colorIndex = 0;
int material   = 0;

const vec3 lightPosition  = vec3(3.5,3.5,-1.0);
const vec3 lightDirection = vec3(-0.5,0.5,-1.0);

float displace(vec3 p) {
	return ((sin(6.*p.x + iGlobalTime/4.1)*sin(6.*p.y)*sin(6.*p.z))*cos(iGlobalTime/4.1))*.5;
}


	
vec3 rotateX(vec3 pos, float alpha) {
	
	mat4 trans= mat4(1.0, 0.0, 0.0, 0.0, 0.0, cos(alpha), -sin(alpha), 0.0, 0.0, sin(alpha), cos(alpha), 0.0, 0.0, 0.0, 0.0, 1.0);
	return vec3(trans * vec4(pos, 1.0));

}


vec3 rotateY(vec3 pos, float alpha) {
	
	mat4 trans2= mat4(cos(alpha), 0.0, sin(alpha), 0.0, 0.0, 1.0, 0.0, 0.0,-sin(alpha), 0.0, cos(alpha), 0.0, 0.0, 0.0, 0.0, 1.0);
	return vec3(trans2 * vec4(pos, 1.0));

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


float sball(vec3 p) {

	vec3 q=p;
	p = normalize(p);
	
	vec4 b = max(max(max(abs(vec4(dot(p,n16 ), dot(p,n17),dot(p, n18), dot(p,n19))),abs(vec4(dot(p,n12 ), dot(p,n13), dot(p, n14), dot(p,n15)))),abs(vec4(dot(p,n8 ), dot(p,n9 ), dot(p, n10), dot(p,n11)))),abs(vec4(dot(p,n4 ), dot(p,n5 ), dot(p, n6), dot(p,n7))));
	
	b.xy = max(b.xy, b.zw);
	b.x = pow(max(b.x, b.y), 40.);
	
	return length(q)-( 1.6 )*pow(1.5,b.x * ( 1.5 * cos(tick*.5) +2.5  - mix( .3 + sin(tick)/ 8.0, .1, .25 ) ) );

}

	
float f(vec3 position) {
	float tick = iGlobalTime * 0.5;
	float dst  = displace(position);
	float size = 0.245;
	vec3 ball0;
	vec3 ball1;
	vec3 ball2;
	vec3 ball3;
	vec3 ball4;
	vec3 ball5;
	vec3 ball6;
	ball0 = vec3(cos(tick),1.0,0.0)-position;
	ball1 = vec3(-1.0,-sin(tick),0.0)-position;
	ball2 = vec3(-cos(tick),-sin(tick),1.0)-position;
	ball3 = vec3(-cos(tick),sin(tick),2.0*sin(tick))-position;
	ball4 = vec3(-2.0*cos(tick),-sin(tick),2.0*sin(tick))-position;
	ball5 = vec3(-0.5*cos(tick),-cos(tick),2.5*sin(tick))-position;
	ball6 = vec3(-0.5*cos(tick),-1.5*cos(tick),1.75*sin(tick))-position;
	
	float ball0dist = dot(ball0, ball0);
	float ball1dist = dot(ball1, ball1);
	float ball2dist = dot(ball2, ball2);
	float ball3dist = dot(ball3, ball3);
	float ball4dist = dot(ball4, ball4);
	float ball5dist = dot(ball5, ball5);
	float ball6dist = dot(ball6, ball6);
	ball0dist = pow(ball0dist, max(0.02, 1.0));
	ball1dist = pow(ball1dist, max(0.02, 1.0));
	ball2dist = pow(ball2dist, max(0.02, 1.0));
	ball3dist = pow(ball3dist, max(0.02, 1.0));
	ball4dist = pow(ball4dist, max(0.02, 1.0));
	ball5dist = pow(ball5dist, max(0.02, 1.0));
	ball6dist = pow(ball6dist, max(0.02, 1.0));
	return mix(sball(rotateY(rotateX(position,tick*.3),tick*.4)) + dst,-(size/ball0dist+size/ball1dist+size/ball2dist+size/ball3dist+size/ball4dist+size/ball5dist+size/ball6dist)+1.0,.5);
}


vec3 ray(vec3 start, vec3 direction, float t) {
	
	return start + t * direction;
	
}



vec3 gradient(vec3 position) {

	return vec3(f(position + vec3(delta, 0.0, 0.0)) - f(position - vec3(delta, 0.0, 0.0)),f(position + vec3(0.0,delta, 0.0)) - f(position - vec3(0.0, delta, 0.0)),f(position + vec3(0.0, 0.0, delta)) - f(position - vec3(0.0, 0.0, delta)));

}

void main(void)
{
	vec2 uv      = gl_FragCoord.xy / iResolution.xy;
	vec3 cam     = vec3( -0.20, -.5, -6.4 );
	float aspect = iResolution.x/iResolution.y;
	vec3 near    = vec3((gl_FragCoord.x - 0.5 * iResolution.x) / iResolution.x * 2.0  * aspect,(gl_FragCoord.y - 0.5 * iResolution.y) / iResolution.y * 2.0,0.0);

	tick = iGlobalTime;

	vec3 vd = normalize(near - cam);
	vd.x -= .03;
	vd.z -= .3005;
	
	float t = 0.0;
	float dst;
	vec3 pos;
	vec4 color = vec4(vec3(1.0),1.0);
	vec3 normal;
	vec3 up = normalize(vec3(-0.0, 1.0,0.0));
	

	for(int i=0; i < 256; i++) {
	
		pos = ray(cam,	vd, t);
		dst = f(pos);
	
		if( abs(dst) < 0.00008 ) {
			
			normal = normalize(gradient(pos));
			
			vec4 color1 = vec4(.75, 0.99, 0.5,1.0);
			vec4 color2 = vec4(.50, 0.8, 0.11,1.0);
			
			vec4 color3 = mix(color2, color1, (1.0+dot(up, normal))/16.0);
			color = color3 * max(dot(normal, normalize(lightDirection)),0.0) +vec4(0.1,0.1,0.1,1.0);
			
			vec3 E = normalize(cam - pos);
			vec3 R = reflect(-normalize(lightDirection), normal);
			float specular = pow( max(dot(R, E), 0.0), 8.0);
			color +=vec4(.16, .4,0.4,0.0)*specular;
			color += vec4(vec3(.25, 1.0,0.5)*pow(float(i)/256.0 * 1.1, 2.0) *1.0,1.0);
			break;
			
			
		}
	
		t = t + dst * .56;
	
	}	
	
	gl_FragColor = color;
		
}
