// https://www.shadertoy.com/view/MlB3zz

// iChannel0: t11
// iChannel1: t3

mat3 rotY(in float a) {
	return mat3( cos(a), 0.0, sin(a),
				 0.0,    1.0, 0.0,
				-sin(a), 0.0, cos(a)
			    );
}
float smin( float a, float b, float k ) {
    a = pow( a, k ); b = pow( b, k );
    return pow( (a*b)/(a+b), 1.0/k );
}
float length2( vec2 p ) {
	return sqrt( p.x*p.x + p.y*p.y );
}

float length6( vec2 p ) {
	p = p*p*p; p = p*p;
	return pow( p.x + p.y, 1.0/6.0 );
}

float length8( vec2 p ) {
	p = p*p; p = p*p; p = p*p;
	return pow( p.x + p.y, 1.0/8.0 );
}
float sdTorus82( vec3 p, vec2 t ){
  vec2 q = vec2(length2(p.xz)-t.x,p.y);
  return length8(q)-t.y;
}
float sdTorus( vec3 p, vec2 t ) {
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}
float udBox( vec3 p, vec3 b ){
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}
float sdSphere( vec3 p, float s ){
  return length(p)-s;
}
float udRoundBox( vec3 p, vec3 b, float r )
{
  return length(max(abs(p)-b,0.0))-r;
}
float sdCapsule( vec3 p, vec3 a, vec3 b, float r ) {

    vec3 pa = p - a , ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h ) - r;
}
float sdHexPrism( vec3 p, vec2 h ) {
    vec3 q = abs(p);
    return max(q.z-h.y,max(q.x+q.y*0.57735,q.y*1.1547)-h.x);
}
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float palkki(in vec3 p, out vec3 color){
    
    float finalDistance = 10000000.;
    
    finalDistance = min(finalDistance, udRoundBox( p-vec3(0.0), vec3(2.0,8.0+sin(p.x)*2.,2.0) , 0.1) );
    vec3 p2 = p;
    vec3 p3 = p;
    vec3 p4 = p;
    p2.yxz *= rotY(3.14145*0.25);
    finalDistance = min(finalDistance, udRoundBox( (p2-vec3(2.0,0.0,0.0)), vec3(0.2,1.0,1.0) , 0.1 ) );
    finalDistance = min(finalDistance, udRoundBox( (p2-vec3(2.0,0.0,0.0)), vec3(0.4,0.8,0.8) , 0.1 ) );
    finalDistance = min(finalDistance, udRoundBox( (p-vec3(2.0,-0.8,0.0)), vec3(0.4,1.2,0.8) , 0.1 ) );
    
    
    p3.zyx *= rotY(3.14145*0.5);
    p3.yxz *= rotY(3.14145*0.25);
    p.zyx *= rotY(3.14145*0.5);
    finalDistance = min(finalDistance, udRoundBox( (p3-vec3(2.0,0.0,0.0)), vec3(0.2,1.0,1.0) , 0.1 ) );
    finalDistance = min(finalDistance, udRoundBox( (p3-vec3(2.0,0.0,0.0)), vec3(0.4,0.8,0.8) , 0.1 ) );
    finalDistance = min(finalDistance, udRoundBox( (p-vec3(2.0,-0.8,0.0)), vec3(0.4,1.2,0.8) , 0.1 ) );
    
    
    p4.zyx *= rotY(-3.14145*0.5);
    p4.yxz *= rotY(3.14145*0.25);
    p.zyx *= rotY(3.14145);
    finalDistance = min(finalDistance, udRoundBox( (p4-vec3(2.0,0.0,0.0)), vec3(0.2,1.0,1.0) , 0.1 ) );
    finalDistance = min(finalDistance, udRoundBox( (p4-vec3(2.0,0.0,0.0)), vec3(0.4,0.8,0.8) , 0.1 ) );
    finalDistance = min(finalDistance, udRoundBox( (p-vec3(2.0,-0.8,0.0)), vec3(0.4,1.2,0.8) , 0.1 ) );
    return finalDistance;
}
float calculateDistance(in vec3 p, out vec3 color) {
    float height = texture2D(iChannel1, vec2(p.xz+iGlobalTime)*0.05 ).r;
    color = height+vec3(1.0);  
    float finalDistance = 10000000.;
    finalDistance = min(finalDistance, udBox( p-vec3(0.0,-2.0+height*0.1+sin(p.x*0.6+iGlobalTime)*0.1,0.0), vec3(30.,0.7,30.0) ) );
    float dist1 = palkki(p,color);
    if(dist1 < finalDistance){
        finalDistance = dist1;
        color = vec3(5.0,0.0,0.0);
    }
    return finalDistance;
}

float THESHITYO( in vec3 ro, in vec3 rd, float mint, float k )
{
    float res = 1.0;
    float t = mint;
    for( int i=0; i<32; i++ )
    {
        vec3 C;
        float h = calculateDistance(ro + rd*t, C);
		h = max( h, 0.0 );
        res = min( res, k*h/t );
        t += clamp( h, 0.02, 0.8 );
		if( h<0.001 ) break;
    }
    return clamp(res,0.0,1.0);
}

vec3 tracer(vec3 rayStartPosition, vec3 rayDirection) {
    vec3 finalColor = vec3(0., 0., 0.);
    vec3 rayPosition = rayStartPosition;
    float stepable = 0.4;
    float dist = 0.01;
    vec3 normalVector = rayDirection;
    float coff = 0.3;
    float find;
    vec3 lightSource = vec3(120.0,120.0,120.0);
    
    for(float k=0.; k<3.; k++) {
        find = 0.0;
        for(float i=0.; i<50.; i++) {
            vec3 color;
            stepable = calculateDistance(rayPosition, color);
            dist += stepable;
            rayPosition = rayStartPosition + dist * rayDirection;
            
            
            vec3 lightDir = (lightSource-rayPosition);
            lightDir = normalize(lightDir);
            float directLight = dot(normalVector, lightDir);
            finalColor+= texture2D(iChannel0, (rayPosition.xy + rayPosition.y) * 0.8 ).r * texture2D(iChannel1, (rayPosition.xz + rayPosition.z) * 0.8 ).r * 
                			0.2 * max(pow(directLight,7.)*  vec3(1.7,1.1,.9) * 1.8,0.0001);
            
            if(length(rayPosition)>25.) { 
                vec3 lightDir = (lightSource-rayStartPosition);
                lightDir = normalize(lightDir);
                float directLight = dot(normalVector, lightDir);
                finalColor+=max( pow(directLight,7.) * vec3(1.7,1.1,.9) * 1.8, 0.01) + 0.2 * texture2D(iChannel1, 2.0*vec2(abs(atan(rayDirection.x,rayDirection.z)/3.141),rayDirection.y)).rgb * 0.7 * min(max(length(rayDirection.xz) * 3.0,0.0),1.0);;
                return finalColor;
            }
            
            if( abs(stepable) <= 0.005){
                const float epsilon = 0.005;
                vec3 C;
                normalVector = vec3(	calculateDistance(rayPosition+vec3(epsilon,0,0),C)-calculateDistance(rayPosition+vec3(-epsilon,0,0),C),
                                        calculateDistance(rayPosition+vec3(0,epsilon,0),C)-calculateDistance(rayPosition+vec3(0,-epsilon,0),C),
                                        calculateDistance(rayPosition+vec3(0,0,epsilon),C)-calculateDistance(rayPosition+vec3(0,0,-epsilon),C));
                normalVector = normalize(normalVector);
                
                
                float shadow = (THESHITYO(rayPosition + normalVector, normalize(lightSource), 0.01, 0.6) * 1.9);
                finalColor = mix(finalColor, (color * vec3(dot(normalVector, -rayDirection)) / (k*9.0*(shadow+1.0) + 1.0)) * shadow, coff) ;
                finalColor = mix(finalColor, vec3(0.0), dist/24.0);  /*fog*/
                find = 1.0;
                break;
                
            }
        }
        dist = 0.1;
        rayStartPosition = rayPosition + normalVector;
        rayPosition = rayStartPosition;
        rayDirection = reflect(rayDirection, normalVector);
        coff *= 0.3;
    }
    return finalColor;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord ) {
    vec3 cameraPosition = vec3( -0.7, 0.0, -17.0 );
    cameraPosition.yxz *= rotY( -0.2 );
    cameraPosition *= rotY( 0.1 * 2.0 * 3.1415 * iGlobalTime );
    vec2 uv = 2.0 * fragCoord.xy / iResolution.xy - 1.0;
    uv *= vec2(sin(iGlobalTime+uv.x)/10.+1.,1.0);
    float aspect = iResolution.x / iResolution.y;
    if(length(uv*vec2(0.,aspect))>0.9) discard;
    vec3 direction = normalize(vec3(.5 * uv * vec2(aspect, 1.0), 1. )) ;
    direction.yxz *= rotY( -0.2 );
    direction *= rotY( 0.1 * 2.0 * 3.1415 * iGlobalTime );
    fragColor = vec4(tracer(cameraPosition, direction),1.0);
}
