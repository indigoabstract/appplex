// https://www.shadertoy.com/view/MsfGzB

// iChannel0: t14
// iChannel1: c2

void sampleCamera(vec2 u, out vec3 rayOrigin, out vec3 rayDir)
{
	vec2 filmUv = (gl_FragCoord.xy + u)/iResolution.xy;

	float tx = (2.0*filmUv.x - 1.0)*(iResolution.x/iResolution.y);
	float ty = (1.0 - 2.0*filmUv.y);
	float tz = 0.0;

	rayOrigin = vec3(0.0, 0.0, 5.0);
	rayDir = vec3(tx, ty, tz) - rayOrigin;
}

#define DO_BLOBS(DO) vec4 b; b=vec4(-0.38 + 0.25*sin(iGlobalTime+0.00), -0.60 + 0.25*cos(iGlobalTime+0.00), -0.67, 0.17); DO; b=vec4(-0.33 + 0.25*sin(iGlobalTime+1.00), -0.59 + 0.25*cos(iGlobalTime+1.00), 0.02, 0.19); DO; b=vec4(-0.33 + 0.25*sin(iGlobalTime+2.00), -0.42 + 0.25*cos(iGlobalTime+2.00), 0.48, 0.12); DO; b=vec4(-0.50 + 0.25*sin(iGlobalTime+3.00), -0.18 + 0.25*cos(iGlobalTime+3.00), -0.30, 0.15); DO; b=vec4(-0.57 + 0.25*sin(iGlobalTime+4.00), 0.09 + 0.25*cos(iGlobalTime+4.00), 0.14, 0.16); DO; b=vec4(-0.58 + 0.25*sin(iGlobalTime+5.00), -0.13 + 0.25*cos(iGlobalTime+5.00), 0.58, 0.12); DO; b=vec4(-0.48 + 0.25*sin(iGlobalTime+6.00), 0.67 + 0.25*cos(iGlobalTime+6.00), -0.66, 0.13); DO; b=vec4(-0.37 + 0.25*sin(iGlobalTime+7.00), 0.43 + 0.25*cos(iGlobalTime+7.00), -0.16, 0.18); DO; b=vec4(-0.49 + 0.25*sin(iGlobalTime+8.00), 0.41 + 0.25*cos(iGlobalTime+8.00), 0.62, 0.16); DO; b=vec4(0.19 + 0.25*sin(iGlobalTime+9.00), -0.64 + 0.25*cos(iGlobalTime+9.00), -0.47, 0.18); DO; b=vec4(0.19 + 0.25*sin(iGlobalTime+10.00), -0.43 + 0.25*cos(iGlobalTime+10.00), -0.04, 0.13); DO; b=vec4(-0.01 + 0.25*sin(iGlobalTime+11.00), -0.40 + 0.25*cos(iGlobalTime+11.00), 0.39, 0.11); DO; b=vec4(-0.12 + 0.25*sin(iGlobalTime+12.00), -0.06 + 0.25*cos(iGlobalTime+12.00), -0.70, 0.12); DO; b=vec4(0.08 + 0.25*sin(iGlobalTime+13.00), 0.18 + 0.25*cos(iGlobalTime+13.00), 0.07, 0.15); DO; b=vec4(-0.15 + 0.25*sin(iGlobalTime+14.00), -0.12 + 0.25*cos(iGlobalTime+14.00), 0.51, 0.19); DO; b=vec4(0.09 + 0.25*sin(iGlobalTime+15.00), 0.57 + 0.25*cos(iGlobalTime+15.00), -0.48, 0.10); DO; b=vec4(0.12 + 0.25*sin(iGlobalTime+16.00), 0.64 + 0.25*cos(iGlobalTime+16.00), 0.19, 0.14); DO; b=vec4(-0.11 + 0.25*sin(iGlobalTime+17.00), 0.67 + 0.25*cos(iGlobalTime+17.00), 0.42, 0.20); DO; b=vec4(0.55 + 0.25*sin(iGlobalTime+18.00), -0.69 + 0.25*cos(iGlobalTime+18.00), -0.35, 0.18); DO; b=vec4(0.33 + 0.25*sin(iGlobalTime+19.00), -0.49 + 0.25*cos(iGlobalTime+19.00), -0.03, 0.17); DO; b=vec4(0.35 + 0.25*sin(iGlobalTime+20.00), -0.66 + 0.25*cos(iGlobalTime+20.00), 0.55, 0.15); DO; b=vec4(0.51 + 0.25*sin(iGlobalTime+21.00), -0.12 + 0.25*cos(iGlobalTime+21.00), -0.66, 0.14); DO; b=vec4(0.48 + 0.25*sin(iGlobalTime+22.00), -0.08 + 0.25*cos(iGlobalTime+22.00), -0.12, 0.11); DO; b=vec4(0.50 + 0.25*sin(iGlobalTime+23.00), 0.15 + 0.25*cos(iGlobalTime+23.00), 0.60, 0.16); DO; b=vec4(0.59 + 0.25*sin(iGlobalTime+24.00), 0.43 + 0.25*cos(iGlobalTime+24.00), -0.52, 0.11); DO; b=vec4(0.50 + 0.25*sin(iGlobalTime+25.00), 0.66 + 0.25*cos(iGlobalTime+25.00), 0.15, 0.18); DO; b=vec4(0.35 + 0.25*sin(iGlobalTime+26.00), 0.44 + 0.25*cos(iGlobalTime+26.00), 0.37, 0.14); DO; 

float k = 10.0;

float sdf(vec3 x)
{
	//http://www.johndcook.com/blog/2010/01/13/soft-maximum/
	float sum = 0.0;
	DO_BLOBS( sum += exp( k*(b.w - length(x-b.xyz)) ) )
	return log( sum ) / k;	
}

vec3 BlobNor(vec3 x)
{
	vec3 sum=vec3(0.0,0.0,0.0);

	float w;
	vec3 n;
	DO_BLOBS( n=normalize(x-b.xyz); w = exp( k*(b.w - length(x-b.xyz))); sum += w*n );
	return normalize( sum );	
	
}

vec3 ss_nor(vec3 X)
{
	return normalize(cross(dFdx(X),-dFdy(X)));
}
vec3 ss_grad(vec3 X)
{
	return cross(dFdx(X),-dFdy(X));
}

void main(void)
{
	vec3 P, V;
	sampleCamera(vec2(0.5,0.5), P, V);
	V = normalize(V);
		
	float t=0.0;

	float d;	
	vec3 X;
	for (int i=0; i<64; i++)
	{			
		X = P+V*t;
		d = sdf(X);
		
		if (abs(d) < 0.0001)
		{		
			break;
		}
		
		vec3 G=-ss_grad(X);
		if (dot(G,V) < 0.0)	break;
		
		t -= d;		
	}
	
	//seems a waste to recalculate this but otherwise shader compiler gets upset!
	X = P+V*t;
	d = sdf(X);
	float e = 0.005;
	vec3 N = //BlobNor(X);
		normalize( vec3(sdf(X-vec3(e,0,0)),sdf(X-vec3(0,e,0)),sdf(X-vec3(0,0,e)) - vec3(d,d,d) ));
	//ss_nor(X);	  
	vec3 L = normalize(vec3(1,-1,1));
	
	vec3 H = normalize(L-V);
	float s = pow(max(0.0,dot(N,H)),100.0);
	
	float dif = max(dot(N,L),0.0);
	dif += s;
		
	float f = dot(-V,N);
	f = 1.0-f;
	
	float ff = f;
	f *= f;		//2
	f *= f;		//4
	f *= ff;	//5
	float r0 = 0.4;
	f = r0 + (1.0-r0)*f;
	
	vec3 c = dif * (f) * textureCube(iChannel1,N).xyz; 

	vec3 bg = textureCube(iChannel1,V).xyz;
	float n=dot(-N,V);

	//weird, which way looks less bad on your set up? vote in the commments!! :-)
#if 0	
	c = mix(c,bg,pow(0.9-abs(n),1.9));
#else	
	c = mix(c,bg,pow(abs(0.9-n),1.9));
#endif
	
	gl_FragColor = vec4(c,1.0);
}
