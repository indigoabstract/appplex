// https://www.shadertoy.com/view/4sXGDM
const int iMax = 255;

vec3 MBrot(vec2 c, vec2 z, vec2 uv) {
	float i = 0.0;
	for( int j=0; j<255; j++ )
	{
	    if( z.x*z.x + z.y*z.y > 2.0*2.0) 
			break;	
	
		z = vec2((z.x*z.x-z.y*z.y), (z.x*z.y+z.x*z.y))+c;
		i++;
	}
	if (i<float(iMax)){
		return vec3 (mod(i,1.0), mod(i,1.0), mod(i,1.0075));
	}
	else {
		return vec3(0.0,0.0,0.0);
	}
}

void main(void)
{
	vec2 uv = (gl_FragCoord.xy / iResolution.x)*2.0;
	float ang = iGlobalTime/5.0;
	mat2 rotation = mat2(cos(ang), sin(ang),-sin(ang), cos(ang));
	uv = rotation*(uv-vec2(1.0,(iResolution.y/iResolution.x)));
	float k = 0.1/(iGlobalTime);
	uv.x*=k/iGlobalTime;
	uv.x-=1.23;
	uv.y*=k/iGlobalTime;
	uv.y+=0.105;
	vec2 c = vec2(uv.x, uv.y);
	vec2 z = vec2(0.0, 0.0);
	vec3 col = MBrot(c, z, uv);
	gl_FragColor = vec4(col,1.0);
}
