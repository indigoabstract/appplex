// https://www.shadertoy.com/view/Xdl3zn
struct Circle
{
	vec2 pos;
	float rad;
	vec4 speed;
	vec4 col;
};
	
Circle circles[32];

void createCirlces(void)
{
	circles[0].pos = vec2(0.184783, 0.674095);
	circles[0].rad = 0.010634;
	circles[0].speed = vec4(0.059360, 11.913880, 0.045413, 0.073503);
	circles[0].col = vec4(0.300446, 0.479247, 0.106032, 0.281362);

	circles[1].pos = vec2(0.060074, 0.732232);
	circles[1].rad = 0.014220;
	circles[1].speed = vec4(0.110291, 19.026543, 0.019721, 0.055908);
	circles[1].col = vec4(0.161555, 0.354508, 0.207343, 0.100299);

	circles[2].pos = vec2(0.666543, 0.828674);
	circles[2].rad = 0.071724;
	circles[2].speed = vec4(0.041815, 2.442495, 0.008198, 0.063925);
	circles[2].col = vec4(0.306825, 0.444590, 0.588202, 0.313171);

	circles[3].pos = vec2(0.753809, 0.680146);
	circles[3].rad = 0.041653;
	circles[3].speed = vec4(0.070356, 1.058587, 0.005661, 0.033363);
	circles[3].col = vec4(0.673544, 0.727912, 0.888115, 0.527714);

	circles[4].pos = vec2(0.620868, 0.707706);
	circles[4].rad = 0.016268;
	circles[4].speed = vec4(0.113612, 7.114663, 0.036755, 0.058647);
	circles[4].col = vec4(0.189830, 0.100602, 0.563803, 0.023596);

	circles[5].pos = vec2(0.041932, 0.561947);
	circles[5].rad = 0.016370;
	circles[5].speed = vec4(0.095721, 17.507710, 0.001764, 0.066960);
	circles[5].col = vec4(0.435214, 0.022308, 0.667905, 0.740477);

	circles[6].pos = vec2(0.828362, 0.722690);
	circles[6].rad = 0.010030;
	circles[6].speed = vec4(0.196480, 2.744880, 0.004094, 0.034335);
	circles[6].col = vec4(0.431570, 0.145670, 0.309245, 0.061200);

	circles[7].pos = vec2(0.247999, 0.274433);
	circles[7].rad = 0.019116;
	circles[7].speed = vec4(0.093826, 15.075845, 0.021061, 0.060139);
	circles[7].col = vec4(0.543448, 0.805997, 0.292827, 0.189342);

	circles[8].pos = vec2(0.907249, 0.076840);
	circles[8].rad = 0.026943;
	circles[8].speed = vec4(0.079762, 0.643762, 0.038929, 0.075484);
	circles[8].col = vec4(0.329489, 0.598485, 0.443099, 0.303182);

	circles[9].pos = vec2(0.192826, 0.870024);
	circles[9].rad = 0.051593;
	circles[9].speed = vec4(0.047412, 17.878843, 0.031734, 0.097182);
	circles[9].col = vec4(0.531992, 0.836943, 0.003187, 0.611026);

	circles[10].pos = vec2(0.206192, 0.699912);
	circles[10].rad = 0.023598;
	circles[10].speed = vec4(0.159395, 7.577319, 0.046251, 0.087692);
	circles[10].col = vec4(0.615181, 0.084296, 0.861626, 0.682288);

	circles[11].pos = vec2(0.179307, 0.749848);
	circles[11].rad = 0.034115;
	circles[11].speed = vec4(0.182594, 9.358456, 0.010732, 0.093242);
	circles[11].col = vec4(0.546029, 0.119802, 0.891114, 0.363523);

	circles[12].pos = vec2(0.550995, 0.345567);
	circles[12].rad = 0.056545;
	circles[12].speed = vec4(0.132849, 11.512289, 0.033021, 0.062413);
	circles[12].col = vec4(0.391815, 0.905318, 0.833048, 0.134351);

	circles[13].pos = vec2(0.856189, 0.710541);
	circles[13].rad = 0.074435;
	circles[13].speed = vec4(0.172095, 14.280527, 0.038392, 0.036979);
	circles[13].col = vec4(0.548681, 0.541741, 0.496457, 0.454876);

	circles[14].pos = vec2(0.998798, 0.999734);
	circles[14].rad = 0.016081;
	circles[14].speed = vec4(0.053323, 12.044764, 0.028780, 0.031195);
	circles[14].col = vec4(0.614469, 0.122868, 0.409124, 0.595180);

	circles[15].pos = vec2(0.683893, 0.528853);
	circles[15].rad = 0.050703;
	circles[15].speed = vec4(0.196596, 8.881190, 0.004109, 0.043887);
	circles[15].col = vec4(0.730020, 0.735296, 0.966182, 0.598377);

	circles[16].pos = vec2(0.147074, 0.190533);
	circles[16].rad = 0.044475;
	circles[16].speed = vec4(0.115804, 2.688334, 0.044752, 0.072971);
	circles[16].col = vec4(0.331319, 0.057126, 0.841620, 0.954216);

	circles[17].pos = vec2(0.477116, 0.089558);
	circles[17].rad = 0.029295;
	circles[17].speed = vec4(0.199472, 6.169600, 0.011565, 0.035789);
	circles[17].col = vec4(0.437758, 0.337002, 0.806245, 0.770872);

	circles[18].pos = vec2(0.476771, 0.277984);
	circles[18].rad = 0.011655;
	circles[18].speed = vec4(0.046959, 6.352234, 0.012633, 0.042788);
	circles[18].col = vec4(0.675581, 0.006730, 0.108737, 0.816935);

	circles[19].pos = vec2(0.899667, 0.157948);
	circles[19].rad = 0.010387;
	circles[19].speed = vec4(0.069773, 13.920276, 0.036508, 0.095677);
	circles[19].col = vec4(0.617486, 0.406893, 0.794381, 0.126203);

	circles[20].pos = vec2(0.975675, 0.725997);
	circles[20].rad = 0.030177;
	circles[20].speed = vec4(0.122421, 1.331355, 0.045944, 0.049120);
	circles[20].col = vec4(0.167391, 0.672596, 0.250681, 0.370043);

	circles[21].pos = vec2(0.266706, 0.999846);
	circles[21].rad = 0.026530;
	circles[21].speed = vec4(0.051457, 6.337456, 0.039916, 0.040136);
	circles[21].col = vec4(0.506839, 0.378313, 0.719446, 0.090986);

	circles[22].pos = vec2(0.123542, 0.227158);
	circles[22].rad = 0.014531;
	circles[22].speed = vec4(0.040700, 10.315736, 0.045837, 0.065453);
	circles[22].col = vec4(0.890885, 0.140694, 0.877209, 0.538920);

	circles[23].pos = vec2(0.562675, 0.704033);
	circles[23].rad = 0.057699;
	circles[23].speed = vec4(0.112986, 17.283276, 0.049629, 0.047374);
	circles[23].col = vec4(0.892811, 0.812185, 0.107720, 0.156548);

	circles[24].pos = vec2(0.533912, 0.851256);
	circles[24].rad = 0.020802;
	circles[24].speed = vec4(0.189053, 18.590647, 0.003504, 0.043435);
	circles[24].col = vec4(0.212214, 0.861233, 0.251263, 0.043951);

	circles[25].pos = vec2(0.112521, 0.717123);
	circles[25].rad = 0.020963;
	circles[25].speed = vec4(0.173806, 9.059398, 0.040831, 0.078128);
	circles[25].col = vec4(0.415915, 0.208088, 0.089269, 0.822717);

	circles[26].pos = vec2(0.338611, 0.329841);
	circles[26].rad = 0.033770;
	circles[26].speed = vec4(0.084999, 19.361185, 0.031563, 0.076479);
	circles[26].col = vec4(0.484829, 0.251210, 0.505528, 0.488991);

	circles[27].pos = vec2(0.267784, 0.835757);
	circles[27].rad = 0.062144;
	circles[27].speed = vec4(0.160751, 14.861016, 0.038048, 0.065967);
	circles[27].col = vec4(0.703981, 0.295035, 0.152670, 0.902072);

	circles[28].pos = vec2(0.497819, 0.996549);
	circles[28].rad = 0.062501;
	circles[28].speed = vec4(0.131933, 4.648332, 0.008874, 0.095392);
	circles[28].col = vec4(0.574385, 0.932725, 0.199568, 0.065113);

	circles[29].pos = vec2(0.751018, 0.462279);
	circles[29].rad = 0.064170;
	circles[29].speed = vec4(0.045587, 15.146783, 0.037428, 0.073671);
	circles[29].col = vec4(0.506473, 0.429928, 0.686530, 0.930379);

	circles[30].pos = vec2(0.389404, 0.301199);
	circles[30].rad = 0.070333;
	circles[30].speed = vec4(0.143132, 9.030219, 0.034821, 0.034135);
	circles[30].col = vec4(0.710611, 0.335499, 0.313997, 0.916919);

	circles[31].pos = vec2(0.768014, 0.239820);
	circles[31].rad = 0.010123;
	circles[31].speed = vec4(0.083778, 16.663416, 0.016606, 0.077051);
	circles[31].col = vec4(0.428833, 0.050312, 0.234462, 0.899905);
}

void updateCircles(void)
{
	for(int i=0; i<32; i++)
	{
		float y = circles[i].pos.y + iGlobalTime * circles[i].speed.x;
		float x = circles[i].pos.x + sin(y * circles[i].speed.y) * circles[i].speed.z + iGlobalTime * circles[i].speed.w;
		x = mod(x, 1.0);
		y = mod(y, 1.0);
		circles[i].pos.x = (x - 0.2) * 1.2;
		circles[i].pos.y = (y - 0.2) * 1.2;
	}
}

float isInside(in vec2 cp, in float cr, in vec2 p)
{
	float dist = distance(cp, p);
	float alpha = 1.0 - smoothstep(cr, cr + 0.01, dist);
	
	float fade = smoothstep(0.0, 0.1, cp.x);
	fade *= 1.0 - smoothstep(0.9, 1.0, cp.x);
	
	fade *= smoothstep(0.0, 0.1, cp.y);
	fade *= 1.0 - smoothstep(0.9, 1.0, cp.y);
	
	return alpha * fade;
}

float intersectCircle(in vec2 cp, in float cr, in vec2 p)
{
	float alpha = isInside(cp, cr, p);
	
	vec2 pair_cp = vec2(1.0 - cp.x, 1.0 - cp.y);
	alpha += isInside(pair_cp, cr * 1.2, p);
	
	return alpha;
}

void main(void)
{
	createCirlces();
	updateCircles();
	
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv.y = 1.0 - uv.y;
	float aspect = iResolution.y / iResolution.x;
	uv.y *= aspect;
	uv.y += aspect * 0.5;

	float resolution = 10.0;
	vec2 backUv = floor(uv * resolution) / resolution;
	
	vec3 color = vec3(backUv,0.5+0.5*sin(iGlobalTime)) * 0.4;
	for(int i=0; i<32; i++)
	{
		float a = intersectCircle(circles[i].pos, circles[i].rad, uv);
		color += circles[i].col.rgb * a * circles[i].col.a;
		//color = mix(color, circles[i].col.rgb, a * circles[i].col.a);
	}

	gl_FragColor = vec4(color,1.0);
}
