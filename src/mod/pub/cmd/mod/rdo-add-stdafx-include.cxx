#include "stdafx.hxx"

#include "appplex-conf.hxx"

#ifdef MOD_CMD

#include "rdo-add-stdafx-include.hxx"
#include "min.hxx"
#include "com/util/unicode/boost-filesystem-util.hxx"
#include "com/util/unicode/boost-program-options-util.hxx"
#include "rdo-recursive-copy.hxx"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>
#include <exception>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

using namespace boost::algorithm;
using namespace boost::filesystem;
using namespace boost::program_options;
using std::string;
using std::vector;


class rec_dir_op_add_stdafx_include: public recursive_dir_op
{
public:
	rec_dir_op_add_stdafx_include(path isrc_path, vector<unicodestring> iexclude_path);
	void on_start(mws_sp<dir_node> dir);
	void on_finish(mws_sp<dir_node> dir);
	bool on_entering_dir(mws_sp<dir_node> dir);
	void on_leaving_dir(mws_sp<dir_node> dir);
	void apply_to_file(mws_sp<file_node> file);

private:
	path src_path;
	vector<path> exclude_path;
	int file_count;
	bool has_changes;
};


std::string mod_cmd_add_stdafx_include::get_module_name()
{
	return "add-stdafx-include";
}


const string SOURCE_PATH			= "source-path";
const string EXCLUDE_PATH			= "exclude-path";


boost::program_options::options_description mod_cmd_add_stdafx_include::get_options_description()
{
	options_description desc(trs("available options for module [{}]", get_module_name()));

	desc.add_options()
		(SOURCE_PATH.c_str(), unicodevalue<unicodestring>()->required(), "source path. must be an absolute path")
		(EXCLUDE_PATH.c_str(), unicodevalue<vector<unicodestring> >(), "exclude path. must be relative to the source path")
	;

	return desc;
}

mws_sp<long_operation> mod_cmd_add_stdafx_include::run(const vector<unicodestring>& args)
{
	options_description desc = get_options_description();
	variables_map vm;
	positional_options_description p;

	p.add(EXCLUDE_PATH.c_str(), -1);
	unicode_parsed_options parsed = unicode_command_line_parser(args).options(desc).positional(p).run();

	store(parsed, vm);
	notify(vm);

	vector<unicodestring> excludePath;

	if(vm.count(EXCLUDE_PATH))
	{
		excludePath = vm[EXCLUDE_PATH].as<vector<unicodestring> >();
	}

	mws_sp<long_operation> lop = mws_sp<long_operation>(new long_op_add_stdafx_include(vm[SOURCE_PATH].as<unicodestring>(), excludePath));

	return lop;
}


long_op_add_stdafx_include::long_op_add_stdafx_include(unicodestring isrc_path, vector<unicodestring> iexclude_path)
{
	src_path = path(isrc_path);
	exclude_path = iexclude_path;
}

void long_op_add_stdafx_include::run()
{
	if (exists(src_path))
	{
		if (is_directory(src_path))
		{
			utrx(untr("starting longOpAddStdafxInclude in directory [{}]"), path2string(src_path));

			mws_sp<directory_tree> dirtree = directory_tree::new_directory_tree(src_path);
			rec_dir_op_add_stdafx_include rdo(src_path, exclude_path);

			dirtree->recursive_apply(rdo);
		}
		else
		{
         mws_throw mws_exception(trs("longOpAddStdafxInclude: {} is not a directory", src_path.string()));
		}
	}
	else
	{
      mws_throw mws_exception(trs("longOpAddStdafxInclude: {} does not exist", src_path.string()));
	}
}


rec_dir_op_add_stdafx_include::rec_dir_op_add_stdafx_include(path iSrcPath, vector<unicodestring> iExcludePath)
{
	src_path = iSrcPath;

	int k = 0;
	for(unicodestring s : iExcludePath)
	{
		exclude_path.push_back(path(s));
		path& p = exclude_path.back();

		if(!p.is_relative())
		{
         mws_throw mws_exception(trs("path [{0}] is not relative to [{1}]", p.string(), src_path.string()));
		}

		utrx(untr("{0} {1}"), k, s);
		k++;
	}
}

void rec_dir_op_add_stdafx_include::on_start(mws_sp<dir_node> dir)
{
	file_count = 0;
	has_changes = false;
}

void rec_dir_op_add_stdafx_include::on_finish(mws_sp<dir_node> dir)
{
	if(!has_changes)
	{
		trx("no changes made");
	}

	trx("cpp files [{}]", file_count);
}

bool rec_dir_op_add_stdafx_include::on_entering_dir(mws_sp<dir_node> dir)
{
	path& relPath = dir->rel_dir_path;

	for(path p : exclude_path)
	{
		if(p == relPath)
		{
			return false;
		}
	}

	//trx(dir->relDirPath.string());

	return true;
}

void rec_dir_op_add_stdafx_include::on_leaving_dir(mws_sp<dir_node> dir)
{
}

void rec_dir_op_add_stdafx_include::apply_to_file(mws_sp<file_node> file)
{
	unicodestring ext = path2string(file->rel_file_path.extension());

	if(ext != untr(".cpp") && ext != untr(".c"))
		// not a .c / .cpp source file
	{
		return;
	}

	path& p = file->abs_file_path;
	//std::time_t tt = last_write_time(p);
    std::fstream fs(p.string().c_str(), std::ios_base::in | std::ios_base::out);
	string str((std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>());

	string newstr;
	bool hasStdafx = false;
	bool hasEndoffileNewline = true;
	bool  fileChanged = false;
	int idxnl = str.find("\n");

    if(str[str.length() - 1] != '\n')
	{
		hasEndoffileNewline = false;
	}

	if(idxnl != string::npos)
	{
		int idxinclude = str.find("include");
		int idxstdafx = str.find("stdafx.h");

		if(idxinclude != string::npos && idxinclude < idxnl && idxstdafx != string::npos && idxstdafx < idxnl)
		{
			hasStdafx = true;
		}
	}

	if(!hasStdafx || !hasEndoffileNewline)
		// need to change file
	{
		newstr = str;

		if(!hasEndoffileNewline)
		{
			trx("added end of file new line in file [{}]", p.string());
			newstr += "\n";
		}

		if(!hasStdafx)
		{
			newstr = "#include \"stdafx.h\"\n\n" + newstr;
			trx("added stdafx include in file [{}]", p.string());
		}

		fs.seekp(0, std::ios_base::beg);
		fs << newstr;

		fileChanged = true;
		has_changes = true;
	}

	fs.close();

	//if(fileChanged)
	//{
	//	last_write_time(p, tt);
	//}

	file_count++;
}

#endif
