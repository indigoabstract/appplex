// https://www.shadertoy.com/view/lsl3zM
#define PI 3.14159265
#define time (iGlobalTime + 20.)
const vec3 backc_top=vec3(0.1,0.4,0.7);
const vec3 backc_bot=vec3(0.0,0.1,0.2);
const vec3 mainc=vec3(0.2,0.8,1.0);

//*math**********************************************/

float hash( float n )
{
    return fract(sin(n)*43758.5453123);
}
float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0 + 113.0*p.z;

    return mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
}
/*shape**********************************************/
// from http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float sdCylinder( vec3 p, vec3 c )
{
  return length(p.xz-c.xy)-c.z;
}

/*deform**********************************************/
vec3 opCheapBendX( vec3 p, float rad_bend )
{
    float c = cos(rad_bend*cos(p.y+  iGlobalTime));
    float s = sin(rad_bend*cos(p.y+  iGlobalTime));
    mat2  m = mat2(c,-s,s,c);
    return vec3(m*p.xy,p.z);
}
vec3 opTwistY( vec3 p, float rad_twist)
{
	float t = time * 0.03 ;
    float c = cos(rad_twist*p.y+t);
    float s = sin(rad_twist*p.y+t);
    mat2  m = mat2(c,-s,s,c);
	vec2 xz = m*p.xz;
    return  vec3(xz.x, p.y,xz.y );
}
//*transform*************************************/
vec3 stepspace(in vec3 p,  in float s)
{
  return p-mod(p-s/2.0,s);
}
vec2 sim2d(
  in vec2 p,
  in float s)
{
   return fract((p+s/2.0)/s)*s-s/2.0;
}
vec3 tiling(in vec3 p, in float interval )
{ 
	vec3 fp=stepspace(p,interval);
	float d= sin(fp.x*0.3)+cos(fp.z*0.3);
	vec3 ret;
	ret.y=p.y+d;
	ret.xz=sim2d(p.xz,interval);
	return ret;
}
//*Raymarching Functions***************************************************************/
//====================
// distanceFunction
//====================
float distanceFunction(vec3 p)
{
	p = opTwistY( p, 0.08 );
	p.z +=10.+time*0.2;
	p = tiling(p,6.0);
	p = opCheapBendX( p, 0.1 );
	return sdCylinder(p,vec3(0.6,0.6,0.6) ) 
		+0.5*(noise(p*1.1)-0.7) 
		+0.3*noise(p*2.0)
		+0.1*noise(p*8.0)
	;
}
//Normal
vec3 getNormal(vec3 p, float t)
{
	float e = 0.001*t;
    vec3  eps = vec3(e,0.0,0.0);
    return normalize( vec3(
    	distanceFunction(p+eps.xyy) - distanceFunction(p-eps.xyy),
		distanceFunction(p+eps.yxy) - distanceFunction(p-eps.yxy),
		distanceFunction(p+eps.yyx) - distanceFunction(p-eps.yyx)
		));
}
//*Shading Functions*************************************************/
vec3 phong(
  in vec3 pt,
  in vec3 prp,
  in vec3 normal,
  in vec3 light,
  in vec3 color,
  in float spec,
  in vec3 ambLight)
{
	vec3 lightv=normalize(light-pt);
	float diffuse=dot(normal,lightv);
	vec3 refl=-reflect(lightv,normal);
	vec3 viewv=normalize(prp-pt);
	float rim = max(0.0, 1.0-dot(viewv,normal));
	float specular=pow(max(dot(refl,viewv),0.0),spec);
	return (max(diffuse*0.5,0.0)+rim*rim)*color+ambLight+specular;
}
//*Render Functions*************************************************/
float raymarching(
  in vec3 camPos,
  in vec3 rayDir,
  in int maxite,
  in float precis,
  in float startf,
  in float maxd,
  out int objfound)
{ 
	const vec3 e=vec3(0.1,0,0.0);
	float s=startf;
	vec3 p;
	float f=startf;
	objfound=1;
	for(int i=0;i<256;i++){
	if (abs(s)<precis||f>maxd||i>maxite) break;
		f+=s;
		p=camPos+rayDir*f;
		s=distanceFunction(p)*0.5;
	}
	if (f>maxd) objfound=-1;
	return f;
}
//render
vec3 render(
	in vec3 camPos,
	in vec3 rayDir,
	in int maxite,
	in float precis,
	in float startf,
	in float maxd,
	in vec3 background,
	in vec3 light,
	in float spec,
	in vec3 ambLight)
{ 

	int objfound=-1;
	float f=raymarching(camPos,rayDir,maxite,precis,startf,maxd,objfound);
	if (objfound>0){
		vec3 p=camPos+rayDir*f;
		vec3 n = getNormal(p, f);
		return mix(
			phong(p,camPos,n,light,mainc,spec,ambLight),
			background,
			min(1.0,f*1.5/maxd) 
		);
	}
	f=maxd;
	return background;
}
//*Camera Functions*************************************************/
vec3 camera(
  in vec3 camPos,
  in vec3 camAt,
  in vec3 camUp,
  in float focus)
{
	vec2 vPos=-1.0+2.0*gl_FragCoord.xy/iResolution.xy;
	vec3 camDir=normalize(camAt-camPos);
	vec3 u=normalize(cross(camUp,camDir));
	vec3 v=cross(camDir,u);
	vec3 scrCoord=camPos+camDir*focus+vPos.x*u*iResolution.x/iResolution.y+vPos.y*v;
	return normalize(scrCoord-camPos);
}
vec3 prp_mouse(){
	 float mx=iMouse.x/iResolution.x*PI*2.0;
	 float my=((iMouse.y+1.0)/iResolution.y - 0.5)*PI;
	 return vec3(cos(my)*cos(mx),sin(my),cos(my)*sin(mx))*12.0; //Trackball style camera pos
}
//*Main***************************************************************/
void main(void)
{
	const vec3 camUp  =vec3(0,1,0);
	const vec3 camAt  =vec3(0.0,0.0,0.0);
	const float vpd=1.5;
	vec3 camPos = camAt+prp_mouse();
	vec3 rayDir= camera(camPos,camAt,camUp,vpd);
	vec3 light= camPos+vec3(0.0,5.0,0.0);

	const float maxe=0.01;
	const float startf=0.1;
	const float spec=8.0;
	const vec3 ambi=vec3(0.0,0.2,0.2);
	
	float latitude = 0.5+0.5+rayDir.y; //0.5+0.5*noise( normalize(scp) );
	vec3 back =mix(backc_bot,backc_top,latitude*latitude);

	vec3 c1=render(camPos,rayDir,256,maxe,startf,40.0,back,light,spec,ambi);
	gl_FragColor=vec4(c1.xyz,1.0);
}
