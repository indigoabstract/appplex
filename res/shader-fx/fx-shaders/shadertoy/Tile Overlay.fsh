// https://www.shadertoy.com/view/ldfXz7
/*--------------------------------------------------------------
 * Shader Tiles: Tesselation of the coordinate space into 
 * regular polygons. 
 *
 * License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
 *
 * https://www.shadertoy.com/user/TheVaporTrail
 * Contact: david (at) TheVaporTrail (dot) com
 * Projects: http://www.TheVaporTrail.com
 *
 * Supported tesselations: Square, Triangle, Hexagon, Octagon
 *
 * These routines work with the idea of a "tile", which describes
 * a regular polygon. A tile size is given as the radius of a 
 * circle that encloses the polygon. The tile definition has the
 * radius, center, and number of sides. It also has a rotation,
 * direction, inner radius, "horizontal angle". (The horizontal
 * angle is the angle between the first vertex and horizontal.)
 *
 * Different tesselation "styles" are supported. A style describes
 * the orientations and directions of tiles:
 *   STYLE_MATCHING: All tiles have the same direction and same 
 *     starting angle (except alternating triangles are rotated 180)
 *   STYLE_SEAMLESS: Triangles and squares are arranged so that all
 *     vertices 0 are together, vertices 1 are together, etc. The 
 *     direction will therefore alternate in adjacent tiles.
 *
 * There are two sets of routines: locate routines and calc routines.
 * Locate routines find a tile given a coordinate and a tile size.
 * Calc routines calculate some piece of information given a tile
 * and a coordinate or other argument.
 *
 * Calc routines:
 *   Vertex 
 *   Angle (of point in tile relative to horizontal)
 *   Edge vector
 *   Relative Position
 *   Relative Position UV (0.0<=x<=1.0, 0.0<=y<=1.0)
 *   Contains Point
 *   Distance to Edge
 *   Distance to Closest Edge
 *   Distance to Vertex
 *   Distance to Closest Vertex
 *
 *
 *
 *--------------------------------------------------------------*/

#define DegreeToRad (3.14159/180.0)
#define Tau (3.14159 * 2.0)
#define cos30 (0.8660254) /* sqrt(3)/2 */

#define TILE_TRI 0
#define TILE_SQU 1
#define TILE_HEX 2
#define TILE_OCT 3

#define STYLE_MATCHING 0
#define STYLE_SEAMLESS 1

#define TILE_MAX_SIDES 8

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
struct Tile_t
{
	vec2  center;     // Center of the tile
	int   sides;      // Number of sides
	float len;        // Length of the side
	float radius;     // Radius to the vertex
	float inner;      // Radius to closest point on the edge
	float angle;      // Angle to first vertex
	float horzangle;  // Angle between canonical first vertex and horizontal 
	int   direction;  // Rotation direction (+1 or -1)
};

/*--------------------------------------------------------------
 * Locate (Equilateral) Triangle Tile
 *
 * Canonical tile: vertex zero at 90 degrees, alternating tiles rotated 180 degrees
 *
 * Styles:
 *   MATCHING
 *   SEAMLESS
 *--------------------------------------------------------------*/
Tile_t tile_LocateTriangle(vec2 aCoord, int tileStyle, float radius)
{
	Tile_t t;
	
	float sideLen = radius * 2.0 * cos30;
	
	// Compute the size of a box to contain an equilateral triangle with a side of length=sideLen
	vec2 boxSize = vec2(sideLen, sideLen * cos30);

	// Determine if this is even or odd row. First convert the vertical location to a row number
	// Determine if it is an odd or even row
	// For odd rows, invert the triangle
	// -- This inverts the results when y<0 -- int row = int(aCoord.y/boxSize.y);
	// -- This inverts the results when y<0 -- bool evenRow = ((row - 2*(row/2)) == 0);
	float row = floor(aCoord.y/boxSize.y);
	bool evenRow = ((row - 2.0*floor(row/2.0)) < 0.01);
	
	// Compute the center of the triangle relative to the bottom-left corner of the box
	// Note that triangles are inverted for odd rows, so the center is shifted
	vec2 ctrAdjA = vec2(boxSize.x * 0.5, boxSize.y * (evenRow?1.0:2.0)/3.0);
	vec2 coordA  = aCoord.xy;
	// Find the box containing the coord, then compute the triangle center
	vec2 boxA    = floor(coordA/boxSize);
	vec2 ctrA    = boxA * boxSize + ctrAdjA;
	// Triangles are inverted on odd rows
	float angleA = evenRow ? 90.0 : 270.0;
	int   dirA   = 1;
	
    if (tileStyle == STYLE_SEAMLESS)
    {
		int idx = int(boxA.x - 3.0 * floor(boxA.x/3.0));
		dirA = evenRow ? 1 : -1;
		angleA = 330.0 - float(idx) * 120.0;
		if (!evenRow)
			angleA = -angleA;
    }
    
    // Same as above, but we shift sideways by half a box
    // and invert all of the triangles
	vec2 shiftB  = vec2(boxSize.x * 0.5, 0.0);
	vec2 ctrAdjB = vec2(boxSize.x * 0.5, boxSize.y * (evenRow?2.0:1.0)/3.0);
	vec2 coordB  = aCoord.xy + shiftB;
	vec2 boxB    = floor((coordB)/boxSize);
	vec2 ctrB    = boxB * boxSize - shiftB + ctrAdjB;
	float angleB = evenRow ? 270.0 : 90.0;
	int   dirB   = 1;
	
    if (tileStyle == STYLE_SEAMLESS)
    {
		int idx = int(boxB.x - 3.0 * floor((boxB.x)/3.0));
		dirB = evenRow ? -1 : 1;
		angleB = 150.0 + float(idx) * 120.0;
		if (!evenRow)
			angleB = -angleB;
    }
	
	bool chooseA = (distance(aCoord, ctrA) < distance(aCoord, ctrB));
	vec2 ctr     = (chooseA) ? ctrA : ctrB;
	float angle  = (chooseA) ? angleA : angleB;
    int   dir    = (chooseA) ? dirA : dirB;

	t.center    = ctr;
	t.len       = sideLen;
	t.sides     = 3;
	t.radius    = radius;
	t.inner     = sideLen/4.0;
	t.angle     = angle;
	t.horzangle = -90.0;
	t.direction = dir;
	
	
	return t;
}

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
Tile_t tile_LocateHexagon(vec2 aCoord, int tileStyle, float radius)
{
	Tile_t t;
	
	float sideLen = radius;
	
	// Compute the size of a box to contain hexagon with a side of length=sideLen
	vec2 boxSize = vec2(sideLen * 3.0, sideLen*cos30*2.0);
	float hexWidth = sideLen * 2.0;

	// Convert the vertical location to a row number
	float row = floor(aCoord.y/boxSize.y);
	
	// Determine if it is an odd or even row
	// For odd rows, invert the triangle
	bool evenRow = ((row - 2.0*floor(row/2.0)) < 0.01);
	
	vec2 ctrAdj = vec2(sideLen, sideLen * cos30);
	
	vec2 shiftA  = vec2(0.0);
	float angleA = 0.0;
	int   dirA   = evenRow ? 1 : -1;
	vec2 coordA  = aCoord.xy + shiftA;
	vec2 boxA    = floor(coordA/boxSize);
	vec2 ctrA    = boxA * boxSize - shiftA + ctrAdj;
	bool hasA    = (coordA.x - boxA.x * boxSize.x < hexWidth);

	vec2 shiftB  = vec2(sideLen + sideLen * 0.5, sideLen * cos30);
	float angleB = 0.0;
	int   dirB   = evenRow ? 1 : -1;
	vec2 coordB  = aCoord.xy + shiftB;
	vec2 boxB    = floor((coordB)/boxSize);
	vec2 ctrB    = boxB * boxSize - shiftB + ctrAdj;
	bool hasB    = (coordB.x - boxB.x * boxSize.x < hexWidth);
	
	bool chooseA = (!hasB || (hasA && distance(aCoord, ctrA) < distance(aCoord, ctrB)));
	vec2 ctr     = (chooseA) ? ctrA : ctrB;
	float angle  = (chooseA) ? angleA : angleB;
    int   dir    = (chooseA) ? dirA : dirB;

	t.center    = ctr;
	t.len       = sideLen;
	t.sides     = 6;
	t.radius    = radius;
	t.inner     = sideLen/cos30;
	t.angle     = angle;
	t.horzangle = -30.0;
	t.direction = 1;
	
	return t;
}

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
Tile_t tile_LocateSquare(vec2 aCoord, int tileStyle, float radius)
{
	Tile_t t;
	
	float sideLen = radius*sqrt(2.0);
	
	// Compute the size of a box 
	vec2 boxSize = vec2(sideLen, sideLen);

	// Convert the vertical location to a row number
	float row = floor(aCoord.y/boxSize.y);
	
	// Determine if it is an odd or even row
	bool evenRow = ((row - 2.0*floor(row/2.0)) < 0.01);
	
	
	float angle   = 45.0;
	vec2  coord   = aCoord.xy;
	vec2  box     = floor(coord/boxSize);
	vec2  ctr     = box * boxSize + boxSize/2.0;

	t.center    = ctr;
	t.len       = sideLen;
	t.sides     = 4;
	t.radius    = radius;
	t.inner     = sideLen/2.0;
	t.angle     = angle;
	t.horzangle = -45.0;
	t.direction = 1;
	
	return t;
}

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
Tile_t tile_LocateOctagon(vec2 aCoord, int tileStyle, float radius)
{
	Tile_t t;
	
	float sideLen = radius * 2.0 * sin(Tau/16.0);
	float inner   = radius * cos(Tau/16.0);
	
	// Compute the size of a box 
	vec2 octBoxSize = vec2(inner * 2.0, inner * 2.0);

	float angle   = 360.0/16.0;
	vec2  coord   = aCoord.xy;
	vec2  box     = floor(coord/octBoxSize);
	vec2  ctrOct  = box * octBoxSize + octBoxSize/2.0;

	// Compute the size of the grid to locate the squares
	// The squares are rotated 45 degrees, so the "width" is measured across the diagonal..
	float squRadius   = sideLen / sqrt(2.0);
	vec2 squBoxSize   = vec2(squRadius * 2.0 + sideLen);
	vec2 shiftB  = vec2(sideLen + squRadius, sideLen + squRadius);
	vec2 coordB  = aCoord.xy - shiftB;
	vec2 boxB    = floor((coordB)/squBoxSize);
	vec2 ctrSqu  = boxB * squBoxSize + shiftB + vec2(squRadius);
	// We can use the fact that the sides of the square are diagonal lines with 
	// slope=+1 or slope=-1 to determine if a point is in the square.
	vec2 relPt   = coordB - boxB * squBoxSize - vec2(squRadius);
	bool inSqu   = ((abs(relPt.x) + abs(relPt.y)) < squRadius);
	//bool hasSqu  = ((coordB.x - boxB.x * squBoxSize.x < squRadius * 2.0) && (coordB.y - boxB.y * squBoxSize.y < squRadius * 2.0));

	if (inSqu)
	{
		t.center    = ctrSqu;
		t.len       = sideLen;
		t.sides     = 4;
		t.radius    = squRadius;
		t.inner     = squRadius/sqrt(2.0);
		t.angle     = 0.0;
		t.horzangle = 0.0;
		t.direction = 1;
	}
	else
	{
		t.center    = ctrOct;
		t.len       = sideLen;
		t.sides     = 8;
		t.radius    = radius;
		t.inner     = inner;
		t.angle     = angle;
		t.horzangle = -360.0/16.0;
		t.direction = 1;
	}
	
	return t;
}

/*--------------------------------------------------------------
 * Locate dispatch routine
 *--------------------------------------------------------------*/
Tile_t tile_Locate(vec2 aCoord, int tileType, int tileStyle, float radius)
{
	Tile_t t;
	
	if (tileType == TILE_TRI)
		t = tile_LocateTriangle(aCoord, tileStyle, radius);
	else if (tileType == TILE_SQU)
		t = tile_LocateSquare(aCoord, tileStyle, radius);
	else if (tileType == TILE_HEX)
		t = tile_LocateHexagon(aCoord, tileStyle, radius);
	else if (tileType == TILE_OCT)
		t = tile_LocateOctagon(aCoord, tileStyle, radius);
	
	return t;
}


/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
vec2 tile_CalcVertex(Tile_t tile, int vertexIdx)
{
	vec2 pt;
	float angle;
	
	angle = DegreeToRad * (tile.angle + float(vertexIdx) * float(tile.direction) * 360.0 / float(tile.sides));
	
	pt.x = tile.center.x + tile.radius * cos(angle);
	pt.y = tile.center.y + tile.radius * sin(angle);
	
	return pt;
}

/*--------------------------------------------------------------
 * Calculate the angle between two vectors, one from the center
 * to the coord and the other between the center and vertex zero
 *--------------------------------------------------------------*/
float tile_CalcAngle(Tile_t tile, vec2 coord)
{
	float angle = 0.0;
	
	angle = atan(coord.y - tile.center.y, coord.x - tile.center.x) / DegreeToRad;
	
	angle += tile.angle;
	
	angle *= float(tile.direction);
	
	return angle;
}

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
vec2 tile_CalcEdgeVector(Tile_t tile, int vertexIdx)
{
	vec2 vect;
	
	return vect;
}


/*--------------------------------------------------------------
 * Calculate the position of a coordinate relative to vertex 0,
 * taking into consideration the direction of the tile.
 * The result is -radius<=x<=radius, -radius<=y<=radius
 *--------------------------------------------------------------*/
vec2 tile_CalcRelPosition(Tile_t tile, vec2 coord, float twist)
{
	vec2 relPos;
	
	float angle = -(tile.angle + tile.horzangle + twist * float(tile.direction)) *  DegreeToRad;
	
	float cA = cos(angle);
	float sA = sin(angle);
	
	mat2 rm = mat2(cA, sA, -sA, cA);
	
	relPos = coord - tile.center;
	
	relPos = rm * relPos;
	
	if (tile.direction == -1)
		relPos.x = -relPos.x;
	return relPos;
}


/*--------------------------------------------------------------
 * Calculate the relative position, but return values appropriate
 * for a texture lookup.
 *--------------------------------------------------------------*/
vec2 tile_CalcRelPositionUV(Tile_t tile, vec2 coord, float twist)
{
	vec2 relPos = tile_CalcRelPosition(tile, coord, twist);
	
	vec2 uv = (relPos + vec2(tile.radius, tile.radius))/(tile.radius * 2.0);
	
	return uv;
}


/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
bool tile_ContainsPoint(Tile_t tile, vec2 coord)
{
	bool contains = true;
	
	return contains;
}


/*--------------------------------------------------------------
 * Calculate the distance to a specified edge
 *--------------------------------------------------------------*/
float tile_CalcDistanceToEdge(Tile_t tile, int vertexIdx, vec2 coord)
{
	float d;
	
	vec2 v1 = tile_CalcVertex(tile, vertexIdx);
	vec2 v2 = tile_CalcVertex(tile, vertexIdx+1);
	//vec2 edgeVec = edgeVector_Tile(tile, vertexIdx);
	
	// Calc distance to line
	vec2 vhat = vec2(v2.y - v1.y, v1.x - v2.x);
	vec2 r = v1 - coord;
	
	d = abs(dot(vhat, r))/length(vhat);
	
	return d;
}

/*--------------------------------------------------------------
 * Calculate the distance to the closest edge
 *--------------------------------------------------------------*/
float tile_CalcDistanceToClosestEdge(Tile_t tile, vec2 coord)
{
	float closest;
	float dist;
	
	closest = tile_CalcDistanceToEdge(tile, 0, coord);
	
	for (int i = 1; i < TILE_MAX_SIDES; i++)
	{
		if (i < tile.sides)
		{
			dist = tile_CalcDistanceToEdge(tile, i, coord);
			if (dist < closest)
				closest = dist;
		}
	}
	
	return closest;
}


/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
float tile_CalcDistanceToVertex(Tile_t tile, int vertexIdx, vec2 coord)
{
	vec2 pt = tile_CalcVertex(tile, vertexIdx);

	return distance(coord, pt);
}

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
float tile_CalcDistanceToClosestVertex(Tile_t tile, vec2 coord)
{
	float closest;
	float dist;
	
	closest = tile_CalcDistanceToVertex(tile, 0, coord);
	
	
	for (int i = 1; i < TILE_MAX_SIDES; i++)
	{
		if (i < tile.sides)
		{
			dist = tile_CalcDistanceToVertex(tile, i, coord);
			if (dist < closest)
				closest = dist;
					
		}
	}
	
	return closest;
}


/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void main(void)
{
	Tile_t tile;
	vec2 coord = gl_FragCoord.xy;
	float d;
	float radius = 60.0;
	
	vec4 clr;

	
	float triRadius = radius + 10.0 * sin(40.0 * iGlobalTime * DegreeToRad);
	float angle = 6.0*iGlobalTime * DegreeToRad;
	float cA = cos(angle);
	float sA = sin(angle);
	mat2 rm = mat2(cA, sA, -sA, cA);
	vec2 coord2 = rm * (coord - iResolution.xy/2.0) + iResolution.xy/2.0;
	
	tile = tile_Locate(coord2, TILE_TRI, 0, triRadius);	
	d = tile_CalcDistanceToClosestEdge(tile, coord2);	
	float cTri = (d < 5.0) ? (5.0-d)/5.0 : 0.0;
	
	float octRadius = radius - 15.0 * sin(30.0 * iGlobalTime * DegreeToRad);
	angle = -4.0*iGlobalTime * DegreeToRad;
	cA = cos(angle);
	sA = sin(angle);
	rm = mat2(cA, sA, -sA, cA);
	vec2 coord3 = rm * (coord - iResolution.xy/2.0) + iResolution.xy/2.0;
	tile = tile_Locate(coord3, TILE_OCT, 0, octRadius);	
	d = tile_CalcDistanceToClosestEdge(tile, coord3);	
	float cOct = (d < 5.0) ? (5.0-d)/5.0 : 0.0;
	
	vec4 clrTri;
	vec4 clrOct;

	if (triRadius < octRadius)
	{
		clrTri =  vec4(cTri, 0., 0.0, cTri);
		clrOct =  vec4(0., 1.0, 0.0, cOct);
		gl_FragColor = mix(clrTri, clrOct, clrOct.a);
	}
	else
	{
		clrTri =  vec4(1.0, 0., 0.0, cTri);
		clrOct =  vec4(0., cOct, 0.0, cOct);
		gl_FragColor = mix(clrOct, clrTri, clrTri.a);
	}
		
}
