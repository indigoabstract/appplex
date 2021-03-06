// http://glsl.heroku.com/e#15514.0
#ifdef GL_ES
precision mediump float;
#endif

// rakesh@picovico.com : www.picovico.com

uniform float u_v1_time;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

const float fRadius = 0.05;

void main(void)
{
	vec2 uv = -1.0 + 2.0 * v_v2_tex_coord;
	uv.x *=  u_v2_dim.x / u_v2_dim.y;
	
	vec3 color = vec3(0.0);

    	// bubbles
	for( int i=0; i<64; i++ )
	{
        	// bubble seeds
		float pha = sin(float(i)*5.13+1.0)*0.5 + 0.5;
		float siz = pow( sin(float(i)*1.74+5.0)*0.5 + 0.5, 4.0 );
		float pox = sin(float(i)*3.55+4.1) * u_v2_dim.x / u_v2_dim.y;
		
        	// buble size, position and color
		float rad = fRadius + sin(float(i))*0.12+0.08;
		vec2  pos = vec2( pox+sin(u_v1_time/30.+pha+siz), -1.0-rad + (2.0+2.0*rad)
						 *mod(pha+0.1*(u_v1_time/5.)*(0.2+0.8*siz),1.0)) * vec2(1.0, 1.0);
		float dis = length( uv - pos );
		vec3  col = mix( vec3(0.8, 0.2, 0.0), vec3(0.8,0.5,0.2), 0.5+0.5*sin(float(i)*sin(u_v1_time*pox*0.03)+1.9));
		
        	// render
		color += col.xyz *(1.- smoothstep( rad*(0.65+0.20*sin(pox*u_v1_time)), rad, dis )) * (1.0 - cos(pox*u_v1_time));
	}

	gl_FragColor = vec4(color,1.0);
}
