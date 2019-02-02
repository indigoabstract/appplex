#include "stdafx.hxx"

#include "util.hxx"
#include "com/mod/transitions.hxx"
#include "mws-mod-ctrl.hxx"


using std::string;
using std::wstring;


float sigmoid(float v, float vmax)
{
	float vmid=vmax/2;
	float i=vmax/3;
	int j=80;
	float r=100.f / ( powf(j,((vmid+i/2.f-v)/i))) ;
	return r;
}

float inverse(float v,float vmax)
{
	float i=v/vmax;
	float r=1+powf((1-i),3)*-1; 
	return r;
}

float interpolate_smooth_step(float intervalPosition, float start, float end, int octaveCount)
{
	float v = intervalPosition;

	for(int k = 0; k < octaveCount; k++)
	{
		v = smooth_step(v);
	}

	return (end * v) + (start * (1 - v));
}

//void draw_border_rect(mws_sp<mws_camera> g, float x, float y, float w, float h, float borderSize, int borderColor)
//{
//	if(w <= 0 || h <= 0)
//	{
//		return;
//	}
//
//	g->setColor(borderColor);
//
//	float hbs = borderSize / 2;
//
//	// left border
//	g->fillRect(x - hbs, y, borderSize, h);
//	// bottom border
//	g->fillRect(x - hbs, y - hbs + h, w + borderSize, borderSize);
//	// right border
//	g->fillRect(x - hbs + w, y, borderSize, h);
//	// top border
//	g->fillRect(x - hbs, y - hbs, w + borderSize, borderSize);
//}
//
//void fill_border_rect(mws_sp<mws_camera> g, float x, float y, float w, float h, float borderSize, int fillColor, int borderColor)
//{
//	if(w <= 0 || h <= 0)
//	{
//		return;
//	}
//
//	g->setColor(fillColor);
//	g->fillRect(x, y, w, h);
//
//	if(borderSize > 0)
//	{
//		draw_border_rect(g, x, y, w, h, borderSize, borderColor);
//	}
//}


kinetic_scrolling::kinetic_scrolling()
{
	reset();
}

// decrease scrolling speed once mouse is released
point2d kinetic_scrolling::update()
{
	point2d d(0, 0);

	if(active)
	{
		// if there is still some scrolling energy
		if(decay > 0)
		{
         uint32 delta_t = pfm::time::get_time_millis() - decay_start;
			//i tried a few slowing formulas
			decay = 1.f - inverse(delta_t, decay_maxmillis);
			
			//decay = 1 - inverse(mws_mod_ctrl::get_current_mod()->update_ctrl->getTime() - decay_start, decay_maxmillis);

			//decay = 1 - sigmoid(pfm::getCurrentTime() - decay_start, decay_maxmillis);
			//decay=decay*.9;
			d.x = speedx * decay;
			d.y = speedy * decay;
		}
		else
		{
			decay = 0;
			active = false;
		}
	}

	return d;
}

void kinetic_scrolling::begin(float x, float y)
{
	// the initial scrolling speed is the speed it was being dragged
	speedx = x-startx;
	speedy = y-starty;
	startx = x;
	starty = y;
}

void kinetic_scrolling::grab(float x, float y)
{
	reset();
	// while grabbing is occuring, the scrolling link to mouse movement
	startx = x;
	starty = y;
}

void kinetic_scrolling::start_slowdown()
{
	// the mouse has been released and we can start slowing the scroll
	// the speed starts at 100% of the current scroll speed
	// record the time so we can calualte the decay rate

	if(speedx != 0 || speedy != 0)
	{
		decay = 1;
		decay_start = pfm::time::get_time_millis();
		
		//decay_start = mws_mod_ctrl::get_current_mod()->update_ctrl->getTime();
		active = true;
	}
}

void kinetic_scrolling::reset()
{
	speedx = 0;
	speedy = 0;
	decay_maxmillis = 3500;
	decay = 0;
	active = false;
}

bool kinetic_scrolling::is_active()
{
	return active;
}

point2d kinetic_scrolling::get_speed()
{
	return point2d(speedx, speedy);
}


slide_scrolling::slide_scrolling(int transitionms)
{
	int tduration[] = {transitionms};

	mslt = ms_linear_transition::nwi(ms_transition_data::new_duration_data(tduration, 1));
	set_scroll_dir(SD_LEFT_RIGHT);
}

bool slide_scrolling::is_finished()
{
	return mslt->is_finished();
}

void slide_scrolling::start()
{
	switch(type)
	{
	case SD_LEFT_RIGHT:
		pstart = 0;
		pstop = 1;
		break;

	case SD_RIGHT_LEFT:
		pstart = 0;
		pstop = -1;
		break;

	case SD_UP_DOWN:
		pstart = 0;
		pstop = 1;
		break;

	case SD_DOWN_UP:
		pstart = 0;
		pstop = -1;
		break;
	}

	srcpos = dstpos = point2d(0, 0);

	mslt->start();
}

void slide_scrolling::stop()
{
	mslt->stop();

	float x = interpolate_smooth_step(1, pstart, pstop, 0);

	switch(type)
	{
	case SD_LEFT_RIGHT:
	case SD_RIGHT_LEFT:
		srcpos.x = x;
		dstpos.x = x - pstop;
		break;

	case SD_UP_DOWN:
	case SD_DOWN_UP:
		srcpos.y = x;
		dstpos.y = x - pstop;
		break;
	}
}

void slide_scrolling::update()
{
	mslt->update();

	float x = interpolate_smooth_step(mslt->get_position(), pstart, pstop, 0);

	switch(type)
	{
	case SD_LEFT_RIGHT:
	case SD_RIGHT_LEFT:
		srcpos.x = x;
		dstpos.x = x - pstop;
		break;

	case SD_UP_DOWN:
	case SD_DOWN_UP:
		srcpos.y = x;
		dstpos.y = x - pstop;
		break;
	}
}

const mws_sp<ms_linear_transition> slide_scrolling::get_transition()
{
	return mslt;
}

void slide_scrolling::set_scroll_dir(scroll_dir itype)
{
	type = itype;
}


ia_console::ia_console()
{
	//lines = mws_sp<boost::circular_buffer<text_line> >(new boost::circular_buffer<text_line>(5));
}

void ia_console::add_line(const std::string& iline)
{
	text_line line;

	line.line = mws_sp<string>(new string(iline));
	//lines->push_back(line);
}

void ia_console::clear()
{
	//lines->clear();
}

//void ia_console::draw(mws_sp<mws_camera> g)
//{
//	int size = lines->size();
//	float text_dy = get_text_height(g->getFont(), "Mp");
//	text_dy *= 1.4f;
//
//	for(int k = size - 1, l = 0; k >= 0; k--, l++)
//	{
//		text_line& line = lines->at(k);
//		int y_off = text_dy / 2 + l * text_dy;
//
//		if(line.line)
//		{
//			g->drawText(*line.line, 10, pfm::screen::get_height() - y_off);
//		}
//		else
//		{
//			g->drawText(*line.wline, 10, pfm::screen::get_height() - y_off);
//		}
//	}
//}
