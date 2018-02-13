#include "stdafx.h"

#include "unit-test-openvg.hpp"

#ifdef UNIT_TEST_OPENVG

#include "com/ux/ux-camera.hpp"
#include "com/ux/ux-com.hpp"
#include "ovg-obj.hpp"
#include <openvg.h>
#include <vgu.h>
#include <vgext.h>
#include <algorithm>
#include <vector>

using std::string;
using std::vector;


namespace shivavg
{
	int testWidth()
	{
		return pfm::screen::get_width();
	}

	int testHeight()
	{
		return pfm::screen::get_height();
	}

	VGPath testCreatePath()
	{
		return vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F, 1,0,0,0, VG_PATH_CAPABILITY_ALL);
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

	void testQuadTo(VGPath p, float x1, float y1, float x2, float y2, VGPathAbsRel absrel)
	{
		VGubyte seg = VG_QUAD_TO | absrel;
		VGfloat data[4];

		data[0] = x1; data[1] = y1;
		data[2] = x2; data[3] = y2;
		vgAppendPathData(p, 1, &seg, data);
	}

	void testCubicTo(VGPath p, float x1, float y1, float x2, float y2, float x3, float y3, VGPathAbsRel absrel)
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

	void testScubicTo(VGPath p, float x2, float y2, float x3, float y3, VGPathAbsRel absrel)
	{
		VGubyte seg = VG_SCUBIC_TO | absrel;
		VGfloat data[4];

		data[0] = x2; data[1] = y2;
		data[2] = x3; data[3] = y3;
		vgAppendPathData(p, 1, &seg, data);
	}

	void testArcTo(VGPath p, float rx, float ry, float rot, float x, float y, VGPathSegment type, VGPathAbsRel absrel)
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
}


namespace shivavg_tp1
{
#define NUM_PRIMITIVES 9
	VGPaint strokePaint;
	VGPaint fillPaint;
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

	void testopenvgInit()
	{
		VGfloat points[] = {-30,-30, 30,-30, 0,30};

		// create a paint
		strokePaint = vgCreatePaint();
		vgSetColor(strokePaint, 0xffff00ff);
		fillPaint = vgCreatePaint();
		vgSetColor(fillPaint, 0x000000ff);

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

	void testopenvgDestroy()
	{
		vgDestroyPaint(strokePaint);
		vgDestroyPaint(fillPaint);

		for (int k = 0; k < NUM_PRIMITIVES; k++)
		{
			vgDestroyPath(primitives[k]);
		}
	}

	void testopenvgDraw(float tx, float ty)
	{
		//const int NUM_RECTS = 1;
		//VGfloat rects[4*NUM_RECTS];
		//VGint maxScissorRects = vgGeti(VG_MAX_SCISSOR_RECTS);

		//rects[0] = 0;
		//rects[1] = pfm::getScreenHeight() / 2;
		//rects[2] = pfm::getScreenWidth() / 2;
		//rects[3] = pfm::getScreenHeight() / 2;
		//vgSeti(VG_SCISSORING, VG_TRUE);
		//vgSetfv(VG_SCISSOR_RECTS, 4*NUM_RECTS, rects);


		int x,y,p;
		VGfloat white[] = {0,0,1,1};

		vgSetPaint(strokePaint, VG_FILL_PATH);
		vgSetPaint(fillPaint, VG_STROKE_PATH);

		vgSetfv(VG_CLEAR_COLOR, 4, white);
		vgClear(0, 0, pfm::screen::get_width(), pfm::screen::get_height());

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

		/* Set the line width to 2.5 */
		vgSetf(VG_STROKE_LINE_WIDTH, 2.5f);
		/* Set the miter limit to 10.5 */
		vgSetf(VG_STROKE_MITER_LIMIT, 10.5f);
		/* Set the cap style to CAP_SQUARE */
		vgSeti(VG_STROKE_CAP_STYLE, VG_CAP_BUTT);
		/* Set the join style to JOIN_MITER */
		vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_MITER);
		/* Set the dash pattern */
		VGfloat dashes[] = { 4.0f, 2.0f };
		vgSetfv(VG_STROKE_DASH_PATTERN, 2, dashes);
		/* Set the dash phase to 0.5 and reset it for every subpath */
		vgSetf(VG_STROKE_DASH_PHASE, 0.5f);
		vgSeti(VG_STROKE_DASH_PHASE_RESET, VG_TRUE);
		vgSetf(VG_STROKE_LINE_WIDTH, 7.5);

		float s = 4;

		for (y=0, p=0; y<3; ++y)
		{
			for (x=0; x<3; ++x, ++p) {
				if (p > NUM_PRIMITIVES) break;

				vgLoadIdentity();
				vgTranslate(tx + s*75 + s*x*150, ty + s*50 + s*y*100);
				vgScale(s, s);
				vgDrawPath(primitives[p], VG_STROKE_PATH | VG_FILL_PATH);
			}
		}
	}


	class shivavg_page : public ux_page
	{
	public:
		shivavg_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			ux_page::init();
			testopenvgInit();
		}

		virtual void on_destroy()
		{
			testopenvgDestroy();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			ux_page::receive(idp);
		}

		virtual void update_state()
		{
			uxr.w = 2000;
			uxr.h = 1500;
			ux_page::update_state();
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			ux_page::update_view(g);
			testopenvgDraw(uxr.x, uxr.y);

			g->drawText("shivavg_tp1", 10, 10);
		}
	};
}


namespace shivavg_tp2
{
# define IMAGE_DIR "openvg/"
	typedef struct
	{
		VGfloat from[16];
		VGfloat to[16];
		VGfloat value[16];
		VGfloat fromAlpha;
		VGfloat valueAlpha;
		VGfloat toAlpha;
		VGPaint fillStack;
		VGPaint fillWater;
		VGfloat offset;
		VGint play;
	} CoverState;

	int coversCount = 5;
	spvg_image covers[5];
	CoverState coverStates[5];

	int coversCenter = 2;
	int drawCenter = 2;
	VGfloat swaptime = 0.5;
	VGfloat stackAlphaBack = 0.5f;
	VGfloat stackAlphaFront = 1.0f;

	VGPath frame;
	VGPaint frameStroke;
	VGPaint frameWaterStroke;
	VGfloat white[4] = {1,1,1,1};

	VGPath btnPrev;
	VGPath btnPrevShine;
	VGPath btnPrevArrow;
	VGPath btnNext;
	VGPath btnNextShine;
	VGPath btnNextArrow;
	VGfloat btnOffsetY = 100;
	int btnPrevOver = 0;
	int btnNextOver = 0;

	VGPaint btnPaint;
	VGPaint btnOverPaint;
	VGPaint btnShinePaint;
	VGfloat btnColor[4] = {0,0,0,1};
	VGfloat btnOverColor[4] = {0.1,0.1,0.1,1};
	VGfloat btnShineColor[4] = {0.4,0.4,0.4,0.4};

	void copyMatrix(VGfloat *dst, VGfloat *src)
	{
		int i;
		for (i=0; i<16; ++i)
			dst[i] = src[i];
	}

	void copyColor(VGfloat *dst, VGfloat *src)
	{
		int i;
		for (i=0; i<4; ++i)
			dst[i] = src[i];
	}

	void updateStackFill(VGPaint f, float alpha)
	{
		VGfloat radial[] = {200,300,200,300,250};

		VGfloat stops[] = {
			0.0, 1.0, 1.0, 1.0, 1.0,
			0.5, 1.0, 1.0, 1.0, 1.0,
			1.0, 1.0, 1.0, 1.0, 1.0 };

			VGint stopsize = sizeof(stops) / sizeof(VGfloat);
			VGint numstops = stopsize / 5;

			VGint s, c;
			for (s=0; s<numstops; ++s)
				for (c=1; c<=3; ++c)
					stops[s * 5 + c] = alpha;

			stops[14] = alpha;

			vgSetParameteri(f, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);
			vgSetParameterfv(f, VG_PAINT_RADIAL_GRADIENT, 5, radial);
			vgSetParameterfv(f, VG_PAINT_COLOR_RAMP_STOPS, stopsize, stops);
	}

	void updateWaterFill(VGPaint f, float alpha)
	{
		VGfloat linear[] = {0,0,0,190};

		VGfloat stops[] = {
			0.0, 1.0, 1.0, 1.0, 1.0f,
			1.0, 1.0, 1.0, 1.0, 0.0f};

			VGint numstops = sizeof(stops) / sizeof(VGfloat);

			stops[4] = alpha;
			vgSetParameteri(f, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
			vgSetParameterfv(f, VG_PAINT_LINEAR_GRADIENT, 4, linear);
			vgSetParameterfv(f, VG_PAINT_COLOR_RAMP_STOPS, numstops, stops);
	}

	void interpolateCoverState(CoverState *c, float interval)
	{
		int i;
		VGfloat d;
		VGfloat o;

		c->offset += interval * 2;

		if (c->offset >= 1.0f) {
			c->offset = 1.0f;
			c->play = 0;
		}

		o = sqrt(c->offset);

		for (i=0; i<16; ++i) {
			d = c->to[i] - c->from[i];
			c->value[i] = c->from[i] + o * d;
		}

		d = c->toAlpha - c->fromAlpha;
		c->valueAlpha = c->fromAlpha + o * d;

		updateStackFill(c->fillStack, c->valueAlpha);
		updateWaterFill(c->fillWater, c->valueAlpha*0.5f);

		drawCenter = coversCenter;
	}

	void updateCoverStates(int init)
	{
		int ci; CoverState *c;
		VGfloat alpha = 1.0f;
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);

		for (ci=0; ci<coversCount; ++ci) {
			c = &coverStates[ci];

			vgLoadIdentity();
			vgTranslate(testWidth()/2, testHeight()/2);

			if (ci == coversCenter) {
				alpha = stackAlphaFront;
			}else{
				VGfloat ds = ci - coversCenter;
				VGfloat d = std::abs(ds);
				VGfloat sign = ds / d;
				VGfloat s = 1 - 0.1 - d*0.05;
				VGfloat t = sign * (200 + (d-1) * 50);
				vgTranslate(t, 0);
				vgScale(s,s);
				alpha = stackAlphaBack - d*0.05;
			}

			if (init) {
				c->play = 0;
				c->offset = 0.0f;
				vgGetMatrix(c->value);
				c->valueAlpha = alpha;
				c->fillStack = vgCreatePaint();
				c->fillWater = vgCreatePaint();
				updateStackFill(c->fillStack, alpha);
				updateWaterFill(c->fillWater, alpha*0.5f);
			}else{
				c->play = 1;
				c->offset = 0.0f;
				copyMatrix(c->from, c->value);
				vgGetMatrix(c->to);
				c->fromAlpha = c->valueAlpha;
				c->toAlpha = alpha;
			}
		}
	}

	void drawCover(int c)
	{
		/* position image on stack */
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
		vgLoadMatrix(coverStates[c].value);

		/* Center image and draw */
		vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_MULTIPLY);
		vgSetPaint(coverStates[c].fillStack, VG_FILL_PATH);
		vgTranslate(-200,-200);
		vgDrawImage(covers[c]->handle());

		/* Clear reflection below */
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadMatrix(coverStates[c].value);
		vgTranslate(-200,-200);
		vgScale(1, -1);
		vgSetPaint(VG_INVALID_HANDLE, VG_FILL_PATH);
		vgDrawPath(frame, VG_FILL_PATH);

		/* Mirror water image */
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
		vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_MULTIPLY);
		vgSetPaint(coverStates[c].fillWater, VG_FILL_PATH);
		vgScale(1, -1);
		vgDrawImage(covers[c]->handle());

		/* Draw frame */ 
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadMatrix(coverStates[c].value);
		vgTranslate(-200.5,-200.5);
		vgSetPaint(frameStroke, VG_STROKE_PATH);
		vgSetf(VG_STROKE_LINE_WIDTH, 1.0f);
		vgDrawPath(frame, VG_STROKE_PATH);

		/* Mirror water frame */
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_STROKE_PAINT_TO_USER);
		vgLoadIdentity();
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgSetPaint(frameWaterStroke, VG_STROKE_PATH);
		vgScale(1, -1);
		vgDrawPath(frame, VG_STROKE_PATH);
	}

	void display(float interval)
	{
		VGfloat white[] = {0,0,0,1};
		VGint c;

		vgSetfv(VG_CLEAR_COLOR, 4, white);
		vgClear(0, 0, testWidth(), testHeight());

		for (c=0; c<coversCount; ++c)
			if (coverStates[c].play)
				interpolateCoverState(&coverStates[c], interval);

		for (c=0; c!=drawCenter; ++c)
			drawCover(c);

		for (c=coversCount-1; c!=drawCenter; --c)
			drawCover(c);

		drawCover(drawCenter);

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadIdentity();

		vgTranslate(testWidth()/2, btnOffsetY);

		vgSetPaint(frameStroke, VG_STROKE_PATH);
		vgSetPaint(btnPrevOver ? btnOverPaint : btnPaint, VG_FILL_PATH);
		vgDrawPath(btnPrev, VG_FILL_PATH | VG_STROKE_PATH);
		vgSetPaint(btnNextOver ? btnOverPaint : btnPaint, VG_FILL_PATH);
		vgDrawPath(btnNext, VG_FILL_PATH | VG_STROKE_PATH);

		vgSetPaint(btnShinePaint, VG_FILL_PATH);
		vgDrawPath(btnPrevShine, VG_FILL_PATH);
		vgDrawPath(btnNextShine, VG_FILL_PATH);

		vgSetPaint(frameStroke, VG_FILL_PATH);
		vgDrawPath(btnPrevArrow, VG_FILL_PATH);
		vgDrawPath(btnNextArrow, VG_FILL_PATH);

		//vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_NORMAL);
		//vgSeti(VG_BLEND_MODE, VG_BLEND_ADDITIVE);
		//vgSetPaint(VG_INVALID_HANDLE, VG_FILL_PATH);
		//vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
		//vgLoadIdentity();
		//vgDrawImage(covers[0]);
	}

	void specialKey(int key, int x, int y)
	{
		switch(key)
		{
		case 0:
			if (coversCenter > 0) --coversCenter;
			updateCoverStates(0);
			break;
		case 1:
			if (coversCenter < coversCount-1) ++coversCenter;
			updateCoverStates(0);
			break;
		}
	}

	int isPointInBoundbox(VGPath p, float x, float y)
	{
		VGfloat minX, minY;
		VGfloat width, height;
		vgPathTransformedBounds(p, &minX, &minY, &width, &height);
		return (x >= minX && x <= minX + width &&
			y >= minY && y <= minY + height);
	}

	void click(int button, int state, int x, int y)
	{
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadIdentity();
		vgTranslate(testWidth()/2, btnOffsetY);

		//y = testHeight() - y;
		if (isPointInBoundbox(btnPrev, (float)x, (float)y))
			specialKey(0, x, y);
		else if (isPointInBoundbox(btnNext, (float)x, (float)y))
			specialKey(1, x, y);
	}

	void move(int x, int y)
	{
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadIdentity();
		vgTranslate(testWidth()/2, btnOffsetY);

		y = testHeight() - y;
		btnPrevOver = 0;
		btnNextOver = 0;

		if (isPointInBoundbox(btnPrev, (float)x, (float)y))
			btnPrevOver = 1;
		else if (isPointInBoundbox(btnNext, (float)x, (float)y))
			btnNextOver = 1;
	}

	void createButton(VGPath b)
	{
		testMoveTo(b, 0,0, VG_ABSOLUTE);
		testLineTo(b, -15,0, VG_RELATIVE);
		testArcTo(b,20,20,0,0,-40, VG_SCCWARC_TO, VG_RELATIVE);
		testLineTo(b, 15,0, VG_RELATIVE);
		testClosePath(b);
	}

	void createArrow(VGPath a)
	{
		testMoveTo(a, -12,-12, VG_ABSOLUTE);
		testLineTo(a, -8,-8, VG_RELATIVE);
		testLineTo(a, 8, -8, VG_RELATIVE);
		testClosePath(a);
	}

	void createButtons()
	{
		btnPrev = testCreatePath();
		createButton(btnPrev);

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadIdentity();
		vgScale(-1,1);

		btnNext = testCreatePath();
		vgTransformPath(btnNext,btnPrev);

		vgLoadIdentity();
		vgTranslate(0,-1);
		vgScale(0.9,0.5);

		btnPrevShine = testCreatePath();
		vgTransformPath(btnPrevShine, btnPrev);

		btnNextShine = testCreatePath();
		vgTransformPath(btnNextShine, btnNext);

		btnPrevArrow = testCreatePath();
		createArrow(btnPrevArrow);

		btnPaint = vgCreatePaint();
		vgSetParameterfv(btnPaint, VG_PAINT_COLOR, 4, btnColor);

		btnOverPaint = vgCreatePaint();
		vgSetParameterfv(btnOverPaint, VG_PAINT_COLOR, 4, btnOverColor);

		btnShinePaint = vgCreatePaint();
		vgSetParameterfv(btnShinePaint, VG_PAINT_COLOR, 4, btnShineColor);

		vgLoadIdentity();
		vgScale(-1,1);

		btnNextArrow = testCreatePath();
		vgTransformPath(btnNextArrow, btnPrevArrow);
	}

	void testInit()
	{
		covers[0] = vg_image::load_image(IMAGE_DIR"test_img_guitar.png");
		covers[1] = vg_image::load_image(IMAGE_DIR"test_img_piano.png");
		covers[2] = vg_image::load_image(IMAGE_DIR"test_img_violin.png");
		covers[3] = vg_image::load_image(IMAGE_DIR"test_img_flute.png");
		covers[4] = vg_image::load_image(IMAGE_DIR"test_img_sax.png");

		frame = testCreatePath();
		vguRect(frame,0.0,0.0,400,400);

		frameStroke = vgCreatePaint();
		vgSetParameterfv(frameStroke, VG_PAINT_COLOR, 4, white);

		frameWaterStroke = vgCreatePaint();
		updateWaterFill(frameWaterStroke, 1);

		createButtons();
		updateCoverStates(1);
	}


	class shivavg_page : public ux_page
	{
	public:
		shivavg_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			ux_page::init();
			testInit();
			lastTime = pfm::time::get_time_millis();
		}

		virtual void on_destroy()
		{
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
			{
				shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

				if(ts->get_type() == touch_sym_evt::TS_FIRST_TAP)
				{
					click(0, 0, ts->pressed.pos.x, ts->pressed.pos.y);
				}
			}

			ux_page::receive(idp);
		}

		virtual void update_state()
		{
			ux_page::update_state();
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			ux_page::update_view(g);

			uint32 now = pfm::time::get_time_millis();
			display(now - lastTime);
			lastTime = now;

			g->drawText("shivavg_tp2", 10, 10);
		}

		uint32 lastTime;
	};
}


namespace shivavg_tp3
{
	VGPath src;
	VGPath dst;
	VGPaint srcFill;
	VGPaint dstFill;
	spvg_image isrc;
	spvg_image idst;

	VGfloat srcColor[4] = {0.4, 0.6, 1.0, 1};
	VGfloat dstColor[4] = {1, 1, 0.3, 1};

#ifndef IMAGE_DIR
#  define IMAGE_DIR "./"
#endif

	VGBlendMode blends[5] = {
		VG_BLEND_SRC,
		VG_BLEND_SRC_OVER,
		VG_BLEND_DST_OVER,
		VG_BLEND_SRC_IN,
		VG_BLEND_DST_IN
	};

	void createOperands()
	{
		VGfloat clear[] = {1,0,0,0};

		src = testCreatePath();
		vguEllipse(src, 30,30,40,40);

		dst = testCreatePath();
		vguRect(dst, 0,0,40,30);

		srcFill = vgCreatePaint();
		vgSetParameterfv(srcFill, VG_PAINT_COLOR, 4, srcColor);

		dstFill = vgCreatePaint();
		vgSetParameterfv(dstFill, VG_PAINT_COLOR, 4, dstColor);

		isrc = vg_image::load_image(IMAGE_DIR"test_blend_src.png");
		idst = vg_image::load_image(IMAGE_DIR"test_blend_dst.png");
	}


	void testopenvgInit()
	{
		createOperands();
	}

	void testopenvgDestroy()
	{
		vgDestroyPath(src);
		vgDestroyPath(dst);
		vgDestroyPaint(srcFill);
		vgDestroyPaint(dstFill);
	}

	void testopenvgDraw(float tx, float ty)
	{
		VGfloat clear[] = {0,0,0,0};

		vgSetfv(VG_CLEAR_COLOR, 4, clear);
		vgClear(0, 0, pfm::screen::get_width(), pfm::screen::get_height());

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadIdentity();

		vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_OVER);
		vgDrawImage(idst->handle());
		vgSeti(VG_BLEND_MODE, VG_BLEND_SRC_IN);
		vgDrawImage(isrc->handle());
	}


	class shivavg_page : public ux_page
	{
	public:
		shivavg_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			ux_page::init();
			testopenvgInit();
		}

		virtual void on_destroy()
		{
			testopenvgDestroy();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			ux_page::receive(idp);
		}

		virtual void update_state()
		{
			ux_page::update_state();
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			ux_page::update_view(g);
			testopenvgDraw(uxr.x, uxr.y);

			g->drawText("shivavg_tp3", 10, 10);
		}
	};
}


namespace shivavg_tp4
{

	VGPaint testStroke;
	VGPaint testFill;
	VGPath testPath;
	VGfloat phase=0.0f;

	VGint jindex = 0;
	VGint jsize = 3;
	VGint joins[] = {
		VG_JOIN_MITER,
		VG_JOIN_BEVEL,
		VG_JOIN_ROUND
	};

	VGint cindex = 2;
	VGint csize = 3;
	VGint caps[] = {
		VG_CAP_BUTT,
		VG_CAP_SQUARE,
		VG_CAP_ROUND
	};

	VGfloat sx=1.0f, sy=1.0f;

	VGfloat startX=0.0f;
	VGfloat startY=0.0f;
	VGfloat clickX=0.0f;
	VGfloat clickY=0.0f;
	char mode='d';

	const char commands[] =
		"Click & drag mouse to change\n"
		"value for current mode\n\n"
		"H - this help\n"
		"D - dash phase mode\n"
		"C - dash caps cycle\n"
		"J - dash joins cycle\n"
		"X - scale X mode\n"
		"Y - scale Y mode\n";
	string overlay;

	void createStar()
	{
		VGubyte segs[] =
		{
			VG_MOVE_TO, VG_LINE_TO_REL, VG_LINE_TO_REL, VG_LINE_TO_REL,
			VG_LINE_TO_REL, VG_LINE_TO_REL, VG_LINE_TO_REL, VG_LINE_TO_REL,
			VG_LINE_TO_REL, VG_LINE_TO_REL, VG_CLOSE_PATH
		};

		VGfloat data[] =
		{
			0,50, 15,-40, 45,0, -35,-20,
			15,-40, -40,30, -40,-30, 15,40,
			-35,20, 45,0
		};

		VGfloat cstroke[] = {0.5,0.2,0.8, 0.6};
		VGfloat cfill[] = {0.3,1.0,0.0, 0.6};

		testPath = testCreatePath();
		vgAppendPathData(testPath, sizeof(segs), segs, data);

		testStroke = vgCreatePaint();
		vgSetParameterfv(testStroke, VG_PAINT_COLOR, 4, cstroke);
		vgSetPaint(testStroke, VG_STROKE_PATH);

		testFill = vgCreatePaint();
		vgSetParameterfv(testFill, VG_PAINT_COLOR, 4, cfill);
		vgSetPaint(testFill, VG_FILL_PATH);
	}

	void updateOverlayString()
	{
		switch(mode)
		{
		case 'd':
			overlay = "Dash Phase mode"; break;
		case 'x':
			overlay = trs("Scale X mode: %f") % sx; break;
		case 'y':
			overlay = trs("Scale Y mode: %f") % sy; break;
		}
	}

	void testopenvgInit()
	{
		createStar();
	}

	void testopenvgDestroy()
	{
		vgDestroyPath(testPath);
		vgDestroyPaint(testStroke);
		vgDestroyPaint(testFill);
	}

	void testopenvgDraw(float tx, float ty)
	{
		VGfloat cc[] = {0,0,0,1};
		VGfloat dash[] = {10,15,0,15};

		vgSetfv(VG_CLEAR_COLOR, 4, cc);
		vgClear(0, 0, pfm::screen::get_width(), pfm::screen::get_height());

		vgSetfv(VG_STROKE_DASH_PATTERN, sizeof(dash)/sizeof(float), dash);
		vgSeti(VG_STROKE_DASH_PHASE_RESET, VG_TRUE);
		vgSetf(VG_STROKE_DASH_PHASE, phase);

		vgSetf(VG_STROKE_LINE_WIDTH, 10.0f);
		vgSetf(VG_STROKE_JOIN_STYLE, joins[jindex]);
		vgSetf(VG_STROKE_CAP_STYLE, caps[cindex]);

		vgLoadIdentity();
		vgTranslate(testWidth()/2,testHeight()/2);
		vgScale(3 * sx, 3 * sy);

		vgSetPaint(testFill, VG_FILL_PATH);
		vgDrawPath(testPath, VG_FILL_PATH);
		vgSetPaint(testStroke, VG_STROKE_PATH);
		vgDrawPath(testPath, VG_STROKE_PATH);
	}


	class shivavg_page : public ux_page
	{
	public:
		shivavg_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			ux_page::init();
			testopenvgInit();
		}

		virtual void on_destroy()
		{
			testopenvgDestroy();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
			{
				on_touch_evt(touch_sym_evt::as_touch_sym_evt(idp));
			}
			else if(idp->is_type(key_evt::KEYEVT_EVT_TYPE))
			{
				on_key_evt(key_evt::as_key_evt(idp));
			}

			ux_page::receive(idp);
		}

		virtual void update_state()
		{
			ux_page::update_state();
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			ux_page::update_view(g);
			testopenvgDraw(uxr.x, uxr.y);

			g->drawText("shivavg_tp4", 10, 10);
			g->drawText(overlay, 10, 50);
		}

		void on_touch_evt(shared_ptr<touch_sym_evt> ts)
		{
			switch(ts->get_type())
			{
			case touch_sym_evt::TS_FIRST_TAP:
				{
					float x = ts->crt_state.pos.x;
					float y = ts->crt_state.pos.y;
					//y = pfm::getScreenHeight() - y;
					clickX = x; clickY = y;

					switch (mode) {
					case 'd':
						startY = phase;
						break;
					case 'x':
						startY = sx;
						break;
					case 'y':
						startY = sy;
						break;
					}

					ts->process();
					break;
				}

			case touch_sym_evt::TS_PRESS_AND_DRAG:
				{
					if(ts->is_finished)
					{
					}
					else
					{
						VGfloat dx, dy;
						float x = ts->crt_state.pos.x;
						float y = ts->crt_state.pos.y;

						//y = pfm::getScreenHeight() - y;
						dx = x - clickX;
						dy = y - clickY;

						switch (mode) {
						case 'd':
							phase = startY + dy * 0.1;
							break;
						case 'x':
							sx = startY + dy * 0.01;
							break;
						case 'y':
							sy = startY + dy * 0.01;
							break;
						}

						updateOverlayString();
						ts->process();
					}

					break;
				}
			}
		}

		void on_key_evt(shared_ptr<key_evt> ke)
		{
			if(ke->get_type() == key_evt::KE_RELEASED)
			{
				return;
			}

			bool isAction = true;
			char code = 0;

			switch(ke->get_key())
			{
			case KEY_D:
				code = 'd';
				break;

			case KEY_X:
				code = 'x';
				break;

			case KEY_Y:
				code = 'y';
				break;

			case KEY_C:
				/* Cycle caps type */
				cindex = (cindex+1) % csize;
				switch(caps[cindex]) {
				case VG_CAP_BUTT: overlay = trs("Dash caps: BUTT\n"); break;
				case VG_CAP_SQUARE: overlay = trs("Dash caps: SQUARE\n"); break;
				case VG_CAP_ROUND: overlay = trs("Dash caps: ROUND\n"); break; }
				break;

			case KEY_J:
				/* Cycle joins type */
				jindex = (jindex+1) % jsize;
				switch(joins[jindex]) {
				case VG_JOIN_MITER: overlay = trs("Dash joins: MITER\n"); break;
				case VG_JOIN_BEVEL: overlay = trs("Dash joins: BEVEL\n"); break;
				case VG_JOIN_ROUND: overlay = trs("Dash joins: ROUND\n"); break; }
				break;

			case KEY_H:
				/* Show help */
				overlay = trs(commands);
				break;

			default:
				isAction = false;
			}

			if(isAction)
			{
				if(code > 0)
				{
					/* Switch mode */
					mode = tolower(code);
					updateOverlayString();
				}

				ke->process();
			}
		}
	};
}


namespace shivavg_tp5
{
	VGPath iApple;
	VGPath iPear;
	VGPath iMorph;
	VGfloat angle=3*M_PI/2;
	VGfloat amount=0.0f;
	VGPaint fill;
	VGfloat white[] = {1,1,1,1};

	void createMorph()
	{
		vgClearPath(iMorph, VG_PATH_CAPABILITY_ALL);
		vgInterpolatePath(iMorph, iApple, iPear, amount);
	}

	void createApple(VGPath p)
	{
		VGPath temp;

		VGubyte segs[] =
		{
			VG_MOVE_TO_ABS, VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS,
			VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS,
			VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS, VG_CLOSE_PATH,
			VG_MOVE_TO_ABS, VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS, VG_CLOSE_PATH
		};

		VGfloat data[] =
		{
			1.53125,-44.681982, -3.994719,-44.681982, -8.0085183,-50.562501,
			-26.5625,-50.562501, -42.918439,-50.562501, -56.46875,-34.239393,
			-56.46875,-12.187501, -56.46875,26.520416, -34.65822,61.731799,
			-16.84375,61.812499, -7.1741233,61.812499, -2.9337937,55.656199,
			4.15625,55.656199, 11.746294,55.656199, 17.981627,62.281199,
			25.4375,62.281199, 33.88615,62.281199, 50.53251,44.282999,
			58.75,15.718799, 47.751307,9.086518, 40.999985,-0.228074,
			41,-13.046574, 41,-27.849147, 46.64686,-34.763001,
			52.4375,-39.937501, 46.111827,-47.219094, 39.0413,-50.503784,
			29.09375,-50.446384, 11.146487,-50.342824, 8.6341912,-44.681982,
			1.53125,-44.681982,

			0.23972344,-52.075169, -2.8344902,-69.754133, 5.9303785,-81.915323,
			24.152707,-86.881406, 23.71828,-70.367255, 15.114064,-58.365865,
			0.23972344,-52.075169
		};

		temp = testCreatePath();
		vgAppendPathData(temp, sizeof(segs), segs, data);

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadIdentity();
		//vgScale(1,-1);
		vgTransformPath(p, temp);
		vgDestroyPath(temp);
	}

	void createPear(VGPath p)
	{
		VGPath temp;

		VGubyte segs[] =
		{
			VG_MOVE_TO_ABS, VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS,
			VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS,
			VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS, VG_CLOSE_PATH,
			VG_MOVE_TO_ABS, VG_CUBIC_TO_ABS, VG_CUBIC_TO_ABS, VG_CLOSE_PATH
		};

		VGfloat data[] =
		{
			0.0625,-90.625001, -29.44062,-89.191161, -23.07159,-32.309301,
			-30.5625,-14.062501, -38.29681,4.7771994, -56.8077,20.767199,
			-56.46875,42.812499, -56.1298,64.502999, -40.15822,79.731799,
			-22.34375,79.812499, -4.17446,79.893199, -1.93369,71.113999,
			4.15625,71.156199, 10.49619,71.198499, 13.70293,80.336799,
			30.4375,80.281199, 42.49257,80.241199, 53.53251,70.782999,
			58.75,58.218799, 47.0442,54.768499, 38.5,43.943499,
			38.5,31.124999, 38.50001,22.754099, 42.14686,15.236999,
			47.9375,10.062499, 42.2834,1.5737994, 36.5413,-6.6199006,
			34.09375,-14.062501, 28.48694,-31.111801, 32.99356,-90.265511,
			1.5,-90.625001,

			5.1056438,-97.8762, -12.766585,-99.48239, -22.244878,-111.09615,
			-22.325466,-129.98288, -6.486451,-125.28908, 2.8790668,-113.87186,
			5.1056438,-97.8762
		};

		temp = testCreatePath();
		vgAppendPathData(temp, sizeof(segs), segs, data);

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadIdentity();
		//vgScale(1,-1);
		vgTransformPath(p, temp);
		vgDestroyPath(temp);
	}


	void testopenvgInit()
	{
		iApple = testCreatePath();
		createApple(iApple);
		iPear = testCreatePath();
		createPear(iPear);
		iMorph = testCreatePath();

		fill = vgCreatePaint();
		vgSetParameterfv(fill, VG_PAINT_COLOR, 4, white);
	}

	void testopenvgDestroy()
	{
		vgDestroyPath(iApple);
		vgDestroyPath(iPear);
		vgDestroyPath(iMorph);
		vgDestroyPaint(fill);
	}

	void testopenvgDraw(float tx, float ty, float interval)
	{
		VGfloat cc[] = {0,0,0,1};

		angle += interval * 0.4 * M_PI;
		if (angle > 2*M_PI) angle -= 2*M_PI;
		amount = (sin(angle) + 1) * 0.5f;
		createMorph();

		vgSetfv(VG_CLEAR_COLOR, 4, cc);
		vgClear(0,0,pfm::screen::get_width(),pfm::screen::get_height());

		vgLoadIdentity();
		vgTranslate(pfm::screen::get_width()/2, pfm::screen::get_height()/2);
		vgSetPaint(fill, VG_FILL_PATH);
		vgDrawPath(iMorph, VG_FILL_PATH);
	}


	class shivavg_page : public ux_page
	{
	public:
		shivavg_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			ux_page::init();
			testopenvgInit();
		}

		virtual void on_destroy()
		{
			testopenvgDestroy();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			ux_page::receive(idp);
		}

		virtual void update_state()
		{
			ux_page::update_state();
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			ux_page::update_view(g);
			testopenvgDraw(uxr.x, uxr.y, 0.03f);

			g->drawText("shivavg_tp5", 10, 10);
		}
	};
}


namespace shivavg_tp6
{
	VGfloat o[3] = {0.1f, 0.5f, 1.0f};

	VGfloat gx1, gy1;
	VGfloat gx2, gy2;

	VGfloat tx=0.0f, ty=0.0f;
	VGfloat sx=1.0f, sy=1.0f;
	VGfloat a=0.0f;

	VGint sindex = 0;
	VGint ssize = 3;
	VGint spread[] =
	{
		VG_COLOR_RAMP_SPREAD_PAD,
		VG_COLOR_RAMP_SPREAD_REPEAT,
		VG_COLOR_RAMP_SPREAD_REFLECT
	};

	VGfloat sqx = 200;
	VGfloat sqy = 200;

	VGfloat clickX;
	VGfloat clickY;
	VGfloat startX;
	VGfloat startY;
	char mode = 's';

	VGPaint linearFill;
	VGPaint blackFill;
	VGPath start;
	VGPath end;
	VGPath side1;
	VGPath side2;
	VGPath unitX;
	VGPath unitY;
	VGPath p;

	VGfloat black[] = {1,1,1, 1};

	const char commands[] =
		"Click & drag mouse to change\n"
		"value for current mode\n\n"
		"H - this help\n"
		"TAB - gradient spread mode\n"
		"S - gradient start-point mode\n"
		"E - gradient end-point mode\n"
		"X - scale X mode\n"
		"Y - scale Y mode\n"
		"BACKSPACE - reset value\n";
	string overlay;

	void createArrow(VGPath p, VGfloat cx, VGfloat cy, VGfloat dx, VGfloat dy)
	{
		VGfloat px = -dy, py = dx;
		testMoveTo(p, cx + 10*dx, cy + 10*dy, VG_ABSOLUTE);
		testLineTo(p, cx +  5*px, cy +  5*py, VG_ABSOLUTE);
		testLineTo(p, cx -  5*px, cy -  5*py, VG_ABSOLUTE);
		testClosePath(p);
	}

	void createLinear()
	{
		VGfloat dx = gx2 - gx1;
		VGfloat dy = gy2 - gy1;
		VGfloat n = sqrt(dx*dx + dy*dy);
		VGfloat px = -dy / n;
		VGfloat py = dx / n;

		VGfloat stops[] =
		{
			0.0, 1.0, 0.0, 0.0, 1,
			0.5, 0.0, 1.0, 0.0, 1,
			1.0, 0.0, 0.0, 1.0, 1
		};

		VGint numstops = sizeof(stops) / sizeof(VGfloat);

		VGfloat linear[4];
		linear[0] = gx1;
		linear[1] = gy1;
		linear[2] = gx2;
		linear[3] = gy2;

		vgSetParameteri(linearFill, VG_PAINT_COLOR_RAMP_SPREAD_MODE, spread[sindex]);
		vgSetParameteri(linearFill, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
		vgSetParameterfv(linearFill, VG_PAINT_LINEAR_GRADIENT, 4, linear);
		vgSetParameterfv(linearFill, VG_PAINT_COLOR_RAMP_STOPS, numstops, stops);

		vgClearPath(start, VG_PATH_CAPABILITY_ALL);
		vguEllipse(start, gx1, gy1, 10,10);

		vgClearPath(end, VG_PATH_CAPABILITY_ALL);
		createArrow(end, gx2, gy2, dx / n, dy / n);

		vgClearPath(unitX, VG_PATH_CAPABILITY_ALL);
		testMoveTo(unitX, gx1, gy1, VG_ABSOLUTE);
		testLineTo(unitX, gx2, gy2, VG_ABSOLUTE);

		vgClearPath(unitY, VG_PATH_CAPABILITY_ALL);
		testMoveTo(unitY, gx1 - 50*px, gy1 - 50*py, VG_ABSOLUTE);
		testLineTo(unitY, gx1 + 50*px, gy1 + 50*py, VG_ABSOLUTE);

		vgClearPath(side1, VG_PATH_CAPABILITY_ALL);
		createArrow(side1, gx1 + 50*px, gy1 + 50*py, px, py);

		vgClearPath(side2, VG_PATH_CAPABILITY_ALL);
		createArrow(side2, gx1 - 50*px, gy1 - 50*py, -px, -py);
	}

	void createSquare(VGPath p)
	{
		testMoveTo(p, (testWidth()-sqx)/2, (testHeight()-sqy)/2, VG_ABSOLUTE);
		testLineTo(p, sqx, 0, VG_RELATIVE);
		testLineTo(p, 0, sqy, VG_RELATIVE);
		testLineTo(p, -sqx, 0, VG_RELATIVE);
		testClosePath(p);
	}

	void updateOverlayString()
	{
		switch(mode)
		{
		case 's':
			overlay = trs("Gradient Start-Point mode"); break;
		case 'e':
			overlay = trs("Gradient End-Point mode"); break;
		case 'x':
			overlay = trs("Scale X mode: %f") % sx; break;
		case 'y':
			overlay = trs("Scale Y mode: %f") % sy; break;
		}
	}

	void testopenvgInit()
	{
		p = testCreatePath();
		start = testCreatePath();
		end = testCreatePath();
		unitX = testCreatePath();
		unitY = testCreatePath();
		side1 = testCreatePath();
		side2 = testCreatePath();

		linearFill = vgCreatePaint();
		blackFill = vgCreatePaint();
		vgSetParameterfv(blackFill, VG_PAINT_COLOR, 4, black);

		gx1 = (testWidth() - sqx)/2;
		gy1 = (testHeight() - sqy)/2;
		gx2 = gx1 + sqx;
		gy2 = gy1 + sqy;

		createSquare(p);
		createLinear();
	}

	void testopenvgDestroy()
	{
		vgDestroyPath(p);
		vgDestroyPath(start);
		vgDestroyPath(end);
		vgDestroyPath(unitX);
		vgDestroyPath(unitY);
		vgDestroyPath(side1);
		vgDestroyPath(side2);
		vgDestroyPaint(linearFill);
		vgDestroyPaint(blackFill);
	}

	void testopenvgDraw(float tx, float ty)
	{
		VGfloat cc[] = {0,0,0,1};

		vgSetfv(VG_CLEAR_COLOR, 4, cc);
		vgClear(0,0,testWidth(),testHeight());

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER);
		vgLoadIdentity();
		vgTranslate(tx, tx);
		vgScale(sx, sy);
		vgRotate(a);

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadIdentity();

		vgSetPaint(linearFill, VG_FILL_PATH);
		vgDrawPath(p, VG_FILL_PATH);

		vgTranslate(tx, tx);
		vgScale(sx, sy);
		vgRotate(a);

		vgSetPaint(blackFill, VG_FILL_PATH | VG_STROKE_PATH);
		vgDrawPath(start, VG_FILL_PATH);
		vgDrawPath(end, VG_FILL_PATH);
		vgDrawPath(unitX, VG_STROKE_PATH);
		vgDrawPath(unitY, VG_STROKE_PATH);
		vgDrawPath(side1, VG_FILL_PATH);
		vgDrawPath(side2, VG_FILL_PATH);
	}


	class shivavg_page : public ux_page
	{
	public:
		shivavg_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			ux_page::init();
			testopenvgInit();
		}

		virtual void on_destroy()
		{
			testopenvgDestroy();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
			{
				on_touch_evt(touch_sym_evt::as_touch_sym_evt(idp));
			}
			else if(idp->is_type(key_evt::KEYEVT_EVT_TYPE))
			{
				on_key_evt(key_evt::as_key_evt(idp));
			}

			ux_page::receive(idp);
		}

		virtual void update_state()
		{
			ux_page::update_state();
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			ux_page::update_view(g);
			testopenvgDraw(uxr.x, uxr.y);

			g->drawText("shivavg_tp6", 10, 10);
			g->drawText(overlay, 10, 50);
		}

		void on_touch_evt(shared_ptr<touch_sym_evt> ts)
		{
			switch(ts->get_type())
			{
			case touch_sym_evt::TS_FIRST_TAP:
				{
					float x = ts->crt_state.pos.x;
					float y = ts->crt_state.pos.y;
					//y = pfm::getScreenHeight() - y;
					clickX = x; clickY = y;

					switch (mode)
					{
					case 's':
						startX = gx1;
						startY = gy1;
						break;
					case 'e':
						startX = gx2;
						startY = gy2;
						break;
					case 'x':
						startY = sx;
						break;
					case 'y':
						startY = sy;
						break;
					}

					ts->process();
					break;
				}

			case touch_sym_evt::TS_PRESS_AND_DRAG:
				{
					if(ts->is_finished)
					{
					}
					else
					{
						VGfloat dx, dy;
						float x = ts->crt_state.pos.x;
						float y = ts->crt_state.pos.y;

						//y = pfm::getScreenHeight() - y;
						dx = x - clickX;
						dy = y - clickY;

						switch(mode)
						{
						case 's':
							{
								VGfloat dgx = (startX + dx) - gx1;
								VGfloat dgy = (startY + dy) - gy1;
								gx1 = startX + dx;
								gy1 = startY + dy;
								gx2 += dgx;
								gy2 += dgy;
								createLinear();
								break;
							}
						case 'e':
							gx2 = startX + dx;
							gy2 = startY + dy;
							createLinear();
							break;
						case 'x':
							sx = startY + dy * 0.01;
							break;
						case 'y':
							sy = startY + dy * 0.01;
							break;
						}

						updateOverlayString();
						ts->process();
					}

					break;
				}
			}
		}

		void on_key_evt(shared_ptr<key_evt> ke)
		{
			if(ke->get_type() == key_evt::KE_RELEASED)
			{
				return;
			}

			bool isAction = true;
			char code = 0;

			switch(ke->get_key())
			{
			case KEY_S:
				code = 's';
				break;

			case KEY_E:
				code = 'e';
				break;

			case KEY_X:
				code = 'x';
				break;

			case KEY_Y:
				code = 'y';
				break;

			case KEY_BACK:
				/* Reset value */
				switch (mode)
				{
				case 's':
					{
						VGfloat tgx1 = gx1;
						VGfloat tgy1 = gy1;
						gx1 = (testWidth() - sqx)/2;
						gy1 = (testHeight() - sqy)/2;
						gx2 += gx1 - tgx1;
						gy2 += gy1 - tgy1;
						createLinear();
						break;
					}
				case 'e':
					gx2 = gx1 + sqx;
					gy2 = gy1 + sqy;
					createLinear();
					break;
				case 'x':
					sx = 1.0f;
					break;
				case 'y':
					sy = 1.0f;
					break;
				}

				updateOverlayString();
				break;

			case KEY_SELECT:
				/* Cycle spread mode */
				sindex = (sindex+1) % ssize;
				createLinear();
				switch(sindex)
				{
				case 0: overlay = trs("Gradient Spread mode: PAD\n"); break;
				case 1: overlay = trs("Gradient Spread mode: REPEAT\n"); break;
				case 2: overlay = trs("Gradient Spread mode: REFLECT\n"); break;
				}
				break;

			case KEY_H:
				/* Show help */
				overlay = trs(commands);
				break;

			default:
				isAction = false;
			}

			if(isAction)
			{
				if(code > 0)
				{
					/* Switch mode */
					mode = code;
					updateOverlayString();
				}

				ke->process();
			}
		}
	};
}


namespace shivavg_tp7
{
#ifndef IMAGE_DIR
#  define IMAGE_DIR "./"
#endif

	VGfloat tx=0.0f, ty=0.0f;
	VGfloat sx=1.0f, sy=1.0f;
	VGfloat a=45.0f;

	VGint tindex = 3;
	VGint tsize = 4;
	VGint tile[] = {
		VG_TILE_FILL,
		VG_TILE_PAD,
		VG_TILE_REPEAT,
		VG_TILE_REFLECT
	};

	VGfloat sqx = 200;
	VGfloat sqy = 200;

	VGfloat clickX;
	VGfloat clickY;
	VGfloat startX;
	VGfloat startY;
	char mode = 't';

	spvg_image backImage;
	spvg_image patternImage;
	VGPaint patternFill;
	VGPaint blackFill;
	VGPath org;
	VGPath p;

	VGfloat black[] = {1,1,1, 1};

	const char commands[] =
		"Click & drag mouse to change\n"
		"value for current mode\n\n"
		"H - this help\n"
		"TAB - pattern tiling mode\n"
		"T - translate mode\n"
		"X - scale X mode\n"
		"Y - scale Y mode\n"
		"BACKSPACE - reset value\n";
	string overlay;

	void createPattern()
	{
		VGfloat tileFill[] = {0,0,1,1};
		vgSetParameteri(patternFill, VG_PAINT_TYPE, VG_PAINT_TYPE_PATTERN);
		vgSetParameteri(patternFill, VG_PAINT_PATTERN_TILING_MODE, tile[tindex]);
		vgSetfv(VG_TILE_FILL_COLOR, 4, tileFill);
		vgPaintPattern(patternFill, patternImage->handle());
	}

	void createSquare(VGPath p)
	{
		testMoveTo(p, (testWidth()-sqx)/2, (testHeight()-sqy)/2, VG_ABSOLUTE);
		testLineTo(p, sqx, 0, VG_RELATIVE);
		testLineTo(p, 0, sqy, VG_RELATIVE);
		testLineTo(p, -sqx, 0, VG_RELATIVE);
		testClosePath(p);
	}

	void createOrigin(VGPath p)
	{
		vguEllipse(p, 0,0,10,10);
	}

	void updateOverlayString()
	{
		switch(mode)
		{
		case 't':
			overlay = trs("Translate mode: %f,%f") % tx % ty; break;
		case 'x':
			overlay = trs("Scale X mode: %f") % sx; break;
		case 'y':
			overlay = trs("Scale Y mode: %f") % sy; break;
		}
	}

	void testopenvgInit()
	{
		p = testCreatePath();
		org = testCreatePath();

		blackFill = vgCreatePaint();
		vgSetParameterfv(blackFill, VG_PAINT_COLOR, 4, black);

		backImage = vg_image::load_image(IMAGE_DIR"test_img_violin.png");
		patternImage = vg_image::load_image(IMAGE_DIR"test_img_shivavg.png");
		patternFill = vgCreatePaint();

		createSquare(p);
		createOrigin(org);
		createPattern();
	}

	void testopenvgDestroy()
	{
		vgDestroyPath(p);
		vgDestroyPath(org);
		vgDestroyPaint(blackFill);
		vgDestroyPaint(patternFill);
	}

	void testopenvgDraw(float itx, float ity)
	{
		VGfloat cc[] = {0,0,0,1};

		vgSetfv(VG_CLEAR_COLOR, 4, cc);
		vgClear(0,0,testWidth(),testHeight());

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER);
		vgLoadIdentity();
		vgTranslate(tx, ty);
		vgScale(sx, sy);
		vgRotate(a);

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadIdentity();

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_IMAGE_USER_TO_SURFACE);
		vgSeti(VG_IMAGE_MODE, VG_DRAW_IMAGE_MULTIPLY);
		vgLoadIdentity();

		vgSetPaint(patternFill, VG_FILL_PATH);
		//vgDrawPath(p, VG_FILL_PATH);
		vgDrawImage(backImage->handle());

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadIdentity();
		vgTranslate(tx, ty);
		vgScale(sx, sy);
		vgRotate(a);

		vgSetPaint(blackFill, VG_FILL_PATH | VG_STROKE_PATH);
		vgDrawPath(org, VG_FILL_PATH);
	}


	class shivavg_page : public ux_page
	{
	public:
		shivavg_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			ux_page::init();
			testopenvgInit();
		}

		virtual void on_destroy()
		{
			testopenvgDestroy();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
			{
				on_touch_evt(touch_sym_evt::as_touch_sym_evt(idp));
			}
			else if(idp->is_type(key_evt::KEYEVT_EVT_TYPE))
			{
				on_key_evt(key_evt::as_key_evt(idp));
			}

			ux_page::receive(idp);
		}

		virtual void update_state()
		{
			ux_page::update_state();
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			ux_page::update_view(g);
			testopenvgDraw(uxr.x, uxr.y);

			g->drawText("shivavg_tp7", 10, 10);
			g->drawText(overlay, 10, 50);
		}

		void on_touch_evt(shared_ptr<touch_sym_evt> ts)
		{
			switch(ts->get_type())
			{
			case touch_sym_evt::TS_FIRST_TAP:
				{
					float x = ts->crt_state.pos.x;
					float y = ts->crt_state.pos.y;
					//y = pfm::getScreenHeight() - y;
					clickX = x; clickY = y;

					switch (mode)
					{
					case 't':
						startX = tx;
						startY = ty;
						break;
					case 'x':
						startY = sx;
						break;
					case 'y':
						startY = sy;
						break;
					}

					ts->process();
					break;
				}

			case touch_sym_evt::TS_PRESS_AND_DRAG:
				{
					if(ts->is_finished)
					{
					}
					else
					{
						VGfloat dx, dy;
						float x = ts->crt_state.pos.x;
						float y = ts->crt_state.pos.y;

						//y = pfm::getScreenHeight() - y;
						dx = x - clickX;
						dy = y - clickY;

						switch(mode)
						{
						case 't':
							{
								tx = startX + dx;
								ty = startY + dy;
								break;
							}
						case 'x':
							sx = startY + dy * 0.01;
							break;
						case 'y':
							sy = startY + dy * 0.01;
							break;
						}

						updateOverlayString();
						ts->process();
					}

					break;
				}
			}
		}

		void on_key_evt(shared_ptr<key_evt> ke)
		{
			if(ke->get_type() == key_evt::KE_RELEASED)
			{
				return;
			}

			bool isAction = true;
			char code = 0;

			switch(ke->get_key())
			{
			case KEY_T:
				code = 't';
				break;

			case KEY_X:
				code = 'x';
				break;

			case KEY_Y:
				code = 'y';
				break;

			case KEY_BACK:
				/* Reset value */
				switch (mode)
				{
				case 't':
					tx = 0;
					ty = 0;
					break;
				case 'x':
					sx = 1.0f;
					break;
				case 'y':
					sy = 1.0f;
					break;
				}

				updateOverlayString();
				break;

			case KEY_SELECT:
				/* Cycle spread mode */
				tindex = (tindex+1) % tsize;
				createPattern();
				switch(tindex)
				{
				case 0: overlay = trs("Pattern Tiling mode: FILL\n"); break;
				case 1: overlay = trs("Pattern Tiling mode: PAD\n"); break;
				case 2: overlay = trs("Pattern Tiling mode: REPEAT\n"); break;
				case 3: overlay = trs("Pattern Tiling mode: REFLECT\n"); break;
				}
				break;

			case KEY_H:
				/* Show help */
				overlay = trs(commands);
				break;

			default:
				isAction = false;
			}

			if(isAction)
			{
				if(code > 0)
				{
					/* Switch mode */
					mode = code;
					updateOverlayString();
				}

				ke->process();
			}
		}
	};
}


namespace shivavg_tp8
{
	VGfloat cx, cy;
	VGfloat fx, fy;
	VGfloat r;

	VGfloat sx=1.0f, sy=1.0f;
	VGfloat tx=0.0f, ty=0.0f;
	VGfloat ang=15.0f;

	VGint sindex = 0;
	VGint ssize = 3;
	VGint spread[] =
	{
		VG_COLOR_RAMP_SPREAD_PAD,
		VG_COLOR_RAMP_SPREAD_REPEAT,
		VG_COLOR_RAMP_SPREAD_REFLECT
	};

	VGfloat sqx = 200;
	VGfloat sqy = 200;

	VGPaint radialFill;
	VGPaint blackFill;
	VGPath center;
	VGPath focus;
	VGPath radius;
	VGPath p;

	VGfloat clickX;
	VGfloat clickY;
	VGfloat startX;
	VGfloat startY;
	char mode = 'c';

	VGfloat black[] = {1,1,1,1};

	const char commands[] =
		"Click & drag mouse to change\n"
		"value for current mode\n\n"
		"H - this help\n"
		"TAB - gradient spread mode\n"
		"C - gradient center mode\n"
		"F - gradient focus mode\n"
		"R - gradient radius mode\n"
		"X - scale X mode\n"
		"Y - scale Y mode\n"
		"BACKSPACE - reset value\n";
	string overlay;

	void createCross(VGPath p, VGfloat x, VGfloat y)
	{
		VGfloat D=20;
		testMoveTo(p, x-D/2,     y, VG_ABSOLUTE);
		testLineTo(p,     D,     0, VG_RELATIVE);
		testMoveTo(p,     x, y-D/2, VG_ABSOLUTE);
		testLineTo(p,     0,     D, VG_RELATIVE);
	}

	void createArrow(VGPath p, VGfloat cx, VGfloat cy, VGfloat dx, VGfloat dy)
	{
		VGfloat px = -dy, py = dx;
		testMoveTo(p, cx + 10*dx, cy + 10*dy, VG_ABSOLUTE);
		testLineTo(p, cx +  5*px, cy +  5*py, VG_ABSOLUTE);
		testLineTo(p, cx -  5*px, cy -  5*py, VG_ABSOLUTE);
		testClosePath(p);
	}

	void createRadial()
	{
		VGfloat stops[] =
		{
			0.0, 1.0, 0.0, 0.0, 1,
			0.5, 0.0, 1.0, 0.0, 1,
			1.0, 0.0, 0.0, 1.0, 1
		};

		VGint numstops = sizeof(stops) / sizeof(VGfloat);

		VGfloat radial[5];
		radial[0] = cx;
		radial[1] = cy;
		radial[2] = fx;
		radial[3] = fy;
		radial[4] = r;

		vgSetParameteri(radialFill, VG_PAINT_COLOR_RAMP_SPREAD_MODE, spread[sindex]);
		vgSetParameteri(radialFill, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);
		vgSetParameterfv(radialFill, VG_PAINT_RADIAL_GRADIENT, 5, radial);
		vgSetParameterfv(radialFill, VG_PAINT_COLOR_RAMP_STOPS, numstops, stops);

		vgClearPath(center, VG_PATH_CAPABILITY_ALL);
		createCross(center, cx, cy);

		vgClearPath(focus, VG_PATH_CAPABILITY_ALL);
		vguEllipse(focus, fx, fy, 8,8);

		vgClearPath(radius, VG_PATH_CAPABILITY_ALL);
		vguEllipse(radius, cx, cy, r*2, r*2);
	}

	void createSquare(VGPath p)
	{
		testMoveTo(p, (testWidth()-sqx)/2, (testHeight()-sqy)/2, VG_ABSOLUTE);
		testLineTo(p, sqx, 0, VG_RELATIVE);
		testLineTo(p, 0, sqy, VG_RELATIVE);
		testLineTo(p, -sqx, 0, VG_RELATIVE);
		testClosePath(p);
	}

	void updateOverlayString()
	{
		switch(mode)
		{
		case 'c':
			overlay = trs("Gradient Center mode"); break;
		case 'f':
			overlay = trs("Gradient Focus mode"); break;
		case 'r':
			overlay = trs("Gradient Radius mode"); break;
		case 'x':
			overlay = trs("Scale X mode: %f") % sx; break;
		case 'y':
			overlay = trs("Scale Y mode: %f") % sy; break;
		}
	}

	void testopenvgInit()
	{
		p = testCreatePath();
		center = testCreatePath();
		focus = testCreatePath();
		radius = testCreatePath();

		cx = testWidth()/2;
		cy = testHeight()/2;
		fx = cx;
		fy = cy;
		r = sqx/2;

		radialFill = vgCreatePaint();
		blackFill = vgCreatePaint();
		vgSetParameterfv(blackFill, VG_PAINT_COLOR, 4, black);

		createSquare(p);
		createRadial();
	}

	void testopenvgDestroy()
	{
		vgDestroyPath(p);
		vgDestroyPath(center);
		vgDestroyPath(focus);
		vgDestroyPath(radius);
		vgDestroyPaint(radialFill);
		vgDestroyPaint(blackFill);
	}

	void testopenvgDraw(float itx, float ity)
	{
		VGfloat cc[] = {0,0,0,1};

		vgSetfv(VG_CLEAR_COLOR, 4, cc);
		vgClear(0,0,testWidth(),testHeight());

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER);
		vgLoadIdentity();
		vgTranslate(tx, ty);
		vgRotate(ang);
		vgScale(sx, sy);

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadIdentity();

		vgSetPaint(radialFill, VG_FILL_PATH);
		vgDrawPath(p, VG_FILL_PATH);

		vgTranslate(tx, ty);
		vgRotate(ang);
		vgScale(sx, sy);

		vgSetPaint(blackFill, VG_FILL_PATH | VG_STROKE_PATH);
		vgDrawPath(radius, VG_STROKE_PATH);
		vgDrawPath(center, VG_STROKE_PATH);
		vgDrawPath(focus, VG_FILL_PATH);
	}


	class shivavg_page : public ux_page
	{
	public:
		shivavg_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			ux_page::init();
			testopenvgInit();
		}

		virtual void on_destroy()
		{
			testopenvgDestroy();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
			{
				on_touch_evt(touch_sym_evt::as_touch_sym_evt(idp));
			}
			else if(idp->is_type(key_evt::KEYEVT_EVT_TYPE))
			{
				on_key_evt(key_evt::as_key_evt(idp));
			}

			ux_page::receive(idp);
		}

		virtual void update_state()
		{
			ux_page::update_state();
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			ux_page::update_view(g);
			testopenvgDraw(uxr.x, uxr.y);

			g->drawText("shivavg_tp8", 10, 10);
			g->drawText(overlay, 10, 50);
		}

		void on_touch_evt(shared_ptr<touch_sym_evt> ts)
		{
			switch(ts->get_type())
			{
			case touch_sym_evt::TS_FIRST_TAP:
				{
					float x = ts->crt_state.pos.x;
					float y = ts->crt_state.pos.y;
					//y = pfm::getScreenHeight() - y;
					clickX = x; clickY = y;

					switch (mode)
					{
					case 'c':
						startX = cx;
						startY = cy;
						break;
					case 'f':
						startX = fx;
						startY = fy;
						break;
					case 'r':
						startY = r;
						break;
					case 'x':
						startY = sx;
						break;
					case 'y':
						startY = sy;
						break;
					}

					ts->process();
					break;
				}

			case touch_sym_evt::TS_PRESS_AND_DRAG:
				{
					if(ts->is_finished)
					{
					}
					else
					{
						VGfloat dx, dy;
						float x = ts->crt_state.pos.x;
						float y = ts->crt_state.pos.y;

						//y = pfm::getScreenHeight() - y;
						dx = x - clickX;
						dy = y - clickY;

						switch(mode)
						{
						case 'c':
							{
								VGfloat dcx, dcy;
								dcx = startX + dx - cx;
								dcy = startY + dy - cy;
								cx = startX + dx;
								cy = startY + dy;
								fx += dcx;
								fy += dcy;
								createRadial();
								break;
							}
						case 'f':
							fx = startX + dx;
							fy = startY + dy;
							createRadial();
							break;
						case 'r':
							r = startY + dy;
							createRadial();
							break;
						case 'x':
							sx = startY + dy * 0.01f;
							break;
						case 'y':
							sy = startY + dy * 0.01f;
							break;
						}

						updateOverlayString();
						ts->process();
					}

					break;
				}
			}
		}

		void on_key_evt(shared_ptr<key_evt> ke)
		{
			if(ke->get_type() == key_evt::KE_RELEASED)
			{
				return;
			}

			bool isAction = true;
			char code = 0;

			switch(ke->get_key())
			{
			case KEY_C:
				code = 'c';
				break;

			case KEY_F:
				code = 'f';
				break;

			case KEY_R:
				code = 'r';
				break;

			case KEY_X:
				code = 'x';
				break;

			case KEY_Y:
				code = 'y';
				break;

			case KEY_BACK:
				/* Reset value */
				switch (mode)
				{
				case 'c':
					cx = testWidth()/2;
					cy = testHeight()/2;
					fx = cx; fy = cy;
					createRadial();
					break;
				case 'f':
					fx = cx; fy = cy;
					createRadial();
					break;
				case 'r':
					r = sqx;
					createRadial();
					break;
				case 'x':
					sx = 1.0f;
					break;
				case 'y':
					sy = 1.0f;
					break;
				}

				updateOverlayString();
				break;

			case KEY_SELECT:
				/* Cycle spread mode */
				sindex = (sindex+1) % ssize;
				createRadial();
				switch(sindex)
				{
				case 0: overlay = trs("Gradient Spread mode: PAD"); break;
				case 1: overlay = trs("Gradient Spread mode: REPEAT"); break;
				case 2: overlay = trs("Gradient Spread mode: REFLECT"); break;
				}
				break;

			case KEY_H:
				/* Show help */
				overlay = trs(commands);
				break;

			default:
				isAction = false;
			}

			if(isAction)
			{
				if(code > 0)
				{
					/* Switch mode */
					mode = code;
					updateOverlayString();
				}

				ke->process();
			}
		}
	};
}


namespace shivavg_tp9
{
#include "svg-tiger-paths.inc"
	VGPath *tigerPaths = NULL;
	VGPaint tigerStroke;
	VGPaint tigerFill;

	VGfloat sx=1.0f, sy=1.0f;
	VGfloat tx=1.0f, ty=1.0f;
	VGfloat ang = 0.0f;
	int animate = 1;
	char mode = 'z';

	VGfloat startX = 0.0f;
	VGfloat startY = 0.0f;
	VGfloat clickX = 0.0f;
	VGfloat clickY = 0.0f;

	const char commands[] =
		"Click & drag mouse to change\n"
		"value for current mode\n\n"
		"H - this help\n"
		"Z - zoom mode\n"
		"P - pan mode\n"
		"SPACE - animation pause\\play\n";
	string overlay;

	void loadTiger()
	{
		int i;
		VGPath temp;

		temp = testCreatePath();  
		tigerPaths = (VGPath*)malloc(pathCount * sizeof(VGPath));
		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgTranslate(-100,100);
		vgScale(1,-1);

		for (i=0; i<pathCount; ++i) {

			vgClearPath(temp, VG_PATH_CAPABILITY_ALL);
			vgAppendPathData(temp, commandCounts[i],
				commandArrays[i], dataArrays[i]);

			tigerPaths[i] = testCreatePath();
			vgTransformPath(tigerPaths[i], temp);
		}

		tigerStroke = vgCreatePaint();
		tigerFill = vgCreatePaint();
		vgSetPaint(tigerStroke, VG_STROKE_PATH);
		vgSetPaint(tigerFill, VG_FILL_PATH);
		vgLoadIdentity();
		vgDestroyPath(temp);
	}

	void cleanup()
	{
		free(tigerPaths);
	}

	void updateOverlayString()
	{
		switch(mode)
		{
		case 'z':
			overlay = trs("Zoom Mode"); break;
		case 'p':
			overlay = trs("Pan Mode"); break;
		}
	}

	void testopenvgInit()
	{
		loadTiger();
	}

	void testopenvgDestroy()
	{
		cleanup();
	}

	void testopenvgDraw(float itx, float ity, float interval)
	{
		int i;
		const VGfloat *style;
		VGfloat clearColor[] = {1,1,1,1};

		if (animate)
		{
			ang += interval * 360 * 0.1f;
			if (ang > 360) ang -= 360;
		}

		vgSetfv(VG_CLEAR_COLOR, 4, clearColor);
		vgClear(0,0,testWidth(),testHeight());

		vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
		vgLoadIdentity();
		vgTranslate(testWidth()/2 + tx,testHeight()/2 + ty);
		vgScale(sx, sy);
		vgRotate(ang);

		vgSetPaint(tigerStroke, VG_STROKE_PATH);
		vgSetPaint(tigerFill, VG_FILL_PATH);

		for (i=0; i<pathCount; ++i)
		{

			style = styleArrays[i];
			vgSetParameterfv(tigerStroke, VG_PAINT_COLOR, 4, &style[0]);
			vgSetParameterfv(tigerFill, VG_PAINT_COLOR, 4, &style[4]);
			vgSetf(VG_STROKE_LINE_WIDTH, style[8]);
			vgDrawPath(tigerPaths[i], (VGint)style[9]);
		}
	}


	class shivavg_page : public ux_page
	{
	public:
		shivavg_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			ux_page::init();
			testopenvgInit();
		}

		virtual void on_destroy()
		{
			testopenvgDestroy();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
			{
				on_touch_evt(touch_sym_evt::as_touch_sym_evt(idp));
			}
			else if(idp->is_type(key_evt::KEYEVT_EVT_TYPE))
			{
				on_key_evt(key_evt::as_key_evt(idp));
			}

			ux_page::receive(idp);
		}

		virtual void update_state()
		{
			ux_page::update_state();
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			ux_page::update_view(g);
			testopenvgDraw(uxr.x, uxr.y, 0.01f);

			g->drawText("shivavg_tp9", 10, 10);
			g->drawText(overlay, 10, 50);
		}

		void on_touch_evt(shared_ptr<touch_sym_evt> ts)
		{
			switch(ts->get_type())
			{
			case touch_sym_evt::TS_FIRST_TAP:
				{
					float x = ts->crt_state.pos.x;
					float y = ts->crt_state.pos.y;
					//y = pfm::getScreenHeight() - y;
					clickX = x; clickY = y;

					switch (mode)
					{
					case 'z':
						startY = sx;
						break;
					case 'p':
						startX = tx;
						startY = ty;
						break;
					}

					ts->process();
					break;
				}

			case touch_sym_evt::TS_PRESS_AND_DRAG:
				{
					if(ts->is_finished)
					{
					}
					else
					{
						VGfloat dx, dy;
						float x = ts->crt_state.pos.x;
						float y = ts->crt_state.pos.y;

						//y = pfm::getScreenHeight() - y;
						dx = x - clickX;
						dy = y - clickY;

						switch(mode)
						{
						case 'z':
							sx = startY + dy * 0.01;
							sy = sx;
							break;
						case 'p':
							tx = startX + dx;
							ty = startY + dy;
							break;
						}

						updateOverlayString();
						ts->process();
					}

					break;
				}
			}
		}

		void on_key_evt(shared_ptr<key_evt> ke)
		{
			if(ke->get_type() == key_evt::KE_RELEASED)
			{
				return;
			}

			bool isAction = true;
			char code = 0;

			switch(ke->get_key())
			{
			case KEY_Z:
				code = 'z';
				break;

			case KEY_P:
				code = 'p';
				break;

			case KEY_SELECT:
				/* Toggle animation */
				animate = !animate;
				overlay = trs("%s\n") % (animate ? "Play" : "Pause");
				break;

			case KEY_H:
				/* Show help */
				overlay = trs(commands);
				break;

			default:
				isAction = false;
			}

			if(isAction)
			{
				if(code > 0)
				{
					/* Switch mode */
					mode = code;
					updateOverlayString();
				}

				ke->process();
			}
		}
	};
}


void unit_test_openvg::init_shivavg()
{
	ux_page::new_shared_instance(new shivavg_tp1::shivavg_page(uxroot));
	ux_page::new_shared_instance(new shivavg_tp2::shivavg_page(uxroot));
	ux_page::new_shared_instance(new shivavg_tp3::shivavg_page(uxroot));
	ux_page::new_shared_instance(new shivavg_tp4::shivavg_page(uxroot));
	ux_page::new_shared_instance(new shivavg_tp5::shivavg_page(uxroot));
	ux_page::new_shared_instance(new shivavg_tp6::shivavg_page(uxroot));
	ux_page::new_shared_instance(new shivavg_tp7::shivavg_page(uxroot));
	ux_page::new_shared_instance(new shivavg_tp8::shivavg_page(uxroot));
	ux_page::new_shared_instance(new shivavg_tp9::shivavg_page(uxroot));
}

void unit_test_openvg::destroy_shivavg()
{
}

#endif
