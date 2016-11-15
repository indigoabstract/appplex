#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_ABSTRACT_RACING

#include "track.hpp"
#include "gfx.hpp"
#include "gfx-camera.hpp"
#include "com/util/util.hpp"
#include "spline.hpp"
#include "data-sequence.hpp"
#include <glm/gtc/constants.hpp>
#include <cstdio>

using std::vector;


track_sections::track_sections(int segment_count)
{
	track_seg_section_type.resize(segment_count);

	track_section_type normalType = {0, segment_count, 0xff, 0x40, 0x40, 0x40, 0xff, 0xf0, 0xf0, 0xf0};
	set_section_type(normalType);
}

void track_sections::set_section_type(track_section_type tst)
{
	for(int k = 0; k < tst.segment_count && (k + tst.start_segment) < track_seg_section_type.size(); k++)
	{
		track_seg_section_type[k + tst.start_segment] = tst;
	}
}

track::track()
{
}

void track::loadTrackData(char* track_name)
{
	tex = gfx::tex::new_tex_2d("square.png");

	std::string fn = trs("abstract-racing/%1%") % track_name;
	shared_ptr<std::vector<uint8> > res = pfm::filesystem::load_res_byte_vect(fn);
	memory_data_sequence mds(begin_ptr(res), res->size());
	data_sequence_reader_big_endian dsr(&mds);

	version_number = dsr.read_int32();
	base_point_count = dsr.read_int32();
	interpolation_steps_count = dsr.read_int32();
	start_point = dsr.read_int32();
	road_side_width = dsr.read_real32();

	int bpSize = base_point_count * 5;
	base_points = new int[bpSize];

	for (int i = 0; i < bpSize; i++)
	{
		if (i < start_point * 5)
		{
			base_points[bpSize + i - start_point * 5] = dsr.read_int32();
		}
		else
		{
			base_points[i - start_point * 5] = dsr.read_int32();                    
		}
	}
}

void track::generateTrackVertices()
{
	int roadWallHeight = 50;
	float* xPoints = new float[base_point_count];
	float* yPoints = new float[base_point_count];
	float* zPoints = new float[base_point_count];
	float* wPoints = new float[base_point_count];
	float* aPoints = new float[base_point_count];

	for (int i = 0; i < base_point_count; i++)
	{
		xPoints[i] = (float)base_points[i * 5 + 0];
		yPoints[i] = (float)base_points[i * 5 + 1];
		zPoints[i] = (float)base_points[i * 5 + 2];
		wPoints[i] = (float)base_points[i * 5 + 3];
		aPoints[i] = (float)base_points[i * 5 + 4];
	}

	//calculate splines between the base points (for x,y,z components plus width and orientation of the road)
	int splineLength = base_point_count;
	spline** xSpline = spline::calculateClosedCubic(base_point_count-1, xPoints);
	spline** ySpline = spline::calculateClosedCubic(base_point_count-1, yPoints);
	spline** zSpline = spline::calculateClosedCubic(base_point_count-1, zPoints);
	spline** wSpline = spline::calculateClosedCubic(base_point_count-1, wPoints);
	spline** aSpline = spline::calculateClosedCubic(base_point_count-1, aPoints);

	//create and store interpolated 3d points
	int size = (interpolation_steps_count*base_point_count);
	ipoints = new glm::vec3*[size];
	ipoints_length = size;
	ipoints_width = new float[size];
	points_angle = new float[size];
	trackSections = new track_sections(ipoints_length);

	int trackSectionCount = 5;
	track_section_type tst[] =
	{
		{0, 5, 0xff, 0, 0, 0x20, 0xff, 0, 0, 0xc0},
		{20, 7, 0xff, 0x20, 0, 0, 0xff, 0xc0, 0, 0},
		{70, 7, 0xff, 0, 0x20, 0, 0xff, 0, 0xc0, 0},
		{90, 7, 0xff, 0x40, 0x40, 0, 0xff, 0xff, 0xff, 0},
		{120, 7, 0xff, 0x10, 0, 0x20, 0xff, 0x5f, 0, 0xc0},
	};

	for(int k = 0; k < trackSectionCount; k++)
	{
		trackSections->set_section_type(tst[k]);
	}

	int iPointNr = 0;

	for (int i = 0; i < base_point_count; i++)
	{
		for (int j = 0; j < interpolation_steps_count; j++)
		{
			float u = j / (float)interpolation_steps_count;
			ipoints[iPointNr] = new glm::vec3(xSpline[i]->eval(u), ySpline[i]->eval(u), zSpline[i]->eval(u));
			ipoints_width[iPointNr] = wSpline[i]->eval(u);
			points_angle[iPointNr] = aSpline[i]->eval(u);
			iPointNr++;
		}
	}

	//generate vertex positions from the iPoints
	int roadVerticesLength = ipoints_length * 2 * 3;

	// indices
	int roadTriangleCount = roadVerticesLength / 3 - 2;
	int roadIndicesCount = (roadTriangleCount + 2) * 3;
	road_indices.resize(roadIndicesCount);

	for(int k = 0 ; k < roadTriangleCount / 2; k++)
	{
		road_indices[6 * k + 0] = 2*k + 0;
		road_indices[6 * k + 1] = 2*k + 1;
		road_indices[6 * k + 2] = 2*k + 2;
		road_indices[6 * k + 3] = 2*k + 2;
		road_indices[6 * k + 4] = 2*k + 1;
		road_indices[6 * k + 5] = 2*k + 3;
	}

	// last quad
	int k = roadTriangleCount;
	road_indices[3 * k] = k;
	road_indices[3 * k + 1] = k + 1;
	road_indices[3 * k + 2] = 0;

	k++;
	road_indices[3 * k] = 1;
	road_indices[3 * k + 1] = 0;
	road_indices[3 * k + 2] = k;

	// road vertices
	road_vertices.resize(ipoints_length*2);
	left_wall_vertices.resize(ipoints_length*2);
	right_wall_vertices.resize(ipoints_length*2);

	float uv[][2] = {{0, 0}, {1, 0}, {0, 1}, {1, 1}};

	for (int i = 0, uvx = 0; i < ipoints_length; i++)
	{
		glm::vec3 vTmp;

		if (i < ipoints_length-1)
		{
			vTmp = *ipoints[i+1] - *ipoints[i];
		}
		else
		{
			vTmp = *ipoints[0] - *ipoints[i];
		}

		vTmp = glm::normalize(vTmp);
		glm::vec3 vUp = glm::vec3(0, 1, 0);
		float angle = (float)(points_angle[i]*glm::pi<float>())/180.0f;

		if (angle != 0)
		{
			float nx = vUp.x*cosf(angle) - vUp.y*sinf(angle);
			float ny = vUp.x*sinf(angle) + vUp.y*cosf(angle);
			float nz = vUp.z;
			vUp = glm::vec3(nx, ny, nz);
		}

		glm::vec3 vD = glm::cross(vTmp, vUp);
		vD *= (ipoints_width[i] + road_side_width);
		glm::vec3 v3 = *ipoints[i] - vD; //left side
		glm::vec3 v4 = *ipoints[i] + vD; //right

		track_section_type& t = trackSections->track_seg_section_type[i];

		//left vertices
		vxfmt_v3fc4bn3t2f vbrl = {v3.x, v3.y, v3.z, t.lr, t.lg, t.lb, t.la, 1, 1, 1, uv[uvx][0], uv[uvx][1]};
		vxfmt_v3fc4bn3t2f vblwl = {v3.x, v3.y + roadWallHeight, v3.z, t.lr, t.lg, t.lb, t.la, 1, 1, 1, uv[uvx][0], uv[uvx][1]};
		vxfmt_v3fc4bn3t2f vbrwl = {v4.x, v4.y + roadWallHeight, v4.z, t.lr, t.lg, t.lb, t.la, 1, 1, 1, uv[uvx][0], uv[uvx][1]};

		road_vertices[2 * i + 0] = vbrl;
		left_wall_vertices[2 * i + 0] = vblwl;
		right_wall_vertices[2 * i + 0] = vbrwl;
		uvx++;

		//right vertices
		vxfmt_v3fc4bn3t2f vbrr = {v4.x, v4.y, v4.z, t.rr, t.rg, t.rb, t.ra, 1, 1, 1, uv[uvx][0], uv[uvx][1]};
		vxfmt_v3fc4bn3t2f vblwr = {v3.x, v3.y, v3.z, t.rr, t.rg, t.rb, t.ra, 1, 1, 1, uv[uvx][0], uv[uvx][1]};
		vxfmt_v3fc4bn3t2f vbrwr = {v4.x, v4.y, v4.z, t.rr, t.rg, t.rb, t.ra, 1, 1, 1, uv[uvx][0], uv[uvx][1]};

		road_vertices[2 * i + 1] = vbrr;
		left_wall_vertices[2 * i + 1] = vblwr;
		right_wall_vertices[2 * i + 1] = vbrwr;
		uvx++;

		uvx %= 4;

		glm::vec3 pos;
		pos.x = (road_vertices[2 * i].x + road_vertices[2 * i + 1].x) / 2;
		pos.y = (road_vertices[2 * i].y + road_vertices[2 * i + 1].y) / 2;
		pos.z = (road_vertices[2 * i].z + road_vertices[2 * i + 1].z) / 2;
		segment_crd.push_back(pos);
	}

	start_pos = segment_crd[start_point * interpolation_steps_count];
}

glm::vec3& track::get_segment_pos_at(int isegment_idx)
{
	isegment_idx %= segment_crd.size();

	return segment_crd[isegment_idx];
}

void track::drawTrack(shared_ptr<gfx_camera> r)
{
	//r->tx.set_texture_id(tex->get_texture_id());

	//decl_scglpl(pl1)
	//{
	//	{gl::CULL_FACE, gl::TRUE_GL}, {gl::CULL_FACE_MODE, gl::BACK_GL},
	//	{},
	//};
	//r->st.set_state(pl1);

	//r->vx.set_vertex_data(begin_ptr(road_vertices), &vxfmt_v3fc4bn3t2f::vxi);
 //   r->draw_elements(GLPT_TRIANGLES, road_indices.size(), begin_ptr(road_indices));

	//r->st.disable_state(gl::CULL_FACE);
	//r->vx.set_vertex_data(begin_ptr(left_wall_vertices), &vxfmt_v3fc4bn3t2f::vxi);
 //   r->draw_elements(GLPT_TRIANGLES, road_indices.size(), begin_ptr(road_indices));

	//r->vx.set_vertex_data(begin_ptr(right_wall_vertices), &vxfmt_v3fc4bn3t2f::vxi);
 //   r->draw_elements(GLPT_TRIANGLES, road_indices.size(), begin_ptr(road_indices));
}

#endif
