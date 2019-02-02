// https://www.shadertoy.com/view/MtSGz1

// iChannel0: t3
// iChannel1: t12

/*
Copyright (c) 2015 Sampo Savilampi

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

The Software should be used for Good, not Evil.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

struct polygon{
	vec2 A, B, C;
};
float roundBox(vec2 coord, vec2 pos, vec2 b ){
  return length(max(abs(coord-pos)-b,0.0));
}
float box(vec2 coord, vec2 pos, vec2 size){
	if((coord.x<(pos.x+size.x)) &&
	   (coord.x>(pos.x-size.x)) &&
	   (coord.y<(pos.y+size.y)) && 
	   (coord.y>(pos.y-size.y)) ) 
		return 1.0;
	return 0.0;
}
float sun(vec2 coord, vec2 pos, float size){
	if(length(coord-pos)<size)
		return 1.0;
	return 0.0;
}
float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}
float sign(vec2 p1, vec2 p2, vec2 p3){
  return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

mat2 rotate(float Angle)
{
    mat2 rotation = mat2(
        vec2( cos(Angle),  sin(Angle)),
        vec2(-sin(Angle),  cos(Angle))
    );
	return rotation;
}
float sdCapsule( vec2 p, vec2 a, vec2 b, float r ){
    vec2 pa = p - a, ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return min(floor(length( pa - ba*h ) - r),0.);
}
float triangle( vec2 coord, vec2 pos, float angle, float thick, float size){
    vec2 original_coord = coord;
    coord += pos;
    coord *= rotate(angle);
    float collision = 0.0;
    collision += sdCapsule( coord, vec2( 0.0, 0.333)*size, vec2( 0.3,-0.2)*size, thick );
    collision += sdCapsule( coord, vec2( 0.0, 0.333)*size, vec2(-0.3,-0.2)*size, thick );
    collision += sdCapsule( coord, vec2( 0.3,-0.2)*size, vec2(-0.3,-0.2)*size, thick );
    collision += 3.0*texture2D( iChannel0, original_coord * 0.6 + vec2(iGlobalTime*0.02) ).b * 0.6;
    collision += 2.5*texture2D( iChannel0, original_coord * 0.1 + vec2(iGlobalTime*0.001) ).b * 0.4;
    return -min(max(-collision,0.0),1.0);
}




float smin( float a, float b, float k ){
    float res = exp( -k*a ) + exp( -k*b );
    return -log( res )/k;
}
mat3 rotY(in float a) {
	return mat3( cos(a), 0.0, sin(a),
				 0.0,    1.0, 0.0,
				-sin(a), 0.0, cos(a)
			    );
}
float sdSphere( vec3 p, float s ){
  return length(p)-s;
}
float sdBox( vec3 p, vec3 b ){
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) +
         length(max(d,0.0));
}
void objectGroup0(in vec3 p, out vec3 color, out float reflectiveValue, out float finalDistance){
    
        reflectiveValue = 0.9;
        p *= rotY(-iGlobalTime/4.);
    if(length(p)<1.) return;
    vec3 tex = texture2D(iChannel0,vec2(atan(p.x,p.z)*3.1415*0.5,p.y*1.4)*0.03+vec2(0.14)).rgb;
    vec3 tex2 = texture2D(iChannel1,vec2(atan(p.x,p.z)*3.1415*0.5,p.y)*0.3+vec2(0.14)).rgb;

    
    color = vec3( 0.1, 0.3, 0.8);  
    
    finalDistance= min(finalDistance,sdSphere( p-vec3(0.0), 1.83+tex.r*0.03));
    float dist2 = sdSphere( p, 1.75+tex.r*0.3+tex2.b*0.03);
    
    if (finalDistance > dist2) {
        finalDistance = dist2;
        color = vec3( 0.0, 0.5, 0.1) - tex + abs(sin(p.y))/2.;
        reflectiveValue = 0.6;
    }
}


void objectGroup1(in vec3 p, out vec3 color, out float reflectiveValue, out float finalDistance){
    float dist=1000.0;
    for(float i=0.0; i<2.0; i++){
    vec3 p2=p*rotY(0.5*iGlobalTime*2.0+i);
    p2.xzy*=rotY(0.5*iGlobalTime*(1.0+i*0.1)+i);
    dist = min(dist,sdBox( rotY(iGlobalTime+i)*(p2-vec3(3.0+i*0.3,3.*sin(0.6*iGlobalTime+i),0.0)), vec3(0.5+i*0.1+0.35*cos(iGlobalTime+i)) ) );
    }
    if(finalDistance > dist){
        finalDistance = dist;
        color = vec3(0.5,0.2,0.2);
        reflectiveValue = 0.6;
    }
}
float calculateDistance(in vec3 p, out vec3 color, out float reflectiveValue) {
    float finalDistance = 10000000.;
    
    objectGroup0(p,color,reflectiveValue,finalDistance);
    objectGroup1(p,color,reflectiveValue,finalDistance);
    return finalDistance;
}

float traceToLight(vec3 rayPosition, vec3 normalVector, vec3 lightSource){
    const float epsilon = 0.01;
    vec3 rayStartPosition = rayPosition + normalVector*0.001;
    vec3 finalColor = vec3(0., 0., 0.);
	vec3 lightNormal = normalize(lightSource - rayPosition);
    float dist = 0.0;
    float lightAmount = 1.0;
        for(float i=0.; i<32.; i++) {
            rayPosition = rayStartPosition + dist * lightNormal;
            vec3 color;
            float dummy;
            float stepable = calculateDistance(rayPosition, color,dummy);
            dist += stepable;
            lightAmount = min(900.0*abs(stepable),lightAmount);
            if( length(rayPosition.xy)>8.)
                break;
        }
    return clamp(lightAmount,0.5,9.0);
}
vec3 tracer(vec3 rayStartPosition, vec3 rayDirection) {
    const float epsilon = 0.02;
    
    vec3 rayPosition = rayStartPosition;

    vec3 normalVector;
    float dist = 0.0;
    vec3 returnColor = vec3(0.0);
    vec3 finalColor = vec3(0.0);
    vec3 lightSource = vec3(iGlobalTime/16.,0.0,-447.0+iGlobalTime);
    float reflectiveValue = 1.0;
    float reflectionNow = 1.0;
    float finalLight = 1.0;
    
    float possibleFogLight = 1.0;
    
    for(float k=0.; k<2.; k++) {
        for(float i=0.; i<66.; i++) {
            vec3 color;
            float stepable = calculateDistance(rayPosition, color, reflectiveValue);
            dist += stepable;
            rayPosition = rayStartPosition + dist * rayDirection;
            
            if( length(rayPosition.xy)>8.)
                break;
            if( abs(stepable) <= epsilon){
                vec3 C;
                float dummy = 0.0;
                normalVector = vec3(	calculateDistance(rayPosition+vec3(epsilon,0,0),C,dummy)-calculateDistance(rayPosition+vec3(-epsilon,0,0),C,dummy),
                                        calculateDistance(rayPosition+vec3(0,epsilon,0),C,dummy)-calculateDistance(rayPosition+vec3(0,-epsilon,0),C,dummy),
                                        calculateDistance(rayPosition+vec3(0,0,epsilon),C,dummy)-calculateDistance(rayPosition+vec3(0,0,-epsilon),C,dummy));
                normalVector = normalize(normalVector);
                float light = traceToLight(rayPosition, normalVector, lightSource);
                finalLight = min(finalLight, light);
                float lightDistance = distance(rayStartPosition,lightSource);
                
                finalColor = color * vec3(dot(normalVector, -rayDirection));
                
                
                vec3 lightDir = (lightSource-rayPosition);
                lightDir = normalize(lightDir);
                float directLight = dot(normalVector, lightDir);
                reflectionNow = min(reflectionNow,reflectiveValue);
                
                finalColor+=max(0.85*directLight*vec3(1.0,0.9,0.8),0.001)*reflectionNow;
                
                returnColor += ( vec3(finalLight) / (k*0.8/reflectionNow + 1.0)) * finalColor;
               
                break;
                
            }
        }
        dist = 0.1;
        rayStartPosition = rayPosition + normalVector;
        rayPosition = rayStartPosition;
        rayDirection = reflect(rayDirection, normalVector);
    } 
    return returnColor;
}

vec3 piip() {
    vec3 cameraPosition = vec3( 0., 0., -9.);
    vec2 uv = 2.0 * gl_FragCoord.xy / iResolution.xy - 1.0;
    uv/=min(pow(iGlobalTime/4.,2.),1.)*(1.+sin(iGlobalTime*14.)/(1.+mod(iGlobalTime,2.)*16.));
     float aspect = iResolution.x / iResolution.y;
    
    if(length(uv.y)>0.9){
        return vec3(0.0);
    }
	// else if(length(uv.x)<0.9){
        // vec3 direction = normalize(vec3(.5 * uv * vec2(aspect, 1.0), 1. ));
        // direction.xzy *= rotY(iGlobalTime*0.001);
        // cameraPosition *= rotY(-iGlobalTime/75.);
        // direction *= rotY(-iGlobalTime/75.);
        // return tracer(cameraPosition, direction);
    // }
                      return vec3(0.0);
}









void mainImage( out vec4 fragColor, in vec2 fragCoord ){
	vec4 tulos;
	vec4 lopullinentulos=vec4(1.0);
	vec2 uv = fragCoord.xy / iResolution.xy;
    
	float aspectCorrection = (iResolution.x/iResolution.y);
	vec2 coordinate_entered = 2.0 * uv - 1.0;
    if(abs(vec2(aspectCorrection,1.0) *coordinate_entered).y>0.7){
        fragColor =  vec4(0.0,0.0,0.0,1.);
    }else {
	for(float rgbare=0.0; rgbare<2.0; rgbare++){
	vec2 coord = vec2(aspectCorrection,1.0) *coordinate_entered;
	coord.x*=1.0+rgbare*0.009;
	coord*=1.0+0.1*sin(1.01*0.1);
        
	tulos=vec4(vec3(111.0/255.0, 30.0/255.0, 65.0/255.0),1.0);
        
	if(mod(coord.x+coord.y+1.01*0.1+iGlobalTime*0.1,0.2)<0.1){
		if(box(coord,vec2(0.),vec2(4.,0.4))==1.)
		   	tulos.xyz-=vec3(0.4,0.5,0.3);
	}
	tulos.rgb+=sun(coord,vec2(0.),1.);
	tulos.rgb-=sun(coord,vec2(0.),.7);
        
    float collision = 0.0;
    for( float i = 0.0; i < 12.0; i+=3.){
        vec2 triangle_position = mod(iGlobalTime+i,12.) * vec2( -0.5 + rand(vec2(i)) , -0.5 + rand( vec2(i*i) ) );
        collision += triangle( coord, triangle_position, iGlobalTime+i, mod(iGlobalTime+i,12.)*0.01, mod(iGlobalTime+i,12.)*0.3);
    }
    
    if(collision<0.0)
        tulos.xyz += vec3(0.1);
	
	tulos.xyz=tulos.xyz-vec3(min(max(-0.44+length(coord)*0.41,0.0),1.0))+vec3(0.06*rand(vec2(coord.x+coord.y,1.01*coord.y*coord.x)));
	
	if(rgbare==0.0)
		lopullinentulos.r=tulos.r;
	if(rgbare==1.0)
		lopullinentulos.gb=tulos.gb;
	}
	lopullinentulos.xyz=lopullinentulos.xyz*(1.2-0.4*mod(fragCoord.y,2.0));
    lopullinentulos.rgb=piip()+lopullinentulos.rgb/(1.+piip()*1e6);
	fragColor = lopullinentulos;
        }
}
