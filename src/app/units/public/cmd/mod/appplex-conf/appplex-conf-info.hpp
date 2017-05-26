#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "pfm.hpp"
#include <boost/filesystem.hpp>
#include <unordered_map>

class directory_tree;
class rec_dir_op_appplex_conf;
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
using unit_entry_map_type = std::unordered_map < std::string, shared_ptr<unit_entry> >;


class conf_info
{
public:
   std::string appplex_conf_name;
   bfs::path proj_path;
   bfs::path src_path;
   std::vector<unicodestring> exclude_path;
   shared_ptr<directory_tree> dir_tree;
   shared_ptr<rec_dir_op_appplex_conf> rdo;
   shared_ptr<kx_krte> krt;
};


class appplex_conf
{
public:
   appplex_conf(std::shared_ptr<conf_info> iinfo);
   void update();

   shared_ptr<unit_entry_map_type> unit_entry_map;
   shared_ptr<unit_entry_map_type> unit_entry_map_android;

protected:

   std::string get_define_unit_line(shared_ptr<unit_entry> iue, const std::string& idef, const std::string& ifile_name, const std::vector<std::string>& iplatf);
   std::string get_new_unit_line(shared_ptr<unit_entry> iue, const std::string& ifile_name, bool is_selected = false);
   void update_dependencies(shared_ptr<unit_entry> ue, std::vector<std::string>& iuses, std::unordered_map<std::string, bool>& idependencies_def_map);

   std::shared_ptr<conf_info> info;
};


class platform_project_conf
{
public:
   platform_project_conf(std::shared_ptr<conf_info> iinfo, shared_ptr<unit_entry_map_type> iunit_entry_map);
   bool uses_mod(std::string imod_name);
   virtual void update_project() = 0;

protected:
   void calc_modules_used();

   std::shared_ptr<conf_info> info;
   shared_ptr<unit_entry_map_type> unit_entry_map;
   std::unordered_map<std::string, bool> uses_mod_map;
};


class android_studio_project_conf : public platform_project_conf
{
public:
   android_studio_project_conf(std::shared_ptr<conf_info> iinfo, shared_ptr<unit_entry_map_type> iunit_entry_map);
   virtual void update_project() override;
};


class windows_pc_project_conf : public platform_project_conf
{
public:
   windows_pc_project_conf(std::shared_ptr<conf_info> iinfo, shared_ptr<unit_entry_map_type> iunit_entry_map);
   virtual void update_project() override;
};

#endif // UNIT_CMD
