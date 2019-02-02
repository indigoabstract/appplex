// https://www.shadertoy.com/view/MdsSRj
const float PI = 3.14159265;

float width = iResolution.x;
float height = iResolution.y;

float scale = width / 1.25;

float time = iGlobalTime * 0.3;

void main(void)
{	
	// calculate the centre of the circular sines
	vec2 center = vec2((width / 2.0) + sin(time) * (width / 1.5),
					   (height / 2.0) + cos(time) * (height / 1.5));
	
	float distance = length(gl_FragCoord.xy - center);
	
	// circular plasmas sines
	float circ = (sin(distance / (scale / 7.6) + sin(time * 1.1) * 5.0) + 1.25)
			   + (sin(distance / (scale / 11.5) - sin(time * 1.1) * 6.0) + 1.25);
	
	// x and y plasma sines
	float xval = (sin(gl_FragCoord.x / (scale / 6.5) + sin(time * 1.1) * 4.5) + 1.25)
			   + (sin(gl_FragCoord.x / (scale / 9.2) - sin(time * 1.1) * 5.5) + 1.25);
	
	float yval = (sin(gl_FragCoord.y / (scale / 6.8) + sin(time * 1.1) * 4.75) + 1.25)
			   + (sin(gl_FragCoord.y / (scale / 12.5) - sin(time * 1.1) * 5.75) + 1.25);

	// add the values together for the pixel
	float tval = circ + xval + yval / 3.0;
	
	// work out the colour
	vec3 color = vec3((cos(PI * tval / 4.0 + time * 3.0) + 1.0) / 2.0,
					  (sin(PI * tval / 3.5 + time * 3.0) + 1.0) / 2.5,
					  (sin(PI * tval / 2.0 + time * 3.0) + 2.0) / 8.0);
	
	// set the fragment colour
	gl_FragColor = vec4(color, 1.0);
}
