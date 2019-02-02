// https://www.shadertoy.com/view/lsfXR7
// A Tartan shader, but done in a way that you can easily try other weaves (Not really clan stewart's tartan -- just liked the name...)
//super slop
//TODO: need comments
void main(void)
{
	vec2 uv = gl_FragCoord.xy ;
	float x = uv.x;
	float y = uv.y;
	float xo;
	float yo;
	float xp;
	float yp;

	float width  = 1.0;  //FYI: try 40.0 
	float settCount = 200.0;  //fyi, google 'sett' to learn more

	//others to try
	
	//settCount = 40.0;
	//width = 10.0;
	
	//settCount = 100.0;
	//width = 10.0;
	
    //Interesting: Weird color effet 
	//settCount = 100.0;
	//width = 1.0, 4.0, 5.0, 8.0; 10;
	
	
	

    
	
	xp = floor(x/width);
	yp = floor(y/width);


	bool isWarp = false;
	//tweed weaving pattern 
	if (mod(xp - yp, 4.0) < 2.0) {
		isWarp = !isWarp;
	}


	float m = 0.0;
	float sett = 0.0;
	if (isWarp) {
		m = fract(x/width);
		sett = fract(xp/settCount);
	}
	else {
		m = fract(y/width);
		sett = fract(yp/settCount);

	}
	
	//symmetrize
	if (sett > 0.5) {
		sett = 1.0- sett;
	}
	
	vec4 col = 	vec4(0.2,0.2,0.2,1.0);
	if ( sett < .2) {
		col = 	vec4(1.0,0.0,0.0,1.0);
	}
	else if (sett < .3) {
		col = 	vec4(0.8,0.8,0.0,1.0);
	}


	//symmetrize m
	if (m > 0.5) {
		m = 1.0 - m;
	}

	
	//TODO:  cound be slightly refined to include sett location to see slivers underneath doubles
	//thread boundaries
	if (m < 0.1 ) {
		gl_FragColor = vec4(1.0,1.0,1.0,1.0);

	}
	else if (m < 0.15 ) {
		gl_FragColor = vec4(0.1,0.1,1.0,1.0);
	}
	else  {
		gl_FragColor = col;
	}

}
