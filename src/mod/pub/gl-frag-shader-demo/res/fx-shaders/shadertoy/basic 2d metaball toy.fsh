// https://www.shadertoy.com/view/Xs2GD3
// 9 smaller metaballs in a grid + 1 bigger metaball on the mouse
float calcMetaValue(vec2 center, float charge, vec2 point) {
	float distance = length(center - point);
	return charge / ( distance * distance );
}

float calcTotalMeta(vec2 point) {
	return ( calcMetaValue ( vec2 ( 0.25 , 0.25 ) , 0.004 , point )
		   + calcMetaValue ( vec2 ( 0.25 , 0.50 ) , 0.004 , point )
		   + calcMetaValue ( vec2 ( 0.25 , 0.75 ) , 0.004 , point )
		   + calcMetaValue ( vec2 ( 0.50 , 0.25 ) , 0.004 , point )
		   + calcMetaValue ( vec2 ( 0.50 , 0.50 ) , 0.004 , point )
		   + calcMetaValue ( vec2 ( 0.50 , 0.75 ) , 0.004 , point )
		   + calcMetaValue ( vec2 ( 0.75 , 0.25 ) , 0.004 , point )
		   + calcMetaValue ( vec2 ( 0.75 , 0.50 ) , 0.004 , point )
		   + calcMetaValue ( vec2 ( 0.75 , 0.75 ) , 0.004 , point )
		   + calcMetaValue ( iMouse.xy / iResolution.xy , 0.016 , point )
		   )
		   ;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float value = calcTotalMeta(uv);
	gl_FragColor = value < 1.0 ? vec4(0.0,0.0,0.0,1.0) : vec4(1.0,1.0,1.0,1.0);
}
