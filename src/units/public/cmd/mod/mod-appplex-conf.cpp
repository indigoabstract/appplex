#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#pragma warning( disable : 4996 )

#include "mod-appplex-conf.hpp"
#include "min.hpp"
#include "data-sequence.hpp"
#include "../long-operation.hpp"
#include "../recursive-dir.hpp"
#include "com/util/unicode/boost-filesystem-util.hpp"
#include "com/util/unicode/boost-program-options-util.hpp"
#include "rdo-recursive-copy.hpp"
#include <sbmd/sbmd.hpp>
#include <sbmd/kx-krte.hpp>
#include <sbmd/kx-elem.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>
#include <boost/any.hpp>
#include <fmt/ostream.h>
#include <exception>
#include <fstream>
#include <iostream>
#include <locale>
#include <unordered_map>

namespace bfs = ::boost::filesystem;
namespace bpo = ::boost::program_options;
using std::string;
using std::vector;


namespace appplex_conf_ns
{
	const string SOURCE_PATH = "source-path";
	const string EXCLUDE_PATH = "exclude-path";
	std::unordered_map<std::string, std::string> platf_def_map =
	{
		{ "android", "PLATFORM_ANDROID" }, { "ios", "PLATFORM_IOS" }, { "emscripten", "PLATFORM_EMSCRIPTEN" },
		{ "qt_windows", "PLATFORM_QT_WINDOWS_PC" }, { "windows", "PLATFORM_WINDOWS_PC" },
	};


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
		std::vector<std::string> platforms;
	};
	using unit_entry_map_type = std::unordered_map < std::string, shared_ptr<unit_entry> >;


	class rec_dir_op_appplex_conf : public recursive_dir_op
	{
	public:
		rec_dir_op_appplex_conf(bfs::path isrc_path, vector<unicodestring> iexclude_path, shared_ptr<kx_krte> ikrt, shared_ptr<unit_entry_map_type> iunit_entry_map)
		{
			is_inside_units_dir = false;
			src_path = isrc_path;
			krt = ikrt;
			unit_entry_map = iunit_entry_map;
			ref_path = "units";
			boost_thread_path = bfs::path("ext") / "lib-src" / "boost" / "thread" / "win32";

			int k = 0;
			for(unicodestring s : iexclude_path)
			{
				exclude_path.push_back(bfs::path(s));
				bfs::path& p = exclude_path.back();

				if (!p.is_relative())
				{
					throw ia_exception(trs("path [{0}] is not relative to [{1}]", p.generic_string(), src_path.generic_string()));
				}

				utrx(untr("{0} {1}"), k, s);
				k++;
			}
		}

		void on_start(shared_ptr<dir_node> dir)
		{
		}

		void on_finish(shared_ptr<dir_node> dir)
		{
		}

		// return false to skip this directory
		bool on_entering_dir(shared_ptr<dir_node> dir)
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

		void on_leaving_dir(shared_ptr<dir_node> dir)
		{
		}

		void apply_to_file(shared_ptr<file_node> file)
		{
			const std::string unit_pfx = "unit-";
			const std::string unit_sfx = ".hpp";
			std::string fname = file->rel_file_path.filename().generic_string();

			if (is_inside_units_dir && starts_with(fname, unit_pfx) && ends_with(fname, unit_sfx))
			{
				int start_pos = unit_pfx.length();
				int len = fname.length() - start_pos - unit_sfx.length();
				std::string unit_name = fname.substr(start_pos, len);
				//trx("--------file [%1%]") % unit_name;
				auto ue = std::make_shared<unit_entry>();
				ue->unit_name = unit_name;
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

	private:
		bool path_contains_path(const bfs::path& ip1, const bfs::path& ip2)
		{
			auto p1_len = std::distance(ip1.begin(), ip1.end());
			auto p2_len = std::distance(ip2.begin(), ip2.end());

			if (p2_len > p1_len)
			{
				return false;
			}

			return std::equal(ip2.begin(), ip2.end(), ip1.begin());
		}

		bfs::path src_path;
		vector<bfs::path> exclude_path;
		shared_ptr<kx_krte> krt;
		shared_ptr<unit_entry_map_type> unit_entry_map;
		bfs::path ref_path;
		bfs::path boost_thread_path;
		bool is_inside_units_dir;
	};


	class long_op_appplex_conf : public long_operation
	{
	public:
		long_op_appplex_conf(unicodestring iproj_path, vector<unicodestring> iexclude_path)
		{
			proj_path = iproj_path;
			src_path = proj_path;
			src_path /= "src";
			exclude_path = iexclude_path;
		}

		void run()
		{
			if (exists(src_path))
			{
				if (is_directory(src_path))
				{
					utrx(untr("starting mod-appplex-conf in directory [{}]"), path2string(src_path));
					trx("\nloading the source file tree.\n");
					unit_entry_map = std::make_shared<unit_entry_map_type>();
					unit_entry_map_android = std::make_shared<unit_entry_map_type>();
					dir_tree = directory_tree::new_directory_tree(src_path, exclude_path);
					rdo = std::make_shared<rec_dir_op_appplex_conf>(src_path, exclude_path, krt, unit_entry_map);
					dir_tree->recursive_apply(*rdo);

					trx("\nupdating appplex configuration.\n");
					update_appplex_conf();
					trx("\nupdating the android studio project.\n");
					update_android_studio_project();

					trc("finished!");
				}
				else
				{
					throw ia_exception(trs("mod-appplex-conf: {} is not a directory", src_path));
				}
			}
			else
			{
				throw ia_exception(trs("mod-appplex-conf: {} does not exist", src_path));
			}
		}

	private:
		std::string get_define_unit_line(shared_ptr<unit_entry> iue, const std::string& idef, const std::string& ifile_name, const std::vector<std::string>& iplatf)
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

		std::string get_new_unit_line(shared_ptr<unit_entry> iue, const std::string& ifile_name, bool is_selected = false)
		{
			std::string line;
			std::string def = ifile_name;
			std::replace(def.begin(), def.end(), '-', '_');
			std::string unit_name = "unit_" + def;
			std::transform(def.begin(), def.end(), def.begin(), ::toupper);
			def = "#ifdef UNIT_" + def;

			if (is_selected)
			{
				line = "\tadd_unit(" + unit_name + "::new_instance(), true);\n";
			}
			else
			{
				line = "\tadd_unit(" + unit_name + "::new_instance());\n";
			}

			line = def + "\n" + line + "#endif\n\n";

			return line;
		}

		void update_dependencies(shared_ptr<unit_entry> ue, std::vector<std::string>& iuses, std::unordered_map<std::string, bool>& idependencies_def_map)
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

		void update_appplex_conf()
		{
			try
			{
				std::string appplex_conf_name = "appplex-conf.sbmd";
				bfs::path axc_path = proj_path / appplex_conf_name;
				auto sgmd_txt = pfm::filesystem::load_res_as_string(axc_path.generic_string());

				krt = std::make_shared<kx_krte>();
				krt->set_src(sgmd_txt);
				krt->run();

				auto sbmd = krt->kxb;
				std::vector<std::string> default_uses = sbmd_ops::get_sbmd_str_seq("default.uses", sbmd, { "gfx", "boost" });
				bool default_enabled = sbmd_ops::get_bool_from_list(sbmd_ops::get_sbmd_str_seq("default.enabled", sbmd, { "true" }));
				std::vector<std::string> start_unit_list = sbmd_ops::get_sbmd_str_seq("default.start.unit-list", sbmd, { "test" });
				bool launch_unit = sbmd_ops::get_bool_from_list(sbmd_ops::get_sbmd_str_seq("default.start.launch-unit", sbmd, { "true" }));
				std::vector<std::string> default_platf = sbmd_ops::get_sbmd_str_seq("default.platf", sbmd, { "all" });
				bool default_exclusive = sbmd_ops::get_bool_from_list(sbmd_ops::get_sbmd_str_seq("default.start.exclusive", sbmd, { "false" }));
				bool single_unit_build = (start_unit_list.size() == 1) && default_exclusive && launch_unit;
				std::unordered_map<std::string, bool> dependencies_def_map;
				std::unordered_map<std::string, bool> unit_dependencies_def_map;

				bfs::path appplex_conf_path = proj_path / sbmd_ops::get_sbmd_str_seq("paths.proj-rel-appplex-conf-path", sbmd, { "" })[0];
				std::string appplex_conf_hpp = sbmd_ops::get_sbmd_str_seq("paths.appplex-conf-hpp-name", sbmd, { "" })[0];
				std::string appplex_conf_cpp = sbmd_ops::get_sbmd_str_seq("paths.appplex-conf-cpp-name", sbmd, { "" })[0];
				auto hpp = pfm_file::get_inst(appplex_conf_hpp, appplex_conf_path.generic_string());
				auto cpp = pfm_file::get_inst(appplex_conf_cpp, appplex_conf_path.generic_string());

				hpp->io.open("w+t");
				cpp->io.open("w+t");

				auto rw_hpp = rw_file_sequence::new_inst(hpp);
				auto rw_cpp = rw_file_sequence::new_inst(cpp);

				{
					auto px_names = sbmd_ops::get_sbmd_str_seq("units", sbmd);
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
						auto enabled_seq = sbmd_ops::get_sbmd_str_seq(unit_path + ".enabled", sbmd);
						bool define_active = true;

						std::string def = file_name;
						std::replace(def.begin(), def.end(), '-', '_');
						std::transform(def.begin(), def.end(), def.begin(), ::toupper);
						std::string unit_def = "UNIT_" + def;
						def = "#define " + unit_def;

						if (!enabled_seq.empty())
						{
							enabled = sbmd_ops::get_bool_from_list(enabled_seq);
						}

						if (enabled)
						{
							auto ue = (*unit_entry_map)[file_name];

							if (ue)
							{
								enabled = false;
								std::vector<std::string> uses = sbmd_ops::get_sbmd_str_seq(unit_path + ".uses", sbmd);
								auto platf_seq = sbmd_ops::get_sbmd_str_seq(unit_path + ".platf", sbmd);
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
			catch (ia_exception& e)
			{
				trx("exception {}", e.what());
			}
		}

		void update_android_studio_project()
		{
			bfs::path android_gradle_path = proj_path / "pfm" / "android" / "app" / "build.gradle";
			shared_ptr<pfm_file> gradle_file = pfm_file::get_inst(android_gradle_path.generic_string());
			shared_ptr<std::string> gradle_file_txt = pfm::filesystem::load_res_as_string(gradle_file);
			std::string& gf_txt = *gradle_file_txt;
			std::string start = "// [";
			std::string end = "// ]";
			auto idx_start = gf_txt.find(start);

			if (idx_start != string::npos)
			{
				auto idx_end = gf_txt.find(end, idx_start);

				if (idx_end != string::npos)
				{
					std::string unit_list = "";
					int idx_end2 = gf_txt.find_last_of('\n', idx_end);
					int pos = idx_start + start.length() + 1;
					int len = idx_end2 - pos + 1;
					std::string indent = gf_txt.substr(idx_end2 + 1, idx_end - 1 - idx_end2);

					for (auto& k : *unit_entry_map_android)
					{
						std::string unit_path = k.second->unit_path.generic_string();
						unit_list += indent + "'../../../src/units/" + unit_path + "',\n";
					}

					gf_txt.replace(pos, len, unit_list);
					//trx(gf_txt);
					gradle_file->io.open("wt");

					auto rw = rw_file_sequence::new_inst(gradle_file);
					rw->w.write_line(gf_txt, false);
				}
			}
		}
		
		bfs::path proj_path;
		bfs::path src_path;
		std::vector<unicodestring> exclude_path;
		shared_ptr<directory_tree> dir_tree;
		shared_ptr<rec_dir_op_appplex_conf> rdo;
		shared_ptr<kx_krte> krt;
		shared_ptr<unit_entry_map_type> unit_entry_map;
		shared_ptr<unit_entry_map_type> unit_entry_map_android;
		const std::string auto_gen = "// this file is autogenerated. do not edit.\n";
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
