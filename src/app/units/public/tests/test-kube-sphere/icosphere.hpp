#pragma once

#include "unit-test-kube-sphere.hpp"

#ifdef UNIT_TEST_KUBE_SPHERE

#include "gfx-vxo.hpp"
#include "gfx-color.hpp"


class icosphere_face : public gfx_vxo
{
public:
	icosphere_face();
	int get_subdiv_count();
	void gen_geometry(int isubdiv_count);

	float radius;
	gfx_color face_color;
	glm::vec3 corners[3];
	glm::vec3 corner_normals[3];
	glm::vec3 face_normal;
	glm::vec3 middle_point;

private:
	std::vector<int> check_numbers;
	std::vector<int> check_ind;
	int subdiv_count;
};


class icosphere : public gfx_vxo
{
public:
	icosphere();
	virtual void update();
	int get_face_count();
	shared_ptr<icosphere_face> get_face_at(int iidx);
	void set_dimensions(float iradius, int isubdiv_count);

	std::vector<gfx_color> face_colors;
	float radius;
};

#endif
