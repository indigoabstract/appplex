#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "appplex-conf-info.hpp"
#include "rec-dir-op-appplex-conf.hpp"
#include "../../recursive-dir.hpp"
#include "../rdo-recursive-copy.hpp"
#include "data-sequence.hpp"
#include <kxmd/kxmd.hpp>
#include <kxmd/kx-krte.hpp>
#include <kxmd/kx-elem.hpp>


std::unordered_map<std::string, std::string> platf_def_map =
{
   { "android", "PLATFORM_ANDROID" },{ "ios", "PLATFORM_IOS" },{ "emscripten", "PLATFORM_EMSCRIPTEN" },
   { "qt_windows", "PLATFORM_QT_WINDOWS_PC" },{ "windows", "PLATFORM_WINDOWS_PC" },
};


appplex_conf::appplex_conf(std::shared_ptr<conf_info> iinfo)
{
   info = iinfo;
}

void appplex_conf::update()
{
   const std::string auto_gen = "// this file is autogenerated. do not edit.\n";
   info->appplex_conf_name = "appplex-conf.kxmd";

   trx("\nloading {}\n", info->appplex_conf_name);

   bfs::path axc_path = info->proj_path / info->appplex_conf_name;
   auto sgmd_txt = pfm::filesystem::load_res_as_string(axc_path.generic_string());

   if (!sgmd_txt)
   {
      throw ia_exception(trs("error: cannot find {}.", info->appplex_conf_name));
   }

   info->krt = std::make_shared<kx_krte>();
   info->krt->set_src(sgmd_txt);
   info->krt->run();
   auto kxmd = info->krt->kxb;
   auto paths_proj_rel_src_path = kxmd_ops::get_kxmd_str_seq("paths.proj-rel-src-path", kxmd);
   auto paths_proj_rel_units_path = kxmd_ops::get_kxmd_str_seq("paths.proj-rel-units-path", kxmd);

   trx("\nloading the source file tree.\n");

   info->src_path /= paths_proj_rel_src_path[0];
   info->proj_rel_units_path = paths_proj_rel_units_path[0];
   unit_entry_map = std::make_shared<unit_entry_map_type>();
   unit_entry_map_android = std::make_shared<unit_entry_map_type>();
   trx("\nrecursive reading.\n");
   info->dir_tree = directory_tree::new_directory_tree(info->src_path, info->exclude_path);
   info->rdo = std::make_shared<rec_dir_op_appplex_conf>(info->src_path, info->exclude_path, info->krt, unit_entry_map);
   trx("\nrecursive apply.\n");
   info->dir_tree->recursive_apply(*info->rdo);

   trx("\nupdating appplex configuration.\n");


   std::vector<std::string> default_uses = kxmd_ops::get_kxmd_str_seq("default.uses", kxmd, { "gfx", "boost" });
   bool default_enabled = kxmd_ops::get_bool_from_list(kxmd_ops::get_kxmd_str_seq("default.enabled", kxmd, { "true" }));
   std::vector<std::string> start_unit_list = kxmd_ops::get_kxmd_str_seq("default.start.unit-list", kxmd, { "test" });
   bool launch_unit = kxmd_ops::get_bool_from_list(kxmd_ops::get_kxmd_str_seq("default.start.launch-unit", kxmd, { "true" }));
   std::vector<std::string> default_platf = kxmd_ops::get_kxmd_str_seq("default.platf", kxmd, { "all" });
   bool default_exclusive = kxmd_ops::get_bool_from_list(kxmd_ops::get_kxmd_str_seq("default.start.exclusive", kxmd, { "false" }));
   bool single_unit_build = (start_unit_list.size() == 1) && default_exclusive && launch_unit;
   std::unordered_map<std::string, bool> dependencies_def_map;
   std::unordered_map<std::string, bool> unit_dependencies_def_map;

   bfs::path appplex_conf_path = info->proj_path / kxmd_ops::get_kxmd_str_seq("paths.proj-rel-appplex-conf-path", kxmd, { "" })[0];
   std::string appplex_conf_hpp = kxmd_ops::get_kxmd_str_seq("paths.appplex-conf-hpp-name", kxmd, { "" })[0];
   std::string appplex_conf_cpp = kxmd_ops::get_kxmd_str_seq("paths.appplex-conf-cpp-name", kxmd, { "" })[0];
   auto hpp = pfm_file::get_inst(appplex_conf_hpp, appplex_conf_path.generic_string());
   auto cpp = pfm_file::get_inst(appplex_conf_cpp, appplex_conf_path.generic_string());

   hpp->io.open("w+t");
   cpp->io.open("w+t");

   auto rw_hpp = rw_file_sequence::new_inst(hpp);
   auto rw_cpp = rw_file_sequence::new_inst(cpp);

   {
      auto px_names = kxmd_ops::get_kxmd_str_seq("units", kxmd);
      std::string unit_list;

      rw_hpp->w.write_line(auto_gen);
      rw_hpp->w.write_line("#include \"pfm-def.h\"");
      rw_hpp->w.write_line("\n");

      rw_cpp->w.write_line("#include \"stdafx.h\"");
      rw_cpp->w.write_line(auto_gen);
      rw_cpp->w.write_line("#include \"" + appplex_conf_hpp + "\"");
      rw_cpp->w.write_line("#include \"unit.hpp\"");

      for (int k = 0; k < px_names.size(); k++)
      {
         std::string file_name = px_names[k];
         std::string hpp_name = "unit-" + file_name + ".hpp";
         std::string unit_path = "units." + file_name;
         bool enabled = default_enabled;
         auto enabled_seq = kxmd_ops::get_kxmd_str_seq(unit_path + ".enabled", kxmd);
         bool define_active = true;

         std::string def = file_name;
         std::replace(def.begin(), def.end(), '-', '_');
         std::transform(def.begin(), def.end(), def.begin(), ::toupper);
         std::string unit_def = "UNIT_" + def;
         def = "#define " + unit_def;

         if (!enabled_seq.empty())
         {
            enabled = kxmd_ops::get_bool_from_list(enabled_seq);
         }

         if (enabled)
         {
            auto ue = (*unit_entry_map)[file_name];

            if (ue)
            {
               enabled = false;
               std::vector<std::string> uses = kxmd_ops::get_kxmd_str_seq(unit_path + ".uses", kxmd);
               auto platf_seq = kxmd_ops::get_kxmd_str_seq(unit_path + ".platf", kxmd);
               auto platf = platf_seq;
               std::string line;

               if (platf.empty())
               {
                  platf = default_platf;
               }

               // if it's not enabled for 'all' platforms, put an '#if defined' for each platform
               if (std::find(platf.begin(), platf.end(), "all") == platf.end())
               {
                  ue->platforms = platf;
               }
               else
               {
                  for (auto& key : platf_def_map)
                  {
                     ue->platforms.push_back(key.first);
                  }
               }

               if (uses.empty())
               {
                  uses = default_uses;
               }

               if (single_unit_build)
               {
                  if (file_name == start_unit_list[0])
                  {
                     update_dependencies(ue, uses, dependencies_def_map);
                  }
               }
               else
               {
                  if (default_exclusive)
                  {
                     if (std::find(start_unit_list.begin(), start_unit_list.end(), file_name) != start_unit_list.end())
                     {
                        update_dependencies(ue, uses, dependencies_def_map);
                     }
                  }
                  else
                  {
                     update_dependencies(ue, uses, dependencies_def_map);
                  }
               }

               ue->uses = uses;

               if (std::find(start_unit_list.begin(), start_unit_list.end(), file_name) != start_unit_list.end())
               {
                  unit_list += get_new_unit_line(ue, file_name, launch_unit);
                  line = "#include \"" + ue->unit_hpp_path.generic_string() + "\"";
                  rw_cpp->w.write_line(line);
                  def = get_define_unit_line(ue, def, file_name, platf);
                  unit_dependencies_def_map[unit_def] = true;
                  ue->enabled = enabled = true;
               }
               else
               {
                  if (!default_exclusive)
                  {
                     unit_list += get_new_unit_line(ue, file_name);
                     line = "#include \"" + ue->unit_hpp_path.generic_string() + "\"";
                     rw_cpp->w.write_line(line);
                     def = get_define_unit_line(ue, def, file_name, platf);
                     unit_dependencies_def_map[unit_def] = true;
                     ue->enabled = enabled = true;
                  }
               }
            }
         }

         if (!enabled)
         {
            def = "// " + def;
         }

         rw_hpp->w.write_line(def);
      }

      rw_hpp->w.write_line("\n");

      for (auto i : dependencies_def_map)
      {
         std::string s = i.first;

         std::transform(s.begin(), s.end(), s.begin(), ::toupper);

         if (starts_with(s, "UNIT-"))
         {
            std::replace(s.begin(), s.end(), '-', '_');

            if (unit_dependencies_def_map.find(s) == unit_dependencies_def_map.end())
            {
               s = "#define " + s;
               rw_hpp->w.write_line(s);
            }
         }
         else
         {
            s = "#define MOD_" + s;
            rw_hpp->w.write_line(s);
         }
      }

      if (single_unit_build)
      {
         rw_hpp->w.write_line("");
         rw_hpp->w.write_line("#define SINGLE_UNIT_BUILD");
         rw_hpp->w.write_line("");
      }

      rw_cpp->w.write_line("\nvoid app_units_setup::create_units(shared_ptr<unit_list> ul0)\n{\n\tul = ul0;\n");
      rw_cpp->w.write_line(unit_list + "}");
   }
}

std::string appplex_conf::get_define_unit_line(shared_ptr<unit_entry> iue, const std::string& idef, const std::string& ifile_name, const std::vector<std::string>& iplatf)
{
   std::string def = idef;

   if (iplatf.empty())
   {
      def = "// " + def;
   }

   // if it's not enabled for 'all' platforms, put an '#if defined' for each platform
   else if (std::find(iplatf.begin(), iplatf.end(), "all") == iplatf.end())
   {
      std::string prefx = "\n#if defined " + platf_def_map[iplatf[0]];

      for (int k = 1; k < iplatf.size(); k++)
      {
         prefx += " || defined " + platf_def_map[iplatf[k]];
      }

      def = prefx + "\n\t" + def + "\n#endif\n";
   }

   return def;
}

std::string appplex_conf::get_new_unit_line(shared_ptr<unit_entry> iue, const std::string& ifile_name, bool is_selected)
{
   std::string line;
   std::string def = ifile_name;
   std::replace(def.begin(), def.end(), '-', '_');
   std::string unit_name = "unit_" + def;
   std::transform(def.begin(), def.end(), def.begin(), ::toupper);
   def = "#ifdef UNIT_" + def;
   bfs::path unit_path = info->proj_rel_units_path / iue->unit_path;

   if (is_selected)
   {
      line = trs("\tadd_unit({0}::new_instance(), \"{1}\", true);\n", unit_name, unit_path.generic_string());
   }
   else
   {
      line = trs("\tadd_unit({0}::new_instance(), \"{1}\");\n", unit_name, unit_path.generic_string());
   }

   line = def + "\n" + line + "#endif\n\n";

   return line;
}

void appplex_conf::update_dependencies(shared_ptr<unit_entry> ue, std::vector<std::string>& iuses, std::unordered_map<std::string, bool>& idependencies_def_map)
{
   auto& pf = ue->platforms;
   bool add_android = false;

   if (std::find(pf.begin(), pf.end(), "android") != pf.end())
   {
      (*unit_entry_map_android)[ue->unit_name] = ue;
      add_android = true;
   }

   for (int k = 0; k < iuses.size(); k++)
   {
      std::string s = iuses[k];

      if (starts_with(s, "unit-"))
      {
         std::string un = s.substr(5, s.length() - 5);
         auto ue2 = (*unit_entry_map)[un];

         if (ue2 && add_android)
         {
            (*unit_entry_map_android)[ue2->unit_name] = ue2;
         }
      }

      idependencies_def_map[s] = true;
   }
}


platform_project_conf::platform_project_conf(std::shared_ptr<conf_info> iinfo, shared_ptr<unit_entry_map_type> iunit_entry_map)
{
   info = iinfo;
   unit_entry_map = iunit_entry_map;

   calc_modules_used();
}

bool platform_project_conf::uses_mod(std::string imod_name)
{
   return uses_mod_map.find(imod_name) != uses_mod_map.end();
}

void platform_project_conf::calc_modules_used()
{
   for (auto ue : *unit_entry_map)
   {
      if (ue.second)
      {
         auto& uses = ue.second->uses;

         for (auto& mod : uses)
         {
            uses_mod_map[mod] = true;
         }
      }
   }
}


android_studio_project_conf::android_studio_project_conf(std::shared_ptr<conf_info> iinfo, shared_ptr<unit_entry_map_type> iunit_entry_map) :
   platform_project_conf(iinfo, iunit_entry_map) {}

void android_studio_project_conf::update_project()
{
   trx("\nupdating the android studio project.\n");

   auto kxmd = info->krt->kxb;
   auto platforms_android_root_path = kxmd_ops::get_kxmd_str_seq("platforms.android.root-path", kxmd);

   int unit_count = unit_entry_map->size();
   bfs::path and_proj_path_rel_to_appplex;
   bfs::path appplex_path_rel_to_and_proj;
   bool use_default_project = true;

   // android project config/update policy:
   // if we don't have any units in the android build, then just return and don't update anything
   if (unit_count == 0)
   {
      trx("no units in the android build. project not updated.");
      return;
   }
   // if we have exactly one unit, update the corresponding project, if it exists, otherwise update the default/generic project
   else if (unit_count == 1)
   {
      trx("one unit in the android build.");

      // check if there's a custom android project for this unit, otherwise use the default project
      auto ue = unit_entry_map->begin()->second;
      and_proj_path_rel_to_appplex = platforms_android_root_path[0] / ue->unit_path;
      auto platforms_android_gradle_gradle_path = kxmd_ops::get_kxmd_str_seq("platforms.android.gradle.gradle-path", kxmd);
      auto android_gradle_path = info->proj_path / and_proj_path_rel_to_appplex / platforms_android_gradle_gradle_path[0];
      auto gradle_file = pfm_file::get_inst(android_gradle_path.generic_string());

      if (gradle_file->exists())
      {
         auto platforms_android_rel_proj_path = kxmd_ops::get_kxmd_str_seq("platforms.android.rel-proj-path", kxmd);
         appplex_path_rel_to_and_proj = platforms_android_rel_proj_path[0];
         // find out how many parent dirs we have to go up until we arrive at the directory containing all the android projects
         auto itp = ue->unit_path.begin();

         while (itp != ue->unit_path.end())
         {
            appplex_path_rel_to_and_proj /= "..";
            itp++;
         }

         use_default_project = false;
         trx("using custom project with gradle file '{}'.", gradle_file->get_full_path());
      }
      else
      {
         trx("gradle file '{}' not found.", gradle_file->get_full_path());
      }
   }
   // if we have more than one unit, only update the default/generic project.
   // it might not compile or have the desired behaviour, but there is no point in configuring multiple projects all at once,
   // since we can configure any single unit build using the previous step (unit_count == 1).
   else
   {
      trx("{} units in the android build.", unit_count);
   }

   if (use_default_project)
   {
      auto platforms_android_default_proj_name = kxmd_ops::get_kxmd_str_seq("platforms.android.default-proj-name", kxmd);
      auto platforms_android_rel_proj_path = kxmd_ops::get_kxmd_str_seq("platforms.android.rel-proj-path", kxmd);

      and_proj_path_rel_to_appplex = platforms_android_root_path[0];
      and_proj_path_rel_to_appplex /= platforms_android_default_proj_name[0];
      appplex_path_rel_to_and_proj = platforms_android_rel_proj_path[0];
      appplex_path_rel_to_and_proj /= "..";
      trx("using default android project.");
   }

   update_project_files(and_proj_path_rel_to_appplex, appplex_path_rel_to_and_proj, kxmd);
}

void android_studio_project_conf::update_project_files(const bfs::path& and_proj_path_rel_to_appplex, const bfs::path& appplex_path_rel_to_and_proj, std::shared_ptr<kx_block> ikxmd)
{
   auto platforms_android_gradle_gradle_path = kxmd_ops::get_kxmd_str_seq("platforms.android.gradle.gradle-path", ikxmd);
   auto android_gradle_path = info->proj_path / and_proj_path_rel_to_appplex / platforms_android_gradle_gradle_path[0];
   auto gradle_file = pfm_file::get_inst(android_gradle_path.generic_string());

   if (!gradle_file->exists())
   {
      throw ia_exception(trs("error: cannot find android gradle file [{}]", gradle_file->get_full_path()));
   }

   auto paths_proj_rel_units_path = kxmd_ops::get_kxmd_str_seq("paths.proj-rel-units-path", ikxmd);
   bfs::path rel_units_path = appplex_path_rel_to_and_proj;
   rel_units_path /= paths_proj_rel_units_path[0];
   auto gf_txt = pfm::filesystem::load_res_as_string(gradle_file);

   {
      auto platforms_android_gradle_cpp_lib_start = kxmd_ops::get_kxmd_str_seq("platforms.android.gradle.cpp-lib-start", ikxmd);
      auto platforms_android_gradle_cpp_lib_end = kxmd_ops::get_kxmd_str_seq("platforms.android.gradle.cpp-lib-end", ikxmd);
      std::string cpp_lib_start = platforms_android_gradle_cpp_lib_start[0];
      std::string cpp_lib_end = platforms_android_gradle_cpp_lib_end[0];
      auto idx_start = gf_txt->find(cpp_lib_start);

      if (idx_start != std::string::npos)
      {
         auto idx_end = gf_txt->find(cpp_lib_end, idx_start);

         if (idx_end != std::string::npos)
         {
            std::string inc_list = "";
            int idx_end2 = gf_txt->find_last_of('\n', idx_end);
            int pos = idx_start + cpp_lib_start.length() + 1;
            int len = idx_end2 - pos + 1;
            std::string indent = gf_txt->substr(idx_end2 + 1, idx_end - 1 - idx_end2);

            if (uses_mod("boost"))
            {
               auto paths_libs_boost_inc = kxmd_ops::get_kxmd_str_seq("paths.libs.boost.inc", ikxmd);
               inc_list += trs("{0}'{1}/{2}',\n", indent, appplex_path_rel_to_and_proj.generic_string(), paths_libs_boost_inc[0]);
            }

            gf_txt->replace(pos, len, inc_list);
         }
         else
         {
            trx("error[ cannot find cpp-lib-end tag ]");
            return;
         }
      }
      else
      {
         trx("error[ cannot find cpp-lib-start tag ]");
         return;
      }

   }

   {
      auto platforms_android_gradle_cpp_src_start = kxmd_ops::get_kxmd_str_seq("platforms.android.gradle.cpp-src-start", ikxmd);
      auto platforms_android_gradle_cpp_src_end = kxmd_ops::get_kxmd_str_seq("platforms.android.gradle.cpp-src-end", ikxmd);
      std::string cpp_src_start = platforms_android_gradle_cpp_src_start[0];
      std::string cpp_src_end = platforms_android_gradle_cpp_src_end[0];
      auto idx_start = gf_txt->find(cpp_src_start);

      if (idx_start != std::string::npos)
      {
         auto idx_end = gf_txt->find(cpp_src_end, idx_start);

         if (idx_end != std::string::npos)
         {
            std::string rel_units_path_str = rel_units_path.generic_string();
            std::string unit_list = "";
            int idx_end2 = gf_txt->find_last_of('\n', idx_end);
            int pos = idx_start + cpp_src_start.length() + 1;
            int len = idx_end2 - pos + 1;
            std::string indent = gf_txt->substr(idx_end2 + 1, idx_end - 1 - idx_end2);

            for (auto& k : *unit_entry_map)
            {
               std::string unit_path = k.second->unit_path.generic_string();
               unit_list += trs("{0}'{1}/{2}',\n", indent, rel_units_path_str, unit_path);
            }

            if (uses_mod("boost"))
            {
               auto paths_libs_boost_lib = kxmd_ops::get_kxmd_str_seq("paths.libs.boost.lib", ikxmd);
               unit_list += trs("{0}'{1}/{2}',\n", indent, appplex_path_rel_to_and_proj.generic_string(), paths_libs_boost_lib[0]);
            }

            gf_txt->replace(pos, len, unit_list);
         }
         else
         {
            trx("error[ cannot find cpp-src-end tag ]");
            return;
         }
      }
      else
      {
         trx("error[ cannot find cpp-src-start tag ]");
         return;
      }
   }

   gradle_file->io.open("wt");

   auto rw = rw_file_sequence::new_inst(gradle_file);
   rw->w.write_line(*gf_txt, false);
}


windows_pc_project_conf::windows_pc_project_conf(std::shared_ptr<conf_info> iinfo, shared_ptr<unit_entry_map_type> iunit_entry_map) :
   platform_project_conf(iinfo, iunit_entry_map) {}

void windows_pc_project_conf::update_project()
{
   trx("\nupdating the windows pc project.\n");
}

#endif // UNIT_CMD
