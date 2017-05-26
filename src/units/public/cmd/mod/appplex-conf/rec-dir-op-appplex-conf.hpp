#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "../../recursive-dir.hpp"
#include "appplex-conf-info.hpp"

class kx_krte;


class rec_dir_op_appplex_conf : public recursive_dir_op
{
public:
   rec_dir_op_appplex_conf(bfs::path isrc_path, std::vector<unicodestring> iexclude_path, shared_ptr<kx_krte> ikrt, shared_ptr<unit_entry_map_type> iunit_entry_map);
   void on_start(shared_ptr<dir_node> dir);
   void on_finish(shared_ptr<dir_node> dir);
   // return false to skip this directory
   bool on_entering_dir(shared_ptr<dir_node> dir);
   void on_leaving_dir(shared_ptr<dir_node> dir);
   void apply_to_file(shared_ptr<file_node> file);

private:
   bool path_contains_path(const bfs::path& ip1, const bfs::path& ip2);

   bfs::path src_path;
   std::vector<bfs::path> exclude_path;
   shared_ptr<kx_krte> krt;
   shared_ptr<unit_entry_map_type> unit_entry_map;
   bfs::path ref_path;
   bfs::path boost_thread_path;
   bool is_inside_units_dir;
};

#endif // UNIT_CMD
