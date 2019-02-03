#pragma once

#include "pfm-def.h"
#include "gfx-state-const.hxx"

class gl_state_impl;

#define decl_gfxp(name)			gfx_param name
#define decl_cgfxp(name)		const gfx_param name
#define decl_sgfxp(name)		static gfx_param name
#define decl_scgfxp(name)		static const gfx_param name

#define decl_gfxpl(name)		gfx_param name[] =
#define decl_cgfxpl(name)		const gfx_param name[] =
#define decl_sgfxpl(name)		static gfx_param name[] =
#define decl_scgfxpl(name)		static const gfx_param name[] =

#define array_ptr(name) (unsigned int)name


union if_val
   // floating point values must always be defined explicitly as such, i.e. 1.f instead of 1
   // otherwise the compiler will assume we're defining an integer and the
   // floating point value in the union will be wrong since they use the same memory
   // (the bit representation of 1 as an integer is NOT 1.f as a float)
{
   if_val() : int_val(0) {}
   if_val(uint32 iint) : int_val(iint) {}
   if_val(int32 iint) : int_val(iint) {}
   if_val(float ifloat) : float_val(ifloat) {}
   if_val(double idouble) : float_val((float)idouble) {}

   gfx_uint int_val;
   gfx_float float_val;
};


struct gfx_param
{
   gl::rsv_states name;
   if_val val[4];
};


class gfx_state
{
public:
   virtual ~gfx_state() {}

   bool is_enabled(gl::rsv_states istate);
   void enable_state(gl::rsv_states istate);
   void enable_state(gfx_param iplist[], int ielem_count = 0);
   void disable_state(gl::rsv_states istate);
   void disable_state(gfx_param iplist[], int ielem_count = 0);
   void get_state(const gfx_param iplist[], int ielem_count = 0);
   void set_state(const gfx_param& iplist);
   void set_state(const gfx_param iplist[], int ielem_count = 0);
   void sync_opengl_read_state();
   void sync_opengl_write_state();

private:
   friend class gfx;
   gfx_state();
   mws_sp<gl_state_impl> rsi;
};
