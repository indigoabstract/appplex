// https://www.shadertoy.com/view/MdsSR2
// mimic of game of life evolution rules for gliders
mat3 start;
mat3 end;

vec2 displacementAtStep(float t){
	t = floor(t);
	float tt = mod(t, 4.0);
	vec2 base = vec2(floor(tt/3.0), step(1.0, tt)); 
	return floor(t / 4.0) + base;
}
	
mat3 posAtStep(float t){
	t = floor(t);
	mat3 res;
	t = mod(t, 4.0);
	
	mat3 base;
	
	if(mod(t,2.0) == 0.0) base = start;
	else base = end;
	
	bool do_rotate = t > 1.0;
	
	for(int i = 0; i < 3; ++i){
		for(int j = 0; j < 3; ++j){
			if(do_rotate){
				res[i][j] = base[j][i];	
			}
			else{
				res[i][j] = base[i][j];	
			}
		}
	}
	
	return res;
	
}

float gliderAtCell(vec2 cell, float t, vec2 ini_pos, vec2 grid_size){
	vec2 divs = grid_size;
	mat3 pos = posAtStep(t);
	vec2 glider_pos = mod(ini_pos + displacementAtStep(t), divs);
	
	float i_pos = cell.x - glider_pos.x;
	float j_pos = cell.y - glider_pos.y;
	
	float color = 0.0;
	if(glider_pos.x > divs.x - 3.0 && i_pos < 0.0) i_pos = divs.x + i_pos;
	if(glider_pos.y > divs.y - 3.0 && j_pos < 0.0) j_pos = divs.y + j_pos;
	
	//if(glider_pos.x < 2.0 && i_pos > 22.0) i_pos = 25.0 - i_pos;
	//if(glider_pos.y < 2.0 && j_pos > 22.0) j_pos = 25.0 - j_pos;
	int cellp = int( floor(i_pos) * 3.0 + floor(j_pos) );
	if( i_pos < 3.0 && j_pos < 3.0 && i_pos >= 0.0 && j_pos >= 0.0){
		for(int i = 0; i < 3; ++i){
			for(int j = 0; j < 3; ++j){
				if(3 * i + j > cellp) continue;
				color = pos[j][i];
			}
		}
	}
	return color;
}

void main(void)
{
	start = mat3(0,1,0,
				 0,0,1,
				 1,1,1);

	end = mat3(1,0,1,
			   0,1,1,
			   0,1,0);
	
	vec2 divs = vec2(25, 25);
	divs.x = float( ( int(iResolution.x) * int(divs.y) ) / int(iResolution.y) );  
	
	vec2 cell;
	cell.x = floor(gl_FragCoord.x /(iResolution.x / divs.x));
	cell.y = floor(gl_FragCoord.y /(iResolution.y / divs.y));
	
	float bgcolor = 1.0;
	float glidercolor = 0.0;
	float x = 0.0;
	for(int i = 0; i < 4; i++){
		x += float(i + 1) * 4.0;
		vec2 p = vec2(x, 0.0);
		glidercolor += gliderAtCell(cell, iGlobalTime * 5.0 , p, divs);//, 4);
		p.y = 10.0;
		glidercolor += gliderAtCell(cell, iGlobalTime * 5.0 , p, divs);//, 4);
	}
	
	vec2 cell_size = iResolution.xy / divs;
	vec2 cp = mod(gl_FragCoord.xy, cell_size) / cell_size;
	float dc = length(cp -0.5)+0.5;
	glidercolor -= clamp(0.0, 1.0, dc*dc);
	bgcolor *= length(cp - 0.5);
	bgcolor *= bgcolor;
	
	gl_FragColor = vec4(bgcolor , glidercolor, glidercolor,1.0);
}
