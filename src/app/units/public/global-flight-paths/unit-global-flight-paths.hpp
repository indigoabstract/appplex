#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_GLOBAL_FLIGHT_PATHS

#include "unit.hpp"


class unit_global_flight_paths : public unit
{
public:
   static shared_ptr<unit_global_flight_paths> new_instance();

   virtual void init_ux() override;

private:
   unit_global_flight_paths();
};

#endif
