#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "rec-dir-op-appplex-conf.hpp"
#include "appplex-conf-info.hpp"
#include "min.hpp"
#include "data-sequence.hpp"
#include "../../long-operation.hpp"
#include "../../recursive-dir.hpp"
#include "../rdo-recursive-copy.hpp"


rec_dir_op_appplex_conf::rec_dir_op_appplex_conf(bfs::path isrc_path, std::vector<unicodestring> iexclude_path, shared_ptr<kx_krte> ikrt, shared_ptr<unit_entry_map_type> iunit_entry_map)
{
   is_inside_units_dir = false;
   src_path = isrc_path;
   krt = ikrt;
   unit_entry_map = iunit_entry_map;
   ref_path = "units";
   boost_thread_path = bfs::path("ext") / "lib-src" / "boost" / "thread" / "win32";

   int k = 0;
   for (unicodestring s : iexclude_path)
   {
      exclude_path.push_back(bfs::path(s));
      bfs::path& p = exclude_path.back();

      if (!p.is_relative())
      {
         mws_throw mws_exception(trs("path [{0}] is not relative to [{1}]", p.generic_string(), src_path.generic_string()));
      }

      utrx(untr("{0} {1}"), k, s);
      k++;
   }
}

void rec_dir_op_appplex_conf::on_start(shared_ptr<dir_node> dir)
{
}

void rec_dir_op_appplex_conf::on_finish(shared_ptr<dir_node> dir)
{
}

// return false to skip this directory
bool rec_dir_op_appplex_conf::on_entering_dir(shared_ptr<dir_node> dir)
{
   is_inside_units_dir = false;
   bfs::path& crt_path = dir->rel_dir_path;
   bool pcp = path_contains_path(crt_path, ref_path);

   // if this is a subdirectory in "src/units"
   if (pcp && (crt_path != ref_path))
   {
      is_inside_units_dir = true;

      return true;
   }
   else if (path_contains_path(crt_path, boost_thread_path))
   {
      return false;
   }

   return true;
}

void rec_dir_op_appplex_conf::on_leaving_dir(shared_ptr<dir_node> dir)
{
}

void rec_dir_op_appplex_conf::apply_to_file(shared_ptr<file_node> file)
{
   const std::string unit_pfx = "unit-";
   const std::string unit_sfx = ".hpp";
   std::string fname = file->rel_file_path.filename().generic_string();

   if (is_inside_units_dir && mws_str::starts_with(fname, unit_pfx) && ends_with(fname, unit_sfx))
   {
      int start_pos = unit_pfx.length();
      int len = fname.length() - start_pos - unit_sfx.length();
      std::string unit_name = fname.substr(start_pos, len);
      //trx("--------file [%1%]") % unit_name;
      auto ue = std::make_shared<unit_entry>();
      ue->unit_name = unit_name;
      // extract the directories below 'units' and store as the unit path
      bfs::path tp1 = file->rel_file_path.parent_path();
      auto itp = tp1.begin();
      itp++;
      bfs::path tp2 = *itp;
      itp++;

      while (itp != tp1.end())
      {
         tp2 /= *itp;
         itp++;
      }

      std::string ui = tp2.generic_string();
      ue->unit_path = tp2;
      ue->unit_hpp_path = tp2 / file->rel_file_path.filename();
      (*unit_entry_map)[ue->unit_name] = ue;
   }
}

bool rec_dir_op_appplex_conf::path_contains_path(const bfs::path& ip1, const bfs::path& ip2)
{
   auto p1_len = std::distance(ip1.begin(), ip1.end());
   auto p2_len = std::distance(ip2.begin(), ip2.end());

   if (p2_len > p1_len)
   {
      return false;
   }

   return std::equal(ip2.begin(), ip2.end(), ip1.begin());
}

#endif // UNIT_CMD
