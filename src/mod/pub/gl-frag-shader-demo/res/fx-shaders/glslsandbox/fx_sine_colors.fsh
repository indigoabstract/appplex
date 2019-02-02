// http://glslsandbox.com/e#20543.0
#ifdef GL_ES
precision mediump float;
#endif

uniform float time;
uniform vec2 resolution;

void main( void ) {
	vec2 position =(gl_FragCoord.xy / resolution.xy) - 0.5;
	vec3 color = vec3(0.0);
	color += vec3(-position.y,-position.x,0.0) * (1.0 /abs((position.y + sin((position.x * 4.0) + time) * 0.3))) / 4.0;
	color += vec3(0.0,position.y,position.x) * (1.0 /abs((position.y + sin((position.x * 4.0) + time) * 0.3))) / 4.0;
	gl_FragColor = vec4(color,1.0);
}
