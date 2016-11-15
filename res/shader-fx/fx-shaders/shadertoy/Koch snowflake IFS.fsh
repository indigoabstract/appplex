// https://www.shadertoy.com/view/XsfXDH
// Not sure how new or interesting this is, but I've been trying to come up with an efficient way of rendering the Koch snowflake in the past few days, turns out the best way is to use an IFS.
//Koch snowflake by nimitz (stormoid.com) (twitter: @stormoid)

#define time iGlobalTime
const mat2 m2 = mat2( 0.5,  0.866, -0.866,  0.5 );

float koch(vec2 p)
{
	float rz = 1.;
	const float maxitr = 7.;
	float itnum = maxitr;
	for (float i=0.;i<maxitr;i++) 
	{
		if (i>mod(time*0.5,maxitr)) break;
		
		//draw triangle
		float d = max(abs(p.x)*1.73205+p.y, -p.y*2.)-3.;
		
		//edge scaling
		itnum--;
		d *=exp(itnum)*0.05;
		
		//min blending
		rz= min(rz,d);
		
		//show complete edges half the time
		rz *= rz+(abs(d)*step(mod(time,2.),1.));
		
		//fold both axes
		p = abs(p);
		
		//rotate
		p *= m2;
		
		//fold y axis
		p.y = abs(p.y);
		
		//rotate back
		p.yx*= m2;
		
		//move and scale
		p.y-=2.;
		p*=3.;
	}
	return clamp(rz,0.,1.);
}

void main( void )
{
	vec2 p = gl_FragCoord.xy/iResolution.xy-.5;
	p.x *= iResolution.x/iResolution.y;
	p *= 6.;
	
	float rz = 1.-koch(p);
	vec3 col = vec3(1.)*rz;
	float lp = length(p);
	col -= pow(lp*.23,2.)*rz;
	
	//background coloring
	vec3 bg = vec3(0.1,0.2,0.3)*1.3;
	col = mix(bg,col, rz*rz);
	col -= lp*.03;
	
	gl_FragColor = vec4(col,1.);
}
