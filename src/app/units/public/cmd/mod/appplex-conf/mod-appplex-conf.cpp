#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#pragma warning( disable : 4996 )

#include "mod-appplex-conf.hpp"
#include "appplex-conf-info.hpp"
#include "min.hpp"
#include "../../long-operation.hpp"
#include "com/util/unicode/boost-filesystem-util.hpp"
#include "com/util/unicode/boost-program-options-util.hpp"
#include <boost/program_options.hpp>
#include <exception>

using std::string;
using std::vector;
namespace bpo = ::boost::program_options;


namespace appplex_conf_ns
{
   const string SOURCE_PATH = "source-path";
   const string EXCLUDE_PATH = "exclude-path";

   class long_op_appplex_conf : public long_operation
   {
   public:
      long_op_appplex_conf(unicodestring iproj_path, vector<unicodestring> iexclude_path)
      {
         info = std::make_shared<conf_info>();
         info->proj_path = iproj_path;
         info->src_path = info->proj_path;
         info->exclude_path = iexclude_path;
      }

      void run()
      {
         try
         {
            if (exists(info->proj_path))
            {
               if (is_directory(info->proj_path))
               {
                  utrx(untr("starting mod-appplex-conf in directory [{}]"), path2string(info->proj_path));

                  auto appplex_conf_inst = std::make_shared<appplex_conf>(info);
                  appplex_conf_inst->update();

                  auto windows_pc = std::make_shared<windows_pc_project_conf>(info, appplex_conf_inst->unit_entry_map);
                  windows_pc->update_project();

                  auto android_studio = std::make_shared<android_studio_project_conf>(info, appplex_conf_inst->unit_entry_map_android);
                  android_studio->update_project();

                  trc("finished!");
               }
               else
               {
                  throw ia_exception(trs("mod-appplex-conf: {} is not a directory", info->proj_path.string()));
               }
            }
            else
            {
               throw ia_exception(trs("mod-appplex-conf: {} does not exist", info->proj_path.string()));
            }
         }
         catch (ia_exception& e)
         {
            trx("exception {}", e.what());
         }
      }

      std::shared_ptr<conf_info> info;
   };
}
using namespace appplex_conf_ns;


std::string mod_cmd_appplex_conf::get_module_name()
{
   return "appplex-conf";
}


boost::program_options::options_description mod_cmd_appplex_conf::get_options_description()
{
   bpo::options_description desc(trs("available options for module [{}]", get_module_name()));

   desc.add_options()
      (SOURCE_PATH.c_str(), unicodevalue<unicodestring>()->required(), "source path. must be an absolute path")
      (EXCLUDE_PATH.c_str(), unicodevalue<vector<unicodestring> >(), "exclude path. must be relative to the source path")
      ;

   return desc;
}

shared_ptr<long_operation> mod_cmd_appplex_conf::run(const vector<unicodestring>& args)
{
   bpo::options_description desc = get_options_description();
   bpo::variables_map vm;
   bpo::positional_options_description p;

   p.add(EXCLUDE_PATH.c_str(), -1);
   unicode_parsed_options parsed = unicode_command_line_parser(args).options(desc).positional(p).run();

   store(parsed, vm);
   notify(vm);

   vector<unicodestring> exclude_path;

   if (vm.count(EXCLUDE_PATH))
   {
      exclude_path = vm[EXCLUDE_PATH].as<vector<unicodestring> >();
   }

   auto lop = std::make_shared<long_op_appplex_conf>(vm[SOURCE_PATH].as<unicodestring>(), exclude_path);

   return lop;
}

#endif
