// http://glsl.heroku.com/e#15387.3
#ifdef GL_ES
precision mediump float;
#endif
uniform float u_v1_time;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

void main() {
	vec2 R=u_v2_dim,P=2./R .y*v_v2_tex_coord*u_v2_dim-vec2(R.x/R.y,1);
	vec3 C=vec3(0);
	
	for(float i=2.;i<15.;++i)
	{

	  float T=u_v1_time,
	        t=mod(T*.4,1e3)*i,
	        x=P.x-cos(t/2.)*.7,
	        y=P.y-sin(t/2.)*.7,
	        r=t*3.+x*tan(t*1.5)+y*tan(t*2.),
	        s=sin(r),
	        c=cos(r);vec2 
	        d=(2.+cos(T*.2)*.35-abs(vec2(sin(t*.5),cos(t*.5)))) * vec2(x*c+y*s,x*s-y*c);
	        t=6.*dot(d,d)-0.;
	      
	  vec3 u=mod(T*vec3(2,1.9,1.8),1e3)*(i*.1+2.),
	       v=vec3(sin(u.x),cos(u.y),sin(u.z+.8));
	        
	  C+=v*v*.02/abs(t);
	      
	  if(t<.0)
	    C+=.2*sin(u)+.02;
		
	}
	gl_FragColor.xyz=C;
}
