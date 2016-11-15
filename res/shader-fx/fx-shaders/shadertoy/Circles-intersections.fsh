// https://www.shadertoy.com/view/XdBSWh
float Circle(vec2 pix, vec3 C)
{
    float r = length(C.xy - pix);
    float d = abs(r - C.z);  
    return smoothstep(0.03, 0.015, d) + 0.5*smoothstep(0.1, 0.0, r - C.z);
}

float Point(vec2 pix, vec2 X)
{
    float r = length(X - pix);
    return smoothstep(0.04, 0.00, r);
}

/* xy : first intersection point
   zw : second intersection point */
vec4 IntersectCircles(vec3 c1, vec3 c2)
{
	float a = c1.z * c1.z;
    float b = c2.z * c2.z;
    // set c1 to the origin
    c2.xy -= c1.xy;
    
    float z = dot(c2.xy, c2.xy);
    float y = a - b + z;
    float d = sqrt(4.0*a*z - y*y);
    vec2  h = y * c2.xy;
    vec2  j = d * vec2(-c2.y, c2.x);
          
    return vec4(c1.xy, c1.xy) + vec4(h + j, h - j) * (0.5 / z);
}

vec3 Background(vec2 p)
{
 	return length(p) * vec3(0.4 + 0.1*p.y, 0.2 + 0.2*p.y, 0.15*p.x);   
}

vec2 Mouse()
{
    vec2 r = (2.0 * iMouse.xy / iResolution.xy) - 1.0;
    r.x *= iResolution.x / iResolution.y;
    return r;
}

vec3 Scene(vec2 pix)
{
 	vec3 col = Background(pix);
    
    vec3 circle1 = vec3(0.0, -0.3, 0.5);
    vec3 circle2 = vec3(0.2, 0.2, 0.7);   
    circle1.xy = Mouse();	
    circle1.z = 0.3 + abs(0.2*sin(0.5*iGlobalTime));
    
    col += vec3(0.1, 0.2, 0.7) * Circle(pix, circle1);
    col += vec3(0.7, 0.0, 0.3) * Circle(pix, circle2);
    
    vec4 iC = IntersectCircles(circle1, circle2);
    col -= (0.7*col-vec3(0.1, 0.9, 0.7)) * (Point(pix, iC.xy) + Point(pix, iC.zw));
    
    return col;
}

void main(void)
{
	vec2 p = 2.0 * (gl_FragCoord.xy / iResolution.xy) - 1.0;
    p.x *= iResolution.x / iResolution.y;
    
    vec3 col = Scene(p);

    
	gl_FragColor = vec4(col, 1.0);
}
