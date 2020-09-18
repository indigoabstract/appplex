#include "stdafx.hxx"

#include "mod-cxxopts.hxx"
#include "pfm.hxx"
#include <cxxopts/cxxopts.hpp>
#include <vector>


namespace cxxopts_mod_pref
{
   class mod_preferences_detail : public mws_mod_preferences
   {
   public:
      virtual bool requires_gfx() override { return false; }
   };
}


mod_cxxopts::mod_cxxopts() : mws_mod(mws_stringify(MOD_CXXOPTS))
{
   prefs = std::make_shared<cxxopts_mod_pref::mod_preferences_detail>();
}

mws_sp<mod_cxxopts> mod_cxxopts::nwi()
{
   return mws_sp<mod_cxxopts>(new mod_cxxopts());
}

bool mod_cxxopts::update()
{
   mws_mod::set_app_exit_on_next_run(true);

   return true;
}

void mod_cxxopts::load()
{
   mws_try
   {
      int argc = mws::args::get_str_arg_count();
      const char** argv = mws::args::get_str_arg_vect();
      cxxopts::Options options(argv[0], "");

      options.positional_help("[optional args]").show_positional_help();
      options.add_options()
         ("s,source-path", "required argument containing the directory with the sources path. must be an absolute path.", cxxopts::value<std::string>())
         ("e,exclude-path", "optional list of directories excluded from search. if set, must be relative to the source-path argument", cxxopts::value<std::vector<std::string>>())
         ("h,help", "print usage");

      for (int k = 0; k < argc; k++)
      {
         mws_println("arg [%s]", argv[k]);
      }

      auto result = options.parse(argc, argv);

      if (result.count("h"))
      {
         trx("{}", options.help({ "", "Group" }));
         return;
      }

      auto arg_list = result.arguments();

      for (auto& arg : arg_list)
      {
         mws_println("key [ %s ] : val [ %s ]", arg.key().c_str(), arg.value().c_str());
      }

      if (result.count("e"))
      {
         auto& exclude_path = result["e"].as<std::vector<std::string>>();

         for (const auto& path : exclude_path)
         {
            mws_println("ex path [ %s ]", path.c_str());
         }
      }
   }
   mws_catch (const cxxopts::OptionException& i_e)
   {
      mws_println("error parsing options: %s", i_e.what());
      mws_mod::set_app_exit_on_next_run(true);
   }
}
