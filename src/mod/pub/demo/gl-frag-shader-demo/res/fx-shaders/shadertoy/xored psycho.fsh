// https://www.shadertoy.com/view/lslSDn
float sinZeroOne(float what) {
  return (1.0 + sin(what)) / 2.0;
}

float sinZeroOne() {
  return (1.0 + sin(iGlobalTime*4.0)) / 2.0;
}


vec4 generateXORTexture(vec2 coords, float size, float dash, float tolerance) {
	vec4 color = vec4(0.0);
	
	if (abs(mod(coords.x, size)) <= tolerance || abs(mod(coords.y, size)) <= tolerance) {
		if (mod(coords.x, dash*2.0) <= dash ^^ mod(coords.y, dash*2.0) <= dash) {
		    color = vec4(0.0, 0.0, 1.0, 1.0);	
		} else {
			color = vec4(1.0, 1.0, 1.0, 1.0);	
		}
	} else {
		if (mod(coords.x, size*2.0) <= size ^^ mod(coords.y, size*2.0) <= size) {
			color = vec4(sinZeroOne(), sinZeroOne(coords.x), sinZeroOne(coords.y), 1.0);	
		} else {
		    color = vec4(sinZeroOne(coords.y), sinZeroOne(), sinZeroOne(coords.x), 1.0);		
		}	  
	}
	return color;
}

void main(void)
{
	float size = 30.0;
	float dash = 1.0;
	float tolerance = 1.0;
	vec2 center = vec2(iResolution.x / 2.0, iResolution.y / 2.0);
	vec2 coords = gl_FragCoord.xy - center;	
	
	float fi = iGlobalTime*1.0;
	
    coords *= mat2(
	  cos(fi), -sin(fi),
	  sin(fi), cos(fi)
	);
	
	coords /= vec2(1.0 + sinZeroOne()*2.0);
	
	float blur = (1.0 + sin(iGlobalTime*sinZeroOne()*0.02)) / 2.0;
	blur *= 0.1 * distance(gl_FragCoord.xy, center);
	
	vec2 K = vec2(
		distance(gl_FragCoord.xy, center),
		distance(gl_FragCoord.xy, center)
	);
		
	K *= vec2(0.05, 0.05);
	size -= length(K);
				   
	vec4 colorUP    = generateXORTexture( coords+vec2( 0.0,  -blur), size, dash, tolerance);
	vec4 colorDOWN  = generateXORTexture( coords+vec2( 0.0,   blur), size, dash, tolerance);
	vec4 colorLEFT  = generateXORTexture( coords+vec2(-blur,  0.0), size, dash, tolerance);
	vec4 colorRIGHT = generateXORTexture( coords+vec2( blur,  0.0), size, dash, tolerance);

	vec4 colorDUL = generateXORTexture( coords+vec2(-blur, -blur), size, dash, tolerance);
	vec4 colorDUR = generateXORTexture( coords+vec2( blur,  blur), size, dash, tolerance);
	vec4 colorDBL = generateXORTexture( coords+vec2(-blur,  blur), size, dash, tolerance);
	vec4 colorDBR = generateXORTexture( coords+vec2( blur,  blur), size, dash, tolerance);

	vec4 color = colorUP+colorDOWN+colorLEFT+colorRIGHT +  colorDUL + colorDUR + colorDBL + colorDBR;
	color = color / vec4(8.0);
		
	color -= mix(
		vec4(0.0),
		vec4(1.0),
		smoothstep(0.0, min(iResolution.x, iResolution.y), distance( gl_FragCoord.xy, center)));
		

	gl_FragColor = color;
}
