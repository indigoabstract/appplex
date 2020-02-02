#ifdef GL_ES
   precision highp float;
#endif

uniform float microflakePerturbation;
uniform float brightnessFactor;
uniform float glossLevel;
uniform vec4 paintColorMid;
uniform vec4 paintColor2;
uniform vec4 flakeLayerColor;
uniform float normalPerturbation;
uniform float microflakePerturbationA;
uniform vec4 paintColor0;
uniform sampler2D normalMap;
uniform sampler2D microflakeNMap;
uniform samplerCube showroomMap;
//----------------------------------------------------------------------------------
// Phenomenological car paint visual effect: Simulates build up of paint coats       
// on the metallic surface of a car or any other object. This visual effect        
// emulates the suspended layers of microflakes in the paint coat as well as        
// blends between several colors of paint based on the viewing angle.          
//                                        
// Author: Natalya Tatarchuk                              
//      (based on the original assembly shader written by John Isidoro)       
//                                        
// Used in ATI's Car demo for Radeon 9700 launch, found here:             
//       http://www.ati.com/developer/demos/r9700.html                
//                                        
// (C) ATI Research, 2003                                        
//----------------------------------------------------------------------------------


varying vec2 Tex;
varying vec3 Tangent;
varying vec3 Binormal;
varying vec3 Normal;
varying vec3 View;
varying vec3 SparkleTex;

void main()
{ 
   // fetch from the incoming normal map:
   vec3 vNormal = texture2D( normalMap, Tex ).rgb;

   // Scale and bias fetched normal to move into [-1.0, 1.0] range:
   vNormal = 2.0 * vNormal - 1.0;

   // Microflakes normal map is a high frequency normalized
   // vector noise map which is repeated across all surface. 
   // Fetching the value from it for each pixel allows us to 
   // compute perturbed normal for the surface to simulate
   // appearance of microflakes suspected in the coat of paint:
   vec3 vFlakesNormal = texture2D( microflakeNMap, SparkleTex.xy ).rgb;
   
   // Don't forget to bias and scale to shift color into [-1.0, 1.0] range:
   vFlakesNormal = 2.0 * vFlakesNormal - 1.0;

   // This shader simulates two layers of microflakes suspended in 
   // the coat of paint. To compute the surface normal for the first layer,
   // the following formula is used: 
   //   Np1 = ( a * Np + b * N ) /  || a * Np + b * N || where a << b
   //
   vec3 vNp1 = microflakePerturbationA * vFlakesNormal + normalPerturbation * vNormal ; 

   // To compute the surface normal for the second layer of microflakes, which
   // is shifted with respect to the first layer of microflakes, we use this formula:
   //    Np2 = ( c * Np + d * N ) / || c * Np + d * N || where c == d
   // 
   vec3 vNp2 = microflakePerturbation * ( vFlakesNormal + vNormal ) ;

   // The view vector (which is currently in world space) needs to be normalized.
   // This vector is normalized in the pixel shader to ensure higher precision of
   // the resultinv view vector. For this highly detailed visual effect normalizing
   // the view vector in the vertex shader and simply interpolating it is insufficient
   // and produces artifacts.
   vec3 vView = normalize( View );

   // Transform the surface normal into world space (in order to compute reflection
   // vector to perform environment map look-up):
   mat3 mTangentToWorld;
   mTangentToWorld[0] = Tangent;
   mTangentToWorld[1] = Binormal;
   mTangentToWorld[2] = Normal;
   vec3   vNormalWorld    = normalize( mTangentToWorld * vNormal );

   // Compute reflection vector resulted from the clear coat of paint on the metallic
   // surface:
   float  fNdotV     = clamp(dot( vNormalWorld, vView), 0.0, 1.0);
   vec3   vReflection = 2.0 * vNormalWorld * fNdotV - vView;

   // Here we just use a constant gloss value to bias reading from the environment
   // map, however, in the real demo we use a gloss map which specifies which 
   // regions will have reflection slightly blurred.
   float fEnvBias = glossLevel;

   // Sample environment map using this reflection vector:
   vec4 envMap = textureCube( showroomMap, vReflection, fEnvBias );

   // Premultiply by alpha:
   envMap.rgb = envMap.rgb * envMap.a;

   // Brighten the environment map sampling result:
   envMap.rgb *= brightnessFactor;

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

   vec4 paintColor = fFresnel1   * paintColor0 + 
                     fFresnel1Sq * paintColorMid +
                     fFresnel1Sq * fFresnel1Sq * paintColor2 +
                     pow( fFresnel2, 16.0 ) * flakeLayerColor;

   // Combine result of environment map reflection with the paint color:
   float  fEnvContribution = 1.0 - 0.5 * fNdotV;

   vec4 finalColor;
   finalColor.a = 1.0;
   finalColor.rgb = envMap.rgb * fEnvContribution + paintColor.rgb;
              
   gl_FragColor = finalColor;

}
