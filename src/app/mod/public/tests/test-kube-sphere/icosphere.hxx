#pragma once

#include "mod-test-kube-sphere.hxx"

#ifdef MOD_TEST_KUBE_SPHERE

#include "gfx-vxo.hxx"
#include "gfx-color.hxx"


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
	mws_sp<icosphere_face> get_face_at(int iidx);
	void set_dimensions(float iradius, int isubdiv_count);

	std::vector<gfx_color> face_colors;
	float radius;
};

#endif
