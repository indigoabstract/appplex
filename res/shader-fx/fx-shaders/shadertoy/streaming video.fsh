// https://www.shadertoy.com/view/MsB3Wt
// you can move the player slider, and the controls resize if you switch to fullscreen.
#define ANGLE 360./8.
#define R_THROB 20.
#define R_DOT 6.
#define PI 3.14159265359
#define PLAY_H 8.
#define PAUSE_DIV 2.
#define BAR_H 30.
#define STRIP_L BAR_H/2. - 3.
#define STRIP_H BAR_H/2. + 3.
#define STRIP_R_M 35.
#define STRIP_L_M 60.
#define CIRCLE_RAD 7.
#define VOLUME -15.
#define UI_SHADE 0.7

void volume(vec2 pos, inout vec3 col)
{
	pos -= vec2(iResolution.x-5., BAR_H/2.-PLAY_H);
	vec3 col_vol = pos.x < VOLUME ? vec3(UI_SHADE) : vec3(0.4);
	float l = step(0.,pos.y)
		*(1.-step(0.,pos.x))
		*(1.-smoothstep(0.,1.,pos.y-pos.x/2.5-PLAY_H*2.));
	col = mix(col, col_vol, l);
}

void circle(vec2 pos, inout vec3 col)
{
	float l = length(pos-vec2(clamp(iMouse.x, STRIP_R_M, iResolution.x-STRIP_L_M), BAR_H/2.));
	col = mix(vec3(UI_SHADE), col, smoothstep(CIRCLE_RAD, CIRCLE_RAD+1., l));
}

void strip(vec2 pos, inout vec3 col)
{
	float l = smoothstep(STRIP_L-2., STRIP_L+2., pos.y)
		* (1.-smoothstep(STRIP_H-.2,STRIP_H+2., pos.y))
		* step(STRIP_R_M ,pos.x)
		* (1.-step(iResolution.x-STRIP_L_M, pos.x));
	vec3 col_strip = pos.x < iMouse.x ? vec3(1.,0.,0.) : vec3(0.);
	col = mix(col,col_strip,l);
}

void bar(vec2 pos, inout vec3 col)
{
	if (pos.y < BAR_H)
	{
		col = vec3(.3);
	}
}

void gradient(vec2 pos, inout vec3 col)
{
	if (pos.y < BAR_H)
	{
		col *= 1. - pos.y/150.;
	}
}

void play(vec2 pos, inout vec3 col)
{
	pos -= vec2(10.,BAR_H/2.);
	float l = smoothstep(0., 1., pos.x)*smoothstep(0.,1.5,PLAY_H-abs(pos.y)-pos.x/1.5);
	
	col = mix(col,vec3(UI_SHADE),l);
}

void pause(vec2 pos, inout vec3 col)
{
	pos -= vec2(BAR_H/2.,BAR_H/2.);
	float l = step(-PLAY_H, pos.x)
		*(1.-step(PLAY_H, pos.x))
		*step(-PLAY_H, pos.y)
		*(1.-step(PLAY_H, pos.y));
	l*=(1.-step(-PAUSE_DIV, pos.x)
		*(1.-step(PAUSE_DIV, pos.x)));
	col = mix(col,vec3(UI_SHADE),l);
}

vec3 throbber(vec2 pos)
{
	vec3 col = vec3(.0);
	vec2 pol = vec2(length(pos), -atan(pos.y, pos.x)*180./PI);
	float ang = (mod(pol.y+ANGLE/2., ANGLE)-ANGLE/2.)*PI/180.;
	float index = pol.y*PI/180. - ang;
	index -= iGlobalTime*10.;
	index = mod(index,2.*PI);
	pos = pol.x*vec2(cos(ang), sin(ang));
	col = vec3(index)/2./PI * (1.-smoothstep(R_DOT-1.,R_DOT,length(pos - vec2(R_THROB,0.))));
	return col;
}

void main(void)
{
	vec2 pos = gl_FragCoord.xy;
	vec2 mid = iResolution.xy/2.;
	vec3 col = throbber(pos-mid-vec2(0.,BAR_H/2.));
	bar(pos, col);
	//play(pos, col);
	pause(pos, col);
	strip(pos, col);
	circle(pos, col);
	volume(pos, col);
	gradient(pos, col);
	gl_FragColor = vec4(col, 1.);
}
