// https://www.shadertoy.com/view/MdlGRr
// This helper function returns 1.0 if the current pixel is on a grid line, 0.0 otherwise
float IsGridLine()
{
	// Define the size we want each grid square in pixels
	vec2 vPixelsPerGridSquare = vec2(16.0, 16.0);
	
	// gl_FragCoord is an input to the shader, it defines the pixel co-ordinate of the current pixel
	vec2 vScreenPixelCoordinate = gl_FragCoord.xy;
	
	// Get a value in the range 0->1 based on where we are in each grid square
	// fract() returns the fractional part of the value and throws away the whole number part
	// This helpfully wraps numbers around in the 0->1 range
	vec2 vGridSquareCoords = fract(vScreenPixelCoordinate / vPixelsPerGridSquare);
	
	// Convert the 0->1 co-ordinates of where we are within the grid square
	// back into pixel co-ordinates within the grid square 
	vec2 vGridSquarePixelCoords = vGridSquareCoords * vPixelsPerGridSquare;

	// step() returns 0.0 if the second parmeter is less than the first, 1.0 otherwise
	// so we get 1.0 if we are on a grid line, 0.0 otherwise
	vec2 vIsGridLine = step(vGridSquarePixelCoords, vec2(1.0));
	
	// Combine the x and y gridlines by taking the maximum of the two values
	float fIsGridLine = max(vIsGridLine.x, vIsGridLine.y);

	// return the result
	return fIsGridLine;
}

// This helper function returns 1.0 if we are near the mouse pointer, 0.0 otherwise
float IsWithinCircle(vec2 vPos)
{
	// gl_FragCoord is an input to the shader, it defines the pixel co-ordinate of the current pixel
	vec2 vScreenPixelCoordinate = gl_FragCoord.xy;

	// We calculate how far in pixels we are from the mouse pointer
	float fPixelsToPosition = length(vScreenPixelCoordinate - vPos);
	
	// return 1.0 if the distance to the mouse pointer is less than 8.0 pixels, 0.0 otherwise
	return step(fPixelsToPosition, 8.0);
}

// main is the entry point to the shader. 
// Our shader code starts here.
// This code is run for each pixel to determine its colour
void main(void)
{
	// We are goung to put our final colour here
	// initially we set all the elements to 0 
	vec3 vResult = vec3(0.0);

	// We set the blue component of the result based on the IsGridLine() function
	vResult.b = IsGridLine();

	// 1.0 if we are near the mouse co-ordinates, 0.0 otherwise
	float fIsMousePointerPixelA = IsWithinCircle(iMouse.xy);

	// If the mouse button is pressed
	if(iMouse.z >= 0.0)
	{
		// we set the green component of the result
		vResult.g = fIsMousePointerPixelA;
	}
	else
	{
		// we set the red component of the result
		vResult.r = fIsMousePointerPixelA;
	}

	float fIsMousePointerPixelB = IsWithinCircle(abs(iMouse.zw));
	vResult.b = max(vResult.b, fIsMousePointerPixelB);


	// The output to the shader is gl_FragColor. 
	// This is the colour we write to the screen for this pixel
	gl_FragColor = vec4(vResult, 1.0);
}
