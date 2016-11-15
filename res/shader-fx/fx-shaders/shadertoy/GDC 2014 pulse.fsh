// https://www.shadertoy.com/view/Md23Dt
// GDC 2014 pulse
const vec4 orange = vec4(1.0, 0.4, 0.0, 1.0);
const vec4 red = vec4(1.0, 0.0, 0.0, 1.0);
const vec4 black = vec4(0.0, 0.0, 0.0, 0.0);
const vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
const vec4 white = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 gray = vec4(0.4, 0.4, 0.4, 1.0);
const vec4 darkgray = vec4(0.2, 0.2, 0.2, 1.0);


void drawLetterD( in vec2  center, 
		  in vec2  sPos,
		  in float aspRatio,
		  in float baseSize,
		  in float fontThickness)
{
	vec2 lPos = sPos - center;
	lPos.x *= aspRatio; // make square and circle instead of rect and ellipse
	float outSize = baseSize;
	float outHSize = outSize * 0.5;
	
	fontThickness *= baseSize; // font thickness specified is relative to size of font..
	
	
	float inSize = outSize - fontThickness;
	float inHSize = inSize * 0.5;

	if( (length(lPos) < outHSize ||
	     lPos.x > -outHSize &&
	     lPos.y > 0.0 &&
	     lPos.x < 0.0 && 
	     lPos.y < outHSize) &&
	   
	   !(length(lPos) < inHSize ||
	     lPos.x > -inHSize &&
	     lPos.y > 0.0 &&
	     lPos.x < 0.0 && 
	     lPos.y < inHSize ) )
	{
		gl_FragColor = darkgray;
	}
	
#if 1
//draw 14	
	float littleFontThickness = fontThickness * 0.15;
	{
	//draw 1 of '14'
	float up    = 0.3 * outHSize;
	float down  = -up;
	float leftBase = -0.3 * outHSize;
	float left  = leftBase - ((lPos.y > 0.6 * up) ? (fontThickness * 0.1) : 0.0);
	float right = leftBase + littleFontThickness;
	
	if(  lPos.x > left &&
	     lPos.y > down &&
	     lPos.x < right && 
	     lPos.y < up)	   
	{
		gl_FragColor = darkgray;
	}
	}
	
	//draw 4 of '14'
	{
	float up    = 0.3 * outHSize;
	float down  = -0.1 * outHSize;
	float leftBase = -0.012;// * outHSize;
	float left  = leftBase + 0.75 * lPos.y;
	float right = left + littleFontThickness * 1.25;
	
	if(  lPos.x > left &&
	     lPos.y > down &&
	     lPos.x < right && 
	     lPos.y < up)	   
	{
		gl_FragColor = darkgray;
	}
		
	up    = 0.3 * outHSize;
	down  = -0.3 * outHSize;
	leftBase = -0.012;
	left  = leftBase + 0.78 * 0.33 * outHSize;
	right = left + littleFontThickness * 1.0;
	
	if(  lPos.x > left &&
	     lPos.y > down &&
	     lPos.x < right && 
	     lPos.y < up)	   
	{
		gl_FragColor = darkgray;
	}
		
	up    = -0.09 * outHSize;
	down  = -0.2 * outHSize;
	leftBase = -0.0135;
	left  = leftBase - 0.065 * outHSize;
	right = left + 0.5 * outHSize;
	
	if(  lPos.x > left &&
	     lPos.y > down &&
	     lPos.x < right && 
	     lPos.y < up)	   
	{
		gl_FragColor = darkgray;
	}			
	}
	
#endif
}


void drawLetterC( in vec2  center, 
		  in vec2  sPos,
		  in float aspRatio,
		  in float baseSize,
		  in float fontThickness)
{
	vec2 lPos = sPos - center;
	lPos.x *= aspRatio; // make square and circle instead of rect and ellipse
	//but make it a controlled ellipse now
	lPos.x *= 0.85;
	
	fontThickness *= baseSize; // font thickness specified is relative to size of font..
	
	float outSize = baseSize;
	float outHSize = outSize * 0.5;
	float inSize = outSize - fontThickness;
	float inHSize = inSize * 0.5;

	if( (length(lPos) < outHSize &&
		 lPos.x < outHSize*0.45) &&
	   
	   !(length(lPos) < inHSize) )
	{
		gl_FragColor = darkgray;
	}
}

void drawLetterG( in vec2  center, 
		  in vec2  sPos,
		  in float aspRatio,
		  in float baseSize,
		  in float fontThickness)
{
	vec2 lPos = sPos - center;
	lPos.x *= aspRatio; // make square and circle instead of rect and ellipse
	//but make it a controlled ellipse now
	lPos.x *= 0.85;
	
	fontThickness *= baseSize; // font thickness specified is relative to size of font..
	
	float outSize = baseSize;
	float outHSize = outSize * 0.5;
	float inSize = outSize - fontThickness;
	float inHSize = inSize * 0.5;

	if( (length(lPos) < outHSize &&
		 lPos.x < outHSize*0.45) &&
	   
	   !(length(lPos) < inHSize) )
	{
		gl_FragColor = darkgray;
	}

//	float leftBound = smoothstep(inHSize * 0.15;
	float derp = smoothstep(-outHSize*0.3, 0.0, lPos.y);
	float leftBound = inHSize * 0.25 - (derp * 2.0 * inHSize * 0.15);
	
	if( lPos.x > leftBound &&
	    lPos.y > -outHSize*.9 &&
	    lPos.x < outHSize*.45 &&
	    lPos.y < 0.0)
	{
		gl_FragColor = darkgray;
	}
}


void main(void)
{
	vec2 center = vec2(0.5, 0.5);
	float radius = 0.3;
	float aspRatio = iResolution.x / iResolution.y;
	
	float throbAmt = 0.1;
	float size = 0.3 + throbAmt * sin(iGlobalTime*8.0);
	float hSize = size/2.0;
	
	vec2 sPos = gl_FragCoord.xy / iResolution.xy;
	vec2 uv = sPos;
	uv.y = -uv.y;
	
	center.x = (sin(iGlobalTime)+1.5)/3.0;
	center.y = (cos(iGlobalTime)+1.5)/3.0;
	
	vec2 lPos = sPos - center;
	lPos.x *= aspRatio; // make square and circle instead of rect and ellipse
	
	gl_FragColor = gray; // background color

//#define SQUARE
//#define CIRCLE
	
#ifdef SQUARE
//Square
	if(lPos.x > -hSize &&
	   lPos.y > -hSize &&
	   lPos.x < (hSize) && 
	   lPos.y < (hSize) )
	{
		gl_FragColor = texture2D( iChannel0, uv );
	}
	else
	{
		discard;
	}
#elif defined CIRCLE
//Circle
	gl_FragColor = ((length(lPos) < hSize))?red:black;
#else
//GDC Squary Circle throbbing

	float thickness = 0.1;
	float fontThrobAmt = 0.0;
	float fontBaseSize = 0.28;
	float fontThickness = 0.1;
//	drawLetterD(center, sPos, aspRatio, fontThrobAmt, fontBaseSize, fontThickness, true, false);
	
	
//left bottom square
	if(length(lPos) < hSize ||
	   lPos.x > -hSize &&
	   lPos.y > -hSize &&
	   lPos.x < 0.0 && 
	   lPos.y < 0.0 )
	{
		gl_FragColor = orange;
	}
	
//yellow
//right top square 
	center.x = (cos(iGlobalTime*1.84)+1.5)/3.0;
	center.y = (sin(iGlobalTime*2.2)+1.5)/3.0;
	lPos = sPos - center;
	lPos.x *= aspRatio; // make square and circle instead of rect and ellipse
	
	throbAmt = 0.15;
	size = 0.3 + throbAmt * cos(iGlobalTime*6.0);	
	hSize = size * 0.35;

	if(length(lPos) < hSize ||
	   lPos.x > 0.0 &&
	   lPos.y > 0.0 &&
	   lPos.x < hSize && 
	   lPos.y < hSize )
	{
		gl_FragColor = yellow;
	}
	
//red
//left top square
	center.x = (cos(iGlobalTime*0.64)+1.6)/3.2;
	center.y = (sin(iGlobalTime*1.7)+1.1)/2.2;
	lPos = sPos - center;
	lPos.x *= aspRatio; // make square and circle instead of rect and ellipse
	throbAmt = 0.15;
	size = 0.4 + throbAmt * cos(iGlobalTime*7.0);	
	hSize = size * 0.45;

	if(length(lPos) < hSize ||
	   lPos.x > -hSize &&
	   lPos.y > 0.0 &&
	   lPos.x < 0.0 && 
	   lPos.y < hSize )
	{
		gl_FragColor = red;
	}
	
//white
//right bottom square
	center.x = (cos(iGlobalTime*1.2)+1.3)/2.6;
	center.y = (sin(iGlobalTime*0.4)+1.3)/2.6;
	lPos = sPos - center;
	lPos.x *= aspRatio; // make square and circle instead of rect and ellipse
	throbAmt = 0.1;
	size = 0.33 + throbAmt * sin(iGlobalTime*2.0);	
	hSize = size * 0.3;

	if(length(lPos) < hSize ||
	   lPos.x > 0.0 &&
	   lPos.y > -hSize &&
	   lPos.x < hSize && 
	   lPos.y < 0.0 )
	{
		gl_FragColor = white;
	}
	
//black
//right bottom square
	center.x = (cos(iGlobalTime*2.1)+1.3)/2.6;
	center.y = (sin(iGlobalTime*4.0)+1.3)/2.6;
	lPos = sPos - center;
	lPos.x *= aspRatio; // make square and circle instead of rect and ellipse
	throbAmt = 0.1;
	size = 0.28 + throbAmt * sin(iGlobalTime*2.0);	
	hSize = size * 0.3;

	if(length(lPos) < hSize ||
	   lPos.x > -hSize &&
	   lPos.y > -hSize &&
	   lPos.x < 0.0 && 
	   lPos.y < 0.0 )
	{
		gl_FragColor = black;
	}	
	
//letters
//GDC
	fontThickness = 0.38;
	fontBaseSize = 0.4;// + 0.15;// * abs(sin(iGlobalTime));	
	center = vec2(0.5,0.5);
	drawLetterD(center, sPos, aspRatio, fontBaseSize, fontThickness);
	center.x = 0.5 + 1.13 * fontBaseSize / aspRatio;
	drawLetterC(center, sPos, aspRatio, fontBaseSize, fontThickness);
	center.x = 0.5 - 0.85 * fontBaseSize / aspRatio;
	drawLetterG(center, sPos, aspRatio, fontBaseSize, fontThickness);
	
#endif
}
