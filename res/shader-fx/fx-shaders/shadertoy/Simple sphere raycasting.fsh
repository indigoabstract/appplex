// https://www.shadertoy.com/view/4dj3W3
float intersect(vec4 lsphere, vec3 ray){
// camera pos is origo
vec4 sphere= lsphere;
float a = dot(ray,ray);
float b = dot(ray, -2.0* sphere.xyz);
float c = dot(sphere.xyz,sphere.xyz)-dot(sphere.a,sphere.a);
float determinant = b*b-4.0*a*c;
if (determinant < 0.0) return -1.0;
float t2 = (-b-sqrt(determinant))/(2.0*a);
if(t2 > 0.001) return t2;
float t1 = (-b+sqrt(determinant))/(2.0*a);
if(t1 > 0.001) return t1;
return -1.0;
}
vec3 F(vec3 normal, vec3 light,vec3 F0){
float cos_a = abs(dot(normal,light));
	if (cos_a < 0.0) return vec3(0.0);
return F0+(vec3(1.0) - F0)*pow(1.0-cos_a,5.0);
}

vec3 calcColor(vec3 view, vec3 normal, vec3 light,float lightintensity){
vec3 nview = normalize(view);
vec3 nnormal = normalize(normal);
vec3 nlight = normalize(light);
vec3 ambient = vec3(0.12,0.12,0.12);
vec3 nhalf = normalize(nview+nlight);
vec3 reflected = reflect(nnormal*-1.0,nlight);
	
return ambient+(max(dot(nnormal,nlight),0.0)*0.3+0.4*pow(max(dot(reflected,nview),0.0),2.0))*lightintensity;
}

void main() {

  vec3 ray = vec3(gl_FragCoord.xy/iResolution.xx,-3.0);
  vec4 light0 = vec4(sin(5.4*cos(iGlobalTime/2.0))*4.0,4.0+sin(5.4*sin(iGlobalTime/2.0))*4.0,5.0,1.0);


  vec4 sphere = vec4(0.5,0.0,-10.0,0.5);
  ray*=vec3(-1.0,-1.0,1.0);
  ray+=vec3(0.5,0.3,0.0);
  gl_FragColor = vec4(vec3(0.0),0.6);
  float t=intersect(sphere,ray);
	if (t > 0.0){

	  // calc intersection point
	  vec3 point = (t+0.0)*ray;
	  // calc normal
	  vec3 normal = point - sphere.xyz;
	  // calc lightvector
	  vec3 lightvector = light0.xyz - point; 
	  gl_FragColor  = vec4(calcColor(-1.0*ray,normal,lightvector,light0.a),1.0);

	}

}
