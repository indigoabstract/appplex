#include "stdafx.h"

#include "unit-kinetic-scrolling.hpp"

#ifdef UNIT_KINETIC_SCROLLING

#include "com/mws/mws-camera.hpp"
#include "com/mws/mws.hpp"
#include "com/mws/mws-font.hpp"
#include "com/util/util.hpp"
#include <rng/rng.hpp>
#include <string>
#include <vector>

using std::string;
using std::vector;

const int test = 200;
int crtColor;


void stroke(int s)
{
}

void fill(shared_ptr<mws_camera> g, int f)
{
	g->setColor(f, f, f, 255);
};

void fill(shared_ptr<mws_camera> g, float x, float y, float z)
{
	g->setColor(x, y, z, 255);
}

void line(shared_ptr<mws_camera> g, float x1, float y1, float x2, float y2)
{
	g->drawLine(x1, y1, x2, y2);
};

void rect(shared_ptr<mws_camera> g, float x, float y, float w, float h)
{
	g->fillRect(x, y, w, h);
};

int color(int r, int g, int b, int a)
{
	return (a << 24) | (r << 16) | (g << 8) | b;
}

int textWidth(shared_ptr<mws_camera> g, string& s)
{
	return g->get_font()->get_text_width(s);
}

void text(shared_ptr<mws_camera> g, int num, float x, float y)
{
	g->drawText(trs("{}", num), x, y);
}

void text(shared_ptr<mws_camera> g, string& name, float x, float y)
{
	g->drawText(name, x, y);
}

int constrain(int val, int min, int max)
{
	if (val < min)
	{
		val = min;
	}
	else if (val > max)
	{
		val = max;
	}

	return val;
}



//Grabbing with the mouse, and flicking left or right.  
//There is a reasonable feeling of energy being put in to the scroll effort and then slowing to a stop.
// Matthew Joyce
class kscroll2
{
public:
	int lpos;
	int W;
	int H;
	int units;
	int unitsize;
	int lopx;
	int hipx;
	int tick1;
	int tick2;
	int tick3;

	kscroll2();
	bool isAtStart();
	void gen_random_events(int n);
	void setup(int width, int height);
	void draw(shared_ptr<mws_camera> g);
	void draw_scale(shared_ptr<mws_camera> g);
	void mouseDragged(int iMouseX, int iMouseY);
	void mousePressed(int iMouseX, int iMouseY);
	void mouseReleased(int iMouseX, int iMouseY);

private:
	int mouseX;
	int mouseY;
};


class ksevent {
public:
	float x, y, l, h;
	int col;
	string name;

	void draw(kscroll2& ks, shared_ptr<mws_camera> g, int mouseX, int mouseY){
		int lpos = ks.lpos;

		if (x > lpos - (l*ks.unitsize) && x<lpos + ks.W + (l*ks.unitsize))
		{
			stroke(col);
			fill(g, col);
			rect(g, x - lpos, y, l*ks.unitsize, h);
			fill(g, 0);
			text(g, name, x - lpos, y + 12);

			if (is_mouseover(ks, mouseX, mouseY))
			{
				stroke(200);
				fill(g, 0, 0, 255);
				rect(g, mouseX, mouseY - 20, textWidth(g, name) + 10, 25);
				fill(g, 255);
				text(g, name, mouseX + 5, mouseY);
			}
		}
	}

	bool is_mouseover(kscroll2& ks, int mouseX, int mouseY){
		if (mouseX>x - ks.lpos && mouseX<x - ks.lpos + (l*ks.unitsize) && mouseY>y && mouseY < y + h)
		{
			return true;
		}
		else {
			return false;
		}
	}
};



kinetic_scrolling scroller;
ksevent arrEvent[test];
kscroll2 kscrollinst;

kscroll2::kscroll2()
{
	units = 365;
	unitsize = 50;
	lopx = 0;
	hipx = units * unitsize;
	lpos = lopx;
	tick1 = unitsize;
	tick2 = tick1 / 2;
	tick3 = tick2 / 12;
	mouseX = 0;
	mouseY = 0;
}

bool kscroll2::isAtStart()
{
	return lpos == 0;
}

void kscroll2::gen_random_events(int n)
{
	RNG rng;

	for (int i = 0; i < n; i++)
	{
		ksevent e;
		e.l = 3 + rng.nextInt(10);
		e.h = 5 + rng.nextInt(15);
		e.x = rng.nextInt(hipx - (e.l*unitsize));
		e.y = e.h + rng.nextInt(H - e.h);
		e.name = trs("{}", i);
		e.col = color(rng.nextInt(255), rng.nextInt(255), rng.nextInt(255), rng.nextInt(100));
		arrEvent[i] = e;
	}
};

void kscroll2::setup(int width, int height) {
	W = width;
	H = height;

	gen_random_events(test);

}

void kscroll2::draw(shared_ptr<mws_camera> g) {
	point2d p = scroller.update();
	lpos = constrain(lpos - p.x, lopx, hipx - W);

	for (int i = 0; i < test; i++)
	{
		ksevent e = arrEvent[i];
		e.draw(*this, g, mouseX, mouseY);
	}

	draw_scale(g);
}

void kscroll2::draw_scale(shared_ptr<mws_camera> g)
{
	stroke(255);
	fill(g, 100);
	line(g, 0, H / 2, W, H / 2);

	for (int i = lpos; i < lpos + W; i++)
	{
		float x = i - lpos;
		if (i % tick1 == 0) {
			stroke(200);
			line(g, x, H / 2 + 15, x, H / 2 - 15);
			text(g, i / tick1 + 1, x, H / 2 - 20);
		}
		else if (i % tick2 == 0) {
			stroke(200);
			line(g, x, H / 2 + 10, x, H / 2 - 10);
		}
	}
}


void kscroll2::mouseDragged(int iMouseX, int iMouseY)
{
	mouseX = iMouseX;
	mouseY = iMouseY;
	scroller.begin(mouseX, mouseY);
	lpos = constrain(lpos - int(scroller.get_speed().x), lopx, hipx - W);
}

void kscroll2::mousePressed(int iMouseX, int iMouseY)
{
	mouseX = iMouseX;
	mouseY = iMouseY;
	scroller.grab(mouseX, mouseY);
}

void kscroll2::mouseReleased(int iMouseX, int iMouseY)
{
	mouseX = iMouseX;
	mouseY = iMouseY;
	scroller.start_slowdown();
}




unit_kinetic_scrolling::unit_kinetic_scrolling() : unit(mws_stringify(UNIT_KINETIC_SCROLLING)) {}

shared_ptr<unit_kinetic_scrolling> unit_kinetic_scrolling::nwi()
{
	return shared_ptr<unit_kinetic_scrolling>(new unit_kinetic_scrolling());
}

void unit_kinetic_scrolling::init()
{
	kscrollinst.setup(get_width(), get_height());
}

void unit_kinetic_scrolling::init_mws()
{
	class mainpage : public mws_page
	{
	public:
		mainpage(shared_ptr<mws_page_tab> iparent) : mws_page(iparent){}

		virtual void receive(shared_ptr<mws_dp> idp)
		{
			if (idp->is_processed())
			{
				return;
			}

			if (idp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
			{
				shared_ptr<pointer_evt> ts = pointer_evt::as_pointer_evt(idp);

				int x = ts->points[0].x;
				int y = ts->points[0].y;

				switch (ts->get_type())
				{
				case touch_sym_evt::TS_PRESSED:
					kscrollinst.mousePressed(x, y);
					break;

				case touch_sym_evt::TS_PRESS_AND_DRAG:
					kscrollinst.mouseDragged(x, y);
					break;

				case touch_sym_evt::TS_RELEASED:
					kscrollinst.mouseReleased(x, y);
					break;
				}
			}

			mws_page::receive(idp);
		}

		virtual void update_view(shared_ptr<mws_camera> g)
		{
			mws_page::update_view(g);

			kscrollinst.draw(g);
		}
	};

   mws_root->new_page<mainpage>();
   mws_cam->clear_color = true;
}

#endif
