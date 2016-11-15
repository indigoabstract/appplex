// https://www.shadertoy.com/view/ls23Dc

// iChannel0: t4

// Samples from the nearest hexagon to the pixel.
float PI = 3.14159265359;
float TAU = 2.0*PI;
float deg30 = TAU/12.0;

// http://www.gamedev.net/page/resources/_/technical/game-programming/coordinates-in-hexagon-based-tile-maps-r1800
// nearest hexagon sampling, not quite sure if it's correct

float hexDist(vec2 a, vec2 b){
	vec2 p = abs(b-a);
	float s = sin(deg30);
	float c = cos(deg30);
	
	float diagDist = s*p.x + c*p.y;
	return max(diagDist, p.x)/c;
}

vec2 nearestHex(float s, vec2 st){
	float h = sin(deg30)*s;
	float r = cos(deg30)*s;
	float b = s + 2.0*h;
	float a = 2.0*r;
	float m = h/r;

	vec2 sect = st/vec2(2.0*r, h+s);
	vec2 sectPxl = mod(st, vec2(2.0*r, h+s));
	
	float aSection = mod(floor(sect.y), 2.0);
	
	vec2 coord = floor(sect);
	if(aSection > 0.0){
		if(sectPxl.y < (h-sectPxl.x*m)){
			coord -= 1.0;
		}
		else if(sectPxl.y < (-h + sectPxl.x*m)){
			coord.y -= 1.0;
		}

	}
	else{
		if(sectPxl.x > r){
			if(sectPxl.y < (2.0*h - sectPxl.x * m)){
				coord.y -= 1.0;
			}
		}
		else{
			if(sectPxl.y < (sectPxl.x*m)){
				coord.y -= 1.0;
			}
			else{
				coord.x -= 1.0;
			}
		}
	}
	
	float xoff = mod(coord.y, 2.0)*r;
	return vec2(coord.x*2.0*r-xoff, coord.y*(h+s))+vec2(r*2.0, s);
}

void main(void){
	vec2 uv = gl_FragCoord.xy/iResolution.xy;
	float s = iResolution.x/80.0;
	vec2 nearest = nearestHex(s, gl_FragCoord.xy);
	vec4 texel = texture2D(iChannel0, nearest/iResolution.xy, -100.0);
	float dist = hexDist(gl_FragCoord.xy, nearest);
	
	float luminance = (texel.r + texel.g + texel.b)/3.0;
	//float interiorSize = luminance*s;
	float interiorSize = s;
	float interior = 1.0 - smoothstep(interiorSize-1.0, interiorSize, dist);
	//gl_FragColor = vec4(dist);
	gl_FragColor = vec4(texel.rgb*interior, 1.0);
	//gl_FragColor = vec4(nearest, 0.0, 1.0);
}
