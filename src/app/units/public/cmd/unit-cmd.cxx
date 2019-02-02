#include "stdafx.hxx"

#include "unit-cmd.hxx"

#ifdef UNIT_CMD

#include "long-operation.hxx"
#include "com/util/unicode/boost-filesystem-util.hxx"
#include "cmd-line-arg.hxx"
#include <boost/filesystem.hpp>
#include <exception>
#include <string>
#include <vector>

namespace bfs = ::boost::filesystem;


namespace unit_cmd_pref
{
   class unit_preferences_detail : public unit_preferences
   {
   public:
      virtual bool requires_gfx() { return false; }
   };
}


unit_cmd::unit_cmd() : unit(mws_stringify(UNIT_CMD))
{
   prefs = mws_sp<unit_preferences>(new unit_cmd_pref::unit_preferences_detail());
}

mws_sp<unit_cmd> unit_cmd::nwi()
{
   return mws_sp<unit_cmd>(new unit_cmd());
}

bool unit_cmd::update()
{
   if (gfx_available())
   {
      //g->clearScreen();
      //g->drawText(get_name(), 10, 10);
   }

   if (lop)
   {
      if (!lop->is_active())
      {
         lop->join();
         unit::set_app_exit_on_next_run(true);
      }
   }
   else
   {
      unit::set_app_exit_on_next_run(true);
   }

   return true;
}

void unit_cmd::load()
{
   const std::vector<unicodestring>& args = pfm::params::get_app_argument_vector();
   //mws_sp<directoryTree> dirtree = directoryTree::newDirectoryTree(srcPath);
   //rdoListFiles rlf;
   //dirtree->recursiveApply(rlf);

   mws_try
   {
      lop = cmd_line_arg::run(args);

      if (lop)
      {
         long_operation::run_on_separate_thread(lop);
         trx("started operation thread. please wait..\n");
      }
   }
      mws_catch(std::exception& e)
   {
      trx("error. [{}]", e.what());
   }
   mws_catch(...)
   {
      trx("exception of unknown type!");
   }
}

#endif
