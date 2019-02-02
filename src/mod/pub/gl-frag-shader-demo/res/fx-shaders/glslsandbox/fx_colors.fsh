// http://glslsandbox.com/e#23793.1
#ifdef GL_ES
precision mediump float;
#endif




uniform float time;


uniform vec2 mouse;


uniform vec2 resolution;


void main( void ) {
	// Calcul du ratio de l'affichage
	float ratio = resolution.x/resolution.y;
	vec2 position = gl_FragCoord.xy/resolution.xy; // gl_FragCoord correspond à la position du pixel dessiné
	gl_FragColor = vec4(sin(position.x*4.0+time)*2.0*sin(time)+sin(position.y*4.0-time)*2.0,cos(position.x*4.0+time)*2.0+cos(position.y*4.0-time)*2.0*sin(time),1.5+cos(position.x*4.0+time)*2.0+sin(position.y*4.0-time)*2.0*sin(time), 1.);

}
