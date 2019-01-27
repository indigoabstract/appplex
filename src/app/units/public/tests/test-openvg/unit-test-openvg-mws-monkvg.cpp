#include "stdafx.h"

#include "unit-test-openvg.hpp"

#ifdef UNIT_TEST_OPENVG

//#define ENABLE_MONKVG
#ifdef ENABLE_MONKVG

#include "com/mws/mws-com.hpp"

#include <MonkVG/openvg.h>
#include <MonkVG/vgu.h>


namespace MonkVG
{
#define NUM_PRIMITIVES 9
	VGPaint _paint;
	VGPath _path;
	VGPath line;
	VGPath polyOpen;
	VGPath polyClosed;
	VGPath rect;
	VGPath rectRound;
	VGPath ellipse;
	VGPath arcOpen;
	VGPath arcChord;
	VGPath arcPie;
	VGPath primitives[NUM_PRIMITIVES];

	VGPath testCreatePath()
	{
		return vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
			1,0,0,0, VG_PATH_CAPABILITY_ALL);
	}

	void testMoveTo(VGPath p, float x, float y, VGPathAbsRel absrel)
	{
		VGubyte seg = VG_MOVE_TO | absrel;
		VGfloat data[2];

		data[0] = x; data[1] = y;
		vgAppendPathData(p, 1, &seg, data);
	}

	void testLineTo(VGPath p, float x, float y, VGPathAbsRel absrel)
	{
		VGubyte seg = VG_LINE_TO | absrel;
		VGfloat data[2];

		data[0] = x; data[1] = y;
		vgAppendPathData(p, 1, &seg, data);
	}

	void testHlineTo(VGPath p, float x, VGPathAbsRel absrel)
	{
		VGubyte seg = VG_HLINE_TO | absrel;
		VGfloat data = x;

		vgAppendPathData(p, 1, &seg, &data);
	}

	void testVlineTo(VGPath p, float y, VGPathAbsRel absrel)
	{
		VGubyte seg = VG_VLINE_TO | absrel;
		VGfloat data = y;

		vgAppendPathData(p, 1, &seg, &data);
	}

	void testQuadTo(VGPath p, float x1, float y1, float x2, float y2,
		VGPathAbsRel absrel)
	{
		VGubyte seg = VG_QUAD_TO | absrel;
		VGfloat data[4];

		data[0] = x1; data[1] = y1;
		data[2] = x2; data[3] = y2;
		vgAppendPathData(p, 1, &seg, data);
	}

	void testCubicTo(VGPath p, float x1, float y1, float x2, float y2, float x3, float y3,
		VGPathAbsRel absrel)
	{
		VGubyte seg = VG_CUBIC_TO | absrel;
		VGfloat data[6];

		data[0] = x1; data[1] = y1;
		data[2] = x2; data[3] = y2;
		data[4] = x3; data[5] = y3;
		vgAppendPathData(p, 1, &seg, data);
	}

	void testSquadTo(VGPath p, float x2, float y2,VGPathAbsRel absrel)
	{
		VGubyte seg = VG_SQUAD_TO | absrel;
		VGfloat data[2];

		data[0] = x2; data[1] = y2;
		vgAppendPathData(p, 1, &seg, data);
	}

	void testScubicTo(VGPath p, float x2, float y2, float x3, float y3,
		VGPathAbsRel absrel)
	{
		VGubyte seg = VG_SCUBIC_TO | absrel;
		VGfloat data[4];

		data[0] = x2; data[1] = y2;
		data[2] = x3; data[3] = y3;
		vgAppendPathData(p, 1, &seg, data);
	}

	void testArcTo(VGPath p, float rx, float ry, float rot, float x, float y,
		VGPathSegment type, VGPathAbsRel absrel)
	{
		VGubyte seg = type | absrel;
		VGfloat data[5];

		data[0] = rx; data[1] = ry;
		data[2] = rot;
		data[3] = x;  data[4] = y;
		vgAppendPathData(p, 1, &seg, data);
	}

	void testClosePath(VGPath p)
	{
		VGubyte seg = VG_CLOSE_PATH;
		VGfloat data = 0.0f;
		vgAppendPathData(p, 1, &seg, &data);
	}

	void testopenvgInit()
	{
		// create a paint
		_paint = vgCreatePaint();
		vgSetPaint(_paint, VG_FILL_PATH );
		VGfloat color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		vgSetParameterfv(_paint, VG_PAINT_COLOR, 4, &color[0]);

		// create a box path
		_path = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,1,0,0,0, VG_PATH_CAPABILITY_ALL);
		vguRect( _path, 50.0f, 50.0f, 90.0f, 50.0f );

		VGfloat points[] = {-30,-30, 30,-30, 0,30};

		line = testCreatePath();
		vguLine(line, -30,-30,30,30);
		primitives[0] = line;

		polyOpen = testCreatePath();
		vguPolygon(polyOpen, points, 3, VG_FALSE);
		primitives[1] = polyOpen;

		polyClosed = testCreatePath();
		vguPolygon(polyClosed, points, 3, VG_TRUE);
		primitives[2] = polyClosed;

		rect = testCreatePath();
		vguRect(rect, -50,-30, 100,60);
		primitives[3] = rect;

		rectRound = testCreatePath();
		vguRoundRect(rectRound, -50,-30, 100,60, 30,30);
		primitives[4] = rectRound;

		ellipse = testCreatePath();
		vguEllipse(ellipse, 0,0, 100, 60);
		primitives[5] = ellipse;

		arcOpen = testCreatePath();
		vguArc(arcOpen, 0,0, 100,60, 0, 270, VGU_ARC_OPEN);
		primitives[6] = arcOpen;

		arcChord = testCreatePath();
		vguArc(arcChord, 0,0, 100,60, 0, 270, VGU_ARC_CHORD);
		primitives[7] = arcChord;

		arcPie = testCreatePath();
		vguArc(arcPie, 0,0, 100,60, 0, 270, VGU_ARC_PIE);
		primitives[8] = arcPie;
	}

	void testopenvgDraw()
	{
		int x,y,p;
		VGfloat white[] = {1,1,1,1};

		vgSetfv(VG_CLEAR_COLOR, 4, white);
		vgClear(0, 0, pfm::screen::get_width(), pfm::screen::get_height());

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgSetf(VG_STROKE_LINE_WIDTH, 7.5);

		vgLoadIdentity();

		for (y=0, p=0; y<3; ++y) {
			for (x=0; x<3; ++x, ++p) {
				if (p > NUM_PRIMITIVES) break;

				vgLoadIdentity();
				vgTranslate(100 + x*150, 100 + y*100);
				vgDrawPath(primitives[p], VG_STROKE_PATH);
			}
		}

		/// draw the basic path
		vgLoadIdentity();
		vgTranslate( pfm::screen::get_width()/2, pfm::screen::get_height()/2 );
		vgSetPaint( _paint, VG_FILL_PATH );
		vgDrawPath( _path, VG_FILL_PATH );
	}


	class monkvgpage : public mws_page
	{
	public:
		monkvgpage(shared_ptr<mws_page_tab> iparent) : mws_page(iparent){}

		virtual void init()
		{
			mws_page::init();
			testopenvgInit();
		}

		virtual void on_destroy()
		{
		}

		virtual void receive(shared_ptr<mws_dp> idp)
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
			testopenvgDraw();

			g->drawText("monkvg", 10, 10);
		}
	};
}

using namespace MonkVG;

#endif

void unit_test_openvg::init_monkvg()
{
#ifdef ENABLE_MONKVG
	// setup the OpenVG context
	vgCreateContextSH(pfm::screen::get_width(), pfm::screen::get_height());

   mws_root->new_page<monkvgpage>();
#endif
}

void unit_test_openvg::destroy_monkvg()
{
#ifdef ENABLE_MONKVG
	vgDestroyContextSH();
#endif
}

#endif
