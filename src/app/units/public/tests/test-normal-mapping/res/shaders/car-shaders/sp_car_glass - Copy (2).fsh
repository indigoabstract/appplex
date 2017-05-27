#ifdef GL_ES
	precision highp float;
#endif

uniform float rainbowSpread;
uniform float ambient;
uniform float rainbowScale;
uniform float reflectionScale;
uniform float refractionScale;
uniform vec4 baseColor;
uniform float indexOfRefractionRatio;

uniform sampler2D Rainbow;
uniform samplerCube u_scm_skybox;

varying vec3 vNormal;
varying vec3 vViewVec;

void main(void)
{
   
   vec3 normal = normalize(vNormal);
   vec3 viewVec = normalize(vViewVec);

   vec3 reflVec = reflect(-viewVec, normal);
   vec4 reflection = textureCube(u_scm_skybox, reflVec.xyz);

   float cosine = dot(viewVec, normal);
   float sine = sqrt(1.0 - cosine * cosine);

   float sine2 = clamp(indexOfRefractionRatio * sine, 0.0, 1.0);   // same as saturate
   float cosine2 = sqrt(1.0 - sine2 * sine2);

   // Out of the sine and cosine of the angle between the
   // refraction vector and the normal we can construct the
   // refraction vector itself given two base vectors.
   // These two base vectors are the negative normal and
   // a tangent vector along the path of the incoming vector
   // relative to the surface.
   vec3 x = -normal;
   vec3 y = normalize(cross(cross(viewVec, normal), normal));

   // Refraction
   vec3 refrVec = x * cosine2 + y * sine2;
   vec4 refraction = textureCube(u_scm_skybox, refrVec.xyz);

   // Colors refract differently and the difference is more
   // visible the stronger the refraction. We'll fake this
   // effect by adding some rainbowish colors accordingly.
   vec4 rainbow = texture2D(Rainbow, vec2( pow(cosine, rainbowSpread), 0.0));

   vec4 rain = rainbowScale * rainbow * baseColor;
   vec4 refl = reflectionScale * reflection;
   vec4 refr = refractionScale * refraction * baseColor;

   // There is more light reflected at sharp angles and less
   // light refracted. There is more color separation of refracted
   // light at sharper angles
   gl_FragColor =  sine * refl + (1.0 - sine2) * refr + sine2 * rain + ambient;
}
