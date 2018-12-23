#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_VORONOI_KEYBOARD

#include "unit.hpp"


class unit_voronoi_keyboard : public unit
{
public:
   static shared_ptr<unit_voronoi_keyboard> nwi();

   void init() override;
   void init_mws() override;
   void load() override;

private:
   unit_voronoi_keyboard();
};

#endif
