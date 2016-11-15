// https://www.shadertoy.com/view/4slGWn

// iChannel0: t4

// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// The texture2D() call accepts a mip level offset as an optional parameter, which
// allows one to sample from different LODs of the texture. Besides being handy in
// some special situations, it also allows you to fake (box) blur of textures without
// having to perform a blur youtself. This has been traditionally used in demos and
// games to fake deph ot field and other similar effects in a very cheap way.

void main( void )
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	float lod = (5.0 + 5.0*sin( iGlobalTime ))*step( uv.x, 0.5 );
	
	vec3 col = texture2D( iChannel0, vec2(uv.x,1.0-uv.y), lod ).xyz;
	
	gl_FragColor = vec4( col, 1.0 );
}
