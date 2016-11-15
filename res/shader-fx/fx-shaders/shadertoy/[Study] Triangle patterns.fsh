// https://www.shadertoy.com/view/Xs23W3

// iChannel0: t12

// A study for finding whether the pixel is inside one of the given set of triangles, and painting the triangles. A Delanuay triangulation of random points is done via Python beforehand and triangles are loaded by assignment.
#define TWOPI 6.28318530718

// Please try METHOD = 0, 1 and 2
#define METHOD 1

float determinant(mat2 m) {
	return m[0][0]*m[1][1]-m[0][1]*m[1][0];
}

mat2 inverse(mat2 m) {
	return mat2(m[1][1], -m[0][1], -m[1][0], m[0][0]) / determinant(m);
}

float disk(vec2 r, vec2 center, float radius) {
	return 1.0 - smoothstep( radius-0.005, radius+0.005, length(r-center));
}

float triangle(vec2 r, vec2 A1, vec2 A2, vec2 A3) {
	vec2 q = r - A3;
	vec2 A1q = A1 - A3;
	vec2 A2q = A2 - A3;
	mat2 Bq = mat2(A1q, A2q);
	vec2 c = inverse(Bq)*q;
	
	float ret = 0.0;
	if(c.x>0. && c.y>0.0 && c.x+c.y<1.0) {
		// solid color
		//ret = 1.0;
		// darker towards edges
		float x = c.x*c.y*(1.-c.x-c.y)*75.;
		//x = 1.0-smoothstep(0.2, 5.8, x);
		ret = pow(x,0.25);
		//ret = pow((1.0-c.x*c.y*(1.-c.x-c.y)),64.);
		ret -= 0.5*pow(texture2D(iChannel0, 0.5*c).x, 4.0 );
	}
	
		
	return ret;
}

float triangle2(vec2 r, vec2 A1, vec2 A2, vec2 A3) {
	mat2 B = mat2(A1-A3, A2-A3);
	vec2 c = inverse(B)*(r-A3);
	
	float ret = 0.0;
	if(c.x>0. && c.y>0.0 && 1.-c.x-c.y>0.0) {
		ret = 1.0;
	}
	return ret;
}

float triangle3(vec2 r, vec2 A3, mat2 inv) {
	vec2 c = inv*(r-A3);
	
	float ret = 0.0;
	if(c.x>0. && c.y>0.0 && 1.-c.x-c.y>0.0) {
		// solid color
		//ret = 1.0;
		// darker towards edges
		ret = pow(c.x*c.y*(1.-c.x-c.y)*75.,1.15+sin(iGlobalTime));
	}
	return ret;	
}


// a new method to test being inside of triangle
// no need of matrices etc. uses vectors and trigonometry
//
// Hmm... I expected this to be faster than matrix method,
// but I was wrong. I don't get it. :-(
float triangle4(vec2 r, vec2 A1, vec2 A2, vec2 A3) {
	float ret = 0.0;
	// Draw arrows from pixel to vertices	
	vec2 A1p = A1 - r;
	vec2 A2p = A2 - r;
	vec2 A3p = A3 - r;
	// normalize them
	vec2 A1pn = normalize(A1p);
	vec2 A2pn = normalize(A2p);
	vec2 A3pn = normalize(A3p);
	// find the angles between the arrows
	float an12 = acos(dot(A1pn, A2pn));
	float an23 = acos(dot(A2pn, A3pn));
	float an31 = acos(dot(A3pn, A1pn));	
	
	// if they add up to 2Pi then inside the triangle!
	if( abs(an12+an23+an31-TWOPI) < 0.01 ) {
		//float man = max(an12, an23);
		//man = max(man, an31);
		ret = 1.0;
		//float col = 1. - man/PI;
		//col = pow(col, .4)*1.;
		//ret = (sin(man));	
	}
	return ret;
}


void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 r = 2.0*(gl_FragCoord.xy-0.5*iResolution.xy) / iResolution.y;

	vec3 pixel = vec3(0.0, 0.1, 0.0);

const int nPoints = 20;
vec2 pt[20];
const int nTriangles = 31;
ivec3 triangles[31];
mat3 tri[31];
mat2 invB[31];
	
#if METHOD == 0
pt[0] = vec2(-0.752524495777,0.276016142992);
pt[1] = vec2(-0.545694726988,0.507777307568);
pt[2] = vec2(0.0344987625226,-0.306673145755);
pt[3] = vec2(-0.297237538054,0.191710047873);
pt[4] = vec2(-0.715035346718,0.0364155198627);
pt[5] = vec2(0.612024151395,0.536568473836);
pt[6] = vec2(-0.40447145914,-0.79573858745);
pt[7] = vec2(-0.0664654101251,0.789470572312);
pt[8] = vec2(0.262284652061,-0.61074282809);
pt[9] = vec2(-0.350794881771,0.986410561889);
pt[10] = vec2(-0.794242268159,0.00942264957239);
pt[11] = vec2(0.45294387004,0.544629519029);
pt[12] = vec2(-0.670283449528,0.248633296271);
pt[13] = vec2(0.132599909009,-0.241305025684);
pt[14] = vec2(0.0758491466031,-0.370676582564);
pt[15] = vec2(-0.251344572955,0.381006780584);
pt[16] = vec2(-0.48716276886,0.84889489905);
pt[17] = vec2(-0.402684172294,0.388539347107);
pt[18] = vec2(0.851456864277,0.970948923036);
pt[19] = vec2(-0.392850985248,-0.651126446093);
triangles[0] = ivec3(19,6,10);
triangles[1] = ivec3(8,19,6);
triangles[2] = ivec3(11,7,18);
triangles[3] = ivec3(9,7,18);
triangles[4] = ivec3(14,8,13);
triangles[5] = ivec3(14,8,19);
triangles[6] = ivec3(4,19,10);
triangles[7] = ivec3(5,8,18);
triangles[8] = ivec3(5,11,18);
triangles[9] = ivec3(5,8,13);
triangles[10] = ivec3(5,11,13);
triangles[11] = ivec3(15,11,7);
triangles[12] = ivec3(15,3,13);
triangles[13] = ivec3(15,11,13);
triangles[14] = ivec3(2,14,19);
triangles[15] = ivec3(2,4,19);
triangles[16] = ivec3(2,4,3);
triangles[17] = ivec3(2,3,13);
triangles[18] = ivec3(2,14,13);
triangles[19] = ivec3(16,9,7);
triangles[20] = ivec3(16,15,7);
triangles[21] = ivec3(1,16,15);
triangles[22] = ivec3(12,4,3);
triangles[23] = ivec3(17,15,3);
triangles[24] = ivec3(17,1,15);
triangles[25] = ivec3(17,12,3);
triangles[26] = ivec3(17,12,1);
triangles[27] = ivec3(0,1,16);
triangles[28] = ivec3(0,12,1);
triangles[29] = ivec3(0,4,10);
triangles[30] = ivec3(0,12,4);
#else
tri[0] = mat3(vec3(-0.392850985248,-0.651126446093,0.0),vec3(-0.40447145914,-0.79573858745,0.0),vec3(-0.794242268159,0.00942264957239,0.0));
tri[1] = mat3(vec3(0.262284652061,-0.61074282809,0.0),vec3(-0.392850985248,-0.651126446093,0.0),vec3(-0.40447145914,-0.79573858745,0.0));
tri[2] = mat3(vec3(0.45294387004,0.544629519029,0.0),vec3(-0.0664654101251,0.789470572312,0.0),vec3(0.851456864277,0.970948923036,0.0));
tri[3] = mat3(vec3(-0.350794881771,0.986410561889,0.0),vec3(-0.0664654101251,0.789470572312,0.0),vec3(0.851456864277,0.970948923036,0.0));
tri[4] = mat3(vec3(0.0758491466031,-0.370676582564,0.0),vec3(0.262284652061,-0.61074282809,0.0),vec3(0.132599909009,-0.241305025684,0.0));
tri[5] = mat3(vec3(0.0758491466031,-0.370676582564,0.0),vec3(0.262284652061,-0.61074282809,0.0),vec3(-0.392850985248,-0.651126446093,0.0));
tri[6] = mat3(vec3(-0.715035346718,0.0364155198627,0.0),vec3(-0.392850985248,-0.651126446093,0.0),vec3(-0.794242268159,0.00942264957239,0.0));
tri[7] = mat3(vec3(0.612024151395,0.536568473836,0.0),vec3(0.262284652061,-0.61074282809,0.0),vec3(0.851456864277,0.970948923036,0.0));
tri[8] = mat3(vec3(0.612024151395,0.536568473836,0.0),vec3(0.45294387004,0.544629519029,0.0),vec3(0.851456864277,0.970948923036,0.0));
tri[9] = mat3(vec3(0.612024151395,0.536568473836,0.0),vec3(0.262284652061,-0.61074282809,0.0),vec3(0.132599909009,-0.241305025684,0.0));
tri[10] = mat3(vec3(0.612024151395,0.536568473836,0.0),vec3(0.45294387004,0.544629519029,0.0),vec3(0.132599909009,-0.241305025684,0.0));
tri[11] = mat3(vec3(-0.251344572955,0.381006780584,0.0),vec3(0.45294387004,0.544629519029,0.0),vec3(-0.0664654101251,0.789470572312,0.0));
tri[12] = mat3(vec3(-0.251344572955,0.381006780584,0.0),vec3(-0.297237538054,0.191710047873,0.0),vec3(0.132599909009,-0.241305025684,0.0));
tri[13] = mat3(vec3(-0.251344572955,0.381006780584,0.0),vec3(0.45294387004,0.544629519029,0.0),vec3(0.132599909009,-0.241305025684,0.0));
tri[14] = mat3(vec3(0.0344987625226,-0.306673145755,0.0),vec3(0.0758491466031,-0.370676582564,0.0),vec3(-0.392850985248,-0.651126446093,0.0));
tri[15] = mat3(vec3(0.0344987625226,-0.306673145755,0.0),vec3(-0.715035346718,0.0364155198627,0.0),vec3(-0.392850985248,-0.651126446093,0.0));
tri[16] = mat3(vec3(0.0344987625226,-0.306673145755,0.0),vec3(-0.715035346718,0.0364155198627,0.0),vec3(-0.297237538054,0.191710047873,0.0));
tri[17] = mat3(vec3(0.0344987625226,-0.306673145755,0.0),vec3(-0.297237538054,0.191710047873,0.0),vec3(0.132599909009,-0.241305025684,0.0));
tri[18] = mat3(vec3(0.0344987625226,-0.306673145755,0.0),vec3(0.0758491466031,-0.370676582564,0.0),vec3(0.132599909009,-0.241305025684,0.0));
tri[19] = mat3(vec3(-0.48716276886,0.84889489905,0.0),vec3(-0.350794881771,0.986410561889,0.0),vec3(-0.0664654101251,0.789470572312,0.0));
tri[20] = mat3(vec3(-0.48716276886,0.84889489905,0.0),vec3(-0.251344572955,0.381006780584,0.0),vec3(-0.0664654101251,0.789470572312,0.0));
tri[21] = mat3(vec3(-0.545694726988,0.507777307568,0.0),vec3(-0.48716276886,0.84889489905,0.0),vec3(-0.251344572955,0.381006780584,0.0));
tri[22] = mat3(vec3(-0.670283449528,0.248633296271,0.0),vec3(-0.715035346718,0.0364155198627,0.0),vec3(-0.297237538054,0.191710047873,0.0));
tri[23] = mat3(vec3(-0.402684172294,0.388539347107,0.0),vec3(-0.251344572955,0.381006780584,0.0),vec3(-0.297237538054,0.191710047873,0.0));
tri[24] = mat3(vec3(-0.402684172294,0.388539347107,0.0),vec3(-0.545694726988,0.507777307568,0.0),vec3(-0.251344572955,0.381006780584,0.0));
tri[25] = mat3(vec3(-0.402684172294,0.388539347107,0.0),vec3(-0.670283449528,0.248633296271,0.0),vec3(-0.297237538054,0.191710047873,0.0));
tri[26] = mat3(vec3(-0.402684172294,0.388539347107,0.0),vec3(-0.670283449528,0.248633296271,0.0),vec3(-0.545694726988,0.507777307568,0.0));
tri[27] = mat3(vec3(-0.752524495777,0.276016142992,0.0),vec3(-0.545694726988,0.507777307568,0.0),vec3(-0.48716276886,0.84889489905,0.0));
tri[28] = mat3(vec3(-0.752524495777,0.276016142992,0.0),vec3(-0.670283449528,0.248633296271,0.0),vec3(-0.545694726988,0.507777307568,0.0));
tri[29] = mat3(vec3(-0.752524495777,0.276016142992,0.0),vec3(-0.715035346718,0.0364155198627,0.0),vec3(-0.794242268159,0.00942264957239,0.0));
tri[30] = mat3(vec3(-0.752524495777,0.276016142992,0.0),vec3(-0.670283449528,0.248633296271,0.0),vec3(-0.715035346718,0.0364155198627,0.0));
#endif
#if METHOD == 2	
invB[0] = mat2(vec2(12.2510254236,-10.0506623921),vec2(5.93060355006,-6.10741623619));
invB[1] = mat2(vec2(1.53399980438,-1.96237643676),vec2(-0.123266307448,7.07273770067));
invB[2] = mat2(vec2(0.56888588823,-1.33639683109),vec2(-2.87743979553,1.24923120468));
invB[3] = mat2(vec2(-0.780971094288,-0.0665373746576),vec2(3.95017234978,-5.17375134818));
invB[4] = mat2(vec2(-9.78814469667,3.42766092189),vec2(-3.43595869638,-1.50359457115));
invB[5] = mat2(vec2(-0.24503895042,1.70170835607),vec2(3.9752195788,-2.84396975926));
invB[6] = mat2(vec2(10.4592126164,0.427408305219),vec2(6.35567711449,-1.25417177539));
invB[7] = mat2(vec2(-12.8819194429,3.53776514935),vec2(4.79845012174,-1.9500341436));
invB[8] = mat2(vec2(6.001840763,-6.11532635427),vec2(-5.61037454762,3.37079898937));
invB[9] = mat2(vec2(1.32893321047,2.79814875556),vec2(0.46649899066,-1.724573916));
invB[10] = mat2(vec2(6.15892743873,-6.09575755676),vec2(-2.51035563292,3.75697841644));
invB[11] = mat2(vec2(-0.951112766618,1.58672380226),vec2(-2.01770410166,-0.718184020575));
invB[12] = mat2(vec2(4.27714958179,-6.14694694126),vec2(4.24576225912,-3.79244991859));
invB[13] = mat2(vec2(-1.56839020972,1.24186899648),vec2(0.639269943792,0.766189462769));
invB[14] = mat2(vec2(-6.74237275671,8.28109708786),vec2(11.2681495377,-10.2740335142));
invB[15] = mat2(vec2(1.69848014931,-0.850925648119),vec2(0.795913223442,1.05571019566));
invB[16] = mat2(vec2(0.597883981974,-1.91877545315),vec2(-1.60852169555,-1.27718486218));
invB[17] = mat2(vec2(-6.13536188092,-0.926196561268),vec2(-6.0903383017,1.38998864331));
invB[18] = mat2(vec2(-14.4037331347,7.27783586836),vec2(6.3184123048,-10.9222055317));
invB[19] = mat2(vec2(-2.98592777252,0.900968604446),vec2(-4.31089322062,6.37845025774));
invB[20] = mat2(vec2(-2.23416737961,-0.325032218376),vec2(1.01123037862,-2.30108111076));
invB[21] = mat2(vec2(-4.33920223823,1.17567198807),vec2(-2.18698189397,2.72980825286));
invB[22] = mat2(vec2(-1.90045491795,-0.696612229362),vec2(5.11290327071,-4.56524094034));
invB[23] = mat2(vec2(-6.52887308512,6.78867244949),vec2(1.5828553422,3.63687044328));
invB[24] = mat2(vec2(-7.47107010851,0.443922841778),vec2(-17.3471759529,8.91901914235));
invB[25] = mat2(vec2(0.844257950794,-2.91927649076),vec2(5.53283562751,-1.56393322318));
invB[26] = mat2(vec2(4.99159876947,-2.29674632932),vec2(-2.39981202347,-2.75465099603));
invB[27] = mat2(vec2(-5.98579635017,10.0526494462),vec2(1.02709561182,-4.65646245201));
invB[28] = mat2(vec2(-10.4815316901,9.37398467703),vec2(5.03920826492,-8.36559087426));
invB[29] = mat2(vec2(-1.35032087116,13.336364544),vec2(3.96233368338,-2.08693548111));
invB[30] = mat2(vec2(-11.3616396639,12.8276527504),vec2(2.39591111903,2.00708069855));
#endif
	
	pt[4] += 0.1*vec2(0.5*cos(iGlobalTime),sin(iGlobalTime));
	pt[2] += vec2(0.05,0.02)*vec2(cos(-iGlobalTime),sin(iGlobalTime*2.2));
	pt[7] += vec2(0.01,0.2)*vec2(cos(iGlobalTime*3.1),sin(iGlobalTime*1.2));
	pt[8] += vec2(0.2,0.0)+vec2(0.3,0.2)*vec2(cos(iGlobalTime*0.4),sin(iGlobalTime*1.2));
	pt[11] += vec2(0.01,0.03)*vec2(cos(iGlobalTime*3.1),sin(iGlobalTime*5.1));
	float nTrigInv = 1.0/float(nTriangles);
	
	vec3 col1 = vec3(1.0, 0.0, 0.0);
	vec3 col2 = vec3(0.0, 0.0, 1.0);
	
	#if METHOD == 0
 	// 16 FPS
	// Here, the data structures are defined such that
	// points are given in "pt", an array of vec2s. 
	// and each triangle is a set of 3 indices of pt array.
	for(int i=0; i<nTriangles; i++) {
		ivec3 tri = triangles[i];

		vec2 A1, A2, A3;
		// Super stupid method to reach array indices :-(
		// "pt[tri.x]" does not work -Grrr!- because
		// a variable has to be a constant (or a looping variable) 
		// to be used as an index.
		// So, tri to be constant. But I can't make "triangles" 
		// a constant array.
		for(int ind=0; ind<nTriangles; ind++) {
			if(ind==tri.x) A1 = pt[ind];
			if(ind==tri.y) A2 = pt[ind];
			if(ind==tri.z) A3 = pt[ind];		
		}
		pixel += vec3(1.0)*triangle(r,A1,A2,A3)*float(i)/float(nTriangles);
	}
	
	#elif METHOD == 1
	// 25 FPS
	// Here, the data structures are defined such that
	// each triangle is given as three points
	for(int i=0; i<nTriangles; i++) {
		mat3 t = tri[i];
		vec2 A1 = t[0].xy;
		vec2 A2 = t[1].xy;
		vec2 A3 = t[2].xy;
		vec3 col = vec3(0.0);
		
		if( mod(float(i),2.0) < 0.9 ) {
			col = col1;
		} else {
			col = col2;
		}
		
		float x = float(i)-mod(iGlobalTime*1.0,float(nTriangles));
		if(abs(x)<0.5) {
			col=mix(col,vec3(1.0, 1.0, 0.0), 0.8);
			//col=vec3(1.0, 1.0, 0.0);
		}
		pixel = mix(pixel, col, triangle(r,A1,A2,A3)*float(i)*nTrigInv);
	}
	
	#elif METHOD == 2
	// 15 FPS
	// Here, the data structures are defined such that
	// Triangle matrix inverses calculated beforehand
	for(int i=0; i<nTriangles; i++) {
		mat3 t = tri[i];
		vec2 A3 = t[2].xy;
	
		pixel += vec3(1.0)*triangle3(r,A3,invB[i])*float(i)/float(nTriangles);				
	}		
	#endif	
	
	/*
	float c1=0.1;
	float c2=0.3;
	vec2 x = c1*A1+c2*A2+(1.0-c1-c2)*A3;
	pixel = mix(pixel, vec3(1.0,1.0,0.0), disk(r, x, 0.05));
	float u = 0.1;
	float v = 0.1;
	x = u*A1q + v*A2q;
	pixel = mix(pixel, vec3(1.0,0.0,1.0), disk(q, x, 0.05));
	*/
	
	//pixel += 0.3*vec3( pow(texture2D(iChannel0, 1.*r).x, 4.0) );
	gl_FragColor = vec4(pixel,1.0);
}
