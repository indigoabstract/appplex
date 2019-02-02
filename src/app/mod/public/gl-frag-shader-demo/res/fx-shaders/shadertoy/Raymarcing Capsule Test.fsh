// https://www.shadertoy.com/view/4sfSWS
vec3 color1 = vec3(1.0,1.0,1.0);
vec3 color2 = vec3(0.8,0.25,0.25);
float shadow = 0.75;
float reflection = 0.5;

vec2 resolution = vec2(iResolution.x,iResolution.y);
float time = iGlobalTime;

float movespeed = 10.0;

vec3 trans(vec3 p)
{
	return mod(p, 4.0)-2.0;
}
 
float distanceFunction(vec3 pos)
{
	vec3 p = trans(pos);
	vec3 a = vec3(-0.5,0.0,0.0);
	vec3 b = vec3(0.5,0.0,0.0);
	float r = 0.5;
	vec3 pa = p - a, ba = b - a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return length( pa - ba*h ) - r;
}
 
vec3 getNormal(vec3 p)
{
	const float d = 0.0001;
	return
	normalize
	(
		vec3
		(
			distanceFunction(p+vec3(d,0.0,0.0))-distanceFunction(p+vec3(-d,0.0,0.0)),
			distanceFunction(p+vec3(0.0,d,0.0))-distanceFunction(p+vec3(0.0,-d,0.0)),
			distanceFunction(p+vec3(0.0,0.0,d))-distanceFunction(p+vec3(0.0,0.0,-d))
		)
	);
}
 
void main() {
	vec2 pos = (gl_FragCoord.xy*2.0 -resolution) / resolution.y;
 
	vec2 mousepos = (iMouse.xy*2.0 - resolution) / resolution.y;
	
	vec3 camPos = vec3(0.0, 0.0, -mod(time*movespeed,4.0));
	vec3 camDir = normalize(vec3(mousepos.xy, -1.0));
	vec3 camUp = vec3(0.0, 1.0, 0.0);
	vec3 camSide = cross(camDir, camUp);
	float focus = 1.8;
 
	vec3 rayDir = normalize(camSide*pos.x + camUp*pos.y + camDir*focus);
 
	float t = 0.0, d;
	vec3 posOnRay = camPos;
 
	for(int i=0; i<80; ++i)
	{
		d = distanceFunction(posOnRay);
		t += d;
		posOnRay = camPos + t*rayDir;
		if(abs(d)<0.0005) break;
	}
	
	
	vec3 normal = getNormal(posOnRay);
	
	vec3 color;
		
	if(mod(posOnRay.x,4.0)>2.0){
		color = color1;	
	}else{
		color = color2;	
	}
	
	color *= (max(normal.y,0.0)*shadow)+(1.0-shadow);
	
	color = vec3(1.0)-(vec3(1.0)-color)*(vec3(1.0)-vec3(1.0-dot(-rayDir,normal))*reflection);
	
	float dist = length(posOnRay-camPos);

	color = mix(vec3(1.0),color,1.0/(dist*0.25+1.0));
  
	if(abs(d) < 0.001)
	{
		gl_FragColor = vec4(color, 1.0);
	}else{
		gl_FragColor = vec4(1.0);
	}
}
