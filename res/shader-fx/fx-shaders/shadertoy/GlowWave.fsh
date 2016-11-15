// https://www.shadertoy.com/view/Md2XzD
precision highp float;

const float HALF_PI = 1.57079632679;
const float PI = 	  3.14159265359;
const float PI2 = 	  6.28318530718;

const float NUM_OF_LINES = 5.0;
const float SINGLE_LINE = 1.0 / NUM_OF_LINES;

void main(void)
{
    float c1 = cos(iGlobalTime + HALF_PI) * 0.5 + 0.5;
    float c2 = cos(iGlobalTime) * 0.5 + 0.5;
    float c3 = cos(iGlobalTime + PI) * 0.5 + 0.5;

	vec3 colorOne = vec3(c1, c2, c3);
    vec3 colorTwo = vec3(c2, c3, c1);
    
	vec2 uv = gl_FragCoord.xy / iResolution.xy;

    vec3 col = vec3(0.0);
    float idx01 = 0.0;
    float y = 0.0;

    for(float i = 0.0; i < NUM_OF_LINES; i++)
    {
        y = (uv.x + iGlobalTime * 0.25) + idx01 * 0.25;
        y = cos(y * PI2);
        y = cos(y + iGlobalTime) * (1.0 - idx01) * 0.3 + 0.5;
        y = inversesqrt(abs(y - uv.y)) * 0.025;
    	col += mix(colorOne, colorTwo, idx01) * clamp(y, 0.0, 1.0);

        idx01 += SINGLE_LINE;
    }
    col *= col;

	gl_FragColor = vec4(col,1.0);
}
