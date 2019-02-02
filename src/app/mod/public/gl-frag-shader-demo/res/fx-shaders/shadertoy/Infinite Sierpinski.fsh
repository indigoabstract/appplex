// https://www.shadertoy.com/view/MdfGR8
void main(void)
{
    float direction = 0.5; // -1.0 to zoom out
    ivec2 sectors;
    vec2 coordOrig = gl_FragCoord.xy / iResolution.xy - 0.5;
    coordOrig.y *= iResolution.y / iResolution.x;
    const int lim = 6;

    vec2 coordIter = coordOrig / exp(mod(direction*iGlobalTime, 1.1));
	
    for (int i=0; i < lim; i++) {
        sectors = ivec2(floor(coordIter.xy * 3.0));
        if (sectors.x == 1 && sectors.y == 1) {
            // make a hole
            gl_FragColor = vec4(0.0);

            return;
        } else {
            // map current sector to whole carpet
            coordIter.xy = coordIter.xy * 3.0 - vec2(sectors.xy);
        }
    }

    gl_FragColor = vec4(coordOrig.x + 0.5, 0.5, coordOrig.y + 0.5, 1.0) + 0.05*(1.1 - mod(direction*iGlobalTime, 1.1));
}
