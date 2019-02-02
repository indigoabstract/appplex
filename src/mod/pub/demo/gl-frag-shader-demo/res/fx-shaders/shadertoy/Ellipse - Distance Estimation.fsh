// https://www.shadertoy.com/view/MdfGWn
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// An example on how to compute a distance estimation for an ellipse (which provides
// constant thickness to its boundary). This is achieved by dividing the implicit 
// description by the modulo of its gradient. The same process can be applied to any
// shape defined by an implicity formula (ellipses, metaballs, fractals, mandelbulbs).
//
// More info here:
//
// http://www.iquilezles.org/www/articles/distance/distance.htm

float ellipse(vec2 p)
{
	float a = 1.0;
	float b = 3.0;
	float r = 0.9;

    float f = length( p*vec2(a,b) );
	if( p.x<0.0 )
	{
		f = f - r;
		f *= 0.33;
	}
    else 
	{
		f = length( p*vec2(a,b) );
		f = f*(f-r)/length( p*vec2(a*a,b*b) );
	}
	
	return f;
}


void main(void)
{
	vec2 uv = -1.0 + 2.0*gl_FragCoord.xy / iResolution.xy;
	uv.x *= iResolution.x/iResolution.y;
	
	float f = abs(ellipse(uv));
	
	vec3 col = vec3(1.0,0.0,0.0);
	if( uv.x>0.0 ) col = col.yzx;
		
	col = mix( col, vec3(1.0), smoothstep( 0.1, 0.11, f ) );
	
	col *= smoothstep( 0.008,0.01, abs(uv.x) );
	
	gl_FragColor = vec4( col,1.0);
}
