// http://glsl.heroku.com/e#6292.0
#ifdef GL_ES
precision highp float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

void main( void ) {

	vec2 position = ( gl_FragCoord.xy / resolution.xy );

	float color = 0.0;	
	float vx, vy, vz, vxr;
	float dx, llx, dy, lly, dz, llz;
	int px, py, pz, ccc, P;
	float k = 0., k2 = 0.;
	ivec4 di;
	vec4 X;
	vec4 d=vec4(time/1.7,mouse.y*50.0,.0,.0);
	vx=(position.x-0.5)+0.0001;

	vy=(position.y-0.5)+0.0001;
	vz=0.1+.0001;
	vxr=(vx*cos(mouse.x*2.5)+vz*sin(mouse.x*2.5));
	vz=(-vx*sin(mouse.x*2.5)+vz*cos(mouse.x*2.5));
	vx=vxr;
	X=fract(d);
	dx = 4000.0/vx; dy = 4000.0/vy; dz = 4000.0/vz;
	px=1; llx=dx*(1.0-X[0]);
	py=16; lly=dy*(1.0-X[1]);
	pz=256; llz=dz*(1.0-X[2]);
	if (dx<.0) {px=-1; dx=-dx; llx=dx*X[0];}
	if (dy<.0) {py=-16; dy=-dy; lly=dy*X[1];}
	if (dz<.0) {pz=-256; dz=-dz; llz=dz*X[2];}
	ccc=0;
	di=ivec4(d[0],d[1],d[2],d[3]);
	P=di[2]*256+di[1]*16+di[0];
	color=1.0;
	for (int i=0; i<40; i++)
	{
		if ((llx<=lly) && (llx<=llz))
		{
			P+=px; llx+=dx; k=0.75;
		}
		else
		{
			if (lly<=llz)
			{
				P+=py; lly+=dy; k=0.9;
			}
			else
			{
				P+=pz; llz+=dz; k=1.0;
			}
		}
		if ((fract(float(P)/50.0)<.01)&&(color==1.0)) {
			color=float(i)/30.0;
			k2=k;
		}
	}
	gl_FragColor = vec4(k2*(1.4-color),k2*(1.0-color),k2*k2*(2.0-color),1.0 );

}
