// https://www.shadertoy.com/view/ldsSRX

// iChannel0: t14

const float speed = 0.15;
const float scale = 0.075;
const float res = 256.0;
const vec3 rcpRes = vec3(1.0 / res, -0.5 / res, 0.5 / res);

vec2 magnify(vec2 uv)
{
	return (floor(uv) + min(fract(uv) / min(fwidth(uv), 1.0), 1.0)) * rcpRes.x + rcpRes.y;		
}

void main(void)
{
	vec2 screen = gl_FragCoord.xy / iResolution.xy;
	
	//Generate plane
	
	vec2 uv = screen - vec2(0.5, 1.0);
	
	uv.y = 1.0 / uv.y;
	uv.x *= uv.y;
	
	float rot = iGlobalTime * speed;
	vec2 sincos = vec2(sin(rot), cos(rot));
	uv = vec2(dot(uv, sincos), dot(uv, vec2(-sincos.y, sincos.x)));
	uv *= scale;
	
	//Plane to UVs
	
	vec2 uvMul = uv * res;
	vec2 uvAA = magnify(uvMul);
	vec2 uvPoint = floor(uvMul) * rcpRes.x + rcpRes.z;
	float biasPoint = 1.0 - screen.y;
	
	//Compareison
	
	if (screen.x > 1.0 / 3.0)
		uv = uvAA;
	if (screen.x > 1.0 / 1.5)
		uv = uvPoint;
	else
		biasPoint = 0.0;
	
	//Anyone know how to tex2Dfetch, tex2Dgrad or tex2Dlod (Not bias),
	//so I can show point sampling in justice?
	
	vec3 col = texture2D(iChannel0, uv, -biasPoint).rgb;
	gl_FragColor = vec4(col, 0.0);
}
