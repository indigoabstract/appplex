// https://www.shadertoy.com/view/XsXXW8
// vec2 uv = gl_FragCoord.xy / iResolution.xy;
	// gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);

		#define PI 3.141592653589793238462643383279
		float metafactor = 0.04;
			
		float dist2(vec2 p1, vec2 p2)
		{
			float factor = 1.0;
			float f1 = 200000.0 / factor;
			float f2 = f1 * f1;
			vec2 d = p1 - p2;
			return sqrt(dot(d,d)+f2) - f1;
		}

		float dist(vec3 p1, vec3 p2)
		{
			return distance(p1,p2);
		}

		vec4 col(float tick, float v) {
			float r = tick * -0.057;
			float g = tick * 0.063;
			float b = tick * 0.021;
			return vec4(cos(r+v)+1.0,cos(g+v)+1.0,cos(b+v)+1.0,1.0);
		}

		void main( void ) {

			vec2 pos = gl_FragCoord.xy / iResolution.xy;
			float factor = metafactor * 1000.0 / 2.0;
			
			float tick = (iGlobalTime * 10.0);

			float circle1 = tick * 0.085 / 6.0;
			float circle2 = -tick * 0.1023 / 6.0;
			float circle3 = tick * .349834 / 6.0;
			float circle4 = -tick * .205425 / 6.0;
			float circle5 = tick * .400134 / 6.0;
			float circle6 = -tick * .125 / 6.0;
			float circle7 = tick * .315 / 6.0;
			float circle8 = -tick * .052244 / 6.0;

			float roll = tick * 5.0;
		
			vec3 p = vec3(pos.x*2.0-1.0,pos.y*2.0-1.0, 0.8);
			vec3 p1 = vec3( cos(circle3), sin(circle4), 0);
			vec3 p2 = vec3( sin(circle1), cos(circle2), 0);
			vec3 p3 = vec3( cos(circle5), sin(circle6), 0);
			vec3 p4 = vec3( cos(circle7), sin(circle8), 0);
			
			float sum = 0.02*dist(p,p1) + sin(dist(p,p1)) + sin(dist(p,p3)) + sin(dist(p,p4));
			
			gl_FragColor = col(tick,sum*10.0);
		}
