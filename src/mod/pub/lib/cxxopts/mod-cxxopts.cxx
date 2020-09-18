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
   try
   {
      int argc = mws::args::get_str_arg_count();
      const char** argv = mws::args::get_str_arg_vect();

      cxxopts::Options options(argv[0], " - example command line options");
      options.add_options()
         ("s,source-path", "source path - directory to search", cxxopts::value<std::string>())
         ("e,exclude-path", "exclude path - list of directories excluded from search", cxxopts::value<std::vector<std::string>>())
         ("h,help", "print usage");

      for (int k = 0; k < argc; k++)
      {
         mws_println("arg [%s]", argv[k]);
      }

      auto result = options.parse(argc, argv);
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
   catch (const cxxopts::OptionException& i_e)
   {
      mws_println("error parsing options: %s", i_e.what());
      mws_mod::set_app_exit_on_next_run(true);
   }
}
