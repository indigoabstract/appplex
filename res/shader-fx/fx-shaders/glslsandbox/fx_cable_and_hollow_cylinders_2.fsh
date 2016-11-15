// http://glslsandbox.com/e#24325.0
#ifdef GL_ES
precision mediump float;
#endif

// T21: Can anyone fix the fringe around the cylinders ???
// 
// fernlightning says: Lets try adding a ring/disk at the end of the cylinder, and ensuring the color returned is stable

// - retrofitted glowing cable from http://glsl.heroku.com/e#6129.0

uniform float time;
#define time (time/1e1)
uniform vec2 mouse;
uniform vec2 resolution;



vec4 rgbaTOycca(vec4 inColor){
	float R = 0.;
	float G = 0.;
	float B = 0.;
	float Y = 0.;
	float Cb = 0.;
	float Cr = 0.;
	vec3 Crgb = inColor.xyz;
	R = Crgb.x;  G = Crgb.y;  B = Crgb.z;
	Y = .299*R + .587*G + .114*B;
	Cb = .50196 - .168736*R - .331264*G + .50196*B;
	Cr = .50196 + .50196*R - .418688*G - .081312*B;
	return vec4(Y, Cb, Cr, inColor.w);
}

vec4 yccaTOrgba(vec4 inColor){
	float R = 0.;
	float G = 0.;
	float B = 0.;
	float Y = 0.;
	float Cb = 0.;
	float Cr = 0.;
	vec3 Cycc = inColor.xyz;
	Y = Cycc.x;  Cb = Cycc.y;  Cr = Cycc.z;
	R = Y + 1.402*(Cr - .50196);
	G = Y - .34414*(Cb - .5) - .71414*(Cb - .50196);
	B = Y + 1.772*(Cb - .50196);
	return vec4(R, G, B, inColor.w);
}

float rand(vec3 n)
{
  n = floor(n);
  return fract(sin((n.x+n.y*1e2+n.z*1e4)*1e-4)*1e5);
}

// .x is distance, .y = colour
vec2 map( vec3 p )
{
	p += vec3(pow(sin(time*500.)*.2, 2.), pow(cos(time*300.)*.15, 2.), 0.);
	p.z -= time*32.;
	const float RADIUS = 0.2;

	// cylinder
	vec3 f = fract( p ) - 0.5;
	float d = length( f.xy );
        float cr = rand( p );
//	float cd = d - cr*RADIUS;
	float cd = abs(d - cr*RADIUS); // abs for hollow
	// inner cable
	if(d-0.02 < cd) {
		cd = d-0.02;
		cr = 4.0; // glow!
	}
	
        // end - calc (rand) radius at more stable pos
	p.z -= 0.5;
	float rr = rand( p );
	float rn = d - rr*RADIUS;
        float rm = abs( fract( p.z ) - 0.5 );  // offset so at end of cylinder
        
	// float rd = max( rn , rm ); // end with disk
	float rd = sqrt( rn*rn + rm*rm ); // end with ring

	return (cd < rd) ?  vec2( cd, cr ) : vec2( rd, rr ); // min
}


void main( void )
{
	float tmx = time;
    vec2 pos = (gl_FragCoord.xy*2. - resolution.xy) / min(resolution.x, resolution.y);
    vec3 camPos = vec3(cos(tmx*8.), sin(tmx*8.), tmx*150.);
    vec3 camTarget = vec3(0., 0., 0.);

    vec3 camDir = -normalize(camTarget-camPos);
    vec3 camUp  = normalize(vec3(sin(tmx*3.), cos(tmx*3.), 0.0));
    vec3 camSide = cross(camDir, camUp);
    float focus = 1.2;

    vec3 rayDir = normalize(camSide*pos.x + camUp*pos.y + camDir*focus);
    vec3 ray = camPos;
    float m = 0.32;
    vec2 d;
    float total_d = 0.;
    const int MAX_MARCH = 128;
    const float MAX_DISTANCE = 15.;
    for(int i=0; i<MAX_MARCH; ++i) {
        d = map(ray-vec3(0.,0.,time/2.));
        total_d += d.x;
        ray += rayDir * d.x;
        m += 1.0;
        if(abs(d.x)<0.01) { break; }
        if(total_d>MAX_DISTANCE) { total_d=MAX_DISTANCE; break; }
    }

    float c = (total_d)*0.0001;
    vec4 result = vec4( 1.0-vec3((d.y>2.0)?1.0:c, c, c) - vec3(0.025, 0.025, 0.02)*m*0.8, 1.0 );
    gl_FragColor = result*d.y;
    //float ph = time*5.678901234;
    //gl_FragColor = cos(ph)*rgbaTOycca(gl_FragColor) + sin(ph)*gl_FragColor;
    //gl_FragColor = yccaTOrgba(gl_FragColor);
}
