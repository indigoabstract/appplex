#include "stdafx.hxx"

#include "mod-cmd.hxx"

#ifdef MOD_CMD

#include "long-operation.hxx"
#include "util/unicode/boost-filesystem-util.hxx"
#include "cmd-line-arg.hxx"
#include <boost/filesystem.hpp>
#include <exception>
#include <string>
#include <vector>

namespace bfs = ::boost::filesystem;


namespace mod_cmd_pref
{
   class mod_preferences_detail : public mws_mod_preferences
   {
   public:
      virtual bool requires_gfx() { return false; }
   };
}


mod_cmd::mod_cmd() : mws_mod(mws_stringify(MOD_CMD))
{
   prefs = mws_sp<mws_mod_preferences>(new mod_cmd_pref::mod_preferences_detail());
}

mws_sp<mod_cmd> mod_cmd::nwi()
{
   return mws_sp<mod_cmd>(new mod_cmd());
}

bool mod_cmd::update()
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
         mws_mod::set_app_exit_on_next_run(true);
      }
   }
   else
   {
      mws_mod::set_app_exit_on_next_run(true);
   }

   return true;
}

void mod_cmd::load()
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
