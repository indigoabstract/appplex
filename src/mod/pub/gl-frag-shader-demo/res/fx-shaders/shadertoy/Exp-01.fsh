// https://www.shadertoy.com/view/XdfSzn

// iChannel0: t5

float sn(vec3 p) {
    vec2 aa=texture2D(iChannel0,p.xz).xy;
    return smoothstep(aa.x,aa.y,p.y);
}

void main( void ) {
	vec2 pos = gl_FragCoord.xy / iResolution.xy;
	
//	pos.x=sn(vec3(pos,0.1));
	pos.x=sn(vec3(pos,sin(iGlobalTime*0.1)*0.1));
	
	gl_FragColor = vec4( pos, 1.0, 1.0 );
}
