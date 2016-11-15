// https://www.shadertoy.com/view/lssGW4
// Pong shader
// 
// press the mouse and drag up and down to play.
// HF ! jerome.
// 
// todo :ball trail.

vec2 uv;
vec4 col;

void drawCircle( vec2 center, float radius )
{
	vec2 dif = center - uv;
	if( length(dif)*length(dif) < radius*radius )
	{
		col = vec4(1.0);
	}
}

void drawRectangle( vec2 center )
{
	center.y = clamp(center.y, 0.25, 0.75 );
	if( abs( center.x - uv.x ) < 0.02 )
	{
		if( abs( center.y - uv.y ) < 0.05  )
		{
			col = vec4(1,0,0,0);
		}
	}
}

void drawTable()
{
	if( abs( 0.5 - uv.x ) < 0.25 )
	{
		if( abs( 0.5 - uv.y ) < 0.25  )
		{
			col = vec4(0.2,0.2,0.2,1.0);
		}
	}
}

float GetheightForTime( float time )
{
	float y = 0.0;
	
	// the X speed is always the same, only the Y speed varies
	float arraySpeed[3];
	arraySpeed[0] = 0.5;
	arraySpeed[1] = 0.4;
	arraySpeed[2] = 0.3;
		
	float div = floor( time / 3.0 );
	float rest = time - div*3.0;
	float distanceTotale = div * 0.4 * 3.0; // avg speed is 0.4
	
	if( rest > 0.0 && rest <= 1.0)
	{
		distanceTotale += arraySpeed[0] * rest;
	}
	if( rest > 1.0 && rest <= 2.0)
	{
		distanceTotale += arraySpeed[0] + arraySpeed[1] * (rest-1.0);
	}
	else if( rest > 2.0 )
	{
		distanceTotale += arraySpeed[0] + arraySpeed[1] + arraySpeed[2]*(rest-2.0);
	}	
	
	float restant = fract( distanceTotale );
	// move on the Y axis
	// how many time did we bounce so far ?
	if( restant >= 0.5 )
	{
		y = 0.25 - (restant-0.5);// going down
	}
	else
	{
		y = -0.25 + restant; //frc*dec; // going up
	}

	return y;
}

vec2 findCenter()
{
	vec2 start = vec2(0.5,0.5); // center
	
	float speed = 0.50; // 2 secs to go from one side to the other.
	float x = -0.25;
	float y = -0.25;
	float secs = floor(iGlobalTime);	
	float dec = fract( iGlobalTime );
	
	y = GetheightForTime(iGlobalTime);
	
	// move on the X axis (speed is constant, easy):
	float restant = fract( iGlobalTime * 0.5 );
	if( restant >= 0.5  )
	{
		x = 0.25 - (restant-0.5);
	}
	else
	{
		x = -0.25 + restant;
	}
	
	// check if the player missed
	if( x > 0.24 && length( iMouse.x ) > 100.0 )
	{
		// when is the ball going to reach the right side ?
		float timeReach = ceil( iGlobalTime );
		float yfuture = GetheightForTime( timeReach );
		if( abs( (0.5+y) - (iMouse.y / iResolution.y) ) > 0.07 )
		{
			col = vec4(1.0,0.0,0.0,1.0);
		}
	}

	return start + vec2(x,y);	
}

void main(void)
{
	col = vec4(0);
	uv = gl_FragCoord.xy / iResolution.xy;
	vec2 center = findCenter();
	
	drawTable();
	
	drawCircle( center, 0.03 );
	
	if( length( iMouse.x ) > 100.0 )
	{
		drawRectangle( vec2(0.8, iMouse.y / iResolution.y) );
	}
	else
	{
		drawRectangle( vec2(0.8, center.y) );
	}
	
	// AI raquette
	drawRectangle( vec2(0.2, center.y) );
	gl_FragColor = col;
}
