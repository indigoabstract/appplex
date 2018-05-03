#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef UNIT_TEST

#include "unit-test.hpp"
#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-com.hpp"
#include <noise/perlinnoise.hpp>
#include <rng/rng.hpp>
#include <glm/gtc/constants.hpp>


class embeddedRes
{
public:
	static void printByteArray(shared_ptr<std::vector<uint8> > res)
	{
		int cols = 20;
		std::vector<uint8>& r = *res;

		trx("byte array resource");
		trx("int resSize = {};", res->size());
		trx("uint8 res[] =\n{");

		for(int k = 0; k < res->size(); k++)
		{
			uint32 b = r[k];

			if((k + 1) % cols == 0)
			{
				trc("{:02:x},\n", b);
			}
			else
			{
				trc("{:02:x}, ", b);
			}
		}

		trx("\n};\n");
	}
};


unit_test::unit_test()
{
	set_name("test");
    texInit = false;
}

shared_ptr<unit_test> unit_test::nwi()
{
	return shared_ptr<unit_test>(new unit_test());
}

gfx_uint* unit_test::create_texture(int iw, int ih)
{
    Perlin::initSettings();

    gfx_uint* img = new gfx_uint[iw*ih];

    Perlin::genPerlinTexture((int*)img, iw, ih);

    for(int k = 0; k < ih;k++)
    {
        for(int l = 0; l < iw; l++)
        {
            if(k == 0 || k == ih -1 || l == 0 | l == iw - 1)img[k*iw+l] = 0xff;
            //else img[k*iw+l] = 0x00ff00ff;
            //img[k*iw+l] = 0xff0000;
        }
    }

    return img;
}

void unit_test::init()
{
    int iw=64, ih=iw;
    gfx_uint* imgdata = create_texture(iw, ih);

    // circle
    circlePoints = 150;
    gfx_float angle = 2 *  glm::pi<float>() / circlePoints ;
    gfx_float angle1 = 0.0;
    gfx_float rx = 100;
    gfx_float ry = 80;

    circleVertices = new gfx_float[3 * circlePoints];
    RNG rng;//(1736);

    for (int i = 0; i < circlePoints; i++)
    {
        float fx = rx / 1.05;
        float fy = ry / 1.05;
        int vx = rng.nextInt(100);
        int vy = vx;//rng.range(-10, 10);
        float rtx = rx + fx * vx / 100;
        float rty = ry + fy * vy / 100;
        if(!rng.percent(20))
        {
            rtx = rx;
            rty = ry;
        }
        circleVertices[i * 3 + 0]  = (gfx_float)rtx*cos(angle1);
        circleVertices[i * 3 + 1]  = (gfx_float)rty*sin(angle1);
        circleVertices[i * 3 + 2]  = 0;
        angle1 += angle ;
    }

    iw = 256, ih = iw;
    int isize = iw * ih;
    int *argb = new int[isize];

    for(int k = 0; k < isize; k++)
    {
        //        int colors[]= {0xffff0000, 0xff0000ff, 0xff00ff00};
        //        int c = rng.nextInt(3);
        //        argb[k] = colors[c];
        argb[k] = 0;//0xffffffff;
    }

    int s = ih / 2;
    int s2 = s * s;

    for(int y = -s+1; y < s; y++)
    {
        for(int x = -s+1; x < s; x++)
        {
            int r2 = x*x + y*y;

            if(r2 < s2)
            {
                int c = (255 * (s2 - r2)) / s2;
                int color = c | (c << 8) | (c << 16) | (c << 24);
                argb[(y + s - 1) * iw + (x + s - 1)] = color;
            }
        }
    }
}

namespace unit_test_main_page
{
	class main_page : public mws_page
	{
	public:
		main_page(shared_ptr<mws_page_tab> iparent) : mws_page(iparent){}

		virtual void init()
		{
			mws_page::init();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			mws_page::receive(idp);
		}

		virtual void update_state()
		{
			mws_page::update_state();
		}

		virtual void update_view(shared_ptr<mws_camera> g)
		{
			mws_page::update_view(g);

			int linew = 350;
			int lineh = 25;
			int linehs = 5;
			int lineHeight = lineh + linehs;
			int totalLines = 60;
			int visibleLines = get_unit()->get_height() / lineHeight;
			int lineIdx = 0;

			//g->clearScreen();
			g->setColor(0xff9f007f);

			for (int k = 0; k < visibleLines; k++)
			{
				g->fillRect((get_unit()->get_width() - linew) / 2, k * lineHeight, linew, lineh);
			}

			g->setColor(0xffff0000);
			g->drawLine(50, 50, 250, 350);
			//g->fillAlphaRect(122, 233, 255, 122, 0x7fff00ff);
			//g->drawImage(img, 144, 33);

			const std::string& text = get_unit()->get_name();

			g->drawText(text, 10, 20);
		}
	};
}

void unit_test::init_mws()
{
	mws_page::new_shared_instance(new unit_test_main_page::main_page(mws_root));
	mws_cam->clear_color = true;
}

void unit_test::load()
{
}

#endif
