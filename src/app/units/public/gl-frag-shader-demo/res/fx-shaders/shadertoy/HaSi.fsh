// https://www.shadertoy.com/view/MdsXDB
bool txt = true; // show logo with/without text
bool bow = true; // draw logo black on white/white on black

vec2 size = vec2(13.0, txt?17.0:11.0); // size of logo 'canvas'
float scale = 15.0;

float slice(int id)
{
    if (txt)
    {
		if (id == 0) return 6816064.0;  if (id == 1) return 5247616.0; 
    	if (id == 2) return 10503426.0; if (id == 3) return 8388612.0;
		if (id == 4) return 1910541.0;	if (id == 5) return 7347208.0;
		if (id == 6) return 8953856.0;	if (id == 7) return 2235732.0;
		if (id == 8) return 11165010.0; return 8.0;
    }
    else
    {
        if (id == 0) return 6816064.0;  if (id == 1) return 5247616.0;	
        if (id == 2) return 10503426.0; if (id == 3) return 8388612.0;
        if (id == 4) return 1910541.0;  return 7347208.0;
    } 
}

float sprite(vec2 p)
{
    p = floor(p); // pixelise
    if (p.x < 0.0 || p.y < 0.0 || p.x >= size.x || p.y >= size.y) return float(bow); // clip
	    
    float k = p.x + p.y*size.x; // 'project' from 2d to 1d array
    float n = slice(int(floor(k/24.0))); // select 24-bit slice
   
    int j = int(k)/24; // yes, j is needed
    k = float(int(k) - 24*j); // mod workaround (intel mobile graphics)
	
	return abs(float(bow)-floor(mod(n/exp2(k), 2.0))); // bitwise '&' replacement
}

void main(void)
{
    vec2 p = vec2(gl_FragCoord.x, iResolution.y - gl_FragCoord.y); // invert y axis
	p = (p.xy - 0.5 * (iResolution.xy - size*scale)) / scale; // center and scale
    
    vec3 col = vec3(sprite(p));
	
	gl_FragColor = vec4(col,1.0);
}
