// https://www.shadertoy.com/view/Mdf3Wf
// Test of a HexGrid distance function. Implementation taken from (and fixed and resubmitted to) codeheart.js.
struct Grid {
	float xLength;
};

struct HexView {
	vec2 offset;
	vec2 scale;
	float hexRadius;
	float hexHeight;
	Grid  grid;
};
	
float hash(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec2 screenToHex(HexView view, vec2 C) {
    // Transform from pixel space so that the center of hex
    // (0, 0) is at the origin.
    float x = (C.x - view.offset.x) / view.scale.x - view.hexRadius;
    float y = (C.y - view.offset.y) / view.scale.y - view.grid.xLength * view.hexHeight / 2.0;

    // Scale so that each hex is 4 units wide and 2 units high, and then translate
    // to  put vertex 'o' at the origin.
    //
    // There are now three kinds of columns, shown below in (u,v)
    // space with grid squares denoted by dotted lines:
    //
    //                     i=2
    //               i=1  __ __
    //         i=0  __ __/     \                          .
    //   j=0 o__:__/     \__ __/                          .
    //       /: :  \__ __/.....\.......                   .
    //   j=1 \:_:__/     \__ __/.......                   .
    //       /: :  \__ __/.....\.......                   .
    //   j=2 \:_:__/     \__ __/.......                   .
    //       /: :  \__ __/.....\.......                   .
    //   j=3:\:_:__/     \__ __/.......
    //      :/: :  \__ __/:
    //      :\:_:__/: : : :
    //      : : : : : : : :
    //      : : : : : : : :
    //  ... :1:2:0:1:2:0:1:  ...    
    //
    // Columns of type 2 and type 0 are within 2x2 squares
    // that fill the center of a single hex.  Columns of type
    // 1 zig-zag between three hexes.  
    //
    // We divide the (u,v) space into 3x2 rectangles.  In the 
    // left-most 2/3 of a rectangle we simply compute (i,j)
    // directly from (u,v) by dividing by the size of the rectangle
    // and shifting j down based on i.  In the last column
    // we compute the fractional amount of the column traversed
    // in u and the fractional amount of the rectangle traversed
    // in v to see if we crossed either diagonal.

    float r = view.hexRadius;

    float u = 2.0 * x / r + 1.0;
    float v = 2.0 * y / (r * sqrt(3.0)) + 1.0;

    float i = floor(u / 3.0);
    float j = floor((v + i) / 2.0);

    // Horizontal distance of the point to the right of the third
    // column, in (u,v) space (i.e., the "fraction" of the way across
    // the column)
    float fu = u - i * 3.0 - 2.0;
    float fv = v - j * 2.0 + i;

    if (fu > 0.0) {
		// In the diagonal zig-zag column
		if (fv < 1.0) {
			// In a top-diagonal square
			if (fu > fv) {
			// "Above" the diagonal: move one hex along the i axis to the right
			i += 1.0;
			}
		} else {
			// In a bottom-diagonal square
	
			if (fu + (fv - 1.0) >= 1.0) {
			// "Below" the diagonal: move one hex along both i to the right and j down
			i += 1.0; j += 1.0;
			}
		}
    }

    return vec2(i, j);
}

float hexGridDistance(vec2 A, vec2 B) {
	vec2 C  = A - B;
	vec2 uC = abs(C);
	return sign(C.x) == sign(C.y) ? max(uC.x, uC.y) : uC.x + uC.y;
}

#define NUM_HORIZONTAL_HEXAGONS 100.0

void main(void)
{
	vec2 uv = gl_FragCoord.xy*2.0*NUM_HORIZONTAL_HEXAGONS / iResolution.x;
	Grid g;
	g.xLength = 1.0;

	HexView view;
	view.hexHeight = sqrt(3.0)/2.0;
	view.offset = vec2(0.0);
	view.scale = vec2(1.0);
	view.hexRadius = 1.0;
	view.grid = g;
	
	vec2 hexID = screenToHex(view, uv);
	float x = (sin(iGlobalTime*0.4)*.5 + sin(iGlobalTime*0.73)*.2 + cos(iGlobalTime*1.23)*.3)
		*60.0+60.0;
	float y = 0.5*x+
		(sin(iGlobalTime*0.7)*.5+sin(iGlobalTime*0.82)*.2+sin(iGlobalTime*0.2)*.3)
		*35.0+35.0;
	vec2 markerHexID = vec2(x, y);
	float distanceFromMarker = hexGridDistance(hexID, markerHexID);
	
	gl_FragColor = vec4(vec3(distanceFromMarker*0.04),1.0);
}
