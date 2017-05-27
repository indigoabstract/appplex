// https://www.shadertoy.com/view/lsfXzM

// iChannel0: t4
// iChannel1: t14

#define M_PI (3.1415926535897932384626433832795)

float qScanLine (vec2 uv, float n) {
	return abs (sin (uv.y*M_PI*n)) ; 
}

float qVignete (vec2 uv,float q, float o) {
	float x = clamp (1.0 - distance (uv, vec2 (0.5,0.5))*q, 0.0, 1.0);
	return (log((o - 1.0/exp (o))*x + 1.0/exp (o)) + o)/(log(o) + o);
}


vec2 vCrtCurvature (vec2 uv, float q) {
	float x = 1.0- distance (uv, vec2 (0.5, 0.5));
	vec2 g = vec2 (0.5, 0.5) - uv;
	return uv + g*x*q;
}

vec4 v2DNoiseSample (vec2 gPos) {
	vec2 nPos = vec2(
		mod (gPos.x+iGlobalTime*9.66,1.0),
		mod (gPos.y+iGlobalTime*7.77,1.0)
	);		
	return texture2D (iChannel1, nPos);
}

vec4 v1DNoiseSample (float idx, float s) {	
	return texture2D (iChannel1, vec2 (
		mod (idx, 1.0), 
		mod (iGlobalTime*s, 1.0))
	);
}

float q2DNoiseSample (vec2 gPos) {
 	vec4 nPnt = v2DNoiseSample (gPos);
	return nPnt.x;
}

float q1DNoiseSample (float idx, float s){
	vec4 nPnt = v1DNoiseSample (idx, s);
	return nPnt.x;
}

vec4 cSignalNoise (vec4 c,float q, vec2 gPos) {
	return c*(1.0 - q) + q*q2DNoiseSample(gPos);
}

vec2 vScanShift (vec2 uv, float q, float dy, float dt) {
	return vec2 (uv.x + q1DNoiseSample (uv.y*dy, dt)*q, uv.y);
}

vec2 vFrameShift (vec2 uv, float q, float dt) {
	float s = (q1DNoiseSample (0.5, dt) - 0.5)/500.0;
	return vec2 (uv.x, mod (uv.y + iGlobalTime*(q+s), 1.0));	
}

vec2 vDirShift (vec2 uv, float angle, float q) {
	float a =(angle/180.0)*M_PI;
	vec2 dir = vec2 (sin (a), cos (a));
	return uv + dir*q;
}

vec4 vRGBWithShift (vec2 uv, float angle, float q) {
	vec2 rPos = vDirShift (uv, angle, q);
	vec2 gPos = uv;
	vec2 bPos = vDirShift (uv, -angle, q);
	vec4 rPix = texture2D (iChannel0, rPos);
	vec4 gPix = texture2D (iChannel0, gPos);
	vec4 bPix = texture2D (iChannel0, bPos);
	return vec4 (rPix.x, gPix.y, bPix.z, 1.0);
}

vec4 vPowerNoise (vec4 col, vec2 uv, float b, float dt, float w) {
	float s = q1DNoiseSample (0.0, 0.001)/500.0;
	float y = mod (iGlobalTime * (dt + s) , 1.0);
	float d = 1.0 - clamp (abs (uv.y - y), 0.0, w)/w;
	return pow (col,vec4(1.0/(1.0 + b*d)) ) ;
}

vec4 qGamma (vec4 i, vec4 g){
	return pow(i, 1.0/g);
}

vec4 vRGBTint (vec4 col, vec3 g, float q) {	
	return qGamma (col, vec4 (g, 1.0))*q + (1.0 - q)*col;	
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 vColorDrift (vec4 col, float q) {
	vec3 hsv = rgb2hsv (col.xyz);
	hsv.y = mod (hsv.y * q, 1.0);
	return vec4 (hsv2rgb (hsv), col.w);
}

void main(void)
{
	vec2 cRes = iChannelResolution [0].xy;
	vec2 gRes = iResolution.xy;
	vec2 gPos = gl_FragCoord.xy / gRes;	
	vec2 cPos = gPos ;
	vec4 cCol = vec4(1.0);
	vec2 bPos = vec2(1.0);
	float qNoise = q1DNoiseSample(0.01,0.01);
	
	cPos = vScanShift (cPos, 0.02, 0.1, 0.1);			// snaline shift
	cPos = vCrtCurvature (cPos, 0.3);					// crt curving of coords
	bPos = vCrtCurvature (gPos, 0.3);					// curvature for the noize bar
	cPos = vFrameShift (cPos, 0.01, 0.001);				// frame shift		
	cCol = vColorDrift (cCol, 1.0 - qNoise);	
	cCol = vRGBWithShift (cPos, 100.0, 0.01); 			// sample signal color	
	cCol = cSignalNoise (cCol, qNoise * 0.8, gPos);				// add signal noise
	cCol = vPowerNoise (cCol, bPos, 4.0, -0.2, 0.1); 	// power line noize
	cCol = vRGBTint (cCol, vec3 (0.9, 0.7, 1.2), 1.0);	// gamma tint
	cCol = cCol * qScanLine (gPos, 120.0); 				// add scanlines
	cCol = cCol * qVignete (gPos, 1.5, 3.0); 			// add edge darkening
	gl_FragColor = cCol;
		
}
