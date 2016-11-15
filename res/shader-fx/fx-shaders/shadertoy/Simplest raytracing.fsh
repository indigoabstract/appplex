// https://www.shadertoy.com/view/lsX3R4

// iChannel0: t5

//Light setup
vec3 light = vec3(cos(iGlobalTime) * 5.0, 5.0 + cos(iGlobalTime) * 2.0, 5.0);//normalize( vec3(0.57703));
//vec3 light = vec3(1.0 + iMouse.x, iMouse.y, -4.0);
float light_range = 10.0;

float shadow_factor = 1.0;

//Object setup
vec4 sph1 = vec4( 0.0, 1.0, 0.0, 1.0);

float iSphere(in vec3 ro, in vec3 rd, in vec4 sph)
{
	//sphere at origin has equation |xyz| = r
	//sp |xyz|^2 = r^2.
	//Since |xyz| = ro + t*rd (where t is the parameter to move along the ray),
	//we have ro^2 + 2*ro*rd*t + t^2 - r2. This is a quadratic equation, so:
	vec3 oc = ro - sph.xyz; //distance ray origin - sphere center
	
	float b = 2.0 * dot(oc, rd);
	float c = dot(oc, oc) - sph.w * sph.w; //sph.w is radius
	float h = b*b - 4.0 * c; // delta
	if(h < 0.0) 
		return -1.0;
	float t = (-b - sqrt(h)) / 2.0; //Again a = 1.

	return t;
}

//Get sphere normal.
vec3 nSphere(in vec3 pos, in vec4 sph )
{
	return (pos - sph.xyz)/sph.w;
}

//Intersection with plane.
float iPlane(in vec3 ro, in vec3 rd)
{
	
	vec2 uv = (gl_FragCoord.xy / iResolution.xy);
	
	//Plane equation, y = 0 = ro.y + t * rd.y;
	float t = (-ro.y / rd.y);
	return t;
}

//Get plane normal.
vec3 nPlane(in vec3 pos)
{
	return vec3(0.0, 1.0, 0.0);
}


void main(void)
{
	//pixel coordinates from 0 to 1
	vec2 uv = (gl_FragCoord.xy / iResolution.xy);
	
	// Move the sphere ?
	//sph1.x = 0.5 * cos(iGlobalTime * 10.0) * texture2D(iChannel1, mod(vec2(iChannelTime[1]), 1.0)).x;
	//sph1.z = 0.5 * sin(iGlobalTime)* texture2D(iChannel1, mod(vec2(iChannelTime[1]), 1.0)).y;
	
	//generate a ray with origin ro and direction rd
	vec3 ro = vec3(0.0, 0.5, 3.0);
	vec3 rd = normalize(vec3( (-1.0+2.0*uv) * vec2(1.78, 1.0), -1.0));
	
	//intersect the ray with scene

	
	// bg color
	vec3 col = vec3(0.7);
	vec3 shphere_color = vec3(0.7, 0.1, 0.1);
	vec3 light_color = vec3(0.1, 0.8, 0.1);
	
	float intPoint = -1.0;
	intPoint = iSphere(ro, rd, sph1);
	
	if (intPoint > 0.0)
	{
		// hit the sphere
		vec3 pos = ro + intPoint * rd;
		vec3 nor = nSphere(pos, sph1);
		float dif = max(0.0, dot(nor, normalize(light - pos))); //diffuse.
		
		float attenuation = 1.0 - smoothstep(0.0, light_range, length(light - pos));
		dif *= attenuation;
		
		float ao = 0.5 + 0.5 * nor.y;
		//ao = 0.5 * (dot(nor, nPlane(pos)) + 1.0); complete formula. The one above is simplified.
		col = light_color * dif * ao + shphere_color * ao;
	}
	else
	{
		// check if we hit the ground
		intPoint = iPlane(ro, rd);
		
		if (intPoint > 0.0)
		{
		
			// hit the ground
			//t += clamp(-1.0, 0.0, 0.2 + 0.8 * cos(iChannelTime[1])) + uv.x + uv.y * 2.0 + cos(iGlobalTime);
			vec3 pos = ro + intPoint * rd;
			vec3 nor = nPlane( pos );
			//float dif = max(0.0, dot(nor, light));
			
			float amb = smoothstep(0.0, 2.0 * sph1.w, length(pos.xz - sph1.xz)); 
			
			//float amb = min(smoothstep( 0.0, 2.0 * sph1.w, length(pos.xz - sph1.xz) ),
			//				smoothstep( 0.0, 2.0 * sph2.w, length(pos.xz - sph2.xz) ));
			col = vec3(amb * 0.7  * texture2D(iChannel0, mod(vec3(ro + rd * intPoint).xz, 0.0) + vec2(cos(iGlobalTime), iGlobalTime)).xyz);
		}
	}
	
	col *= shadow_factor;
	
	col = sqrt(col);
	
	gl_FragColor = vec4(col,1.0);
	
	//vec2 uv = gl_FragCoord.xy / iResolution.xy;
	//gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);
}
