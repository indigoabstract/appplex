// https://www.shadertoy.com/view/MdfSD2
void value( float v00, float v01, float v10, float v11, vec2 u, out float t, out vec2 grad ) {  
    float a = v01 - v00;
    float b = v10 - v00;
    float c = v11 + v00 - v01 - v10;
    
    t = v00 + a * u.x + b * u.y + c * u.x * u.y;
    
    grad.x = a + c * u.y;
    grad.y = b + c * u.x;
}

void main(void)
{
	vec2 u = gl_FragCoord.xy / iResolution.xy;
    
    float v00 = 0.0;
    float v01 = 1.0;
    
    float v10 =  sin( iGlobalTime * 0.1 ) * 0.5 + 0.5;
    float v11 = -cos( iGlobalTime * 0.1 ) * 0.5 + 0.5;
    
    float t;
    vec2 grad;
    value( v00, v01, v10, v11, u, t, grad );
 
    // improved by @mmalex
    float d = smoothstep( 0.0, 0.004, abs( mod( t + 0.01, 0.02 ) - 0.01 ) / length( grad ) );
      
	gl_FragColor = ( 1.0 - d ) *mix(
       vec4( 0.0, 0.0, 1.0, 0.0 ),
       vec4( 1.0, 0.0, 0.0, 0.0 ),
       t
    );
}
