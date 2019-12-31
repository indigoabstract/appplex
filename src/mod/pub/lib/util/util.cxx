#include "stdafx.hxx"

#include "util.hxx"
#include "input/transitions.hxx"
#include "mws-mod-ctrl.hxx"


using std::string;
using std::wstring;


float sigmoid(float v, float vmax)
{
   float vmid = vmax / 2.f;
   float i = vmax / 3.f;
   float j = 80.f;
   float r = 100.f / (powf(j, ((vmid + i / 2.f - v) / i)));
   return r;
}

float interpolate_smooth_step(float intervalPosition, float start, float end, int octaveCount)
{
   float v = intervalPosition;

   for (int k = 0; k < octaveCount; k++)
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


slide_scrolling::slide_scrolling(int transitionms)
{
   int tduration[] = { transitionms };

   mslt = ms_linear_transition::nwi(ms_transition_data::new_duration_data(tduration, 1));
   set_scroll_dir(SD_LEFT_RIGHT);
}

bool slide_scrolling::is_finished()
{
   return mslt->is_finished();
}

void slide_scrolling::start()
{
   switch (type)
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

   srcpos = dstpos = glm::vec2(0.f);

   mslt->start();
}

void slide_scrolling::stop()
{
   mslt->stop();

   float x = interpolate_smooth_step(1, pstart, pstop, 0);

   switch (type)
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

   switch (type)
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
