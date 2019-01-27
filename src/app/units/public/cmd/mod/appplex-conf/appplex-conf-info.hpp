#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "pfm.hpp"
#include <boost/filesystem.hpp>
#include <unordered_map>

class directory_tree;
class rec_dir_op_appplex_conf;
class kx_block;
class kx_krte;
namespace bfs = ::boost::filesystem;


class unit_entry
{
public:
   unit_entry()
   {
      enabled = false;
   }

   bool enabled;
   std::string unit_name;
   bfs::path unit_path;
   bfs::path unit_hpp_path;
   std::vector<std::string> uses;
   std::vector<std::string> platforms;
};
using unit_entry_map_type = std::unordered_map < std::string, mws_sp<unit_entry> >;


class conf_info
{
public:
   std::string appplex_conf_name;
   bfs::path proj_path;
   bfs::path src_path;
   bfs::path proj_rel_units_path;
   std::vector<unicodestring> exclude_path;
   mws_sp<directory_tree> dir_tree;
   mws_sp<rec_dir_op_appplex_conf> rdo;
   mws_sp<kx_krte> krt;
};


class appplex_conf
{
public:
   appplex_conf(mws_sp<conf_info> iinfo);
   void update();

   mws_sp<unit_entry_map_type> unit_entry_map;
   mws_sp<unit_entry_map_type> unit_entry_map_android;

protected:

   std::string get_define_unit_line(mws_sp<unit_entry> iue, const std::string& idef, const std::string& ifile_name, const std::vector<std::string>& iplatf);
   std::string get_new_unit_line(mws_sp<unit_entry> iue, const std::string& ifile_name, bool is_selected = false);
   void update_dependencies(mws_sp<unit_entry> ue, std::vector<std::string>& iuses, std::unordered_map<std::string, bool>& idependencies_def_map);

   mws_sp<conf_info> info;
};


class platform_project_conf
{
public:
   platform_project_conf(mws_sp<conf_info> iinfo, mws_sp<unit_entry_map_type> iunit_entry_map);
   bool uses_mod(std::string imod_name);
   virtual void update_project() = 0;

protected:
   void calc_modules_used();

   mws_sp<conf_info> info;
   mws_sp<unit_entry_map_type> unit_entry_map;
   std::unordered_map<std::string, bool> uses_mod_map;
};


class android_studio_project_conf : public platform_project_conf
{
public:
   android_studio_project_conf(mws_sp<conf_info> iinfo, mws_sp<unit_entry_map_type> iunit_entry_map);
   virtual void update_project() override;

private:
   void update_project_files(const bfs::path& and_proj_path_rel_to_appplex, const bfs::path& appplex_path_rel_to_and_proj, mws_sp<kx_block> ikxmd);
};


class windows_pc_project_conf : public platform_project_conf
{
public:
   windows_pc_project_conf(mws_sp<conf_info> iinfo, mws_sp<unit_entry_map_type> iunit_entry_map);
   virtual void update_project() override;
};

#endif // UNIT_CMD
