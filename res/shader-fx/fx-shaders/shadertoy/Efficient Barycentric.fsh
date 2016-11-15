// https://www.shadertoy.com/view/lslXDf
vec3 bary(vec2 a, vec2 b, vec2 c, vec2 p) {
    vec2 v0 = b - a, v1 = c - a, v2 = p - a;
    float inv_denom = 1.0 / (v0.x * v1.y - v1.x * v0.y);
    float v = (v2.x * v1.y - v1.x * v2.y) * inv_denom;
    float w = (v0.x * v2.y - v2.x * v0.y) * inv_denom;
    float u = 1.0 - v - w;
    return abs(vec3(u,v,w));
}

void main() {
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    
    vec2 a = vec2(0.0, 0.0);
    vec2 b = vec2(1.0, 0.0);
    vec2 c = vec2(0.0, 1.0);
    
    vec3 bcc = bary(a, b, c, uv);

    if(bcc.x + bcc.y + bcc.z > 1.0001) {
    	discard;   
    }
    
	gl_FragColor = vec4(bcc, 1.0);
}
