// https://www.shadertoy.com/view/XslSRN
//#define USE_WEBCAM_PLANE // #yolo #swag, uncomment this line to be fabulous

// Cartesian equation in form of coefficients
// ax + by + cz + d = 0
// so normal vector and a displacement coefficient along that vector

vec3 frustum = vec3(1., iResolution.y / iResolution.x, 1.);

vec3 intersec;
vec3 normIntersec;
float t;
vec3 tempIntersec;
vec3 tempNormIntersec;
float tempt;


/* INTERSECTION BETWEEN A PLANE AND A RAY

(P) : normal vector n = (nx, ny, nz), displacement Pd -> nx*x + ny*y + nz*z + Pd = 0
(C) : directional vector u = (ux, uy, uz), passes by point C(Cx, Cy, Cz)
    _
-> /  x = Cx + ux * t
  <|  y = Cy + uy * t
   \_ z = Cz + uz * t

(P)∩(C) is the intersection between the camera ray and the plane, i.e
the point that belongs to both of them.
That means :

nx*x + ny*y + nz*z + Pd = 0

x, y and z are the ray's definitions of x, y and z. So we replace them to get t (the
ray's parameter) :

-> nx * (ux * t + Cx) + ny * (uy * t + Cy) + nz * (uz * t + Cz) + Pd = 0
-> nx * ux * t + nx * Cx + ny * uy * t + ny * Cy + nz * uz * t + nz * Cz = -Pd
-> nx * ux * t + ny * uy * t + nz * uz * t = -Pd - nx * Cx - ny * Cy - nz * Cz
-> t * dot(n, u) = -Pd - dot(n, C)
->
      -Pd - dot(n, C)
  t = ---------------
		 dot(n, u)

Now that we know t, we just replace it in the ray's definition :
-> intersec = (Cabc) * t + (Cxyz)
*/
bool intersecPlane(in vec4 plane, in vec3 rayDir, in vec3 rayOrigin)
{
	if(dot(rayDir, plane.xyz) != 0.)
	{
		// the ray hits, now to find where
		float param = (-plane.w - dot(plane.xyz, rayOrigin)) / dot(plane.xyz, rayDir);
		if(param > 0.)
		{
			tempt = param;
			tempIntersec = rayDir * tempt + rayOrigin;
			tempNormIntersec = plane.xyz;
			return true;
		}
	}
	return false;
}

/* INTERSECTION BETWEEN A SPHERE AND A RAY

(S) : center S(Sx, Sy, Sz), radius Sr : (x-Sx)^2 + (y-Sy)^2 + (z-Sz)^2 = Sr^2
(C) : directional vector u = (ux, uy, uz), passes by point C(Cx, Cy, Cz)

(S)∩(C) is not the intersection between the sphere and the ray, it's actually BOTH intersections.
Since a ray is infinite and a sphere is finite, a ray will traverse a sphere, thus having 2
intersections points : the point where the ray enters the sphere, and the point where it exits the
sphere. As for (P)∩(C), we will search for t, but this time it will be a second degree polynomial.
Since we only want the closest point to the camera, we only take the smallest root, as long as it
remains positive (we don't want to display things if they are behind the camera). Here, we assume
that the camera can't be inside the sphere so that we only have to test one root.

D = C - S = D(Cx - Sx, Cy - Sy, Cz - Sz) = D(Dx, Dy, Dz)

(S)∩(C) : (ux*t + Dx)^2 + (uy*t + Dy)^2 + (uz*t + Dz)^2 = Sr^2
-> ux^2 * t^2 + 2*ux*t*Dx + Dx^2 + uy^2 * t^2 + 2*uy*t*Dy + Dy^2 + uz^2 * t^2 + 2*uz*t*Dz + Dz^2 = Sr^2
-> t^2 (ux^2 + uy^2 + uz^2) + 2t(ux*Dx + uy*Dy + uz*Dz) + Dx^2 + Dy^2 + Dz^2 = Sr^2
-> t^2 * dot(u, u) + 2 * t * dot(u, D) + dot(D, D) - Sr^2 = 0
-> at^2 + bt + c = 0
a = dot(u,u)
b = 2 * dot(u, D)
C = dot(D, D) - Sr^2

Here we just have to resolve a common second degree polynomial.

delta = b^2 - 4 * a * c
If negative : no solution, ie no intersection at all
If zero : one solution, the ray is tangent to the sphere - but that's still an intersection
If positive : two intersections, the ray goes through the sphere, so take the closest point
-> t = (-b - sqrt(delta)) / (2 * a)

If negative, intersection is behind the camera, so discard.
Now that we got t, solve the equation :

intersec = camDir * t + camPos

------

You are most likely to see only b, c and d and not a, like in iq's functions for example. That is because
u (rayDir in my function) is assumed to be normalized, so that the square of it is 1.
*/

bool intersecSphere (in vec4 sphere,in vec3 rayDir,in vec3 rayOrigin) {
	vec3 delta = rayOrigin - sphere.xyz;
	float a = dot(rayDir, rayDir);
	float b = dot(rayDir, delta) * 2.;
	float c = dot(delta, delta) - sphere.w * sphere.w;
	float d = b*b - 4.*a*c;
	if(d > 0.)
	{
		float param = (-b - sqrt(d)) / (a * 2.);
		if(param > 0.)
		{
			tempt = param;
			tempIntersec = rayDir * param + rayOrigin;
			tempNormIntersec = normalize(tempIntersec - sphere.xyz);
			return true;
		}
	}
	return false;
}

void main(void)
{
	// Define objects to be raytraced
	vec4 gplane = vec4(0., 1., 0., 2.);
	vec4 suchSphere = vec4(cos(iGlobalTime) * 2., sin(iGlobalTime) + 1., cos(iGlobalTime) * 10. + 10., 1.);
	
	// Final color
	float shade;
	
	// 2D coordinates
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	// Camera position
	vec3 camPos = vec3(0., 1., 0.); // vec3(sin(iGlobalTime) * 5.,cos(iGlobalTime) * 2. + 1., cos(iGlobalTime) * 5.);
	vec3 camDir = vec3(uv, 1.) * 2. - 1.;
	camDir = normalize(camDir * frustum);
	vec4 light = vec4(0., 10., 10., 30);
	bool hasHitAnything = false;
	bool hasHitPlane = false; // dissociate plane and sphere
	
	// Check if the ray hits the sphere, and where
	if(intersecSphere(suchSphere, camDir, camPos))
	{
		hasHitAnything = true;
		intersec = tempIntersec;
		normIntersec = tempNormIntersec;
		t = tempt;
	}
	
	// check if the ray hits the plane, and where
	
	if(intersecPlane(gplane, camDir, camPos))
	{
		if(!hasHitAnything || tempt < t)
		{
			hasHitPlane = true;
			hasHitAnything = true;
			intersec = tempIntersec;
			normIntersec = tempNormIntersec;
			t = tempt;
		}
	}
	
	if(hasHitAnything)
	{
		vec3 lightTranspose = light.xyz - intersec;
		
		if(dot(lightTranspose, lightTranspose) > light.w * light.w)
			shade = 0.;
		else
		{
			// Currently in the process of understanding this formula
			shade = dot(normIntersec, normalize(lightTranspose)) * (1. - length(lightTranspose) / light.w);
			
			if(intersecSphere(suchSphere, normalize(light.xyz - intersec), intersec))
				// dumb projection on the plane, only correct if this latter is horizontal
				// I need an orthogonal projection of a point on a plane using the plane's normal
				// but can't think of a way to get it
				// theorically, let I be the orth. proj. of the intersection and S the orth. proj.
				// of the sphere's center, both on the plane :
				// shade *= distance(I, S) / sphere.w;
				shade *= distance(tempIntersec.xz, suchSphere.xz) / suchSphere.w;
			#ifdef USE_WEBCAM_PLANE
			gl_FragColor = hasHitPlane ? texture2D(iChannel0, vec2(1. - uv.x, uv.y * 2.)) * shade : vec4(shade, shade, shade, 1.);
			#else
			gl_FragColor = vec4(shade, shade, shade, 1.);
			#endif
		}
	}
	else
		gl_FragColor = vec4(0., 0., abs(uv.y - .25), 1.);
}
