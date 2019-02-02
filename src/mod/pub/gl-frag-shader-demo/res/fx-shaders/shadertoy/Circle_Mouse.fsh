// https://www.shadertoy.com/view/4sX3W4
// Simple experimental shader.
// The circle goes where the mouse click is.

bool drawPCBar(vec2 mPos, vec2 fragPos)
{
	return (fragPos.y >= 270.0)
		&& (abs(mPos.x - fragPos.x) <= 50.0);
}

bool drawPlayerBar(vec2 mPos, vec2 fragPos)
{
	return (fragPos.y <= 20.0)
		&& (abs(mPos.x - fragPos.x) <= 50.0);				
}

bool drawCircle(vec2 fragPos)
{
	vec2 dir = vec2(0.0, 1.0);
	vec2 pos = dir * (300.0 * sin(iGlobalTime));
	pos.y += 20.0;
	vec2 vecDiff = gl_FragCoord.xy - pos;
	
	float dis = sqrt ( dot(vecDiff, vecDiff) );
	float dis_T = 20.0;
	
	return dis <= dis_T;
	
}

void main(void)
{
	
	vec2 uv = gl_FragCoord.xy / iResolution.xy;	
	
	if (drawCircle(gl_FragCoord.xy)
	 || drawPlayerBar(iMouse.xy, gl_FragCoord.xy) 
	 || drawPCBar(iMouse.xy, gl_FragCoord.xy) )
		
		gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);
	
	else {
		float col = 0.5 * sin(iGlobalTime);
		gl_FragColor = vec4(0.5, 0.5, 0.5, 1.0);
	}
}
