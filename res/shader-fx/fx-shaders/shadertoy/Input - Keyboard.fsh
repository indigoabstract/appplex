// https://www.shadertoy.com/view/lsXGzf

// iChannel0: kb

const float KEY_LEFT  = 37.5/256.0;
const float KEY_UP    = 38.5/256.0;
const float KEY_RIGHT = 39.5/256.0;
const float KEY_DOWN  = 40.5/256.0;

void main(void)
{
    vec2 uv = gl_FragCoord.xy / iResolution.xy;

    float f = texture2D( iChannel0, vec2(uv.x,0.0) ).x;
    vec3 col = vec3(f);

    uv = -1.0 + 2.0*uv;	
	uv.x *= iResolution.x/iResolution.y;

    // state
    col = mix( col, vec3(1.0,0.0,0.0), 
        (1.0-smoothstep(0.3,0.31,length(uv-vec2(-0.75,0.0))))*
        (0.3+0.7*texture2D( iChannel0, vec2(KEY_LEFT,0.25) ).x) );

    col = mix( col, vec3(1.0,1.0,0.0), 
        (1.0-smoothstep(0.3,0.31,length(uv-vec2(0.0,0.5))))*
        (0.3+0.7*texture2D( iChannel0, vec2(KEY_UP,0.25) ).x));
	
    col = mix( col, vec3(0.0,1.0,0.0),
        (1.0-smoothstep(0.3,0.31,length(uv-vec2(0.75,0.0))))*
        (0.3+0.7*texture2D( iChannel0, vec2(KEY_RIGHT,0.25) ).x));

    col = mix( col, vec3(0.0,0.0,1.0),
        (1.0-smoothstep(0.3,0.31,length(uv-vec2(0.0,-0.5))))*
        (0.3+0.7*texture2D( iChannel0, vec2(KEY_DOWN,0.25) ).x));

    // toggle	
    col = mix( col, vec3(1.0,0.0,0.0), 
        (1.0-smoothstep(0.0,0.01,abs(length(uv-vec2(-0.75,0.0))-0.3)))*
        texture2D( iChannel0, vec2(KEY_LEFT,0.75) ).x);
	
    col = mix( col, vec3(1.0,1.0,0.0),
        (1.0-smoothstep(0.0,0.01,abs(length(uv-vec2(0.0,0.5))-0.3)))*
        texture2D( iChannel0, vec2(KEY_UP,0.75) ).x);

    col = mix( col, vec3(0.0,1.0,0.0),
        (1.0-smoothstep(0.0,0.01,abs(length(uv-vec2(0.75,0.0))-0.3)))*
        texture2D( iChannel0, vec2(KEY_RIGHT,0.75) ).x);
	
    col = mix( col, vec3(0.0,0.0,1.0),
        (1.0-smoothstep(0.0,0.01,abs(length(uv-vec2(0.0,-0.5))-0.3)))*
        texture2D( iChannel0, vec2(KEY_DOWN,0.75) ).x);

    gl_FragColor = vec4(col,1.0);
}
