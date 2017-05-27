// https://www.shadertoy.com/view/Msf3WN
vec2 fViewportDimensions;

const float eye_inner = 0.162;
const float eye_outer = 0.215;

float calc_eye( vec2 pos, vec2 uv, vec2 mouse ) {
   uv -= pos;
   float dist1 =    length( uv ) - eye_outer;
   float dist2 = -( length( uv ) - eye_inner );
	
   vec2 look = ( mouse - pos ) * 0.15;
   look *= min( 0.11, length( look ) ) / length( look );
	
   float dist3 =    length( uv - look ) - 0.035;
   return min( max( dist1, dist2 ), dist3 );
}

float calc_mouth( vec2 uv ) {
   uv += vec2( 0.015, -0.3325 );
   float w = 0.185;
   return length( ( vec2( clamp( uv.x, -w, w ), clamp( uv.y, -0.01, 0.01 ) ) - uv ) ) - 0.0145;
}

float calc_eyebrow( vec2 pos, vec2 uv ) {
   uv -= pos;
   float w = 0.16;
   vec2 dist = clamp( abs( uv + vec2(0,( eye_outer - eye_inner) / 2.0) ) - vec2( w, (eye_outer - eye_inner) / 2.0 ), 0.0, 1.0 );

   float r = 0.061;
   float r2 = eye_outer - eye_inner;
   
   vec2 arc_center = vec2( w, -r-r2 );
   
   float dist1 =    distance( uv, arc_center ) - r-r2;
   float dist2 = -( distance( uv, arc_center ) - r );
   
   float angle = -3.0;
   vec2 norm = vec2( cos(angle), sin(angle) );
   float  pd = -dot( norm, arc_center + vec2(0,r*sin(angle)/norm.y) );

   return min( max( dist.x, dist.y ), max( max( dist1, dist2 ), dot( uv, norm ) + pd ) );
}

void main(void)
{
   vec2 uv = gl_FragCoord.xy / iResolution.xy;
   vec2 mouse = iMouse.xy / iResolution.xy;

   float aspect = iResolution.x / iResolution.y;

   uv -= 0.5;
   uv.x *= aspect;
   uv.y *= -1.0;
    
   mouse -= 0.5;
   mouse.x *= aspect;
   mouse.y *= -1.0;

   float eye1 = calc_eye( vec2( -0.465, 0 ), uv, mouse );
   float eye2 = calc_eye( vec2( +0.437, 0 ), uv, mouse );
   float mouth = calc_mouth( uv );
   float brow1 = calc_eyebrow( vec2( -0.522, -eye_inner ), uv );
   float brow2 = calc_eyebrow( vec2( 0.3805, -eye_inner ), uv );
   float dist = min( min(min( eye1, eye2 ), min( mouth, brow1 ) ), brow2 );
   
   float sharpness = sqrt(2.0) / 2.0;
   float res = clamp( 1.0 - dist * sharpness * iResolution.y, 0.0, 1.0 );
	
   gl_FragColor = vec4( res, res, res, 1.0 );
}
