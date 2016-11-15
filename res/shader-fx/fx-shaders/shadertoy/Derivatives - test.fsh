// https://www.shadertoy.com/view/Msl3RH

// iChannel0: t4

// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec3  col = texture2D( iChannel0, vec2(uv.x,1.0-uv.y) ).xyz;
	float lum = dot(col,vec3(0.333));
	vec3 ocol = col;
	
	if( uv.x>0.5 )
	{
		// right side: changes in luminance
        float f = fwidth( lum );
        col *= 1.5*vec3( clamp(1.0-8.0*f,0.0,1.0) );
	}
    else
	{
		// bottom left: emboss
        vec3  nor = normalize( vec3( dFdx(lum), 64.0/iResolution.x, dFdy(lum) ) );
		if( uv.y<0.5 )
		{
			float lig = 0.5 + dot(nor,vec3(0.7,0.2,-0.7));
            col = vec3(lig);
		}
		// top left: bump
        else
		{
            float lig = clamp( 0.5 + 1.5*dot(nor,vec3(0.7,0.2,-0.7)), 0.0, 1.0 );
            col *= vec3(lig);
		}
	}

    col *= smoothstep( 0.003, 0.004, abs(uv.x-0.5) );
	col *= 1.0 - (1.0-smoothstep( 0.007, 0.008, abs(uv.y-0.5) ))*(1.0-smoothstep(0.49,0.5,uv.x));
    col = mix( col, ocol, pow( 0.5 + 0.5*sin(iGlobalTime), 4.0 ) );
	
	gl_FragColor = vec4( col, 1.0 );
}
