// https://www.shadertoy.com/view/Xdf3WN
#define M_PI 3.141592653

mat3 AddRotateX(mat3 m,float angle)
{
	float cosa = cos(angle);
	float sina = sin(angle);
	mat3 mr = mat3(1.0,  0.0,  0.0,
				   0.0,  cosa, sina,
				   0.0,  -sina,cosa);
	return m * mr;
}
mat3 AddRotateY(mat3 m,float angle)
{
	float cosa = cos(angle);
	float sina = sin(angle);
	mat3 mr = mat3(cosa,  0.0, sina,
				   0.0,   1.0, 0.0,
				   -sina, 0.0, cosa);
	return m * mr;
}
mat3 AddRotateZ(mat3 m, float angle)
{
	float cosa = cos(angle);
	float sina = sin(angle);
	mat3 mr = mat3(cosa, sina, 0.0,
				   -sina,cosa, 0.0,
				   0.0,  0.0,  1.0);
	return m * mr;
}

mat3 Euler2Matrix(vec3 euler)//y:heading, x:pitch, z:bank
{
	mat3 m = mat3(1.0,0.0,0.0,
		   0.0,1.0,0.0,
		   0.0,0.0,1.0);
	return AddRotateZ(AddRotateX(AddRotateY(m, euler.y), euler.x), euler.z);
}

vec2 DoRotate(vec2 pos, float angle)
{
	float cosa = cos(angle);
	float sina = sin(angle);
	return vec2(pos.x*cosa - pos.y*sina, pos.x*sina + pos.y*cosa);
}


vec3 Transform(vec3 xyz, vec3 euler_rotate, float scale)
{
	mat3 r = Euler2Matrix(euler_rotate);
	return r * xyz * scale; 
}


bool HitC(vec3 xyz, float depth, float r1, float r2)
{
	if (abs(xyz.z) < depth && xyz.x < 0.0)
	{
		float rr = xyz.x * xyz.x + xyz.y * xyz.y + xyz.z * xyz.z;
		if (rr < r1*r1 && rr> r2 * r2)
		{
			return true;
		}
	}
	return false;
}

bool HitY(vec3 xyz, float depth, float bold, float len)
{
	if (abs(xyz.z) < depth)
	{
		vec2 pos = xyz.xy;
		if (xyz.y>=0.0)
		{
			if (xyz.x>0.0)
			{
				pos = DoRotate(xyz.xy, 0.25 * M_PI);
			}
			else
			{
				pos = DoRotate(xyz.xy, -0.25 * M_PI);
			}
			if (abs(pos.x)<bold && pos.y<len)
			{
				return true;
			}
		}
		else if (abs(pos.x)<bold && pos.y<0.0 && pos.y>-len)
		{
			return true;
		}
	}
	return false;
}


bool HitUnitCircle(vec3 xyz)
{
	if( xyz.x*xyz.x + xyz.y*xyz.y + xyz.z*xyz.z < 1.0)
		return true;
	return false;
}

float RayMarching(vec2 screen_pos, float near, float far)
{
	const int steps = 128;
	float delta = 1.0/float(steps);
	
	
	for (int i=0; i<steps; ++i)
	{
		vec3 pos = vec3(screen_pos, float(i)*delta);
		
		//ortho inv trans
		pos += vec3(0.0, 0.0, near/(far-near));//obj pos
		pos.z *= (far-near);
		
		//view inv trans
		//pass
		
		
		
		//world inv trans
		vec3 pos1 = pos - vec3(-0.5, 0, 1.0);
		float scale = 0.4;
		pos1 = Transform(pos1, vec3(iGlobalTime, iGlobalTime*0.2, iGlobalTime*0.1), 1.0/scale);
		if (HitC(pos1, 0.5, 1.0,0.9))
		//if (HitUnitCircle(pos))
		{
			return 1.0-float(i)*delta;
		}
		
		
		//world inv trans
		vec3 pos2 = pos - vec3(0.5, 0, 1.0);
		float scale2 = 0.4;
		pos2 = Transform(pos2, vec3(0.0, iGlobalTime*0.5, 0.0), 1.0/scale);
		if (HitY(pos2, 0.5, 0.2, 1.0))
		{
			return 1.0-float(i)*delta;
		}
	}
	return -1.0;
}

void main(void)
{
	//view port inv transform
	vec2 pos = (gl_FragCoord.xy -iResolution.xy*0.5) / iResolution.xy * 2.0;
	
	
	float d = RayMarching(pos, 0.0, 2.0);
	
	if (d>=0.0)
	{
		gl_FragColor = vec4(d, 0.0,0.0,1.0);
	}
	else
	{
		gl_FragColor = vec4(0.0,0.0,0.0,1.0);
	}
}
