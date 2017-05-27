// https://www.shadertoy.com/view/ldBXRw

// iChannel0: t6
// iChannel2: t9

//use or not object raymarch in raytrace

bool useRaymarch = false;
const float t = 16.0;//nr. sample per pixel


#define pi acos(-1.0)
#define pi2 pi/2.0

//for transparency change, change first number in mref() to -1
struct Material
{
    vec4 color;
    vec2 brdf;
};

struct Sphere
{
	vec4 center_radius;
	int idmaterial;
};

struct Box
{
    vec3 min, max;
   int idmaterial;
};
    
struct Cylinder 
{
    vec3 c;
    float r,h;
    int idmaterial;
};

Box box0;
Sphere sfere[7];
Box boxe[4];
Cylinder cylinder[4];
Material material[6];

vec3 light = vec3(0.0, 0.0, 0.0);
vec3 pq, cub, lcub, nrm, crm;
vec4 plane;
vec2 planeStart = vec2(-10.0, 1.0);
vec2 planeWidthHeight = vec2(20.0, 15.0);
vec2 uvCoord;
vec2 p,rv2;
vec2 randv2;
float side = 1.0;
float time = iGlobalTime;
float f0, f1,f2,f3;


vec2 rand2(){// implementation derived from one found at: lumina.sourceforge.net/Tutorials/Noise.html
   randv2+=vec2(1.0,1.0);
   return vec2(fract(sin(dot(randv2.xy ,vec2(12.9898,78.233))) * 43758.5453),
      		   fract(cos(dot(randv2.xy ,vec2(4.898,7.23))) * 23421.631));
}

vec3 CosineWeightedSampleHemisphere ( vec3 normal, vec2 rnd )
{
   //rnd = vec2(rand(vec3(12.9898, 78.233, 151.7182), seed),rand(vec3(63.7264, 10.873, 623.6736), seed));
   float phi = acos( sqrt(1.0 - rnd.x)) ;
   float theta = 2.0 * 3.14 * rnd.y ;

   vec3 sdir = cross(normal, (abs(normal.x) < 0.5001) ? vec3(1.0, 0.0, 0.0) : vec3(0.0, 1.0, 0.0));
   vec3 tdir = cross(normal, sdir);

   return normalize(phi * cos(theta) * sdir + phi * sin(theta) * tdir + sqrt(1.0 - rnd.x) * normal);
}

vec3 cosPowDir(vec3  dir, float power) 
{//creates a biased random sample
   vec2 r=rand2()*vec2(6.2831853,1.0);
   vec3 sdir=cross(dir,((abs(dir.x)<0.5)?vec3(1.0,0.0,0.0):vec3(0.0,1.0,0.0)));
   vec3 tdir=cross(dir,sdir); 
   r.y=pow(r.y,0.01/power);
   float oneminus = sqrt(1.0-r.y*r.y);
   return cos(r.x)*oneminus*sdir + sin(r.x)*oneminus*tdir + r.y*dir;
}

vec2 intersectCube(vec3 origin, vec3 ray, Box cube) {      
   vec3   tMin = (cube.min - origin) / ray;      
   vec3   tMax = (cube.max - origin) / ray;      
   vec3     t1 = min(tMin, tMax);      
   vec3     t2 = max(tMin, tMax);
   float tNear = max(max(t1.x, t1.y), t1.z);
   float  tFar = min(min(t2.x, t2.y), t2.z);
   return vec2(tNear, tFar);   
}

vec3 normalForCube(vec3 hit, Box cube)
{  
   if(hit.x < cube.min.x + 0.0001) return vec3(-1.0, 0.0, 0.0);   
   else if(hit.x > cube.max.x - 0.0001) return vec3( 1.0, 0.0, 0.0);   
   else if(hit.y < cube.min.y + 0.0001) return vec3(0.0, -1.0, 0.0);   
   else if(hit.y > cube.max.y - 0.0001) return vec3(0.0, 1.0, 0.0);      
   else if(hit.z < cube.min.z + 0.0001) return vec3(0.0, 0.0, -1.0);   
   else return vec3(0.0, 0.0, 1.0);   
}

float intersectSphere(vec3 origin, vec3 ray, Sphere s) {   
   vec3 toSphere = origin - s.center_radius.xyz;      
   float sphereRadius = s.center_radius.w;
   float a = dot(ray, ray);      
   float b = dot(toSphere, ray);   
   float c = dot(toSphere, toSphere) - sphereRadius*sphereRadius;   
   float discriminant = b*b - a*c;      
   if(discriminant > 0.0) {      
      float t = (-b - sqrt(discriminant)) ;   
      if(t > 0.0) return t;      
   }   
   return 10000.0;   
}  

vec3 normalForSphere(vec3 hit, Sphere s) {   
   return (hit - s.center_radius.xyz+pq) / s.center_radius.w;   
} 

float iCylinder(vec3 ro, vec3 rd, Cylinder cylinder)
{
	vec3  rc = ro - cylinder.c;
    float a = dot( rd.xz, rd.xz );
	float b = dot( rc.xz, rd.xz );
	float c = dot( rc.xz, rc.xz ) - cylinder.r*cylinder.r;//0.249;
	float d = b*b - a*c;
	if( d>=0.0 )
	{
		// cylinder			
		float s = (-b - sqrt( d ))/a;
        float hy = ro.y+s*rd.y;
		if( s>0.0 && hy<cylinder.h && hy>-cylinder.h )
		{
			return s;
		}
		// cap			
		s = (cylinder.h - ro.y)/rd.y;
		if( s>0.0 && (s*s*a+2.0*s*b+c)<0.0 )
		{
			return s;
		}
	}
    return 100000.0;
}

vec3 normalforCylinder(vec3 hit,Cylinder cylinder)
{
    vec3 nor;
	nor.xz = hit.xz - cylinder.c.xz;
    nor.y = 0.0;
    nor = nor/cylinder.r;
    return nor;
}

float iPlane1(vec3 ro, vec3 rd, in vec4 plane) {
   return -(dot(ro, plane.xyz) + plane.w) / dot(rd, plane.xyz);
}

float iRectangle(vec3 ro, vec3 rd, in vec4 plane, in vec2 constraintsXY, in vec2 widthHeight, out vec2 uvCoord) {
   float planeIntersection = iPlane1(ro,rd, plane);
   vec3 hitPoint = ro + rd * planeIntersection;
   if (hitPoint.z < constraintsXY.x || hitPoint.z > constraintsXY.x+widthHeight.x) {
      planeIntersection = -1.0;
   }
   else if (hitPoint.x < constraintsXY.y || hitPoint.x > constraintsXY.y+widthHeight.y) {
      planeIntersection = -1.0;
   }
   
   if (planeIntersection >= 0.0) {
      uvCoord = vec2(hitPoint.z - constraintsXY.x, hitPoint.y - constraintsXY.y);
   }
   else {
      uvCoord = vec2(-1.0);
   }
   return planeIntersection;
}

//raymarch stuff
float rmPlane(vec3 p, vec3 c, float d)
{
    vec4 n = vec4(0,1,0,d);
   	return dot(p,n.xyz) + n.w;
    vec3 pc = p-c;
    if(pc.x>0.1 || pc.z>0.1) return 1000.0;
    float f = dot((p-c),vec3(0.0,1.0,0.0));
    return f;
}

float scene(vec3 p)
{
   float f = 1000.0;
   
    float d = length(p.xz-vec2( 0.0, 0.0))*0.5;
	float h = 1.4 * cos( d*50.-time*4. ) * (1. - smoothstep( 0., 1.0, d) );
    d = length(p.xz-vec2( 0.7, 0.7))*0.3;
	h += 0.9 * cos( d*50.-time*4. ) * (1. - smoothstep( 0., 1.0, d) );
    d = length(p.xz-vec2(-0.7, 0.7))*0.3;
	h += 0.9 * cos( d*50.-time*4. ) * (1. - smoothstep( 0., 1.0, d) );
    d = length(p.xz-vec2( 0.7,-0.7))*0.3;
	h += 0.9 * cos( d*50.-time*4. ) * (1. - smoothstep( 0., 1.0, d) );
    d = length(p.xz-vec2(-0.7,-0.7))*0.3;
	h += 0.9 * cos( d*50.-time*4. ) * (1. - smoothstep( 0., 1.0, d) );
    //p.y += h;
    
    float f1 = rmPlane(p, vec3(0.0,-0.2,0.0), h+0.2);
    if(f1<f) f=f1;
   return f;
}

//end raymarch stuff

void initscene()
{
   light = vec3(cos(time *0.5)*1.65, sin(time*0.0)*0.65+0.7,   sin(time*0.5)*1.65);

    float h = sin(time*3.0)*0.03;
    float sinr = sin(time)*0.5; float cosr = cos(time)*0.5;
    sfere[0].center_radius = vec4( 0.0, h-0.3, 0.0,    0.523);//rosu
   	sfere[1].center_radius = vec4( 0.0, h-0.29, 0.0,    0.520);//verde
   	sfere[2].center_radius = vec4(sinr, h+0.1,cosr,    0.123);//albastru
   	sfere[3].center_radius = vec4(-sinr, h+0.1,-cosr,    0.123);//rosu
    sfere[4].center_radius = vec4(cosr, h+0.1,-sinr,    0.123);//verde
    sfere[5].center_radius = vec4(-cosr, h+0.1,sinr,    0.123);//albastru
    sfere[6].center_radius = vec4( light,               0.223);//light
    
    cylinder[0].c = vec3(-0.7,0.0,-0.7);
    cylinder[0].r = 0.1;
    cylinder[0].h = 1.0;
    
    cylinder[1].c = vec3(-0.7,0.0, 0.7);
    cylinder[1].r = 0.1;
    cylinder[1].h = 1.0;
    
    cylinder[2].c = vec3( 0.7,0.0,-0.7);
    cylinder[2].r = 0.1;
    cylinder[2].h = 1.0;
    
    cylinder[3].c = vec3( 0.7,0.0, 0.7);
    cylinder[3].r = 0.1;
    cylinder[3].h = 1.0;

   	box0.min = vec3(-2.0, -1.0, -2.0);//room
   	box0.max = vec3( 2.0,  1.5,  2.0);
    
    cub = vec3(0.0, 0.0, 0.0);
   	lcub = vec3(0.8, 1.1, 0.8);

   	boxe[2].min = cub - lcub;
   	boxe[2].max = cub + lcub;
    
    cub = vec3(0.0,- 0.3, 0.0);
   	lcub = vec3(1.5, 0.55, 0.5);

   	boxe[3].min = cub - lcub;
   	boxe[3].max = cub + lcub;
    
    plane = vec4(0.0, 1.0, 0.0, 0.2);
   	planeStart = vec2(-2.0, -2.0);
   	planeWidthHeight = vec2(4.0, 4.0);
    
    //vec4 mat[7];//vec3 color and float power of emisive
	material[0].color = vec4(vec3(0.9 ,0.3 , 0.3), 0.0); //0 - rosu                    
	material[1].color = vec4(vec3(0.3 ,1.9 , 0.3), 0.0); //1 - verde
	material[2].color = vec4(vec3(0.3 ,0.3 , 1.9), 0.0); //2 - albastru
	material[3].color = vec4(vec3(0.9 ,0.9 , 0.9), 0.0); //3 - cilinder
    material[4].color = vec4(vec3(1.0 ,1.0 , 1.0), 0.0); //4 - box on wall
    material[5].color = vec4(vec3(1.0 ,1.0 , 1.0), 1.0); //5 - light

	//reflect/refract(-1 <> 1) and difusse refl/refr (0 <> 1)
	material[0].brdf = vec2( 0.0 ,10000.0 ); //0 - rosu               
	material[1].brdf = vec2( 1.0 ,1.0 ); //1 - verde
	material[2].brdf = vec2( 1.0 ,10000.0 ); //2 - albastru
	material[3].brdf = vec2( 0.0 ,10.0 ); //3 - cilinder
    material[4].brdf = vec2( 1.0 ,10000.0 ); //4 - box on wall
    material[5].brdf = vec2( 0.0 ,0.0 ); //5 - light

}

void intersectscene(vec3 ro, vec3 rd, inout float t, inout int i)
{
    float tSphere6 = intersectSphere(ro, rd, sfere[6]);
    if(tSphere6 < t) { t = tSphere6;i=6;}
    
    /*bool b = boxe[2].min.x <= ro.x && boxe[2].max.x >= ro.x &&
              boxe[2].min.y <= ro.y && boxe[2].max.y >= ro.y &&
              boxe[2].min.z <= ro.z && boxe[2].max.z >= ro.z;*/
    bool b = ( all(lessThanEqual(boxe[2].min,ro)) && all(greaterThanEqual(boxe[2].max,ro)));
    vec2 tbox = intersectCube(ro, rd, boxe[2]); 
    if(tbox.x>0.0 && tbox.x<tbox.y && tbox.x < t || b)
    {    
    	/*bool b = boxe[3].min.x <= ro.x && boxe[3].max.x >= ro.x &&
              	boxe[3].min.y <= ro.y && boxe[3].max.y >= ro.y &&
              	boxe[3].min.z <= ro.z && boxe[3].max.z >= ro.z;*/
        bool b = ( all(lessThanEqual(boxe[3].min,ro)) && all(greaterThanEqual(boxe[3].max,ro)));
    	tbox = intersectCube(ro, rd, boxe[3]); 
    	if(tbox.x>0.0 && tbox.x<tbox.y && tbox.x < t || b)
    	{
   			float tSphere0 = intersectSphere(ro, rd, sfere[0]);
   			float tSphere1 = intersectSphere(ro, rd, sfere[1]);
   			float tSphere2 = intersectSphere(ro, rd, sfere[2]);
   			float tSphere3 = intersectSphere(ro, rd, sfere[3]);
   			float tSphere4 = intersectSphere(ro, rd, sfere[4]);
   			float tSphere5 = intersectSphere(ro, rd, sfere[5]);
    
   			if(tSphere0 < t) { t = tSphere0;i=0;}
   			if(tSphere1 < t) { t = tSphere1;i=1;}   
   			if(tSphere2 < t) { t = tSphere2;i=2;}
   			if(tSphere3 < t) { t = tSphere3;i=3;} 
   			if(tSphere4 < t) { t = tSphere4;i=4;} 
   			if(tSphere5 < t) { t = tSphere5;i=5;} 
    	}
   
		float tcyl = iCylinder(ro, rd, cylinder[0]);
    	if(tcyl<t) {t = tcyl; i = 10;}
    	tcyl = iCylinder(ro, rd, cylinder[1]);
    	if(tcyl<t) {t = tcyl; i = 11;}
    	tcyl = iCylinder(ro, rd, cylinder[2]);
    	if(tcyl<t) {t = tcyl; i = 12;}
    	tcyl = iCylinder(ro, rd, cylinder[3]);
    	if(tcyl<t) {t = tcyl; i = 13;}
    }
    
    float p = iRectangle(ro, rd, plane, planeStart, planeWidthHeight,uvCoord);
   	if(p>0.0 && p<t)
   	{
      	t = p; 
      	i = 30;
   	}
}

void ColorAndNormal(vec3 hit, inout vec4 mcol, inout vec3 normal, vec2 tRoom, inout vec2 mref, inout float t, const int id)
{
	if(t == tRoom.y)
	{            
        mref = vec2(0.0,0.0);
        normal =-normalForCube(hit, box0);   
        if(abs(normal.x)>0.0)
        {
            mcol.xyz = vec3(0.3,0.3,0.3);
            float gr = 2.0; float rost = 0.04;
            float v0 = dot(vec3(gr, gr, 0),hit);
            float v1 = dot(vec3(gr, 0, gr),hit);
            v0 -= floor(v0);
            v1 -= floor(v1);
			if(v0>rost && v1> rost)  mcol.xyz = vec3(0.99,0.99,0.99);
            
            vec3 tcol = texture2D(iChannel0,1.0-(hit.zy-vec2(1.5,1.5))/3.5).xyz;
            float s = tcol.y+0.1;//-d
            s = pow(1.0-s,3.0)*0.75+0.01;
            mref = vec2(s,pow(1.0,1.0-s));
        } 
         else if(abs(normal.y)>0.0)
        {
            mcol.xyz = vec3(0.3,0.3,0.3);
            float gr = 2.0; float rost = 0.04;
            float v0 = dot(vec3(0, gr, gr),hit);
            float v1 = dot(vec3(gr, gr, 0),hit);
            v0 -= floor(v0);
            v1 -= floor(v1);
			if(v0>rost && v1> rost)  mcol.xyz = vec3(0.99,0.99,0.99);
            
            vec3 tcol = texture2D(iChannel0,1.0-(hit.xz-vec2(1.5,1.5))/3.5).xyz;
            float s = tcol.y+0.1;//-d
            s = pow(1.0-s,3.0)*0.75+0.01;
            mref = vec2(s,pow(1.0,1.0-s));
        } 
        else if(abs(normal.z)>0.0)
        {
            mcol.xyz = vec3(0.3,0.3,0.3);
            float gr = 2.0; float rost = 0.04;
            float v0 = dot(vec3(0, gr, gr),hit);
            float v1 = dot(vec3(gr, 0, gr),hit);
            v0 -= floor(v0);
            v1 -= floor(v1);
			if(v0>rost && v1> rost)  mcol.xyz = vec3(0.99,0.99,0.99);
            
            //float d = mcol.x+mcol.y; 
			//d = pow(d-1.0,2.0)*0.4;
            vec3 tcol = texture2D(iChannel0,1.0-(hit.xy-vec2(1.5,1.5))/3.5).xyz;
            float s = 1.0-tcol.y+0.1;//-d
            s = pow(s,3.0)*0.75+0.01;
            mref = vec2(s,pow(1.0,1.0-s));
        }
	}     
	else   
	{
		     if(id==0) {normal = normalForSphere(hit, sfere[0]); mcol = material[0].color;mref = material[0].brdf;}
        else if(id==1) {normal = normalForSphere(hit, sfere[1]); mcol = material[1].color;mref = material[1].brdf;}
        else if(id==2) {normal = normalForSphere(hit, sfere[2]); mcol = material[2].color;mref = material[2].brdf;}
        else if(id==3) {normal = normalForSphere(hit, sfere[3]); mcol = material[2].color;mref = material[2].brdf;}
    	else if(id==4) {normal = normalForSphere(hit, sfere[4]); mcol = material[2].color;mref = material[2].brdf;}
        else if(id==5) {normal = normalForSphere(hit, sfere[5]); mcol = material[2].color;mref = material[2].brdf;}
        else if(id==6) {normal = normalForSphere(hit, sfere[6]); mcol = material[5].color;mref = material[5].brdf;}
    	else if(id==10) {normal = normalforCylinder(hit, cylinder[0]); mcol = material[3].color;mref = material[3].brdf;}
        else if(id==11) {normal = normalforCylinder(hit, cylinder[1]); mcol = material[3].color;mref = material[3].brdf;}
        else if(id==12) {normal = normalforCylinder(hit, cylinder[2]); mcol = material[3].color;mref = material[3].brdf;}
        else if(id==13) {normal = normalforCylinder(hit, cylinder[3]); mcol = material[3].color;mref = material[3].brdf;}
		else if(id==20) {normal = normalForCube(hit, boxe[0]); mcol = material[4].color;mref = material[4].brdf;}
		else if(id==21) {normal = normalForCube(hit, boxe[1]); mcol = material[4].color;mref = material[4].brdf;}
        
        if(id==30)//material for water
        {
            float e = 0.007;
            normal = vec3((scene(hit + vec3(e, 0.0, 0.0)) - scene(hit - vec3(e, 0.0, 0.0)))/0.002*0.01, 
                        0.9,//scene(hit + vec3(0.0, e, 0.0)) - scene(hit - vec3(0.0, e, 0.0))/0.002*0.05, 
                      (scene(hit + vec3(0.0, 0.0, e)) - scene(hit - vec3(0.0, 0.0, e)))/0.002*0.01);
			normal = normalize( normal);

            mref = vec2(-1.0, 10000.0);// transparent, glossines
            mcol.xyz = vec3(0.3,0.7,0.9);// color
        }
        
        if(id>9 && id<20)// material for column
        {
            float u = atan(normal.z, normal.x) / 3.1415*2.0 ;
            float v = hit.y;//asin(normal.y) / 3.1415*2.0 + 0.5;
            vec3 tcol = vec3(mix(vec3(1.0),texture2D(iChannel2,vec2(u*1.55,1.-v*1.55)*0.75).rgb,0.8));
            float s = pow(max(0.0,tcol.y+0.5),6.0);
            s = 1.0-s+0.1;
            s = pow(1.0-tcol.y+0.1,3.0)*0.75+0.01;
            mref = vec2(max(0.0,s+0.3),100.0);
            mcol.xyz = mix(vec3(1.0),tcol,0.9);
            normal.xyz = normal.xyz + (tcol.xyz*2.0-1.0)*0.3;
            if(dot(tcol,tcol)<0.25) {mcol.xyz = vec3(0.2,0.7,0.2); mref.x = 0.0;}
            normal = normalize(normal);
        }
    }  
}

vec3 getColor(vec3 ro, vec3 rd)
{
    vec3 color = vec3(0.0);
    vec3 col = vec3(1.0);
    int id=-1;
    int tm = -1;
    
    for(int i=0; i<5; i++)
    {
    	float t = 10000.0; //seed++;
		
   		vec2 tRoom = intersectCube(ro, rd, box0);          
   		if(tRoom.x < tRoom.y)   t = tRoom.y; 
    
    	intersectscene(ro, rd, t, id);
    
    	vec3 hit = ro + rd * t;        
		vec4 mcol = vec4(vec3(0.9),0.0);
    	vec3 normal; 
    	vec2 mref = vec2(0);
      
    	ColorAndNormal(hit, mcol, normal, tRoom, mref, t, id);
    	hit = hit + normal * 0.00001;
         
        vec2 rnd = rand2();
        col *= mcol.xyz;
        if(mcol.w>0.0) 
        {
            if(i==0) {color = mcol.xyz; break;}
            float df=max(dot(rd,-normal),0.0)*1.0; //if(tm==1) df *= 19.0;
            color += col*mcol.xyz*mcol.w * df ;
            if(tm==2) color += col * 1.0;;
            break;
        }
		tm = -1;
        if(rnd.x>abs(mref.x))//diffuse
        {
        	rd = CosineWeightedSampleHemisphere ( normal, rnd);      
        	tm = 0;     
        	vec3 rl = sfere[6].center_radius.xyz;
            if(rand2().x<0.0) rl = sfere[1].center_radius.xyz;
        	rl = normalize(rl-hit);
        	float p =20.0; 
        	p = max(0.0,dot(rd,rl))*p;
        	rd = normalize(rd + rl*p);
        
        	col *= clamp(dot(normal,rd),0.0,1.0)*0.62;
        }       
        else 
        {
            vec3 nrd = reflect(rd,normal); tm = 1;//reflect
       		if(mref.x<0.0)//refract
            {
                if(id==30)
                    if(dot(rd,normal)>0.0) normal = -normal;
            	vec3 ior=vec3(1.0,1.52,1.0/1.12); tm = 2;
           	 	vec3 refr=refract(rd,normal,(side>=0.0)?ior.z:ior.y);//calc the probabilty of reflecting instead
           	 	vec2 ca=vec2(dot(normal,rd),dot(normal,refr)),n=(side>=0.0)?ior.xy:ior.yx,nn=vec2(n.x,-n.y);
            	if(rand2().y>0.5*(pow(dot(nn,ca)/dot(n,ca),2.0)+pow(dot(nn,ca.yx)/dot(n,ca.yx),2.0)))
               		nrd=refr;
            }
            rd = cosPowDir(nrd, mref.y);
            col *= 1.2;
        }
        
        ro = hit + rd * 0.0001; 
        
        if(dot(col,col) < 0.1 && i>3) break;
    }
    
 	return color;   
}

void main(void)
{
	vec2 q = gl_FragCoord.xy/iResolution.xy;
    p = -1.0+2.0*q;
	p.x *= iResolution.x/iResolution.y;
    vec2 mo = iMouse.xy/iResolution.xy;
		 
	float time = 15.0 + iGlobalTime;
    float seed = iGlobalTime + iResolution.y *(p.x+1.0) + p.y;
	
	randv2=fract(cos((gl_FragCoord.xy+gl_FragCoord.yx*vec2(1000.0,1000.0))+vec2(iGlobalTime))*10000.0);
	
    //move camera with mouse
	vec3 ro = vec3( 0.0+3.0*cos(8.0*mo.x), -0.9 + 2.5*(mo.y), 0.0 + 3.0*sin(8.0*mo.x) );
	vec3 ta = vec3( 0.0, 0.0, 0.0 );
	
    //camera path
    time *= 0.2;
    vec3 path = vec3(sin(time*0.5)*2.0, sin(time)*0.7, cos(time*0.5)*2.0);    
    ro = path; ta = path + vec3(-sin(time*0.5)*0.5+0.0, -cos(time)*0.2, -cos(time*0.5)*0.5+0.0);
    
    //view of raymarch
    //ro = vec3(0.8,0.5, 1.5); ta = vec3(0.0,0.5, -1.0);
	// camera tx
	vec3 cw = normalize( ta-ro );
	vec3 cp = vec3( 0.0, 1.0, 0.0 );
	vec3 cu = normalize( cross(cw,cp) );
	vec3 cv = normalize( cross(cu,cw) );
	vec3 rd = normalize( p.x*cu + p.y*cv + 2.5*cw );
    
    f0 = texture2D( iChannel1, vec2( 0.01, 0.0 ) ).x;
    f1 = texture2D( iChannel1, vec2( 0.56, 0.0 ) ).x;
    f2 = texture2D( iChannel1, vec2( 0.96, 0.0 ) ).x;
    
    f0 = pow(f0*1.0,5.0);
    
    initscene();
	
	vec3 col = vec3(0.0);

    for(float i=0.0; i<t; i++)
    {
        p += (rand2() * 2.0 - 1.0) * 0.5 / iResolution.x;
        rd = normalize( p.x*cu + p.y*cv + 2.5*cw );
    	col += getColor( ro, rd );
    }

    col = pow( clamp( col/t, 0.0, 1.0 ), vec3(0.65) );
    //col.g += tc;
    gl_FragColor=vec4( col, 1.0 );
}
