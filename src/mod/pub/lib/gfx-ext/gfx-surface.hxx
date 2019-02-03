#pragma once

#include "gfx-vxo.hxx"


class gfx_icosahedron : public gfx_vxo
{
public:
   gfx_icosahedron();
   void set_dimensions(float iradius);
};

// variable polygon count
class gfx_vpc_box : public gfx_vxo
{
public:
   gfx_vpc_box();
   void set_dimensions(float iradius, int isegments);
};

class gfx_vpc_kubic_sphere : public gfx_vxo
{
public:
   gfx_vpc_kubic_sphere();
   void set_dimensions(float iradius, int isegments);
};
