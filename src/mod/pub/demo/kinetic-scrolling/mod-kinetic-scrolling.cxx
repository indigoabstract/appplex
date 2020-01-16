#include "stdafx.hxx"

#include "mod-kinetic-scrolling.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws.hxx"
#include "mws/mws-font.hxx"
#include "util/util.hxx"
#include "rng/rng.hxx"
#include <string>
#include <vector>

using std::string;
using std::vector;

const int test = 200;
int crtColor;


void stroke(int s)
{
}

void fill(mws_sp<mws_camera> g, int f)
{
	g->set_color(gfx_color(f, f, f, 255));
};

void fill(mws_sp<mws_camera> g, int x, int y, int z)
{
	g->set_color(gfx_color(x, y, z, 255));
}

void line(mws_sp<mws_camera> g, float x1, float y1, float x2, float y2)
{
	g->drawLine(x1, y1, x2, y2);
};

void rect(mws_sp<mws_camera> g, float x, float y, float w, float h)
{
	g->fillRect(x, y, w, h);
};

int color(int r, int g, int b, int a)
{
	return (a << 24) | (r << 16) | (g << 8) | b;
}

int textWidth(mws_sp<mws_camera> g, string& s)
{
	return g->get_font()->get_text_width(s);
}

void text(mws_sp<mws_camera> g, int num, float x, float y)
{
	g->drawText(trs("{}", num), x, y);
}

void text(mws_sp<mws_camera> g, string& name, float x, float y)
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
	void draw(mws_sp<mws_camera> g);
	void draw_scale(mws_sp<mws_camera> g);
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

	void draw(kscroll2& ks, mws_sp<mws_camera> g, int mouseX, int mouseY){
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

void kscroll2::draw(mws_sp<mws_camera> g) {
	glm::vec2 p = scroller.update();
	lpos = constrain(lpos - p.x, lopx, hipx - W);

	for (int i = 0; i < test; i++)
	{
		ksevent e = arrEvent[i];
		e.draw(*this, g, mouseX, mouseY);
	}

	draw_scale(g);
}

void kscroll2::draw_scale(mws_sp<mws_camera> g)
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




mod_kinetic_scrolling::mod_kinetic_scrolling() : mws_mod(mws_stringify(MOD_KINETIC_SCROLLING)) {}

mws_sp<mod_kinetic_scrolling> mod_kinetic_scrolling::nwi()
{
	return mws_sp<mod_kinetic_scrolling>(new mod_kinetic_scrolling());
}

void mod_kinetic_scrolling::init()
{
	kscrollinst.setup(get_width(), get_height());
}

void mod_kinetic_scrolling::init_mws()
{
	class main_page : public mws_page
	{
	public:
		virtual void receive(mws_sp<mws_dp> idp)
		{
			if (idp->is_processed())
			{
				return;
			}

			if (idp->is_type(mws_ptr_evt::TOUCHSYM_EVT_TYPE))
			{
				mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(idp);

				int x = ts->points[0].x;
				int y = ts->points[0].y;

				switch (ts->type)
				{
				case mws_ptr_evt::touch_began:
					kscrollinst.mousePressed(x, y);
					break;

				case mws_ptr_evt::touch_moved:
					kscrollinst.mouseDragged(x, y);
					break;

				case mws_ptr_evt::touch_ended:
					kscrollinst.mouseReleased(x, y);
					break;
				}
			}

			mws_page::receive(idp);
		}

		virtual void update_view(mws_sp<mws_camera> g)
		{
			mws_page::update_view(g);

			kscrollinst.draw(g);
		}
	};

   mws_root->new_page<main_page>();
   mws_cam->clear_color = true;
}
