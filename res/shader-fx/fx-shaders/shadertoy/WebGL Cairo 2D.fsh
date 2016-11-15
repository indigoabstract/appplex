// https://www.shadertoy.com/view/lslXW8
// 2d shape drawing lib
// after cairo API, with AA
// by Leonard Ritter (@paniq)
// v0.2

// i release this into the public domain.

//#define HEROKU
#ifdef HEROKU
#ifdef GL_ES
precision mediump float;
#endif
// compatibility for glsl.heroku.com 
uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;
#define iGlobalTime time
#define iResolution resolution
#endif

vec2 aspect = vec2(iResolution.x / iResolution.y, 1.0);
vec2 uv = gl_FragCoord.xy / iResolution.xy;
vec2 position = (uv*2.0-1.0)*aspect;
float ScreenH = min(iResolution.x,iResolution.y);
float AA = ScreenH*0.4;
float AAINV = 1.0 / AA;

//////////////////////////////////////////////////////////

float det(vec2 a, vec2 b) { return a.x*b.y-b.x*a.y; }

//////////////////////////////////////////////////////////

vec3 hue(float hue) {
    return clamp( 
        abs(mod(hue * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 
        0.0, 1.0);
}

vec3 hsl(float h, float s, float l) {
    vec3 rgb = hue(h);
    return l + s * (rgb - 0.5) * (1.0 - abs(2.0 * l - 1.0));
}

vec4 hsl(float h, float s, float l, float a) {
    return vec4(hsl(h,s,l),a);
}

//////////////////////////////////////////////////////////

#define DEFAULT_SHAPE_V 1e+20

struct Context {
    vec2 position;
    float scale;
    float shape;
    float line_width;
    vec2 blur;
    vec4 source;
    vec2 start_pt;
    vec2 last_pt;
} _stack = Context(
    position, 1.0,
    DEFAULT_SHAPE_V,
    1.0,
    vec2(AA,0.0),
    vec4(vec3(0.0),1.0),
    vec2(0.0),
    vec2(0.0)
);
vec3 _color = vec3(1.0);

Context save() {
    return _stack;
}

void restore(Context ctx) {
    // preserve shape
    float shape = _stack.shape;
    _stack = ctx;
    _stack.shape = shape;
}

mat3 mat2x3_invert(mat3 s)
{
    float d = det(s[0].xy,s[1].xy);
    d = (d != 0.0)?(1.0 / d):d;

    return mat3(
        s[1].y*d, -s[0].y*d, 0.0,
        -s[1].x*d, s[0].x*d, 0.0,
        det(s[1].xy,s[2].xy)*d,
        det(s[2].xy,s[0].xy)*d,
        1.0);
}

void identity_matrix() {
    _stack.position = position;
    _stack.scale = 1.0;
}

void set_matrix(mat3 mtx) {
    mtx = mat2x3_invert(mtx);
    _stack.position = (mtx * vec3(position,1.0)).xy;
    _stack.scale = length(vec2(mtx[0].x,mtx[1].y));
}

void transform(mat3 mtx) {
    mtx = mat2x3_invert(mtx);
    _stack.position = (mtx * vec3(_stack.position,1.0)).xy;
    vec2 u = vec2(mtx[0].x, mtx[1].x);
    _stack.scale *= length(u);
}

void rotate(float a) {
    float cs = cos(a), sn = sin(a);
    transform(mat3(
        cs, sn, 0.0,
        -sn, cs, 0.0,
        0.0, 0.0, 1.0));
}

void scale(float s) {
    transform(mat3(s,0.0,0.0,0.0,s,0.0,0.0,0.0,1.0));
}

void translate(vec2 p) {
    transform(mat3(1.0,0.0,0.0,0.0,1.0,0.0,p.x,p.y,1.0));
}

void translate(float x, float y) { translate(vec2(x,y)); }

void clear() {
    _color = mix(_color, _stack.source.rgb, _stack.source.a);
}

void add(float d) {
    _stack.shape = min(_stack.shape, d / _stack.scale);
}

void blit() {
    gl_FragColor = vec4(_color.xyz, 1.0);
}

void new_path() {
    _stack.shape = DEFAULT_SHAPE_V;
}

void debug_gradient() {
    _color = mix(_color, 
        hsl(_stack.shape * 6.0, 
            1.0, (_stack.shape>=0.0)?0.5:0.3), 
        0.5);
}

void set_blur(float b) {
    if (b == 0.0) {
        _stack.blur = vec2(AA, 0.0);
        return;
    }
    float a = 1.0 / max(AAINV, b);
    _stack.blur = vec2(
        a,
        0.0); // 0 = blur ends at outline, 1 = blur starts at outline
}

void fill_preserve() {
    float w = clamp(-_stack.shape*AA, 0.0, 1.0);
    _color = mix(_color, _stack.source.rgb, w * _stack.source.a);
}

void fill() {
    fill_preserve();
    new_path();
}

void set_line_width(float w) {
    _stack.line_width = w;
}

void set_line_width_px(float w) {
    _stack.line_width = w*_stack.scale/AA;
}

void stroke_preserve() {
    float w = abs(_stack.shape)- _stack.line_width/_stack.scale;
    vec2 blur = _stack.blur;// / _stack.scale;
    w = clamp(-w*blur.x + blur.y, 0.0, 1.0);
    _color = mix(_color, _stack.source.rgb, w * _stack.source.a);
}

void stroke() {
    stroke_preserve();
    new_path();
}

void set_source_rgba(vec4 c) {
    _stack.source = c;
}

void set_source_rgba(float r, float g, float b, float a) { 
    set_source_rgba(vec4(r,g,b,a)); }

void set_source_rgb(vec3 c) {
    set_source_rgba(vec4(c,1.0));
}

void set_source_rgb(float r, float g, float b) { set_source_rgb(vec3(r,g,b)); }

void set_source(sampler2D image) {
    set_source_rgba(texture2D(image, _stack.position));
}

void rectangle(vec2 o, vec2 s) {
    s*=0.5;
    o = o - _stack.position + s;
    vec2 d = abs(o) - s;
    add(min(max(d.x,d.y),0.0) + length(max(d,0.0)));
}

void rectangle(float ox, float oy, float sx, float sy) {
    rectangle(vec2(ox,oy), vec2(sx,sy));
}

void circle(vec2 p, float r) {
    add(length(_stack.position - p) - r);
}
void circle(float x, float y, float r) { circle(vec2(x,y),r); }

void move_to(vec2 p) {
    _stack.start_pt = p;
    _stack.last_pt = p;
}

void move_to(float x, float y) { move_to(vec2(x,y)); }

// stroke only
void line_to(vec2 p) {
    vec2 pa = _stack.position - _stack.last_pt;
    vec2 ba = p - _stack.last_pt;
    float h = clamp(dot(pa, ba)/dot(ba,ba), 0.0, 1.0);
    add(length(pa - ba*h));
    
    _stack.last_pt = p;
}

void line_to(float x, float y) { line_to(vec2(x,y)); }

void close_path() {
    line_to(_stack.start_pt);
}

// from "Random-access rendering of general vector graphics"
// by Nehab and Hoppe
// only quadratic, not cubic
void curve_to(vec2 b1, vec2 b2)
{
    vec2 b0 = _stack.last_pt - _stack.position;
	_stack.last_pt = b2;
    b1 -= _stack.position;
    b2 -= _stack.position;
    float a=det(b0,b2), b=2.0*det(b1,b0), d=2.0*det(b2,b1);
    float f=b*d-a*a;
    vec2 d21=b2-b1, d10=b1-b0, d20=b2-b0;
    vec2 gf=2.0*(b*d21+d*d10+a*d20);
    gf=vec2(gf.y,-gf.x);
    vec2 pp=-f*gf/dot(gf,gf);
    vec2 d0p=b0-pp;
    float ap=det(d0p,d20), bp=2.0*det(d10,d0p);
    float t=clamp((ap+bp)/(2.0*a+b+d), 0.0, 1.0);
    add(length(mix(mix(b0,b1,t),mix(b1,b2,t),t)));
}

void curve_to(float b1x, float b1y, float b2x, float b2y) {
    curve_to(vec2(b1x,b1y),vec2(b2x,b2y));
}

//////////////////////////////////////////////////////////

void paint() {
    float t = iGlobalTime;
    
    // clear screen
    
    set_source_rgb(vec3(0.0,0.0,0.5));
    clear();
    
    // fill circle
    circle(0.0, 0.0, 0.3);
    set_source_rgb(hsl(0.0, 1.0, 0.5));
    fill_preserve(); // don't reset shape
    
    // add another circle
    circle(0.3 + 0.3*(sin(t)*0.5+0.5), 0.0, 0.2);
    // stroke both circles, twice
    set_source_rgb(hsl(0.0, 1.0, 0.3));
    set_line_width(0.04);
    stroke_preserve();
    set_source_rgb(hsl(0.1, 1.0, 0.5));
    set_line_width(0.02);
    stroke();
    
    // shadowed stop sign stroke
    
    move_to(-0.2,0.0);
    line_to(0.2,0.0);
    
    set_source_rgb(vec3(0.0));
    set_line_width(0.05);
    set_blur(0.1);
    stroke_preserve();
    set_blur(0.0);
    
    set_source_rgb(vec3(1.0));
    set_line_width(0.02);
    stroke();
    
    // transformed glowing triangle
    
    // to preserve stroke width, first save context...
    Context ctx = save();
    translate(-1.0, 0.4);
    scale(0.5 * (sin(t)*0.5+0.5));
    rotate(radians(t*30.0));
    move_to(-0.7,-0.7*0.86);
    line_to(0.7,-0.7*0.86);
    line_to(0.0,0.7*0.86);
    close_path();
    // ...then restore to previous transformation
    restore(ctx);   

    // add glow 
    set_source_rgb(hsl(0.52, 1.0, 0.5));
    set_line_width(0.1);
    set_blur(0.2);
    stroke_preserve();
    set_blur(0.0);
    // and stroke   
    set_line_width(0.02);
    set_source_rgb(hsl(0.5, 1.0, 0.5));
    stroke();
    
    // pink alphablended rectangle

    rectangle(0.6,-0.3,0.6,0.8);
    set_source_rgba(hsl(0.9, 1.0, 0.5, 0.5));
    fill_preserve();

    set_line_width(0.02);
    set_source_rgb(hsl(0.9, 1.0, 0.5));
    stroke();
    
    // quadratic bezier spline
    
    translate(-0.8, -0.8);
    move_to(-0.5, mix(0.0,0.5,sin(t)*0.5+0.5));
    curve_to(0.0, mix(0.0,0.5,sin(t*1.2+0.5)*0.5+0.5), 0.5, mix(0.0,0.5,sin(t*0.91+1.0)*0.5+0.5));
	curve_to(1.0, mix(0.0,0.5,sin(t*1.4+0.5)*0.5+0.5), 1.5, mix(0.0,0.5,sin(t*0.71+1.0)*0.5+0.5));
    set_line_width(0.04);
    set_source_rgb(hsl(0.9, 1.0, 0.5));
    stroke_preserve();
    set_line_width(0.02);
    set_source_rgb(hsl(0.9, 1.0, 0.1));
    stroke();
}

//////////////////////////////////////////////////////////

void main( void ) {
    paint();
    
    blit();
}
