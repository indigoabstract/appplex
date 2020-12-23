#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_ABSTRACT_RACING

#include "gfx-camera.hxx"
#include "gfx-tex.hxx"
#include <glm/inc.hpp>
#include <vector>


namespace track_ns
{
	struct vxfmt_v3fc4bn3t2f
	{
		float x, y, z;
		uint8_t r, g, b, a;
		float nx, ny, nz;
		float u, v;
	};
}
using namespace track_ns;
class mws_mod;


class track_section_type
{
public:
	int start_segment;
	int segment_count;
	unsigned char la, lr, lg, lb;
	unsigned char ra, rr, rg, rb;
};


class track_sections
{
public:
	track_sections(int segment_count);
	void set_section_type(track_section_type tst);

	std::vector<track_section_type> track_seg_section_type;
};


class track
{
public:
	track(mws_sp<mws_mod> i_mod);
	void loadTrackData(char* track_name);
	void generateTrackVertices();
	glm::vec3& get_segment_pos_at(int isegment_idx);
	void drawTrack(mws_sp<gfx_camera> r);

	int version_number;
	int* base_points;
	float* points_angle;
	int base_point_count;
	int interpolation_steps_count;

	glm::vec3** ipoints;
	int ipoints_length;
	float* ipoints_width;

	std::vector<unsigned short> road_indices;
	std::vector<vxfmt_v3fc4bn3t2f> road_vertices;
	std::vector<vxfmt_v3fc4bn3t2f> left_wall_vertices;
	std::vector<vxfmt_v3fc4bn3t2f> right_wall_vertices;
	track_sections* trackSections;

	float road_side_width;
	int start_point;
	std::vector<glm::vec3> segment_crd;
	glm::vec3 start_pos;
	mws_sp<gfx_tex> tex;
	mws_wp<mws_mod> mod;
};

#endif
