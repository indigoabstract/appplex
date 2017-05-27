// https://www.shadertoy.com/view/lsfSRN
#define CLOCK_BORDER 2.0
#define POINTER_WIDTH 2.0

#define EPS 2.
#define PI 3.14159
#define D2R(_d) ((_d)*PI/180.0)

float draw_pointer(vec2 rel_pos,float angle,float pt_len)
{
	float r=length(rel_pos);
	vec2 pt_axis=vec2(cos(angle),sin(angle));
	vec2 pt_perp_axis=vec2(-sin(angle),cos(angle));
	
	if(dot(rel_pos,pt_axis)>0.)
	{
		float d=abs(dot(rel_pos,pt_perp_axis));
		return smoothstep(pt_len+EPS,pt_len,r)*
			smoothstep(POINTER_WIDTH+EPS,POINTER_WIDTH,d);
	}
	return 0.;
}

float draw_circle(float R,float border,float r)
{
	return smoothstep(border+EPS,border,abs(r-R));
}

void main( void ) {
	float CLOCK_R=0.4*min(iResolution.x,iResolution.y);
	float CLOCK_CX=0.5*iResolution.x,CLOCK_CY=0.5*iResolution.y;

	vec2 rel_pos=gl_FragCoord.xy-vec2(CLOCK_CX,CLOCK_CY);
	float r=length(rel_pos);
	float scale=0.;
	vec4 color=vec4(0.);
	
	if((scale=draw_circle(CLOCK_R,CLOCK_BORDER,r))>0.)
		color = vec4( 1.0, 0.0, 0.0, 1.0 );
	else
	{
		float t=iDate.w;
		float h=t/3600.0;
		t-=floor(h)*3600.0;if(h>12.0) h-=12.0;
		float m=t/60.0;
		float s=t-floor(m)*60.0;
		float h_angle=90.0-h*360.0/12.0;
		float m_angle=90.0-m*6.0;
		float s_angle=90.0-s*6.0;
		
		if((scale=draw_pointer(rel_pos,D2R(s_angle),0.9*CLOCK_R))>0.)
			color = vec4( 0.0, 1.0, 0.0, 1.0 );
		else if((scale=draw_pointer(rel_pos,D2R(m_angle),0.7*CLOCK_R))>0.)
			color = vec4( 0.0, 1.0, 1.0, 1.0 );
		else if((scale=draw_pointer(rel_pos,D2R(h_angle),0.5*CLOCK_R))>0.)
			color = vec4( 1.0, 1.0, 1.0, 1.0 );
	}
	
	gl_FragColor=scale*color;
}
