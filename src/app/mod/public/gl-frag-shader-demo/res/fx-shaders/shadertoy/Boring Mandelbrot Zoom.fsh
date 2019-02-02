// https://www.shadertoy.com/view/XdfSz8
void main(void)
{
	const float zoom_rate = 1.5;
	const float max_zoom = 25.0; // also time spent in each zoom
	const int bailout = 400;
	
	int zoom_idx = int(mod(iGlobalTime, max_zoom * 3.0) / max_zoom);
	
	// select zoom target
	float center_r;
	float center_i;
	if (zoom_idx == 0) {
		center_r = 0.28595756563098457;
		center_i = 0.011516107407704111;
	}
	else if (zoom_idx == 1) {
		center_r = -0.70024170070240843;
		center_i = -0.26508919693469957;
	}
	else {
		center_r = -1.1617553883109397;
		center_i = -0.19740616627583873;
	}

	float zoom = pow(zoom_rate, mod(iGlobalTime, max_zoom));

	float w = float(iResolution.x);
	float h = float(iResolution.y);

	float pos_x = float(gl_FragCoord.x) / w;
	float pos_y = float(gl_FragCoord.y) / h;
	
	float cx = center_r + (pos_x - 0.5) / zoom;
	float cy = center_i + ((pos_y - 0.5) * (h / w)) / zoom;

	float x = 0.0;
	float y = 0.0;
	
	int j;
	for (int i = 0; i < bailout; i++)	{
		float xx = x * x - y * y + cx;
		y = 2.0 * x * y + cy;
		x = xx;
		if (x*x + y*y >= 4.0) {
  			j = i;
			break;
		}
		j = 0;
	}

  	vec3 col;
	float c;
	if (j == 0) {
		c = 0.0;
	}
	else {
		c = float(j) / float(bailout);
	}
	
	if (zoom_idx == 0) {
		col = vec3(3.0 * c, 2.0 * c, 0.0 * c);
	}
	else if (zoom_idx == 1) {
		col = vec3(0.5 * c , 1.5 * c, 1.1 * c);
	}
	else {
		col = vec3(0.0 * c , 2.5 * c, 3.0 * c);
	}
	gl_FragColor = vec4(col, 1.0);
}
