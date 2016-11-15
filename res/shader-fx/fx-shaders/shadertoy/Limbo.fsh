// https://www.shadertoy.com/view/4ssXzs
#define BACKGROUND_BLUR_ITER 4.0
#define BACKGROUND_BLUR_SIZE 5.0
#define FOREGROUND_BLUR_ITER 4.0
#define FOREGROUND_BLUR_SIZE 10.0


#define WATER_LEVEL 0.2

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float hash1( float n ){
    return fract(sin(n)*43758.5453123);
}

float terrainHeight(float x){
	float lowF = sin(x+sin(x*3.5)) * 0.60;
	float medF = sin(x*10.0+cos(x*7.7)) * 0.05;
	float highF = sin(x*20.0 + sin(x*70.5)) * 0.015;
	return (lowF + medF + highF) / 4.0 + 0.26;
}

float vignette(vec2 uv){	
	vec2 delta = uv - vec2(iResolution.x / iResolution.y / 2.0, 0.5);
    delta.y *= 3.0;
	return 1.0 - dot(delta,delta) * 0.4;
}

float getBlade(vec2 xy, float seed, float spread, float height){
	float nearestBlade = (floor(xy.x * spread)) / spread;
	float offset = hash1(nearestBlade * seed);
	float base = nearestBlade + offset / spread / 2.0 + 0.25 / spread;
	
	float distFromCenter = abs(xy.x - base - 50.0 * xy.y * (nearestBlade + 0.5 / spread - base));
	float smooth = 1.0 - pow(1.0 - smoothstep(0.000, 0.011, distFromCenter), 5.0);
	return mix(pow(clamp((height - xy.y) / height, 0.0, 1.0), 2.0), 0.0, smooth);
}

float getTree(vec2 xy, float seed, float fade, float spread){
	float nearestBlade = (floor(xy.x * spread)) / spread;
	float offset = (hash1(nearestBlade * seed) - 0.5) * 0.5 / spread + 0.5 / spread;
	float base = nearestBlade + offset;
	
	float distFromCenter = abs(xy.x - base);
    float width = 0.15 + (0.02 / (xy.y + 0.1));
    return 1.0 - smoothstep(width - fade, width, distFromCenter * spread * 2.0);
}

float backgroundTerrainSample(vec2 uv){
	float globalX1 = (uv.x + iGlobalTime / 12.0 + 100.0);
	float height1 = terrainHeight(globalX1) + 0.3;
	float globalX2 = (uv.x + iGlobalTime / 17.0 + 50.0);
	float height2 = terrainHeight(globalX2) + 0.05;
    
    float tree1 = getTree(vec2(uv.x + iGlobalTime / 12.0, height1), 59.0, 0.07, 1.8);
    float tree2 = getTree(vec2(uv.x + iGlobalTime / 17.0, height2), 23.0, 0.07, 1.2);
    
    float back0 = mix(0.5, 1.0, clamp(-tree2 + smoothstep(height2, height2 + 0.15, uv.y), 0.0, 1.0));
    float back1 = mix(0.2, back0, clamp(-tree1 + smoothstep(height2, height2 + 0.15, uv.y), 0.0, 1.0));
    
    return back1;
}

float background(vec2 uv){
	float sum = 0.0;
	for(float dx = -BACKGROUND_BLUR_ITER; dx <= BACKGROUND_BLUR_ITER; dx++){
		sum += backgroundTerrainSample(uv + vec2(dx, 0) * BACKGROUND_BLUR_SIZE / iResolution.y);
	}
	sum /= BACKGROUND_BLUR_ITER*2.0 + 1.0;
	
	float color = sum;
	
	float globalX = (uv.x + iGlobalTime / 10.0);
	float waterSurface = clamp(sin(uv.y * 500.0 + 5.0 * sin(globalX * 3.0)) / 2.0 + 0.5, 0.0, 1.0) / 4.0;
	color += mix(waterSurface, 0.0, clamp(abs(uv.y - WATER_LEVEL) * 80.0, 0.0, 1.0));
	color += (1.0 - smoothstep(WATER_LEVEL - 0.01, WATER_LEVEL + 0.01, uv.y)) * 0.12;
	color = mix(0.0, color, smoothstep(0.04, WATER_LEVEL, uv.y));
	
	return color * vignette(uv);
}


vec2 terrain(vec2 uv){
	float globalX = uv.x + iGlobalTime / 10.0;
	float height = terrainHeight(globalX);
	
	float grassHeight = (sin(globalX * 8.0 + 5.0 * sin(globalX * 2.0)) - 0.3) * 0.05;
	grassHeight = mix(-0.11, grassHeight, smoothstep(-0.1, 0.1, height - WATER_LEVEL));
	
	
	float grassAlpha = 0.0;
	vec2 bladeUV = vec2(globalX, uv.y - height);
	
	if(uv.y < grassHeight + height + 0.1){
		if(uv.y > height){
			grassAlpha += getBlade(bladeUV, 9.0,  30.0, max(0.0, grassHeight + 0.08));
			grassAlpha += getBlade(bladeUV, 99.0, 295.0, max(0.0, grassHeight + 0.07));
			grassAlpha += getBlade(bladeUV, 33.0, 40.0, max(0.0, grassHeight + 0.06));
			grassAlpha += getBlade(bladeUV, 44.0, 45.0, max(0.0, grassHeight + 0.05));
			grassAlpha += getBlade(bladeUV, 5.0,  50.0, max(0.0, grassHeight + 0.09));
			grassAlpha += getBlade(bladeUV, 6.0,  55.0, max(0.0, grassHeight + 0.07));
			grassAlpha += getBlade(bladeUV, 7.0,  60.0, max(0.0, grassHeight + 0.06));
			grassAlpha += getBlade(bladeUV, 8.0,  65.0, max(0.0, grassHeight + 0.10));
			grassAlpha = clamp(grassAlpha, 0.0, 1.0);
		}
	}
	
	float terrainAlpha = 1.0 - smoothstep(height, height + 0.007, uv.y);
    
    grassAlpha += getTree(bladeUV, 22.0, 0.05, 0.7);
	
	return vec2(0.0, clamp(grassAlpha + terrainAlpha, 0.0, 1.0));
}

vec2 foregroundTerrainSample(vec2 uv){
	float globalX = (uv.x + iGlobalTime / 2.0 + 23.0);
	float height = terrainHeight(globalX) - 0.5;
    
	return vec2(0.5, 0.3 - smoothstep(height, height + 1.0, uv.y) * 0.3);
}

vec2 foreground(vec2 uv){
	vec2 sum = vec2(0.0, 0.0);
	for(float dx = -FOREGROUND_BLUR_ITER; dx <= FOREGROUND_BLUR_ITER; dx++){
		sum += foregroundTerrainSample(uv + vec2(dx, 0) * FOREGROUND_BLUR_SIZE / iResolution.y);
	}
	sum /= FOREGROUND_BLUR_ITER*2.0 + 1.0;
	
    sum.y = clamp(sum.y, 0.0, 0.8);
    
	return sum;
}

float scene(vec2 uv){
	float color = background(uv);
	vec2 terrainColor = terrain(uv);
	color = mix(color, terrainColor.x, terrainColor.y);
	vec2 foregroundColor = foreground(uv);
	color = mix(color, foregroundColor.x, foregroundColor.y);
	return color;
}


void main(void){
	vec2 uv = gl_FragCoord.xy / iResolution.y;
	float color = scene(uv) + rand(uv * iGlobalTime) * 0.06;
	color *= smoothstep(0.0, 7.0, iGlobalTime);
	gl_FragColor = vec4(color, color, color, 1);
}
