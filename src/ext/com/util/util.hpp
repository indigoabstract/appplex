#pragma once

#include "pfm.hpp"
#include <string>
#include <vector>


class ms_linear_transition;
class ux_font;


class point2d
{
public:
	point2d(){x = y = 0;}
	point2d(float iX, float iY) : x(iX), y(iY){}
	void set(float iX, float iY){x = iX; y= iY;}

	float x;
	float y;
};

class kinetic_scrolling
{
public:
	kinetic_scrolling();
	point2d update();
	void begin(float x, float y);
	void grab(float x, float y);
	void start_slowdown();
	void reset();

	bool is_active();
	point2d get_speed();

private:
	float speedx;
	float speedy;
	float startx;
	float starty;
	float decay;
	float decay_start;
	int decay_maxmillis;
	bool active;
};


class slide_scrolling
{
public:
	enum scroll_dir
	{
		SD_LEFT_RIGHT,
		SD_RIGHT_LEFT,
		SD_UP_DOWN,
		SD_DOWN_UP,
	};

	slide_scrolling(int transitionms);

	bool is_finished();
	void start();
	void stop();
	void update();

	const shared_ptr<ms_linear_transition> get_transition();
	void set_scroll_dir(scroll_dir itype);

	point2d srcpos;
	point2d dstpos;

private:
	shared_ptr<ms_linear_transition> mslt;
	scroll_dir type;
	float pstart, pstop;
};


class ia_console
{
public:
	struct text_line
	{
		shared_ptr<std::string> line;
		shared_ptr<std::wstring> wline;
	};

	ia_console();
	virtual ~ia_console(){}

	void add_line(const std::string& iline);
	void add_line(const std::wstring& iline);
	void clear();
	//void draw(shared_ptr<ux_camera> g);

	//shared_ptr<boost::circular_buffer<text_line> > lines;
};

float sigmoid(float v, float vmax);
float inverse(float v,float vmax);

inline float smooth_step(float x)
{
	return sqrtf(x);
	//return (x * x * (3 - 2 * x));
}

float interpolate_smooth_step(float intervalPosition, float start, float end, int octaveCount);
//void draw_border_rect(shared_ptr<ux_camera> g, float x, float y, float w, float h, float borderSize, int borderColor);
//void fill_border_rect(shared_ptr<ux_camera> g, float x, float y, float w, float h, float borderSize, int fillColor, int borderColor);
//void draw_bar(shared_ptr<ux_camera> g, int x, int y, int w, int h, int color);
