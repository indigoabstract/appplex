// http://www.ati.com/developer/demos/r9700.html

#ifdef GL_ES
	#extension GL_OES_standard_derivatives : enable
	#ifdef GL_FRAGMENT_PRECISION_HIGH
	   // Default precision
	   precision highp float;
	#else
	   precision mediump float;
	#endif
#endif

uniform mat4 u_m4_model;
uniform mat4 u_m4_view_inv;

uniform float u_v1_microflakePerturbation;
uniform float u_v1_brightnessFactor;
uniform float u_v1_glossLevel;
uniform vec4 u_v4_paintColorMid;
uniform vec4 u_v4_paintColor2;
uniform vec4 u_v4_flakeLayerColor;
uniform float u_v1_normalPerturbation;
uniform float u_v1_microflakePerturbationA;
uniform vec4 u_v4_paintColor0;
uniform sampler2D u_s2d_base_normal_map;
uniform sampler2D u_s2d_micro_flackes_normal_map;
uniform samplerCube u_scm_skybox;

varying vec3 v_v3_pos_ws;
varying vec3 v_v3_normal_ws;
varying vec2 Tex;
varying vec3 Tangent;
varying vec3 Binormal;
varying vec3 Normal;
varying vec3 View;
varying vec2 SparkleTex;

vec3 GetNormal(vec3 N, vec3 C, sampler2D normal_map, vec2 uv);

void main()
{
   // fetch from the incoming normal map:
   vec3 vNormal = texture2D(u_s2d_base_normal_map, Tex).rgb;

   // Scale and bias fetched normal to move into [-1.0, 1.0] range:
   vNormal = 2.0 * vNormal - 1.0;

   // Microflakes normal map is a high frequency normalized
   // vector noise map which is repeated across all surface. 
   // Fetching the value from it for each pixel allows us to 
   // compute perturbed normal for the surface to simulate
   // appearance of microflakes suspected in the coat of paint:
   vec3 vFlakesNormal = texture2D(u_s2d_micro_flackes_normal_map, SparkleTex.xy).rgb;
   
   // Don't forget to bias and scale to shift color into [-1.0, 1.0] range:
   vFlakesNormal = 2.0 * vFlakesNormal - 1.0;

   // This shader simulates two layers of microflakes suspended in 
   // the coat of paint. To compute the surface normal for the first layer,
   // the following formula is used: 
   //   Np1 = ( a * Np + b * N ) /  || a * Np + b * N || where a << b
   //
   vec3 vNp1 = u_v1_microflakePerturbationA * vFlakesNormal + u_v1_normalPerturbation * vNormal ; 

   // To compute the surface normal for the second layer of microflakes, which
   // is shifted with respect to the first layer of microflakes, we use this formula:
   //    Np2 = ( c * Np + d * N ) / || c * Np + d * N || where c == d
   // 
   vec3 vNp2 = u_v1_microflakePerturbation * ( vFlakesNormal + vNormal ) ;

   // The view vector (which is currently in world space) needs to be normalized.
   // This vector is normalized in the pixel shader to ensure higher precision of
   // the resultinv view vector. For this highly detailed visual effect normalizing
   // the view vector in the vertex shader and simply interpolating it is insufficient
   // and produces artifacts.
   vec3 vView = normalize( View );
   //vView = v_v3_pos_ws - u_m4_view_inv[3].xyz;
   //vView = normalize(vView);

   // Transform the surface normal into world space (in order to compute reflection
   // vector to perform environment map look-up):
   mat3 mTangentToWorld;
	mat3 wm = mat3(u_m4_model);
   mTangentToWorld[0] = wm * Tangent;
   mTangentToWorld[1] = wm * Binormal;
   mTangentToWorld[2] = wm * normalize(Normal);
   vec3 vNormalWorld = normalize( mTangentToWorld * vNormal );
   //vNormalWorld    = normalize(wm *Normal);   
   //vNormalWorld = GetNormal(v_v3_normal_ws, v_v3_pos_ws, u_s2d_base_normal_map, Tex);

   // Compute reflection vector resulted from the clear coat of paint on the metallic
   // surface:
   float  fNdotV     = clamp(dot( vNormalWorld, vView), 0.0, 1.0);
   vec3   vReflection = 2.0 * vNormalWorld * fNdotV - vView;

   // Here we just use a constant gloss value to bias reading from the environment
   // map, however, in the real demo we use a gloss map which specifies which 
   // regions will have reflection slightly blurred.
   float fEnvBias = u_v1_glossLevel;

   // Sample environment map using this reflection vector:
   vec4 envMap = textureCube( u_scm_skybox, vReflection, fEnvBias );

   // Premultiply by alpha:
   envMap.rgb = envMap.rgb * envMap.a;

   // Brighten the environment map sampling result:
   envMap.rgb *= u_v1_brightnessFactor;

   // Compute modified Fresnel term for reflections from the first layer of
   // microflakes. First transform perturbed surface normal for that layer into 
   // world space and then compute dot product of that normal with the view vector:
   vec3   vNp1World = normalize( mTangentToWorld * vNp1 );
   float  fFresnel1 = clamp( dot( vNp1World, vView ), 0.0, 1.0);

   // Compute modified Fresnel term for reflections from the second layer of 
   // microflakes. Again, transform perturbed surface normal for that layer into 
   // world space and then compute dot product of that normal with the view vector:
   vec3   vNp2World = normalize( mTangentToWorld * vNp2 );
   float  fFresnel2 = clamp( dot( vNp2World, vView ), 0.0, 1.0);   

   //
   // Compute final paint color: combines all layers of paint as well as two layers
   // of microflakes
   //
   float  fFresnel1Sq = fFresnel1 * fFresnel1;

   vec4 paintColor = fFresnel1   * u_v4_paintColor0 + 
                     fFresnel1Sq * u_v4_paintColorMid +
                     fFresnel1Sq * fFresnel1Sq * u_v4_paintColor2 +
                     pow( fFresnel2, 16.0 ) * u_v4_flakeLayerColor;

   // Combine result of environment map reflection with the paint color:
   float  fEnvContribution = 1.0 - 0.5 * fNdotV;

   vec4 finalColor;
   finalColor.a = 1.0;
   finalColor.rgb = envMap.rgb * fEnvContribution + paintColor.rgb;
              
   gl_FragColor = finalColor;
   //gl_FragColor= vec4(Tangent, 1.0);
}

vec3 GetNormal(vec3 N, vec3 C, sampler2D normal_map, vec2 uv)
{
	vec3 dpx = dFdx(C);
	vec3 dpy = dFdy(C);
	vec2 dtx = dFdx(uv);
	vec2 dty = dFdy(uv);
	
	vec3 T = normalize(dpy * dtx.y - dpx * dty.y);
	vec3 B = cross(T, N);
	vec3 normal = texture2D(normal_map, uv).xyz * 2.0 - 1.0;
	
	return normalize(mat3(T, B, N) * normal);
}
