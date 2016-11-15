#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "rdo-std-fmt-rename.hpp"
#include "min.hpp"
#include "com/util/unicode/boost-filesystem-util.hpp"
#include "com/util/unicode/boost-program-options-util.hpp"
#include "rdo-recursive-copy.hpp"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>
#include <exception>
#include <locale>
#include <string>
#include <vector>

using namespace boost::algorithm;
using namespace boost::filesystem;
using namespace boost::program_options;
using std::string;
using std::vector;


class rec_dir_op_std_fmt_rename: public recursive_dir_op
{
public:
	rec_dir_op_std_fmt_rename(path& isrc_path);
	void on_start(shared_ptr<dir_node> dir);
	void on_finish(shared_ptr<dir_node> dir);
	bool on_entering_dir(shared_ptr<dir_node> dir);
	void on_leaving_dir(shared_ptr<dir_node> dir);
	void apply_to_file(shared_ptr<file_node> file);

private:
	void rename_path(const path& irel_path, unicodestring& iold_filename, unicodestring& inew_filename, bool iis_directory);
	static unicodestring apply_standard_format(const unicodestring& ifilename, bool iis_directory = false);

	path src_dir;
	int ren_exception_count;
	int file_count;
	int directory_count;
	uint64 total_file_size;
	int files_renamed_count;
	int directories_renamed_count;
};


std::string mod_cmd_std_fmt_filenames::get_module_name()
{
	return "standard-format-filenames";
}


const string SOURCE_PATH			= "source-path";
const string DESTINATION_PATH		= "destination-path";
const string COPY_ONLY				= "copy-only";


boost::program_options::options_description mod_cmd_std_fmt_filenames::get_options_description()
{
	options_description desc(trs("available options for module [%1%]") % get_module_name());

	desc.add_options()
		(SOURCE_PATH.c_str(), unicodevalue<unicodestring>()->required(), "source path. must be an absolute path")
		(DESTINATION_PATH.c_str(), unicodevalue<unicodestring>()->default_value(untr(""), ""),
		(trs("destination path. when missing, it's assumed to be the same as [%1%]."
			 " a relative path is considered relative to the [%1%] directory") % SOURCE_PATH).c_str())
		(COPY_ONLY.c_str(), "files will only be copied, not renamed")
	;

	return desc;
}

shared_ptr<long_operation> mod_cmd_std_fmt_filenames::run(const vector<unicodestring>& args)
{
	options_description desc = get_options_description();
	variables_map vm;
	unicode_parsed_options parsed = unicode_command_line_parser(args).options(desc).run();

	store(parsed, vm);
	notify(vm);

	boost::filesystem::path srcPath(vm[SOURCE_PATH].as<unicodestring>());
	boost::filesystem::path dstPath;

	utrx(untr("source-path was set to %1%")) % vm[SOURCE_PATH].as<unicodestring>();
	utrx(untr("destination-path was set to %1%")) % vm[DESTINATION_PATH].as<unicodestring>();

	dstPath = vm[DESTINATION_PATH].as<unicodestring>();

	if (dstPath.is_relative())
	{
		dstPath = srcPath / dstPath;
	}

	bool copyOnly = false;

	if(vm.count(COPY_ONLY))
	{
		copyOnly = true;
	}

	shared_ptr<long_operation> lop = shared_ptr<long_operation>(new long_op_std_fmt_rename(srcPath, dstPath, copyOnly));

	return lop;
}


long_op_std_fmt_rename::long_op_std_fmt_rename(path& isrc_path, path& idst_path, bool icopy_only)
{
	src_path = isrc_path;
	dst_path = idst_path;
	copy_only = icopy_only;
}

void long_op_std_fmt_rename::run()
{
	if(src_path != dst_path)
	{
		long_op_recursive_copy lop(src_path, dst_path);
		lop.run();
	}

	if(copy_only)
	{
		return;
	}

	if (exists(dst_path))
	{
		if (is_directory(dst_path))
		{
			utrx(untr("starting longOpStdFmtRename in directory [%1%]")) % path2string(dst_path);

			shared_ptr<directory_tree> dirtree = directory_tree::new_directory_tree(dst_path);
			rec_dir_op_std_fmt_rename rdo(dst_path);

			dirtree->recursive_apply(rdo);
		}
		else
		{
			throw ia_exception(trs("longOpStdFmtRename: %1% is not a directory") % dst_path);
		}
	}
	else
	{
		throw ia_exception(trs("longOpStdFmtRename: %1% does not exist") % dst_path);
	}
}


rec_dir_op_std_fmt_rename::rec_dir_op_std_fmt_rename(path& srcPath)
{
	src_dir = srcPath;
}

void rec_dir_op_std_fmt_rename::on_start(shared_ptr<dir_node> dir)
{
	file_count = directory_count = total_file_size = ren_exception_count = files_renamed_count = directories_renamed_count = 0;

	utrx(untr("renaming files from directory [%1%]")) % path2string(src_dir);
}

void rec_dir_op_std_fmt_rename::on_finish(shared_ptr<dir_node> dir)
{
	trx("directories [%1%], files [%2%], all-files size [%3%]") % directory_count % file_count % total_file_size;
	trx("renamed directories [%1%], renamed files [%2%], rename exceptions [%3%]") % directories_renamed_count % files_renamed_count % ren_exception_count;
}

bool rec_dir_op_std_fmt_rename::on_entering_dir(shared_ptr<dir_node> dir)
{
	return true;
}

void rec_dir_op_std_fmt_rename::on_leaving_dir(shared_ptr<dir_node> dir)
{
	unicodestring iFilename = path2string(dir->rel_dir_path.filename());
	unicodestring filename = apply_standard_format(iFilename, true);

	rename_path(dir->rel_dir_path.parent_path(), iFilename, filename, true);
	directory_count++;
}

void rec_dir_op_std_fmt_rename::apply_to_file(shared_ptr<file_node> file)
{
	// something will probably break if these file types are renamed
	unicodestring exceptions[] =
	{
		untr(".java"), untr(".class"), untr(".h"), untr(".c"), untr(".hpp"), untr(".cpp"), untr(".js"), untr(".css"),
	};
	int exceptionsLength = sizeof(exceptions) / sizeof(unicodestring);
	unicodestring iFilename = path2string(file->rel_file_path.filename());

	total_file_size += file_size(file->abs_file_path);

	for(int k = 0; k < exceptionsLength; k++)
		// check for exceptions
	{
		if(iends_with(iFilename, exceptions[k]))
		{
			ren_exception_count++;
			//utrx(untr("%1% is excepted. not renamed")) % iFilename;

			return;
		}
	}

	unicodestring filename = apply_standard_format(iFilename);
	shared_ptr<dir_node> dtn = file->parent_dir.lock();

	rename_path(dtn->rel_dir_path, iFilename, filename, false);
	file_count++;
}

void rec_dir_op_std_fmt_rename::rename_path(const path& irel_path, unicodestring& iold_filename, unicodestring& inew_filename, bool iis_directory)
{
	static unicodestring dtype(untr("directory"));
	static unicodestring ftype(untr("file"));
	unicodestring& fileType = (iis_directory) ? dtype : ftype;

	try
	{
		if(inew_filename.compare(iold_filename) != 0 && inew_filename.length() > 0)
			// rename
		{
			path oldRelPath = irel_path / iold_filename;
			path newRelPath = irel_path / inew_filename;
			path oldPath = src_dir / oldRelPath;
			path newPath = src_dir / newRelPath;

			if(!iequals(inew_filename, iold_filename) && exists(newPath))
			{
				utrx(untr("%1% [%2%] was not renamed to [%3%] because a %1% with that name already exists")) % fileType % path2string(oldRelPath)  % path2string(newRelPath);
			}
			else
			{
				rename(oldPath, newPath);
				utrx(untr("%1% [%2%] -> [%3%]")) % fileType % iold_filename % inew_filename;

				if(iis_directory)
				{
					directories_renamed_count++;
				}
				else
				{
					files_renamed_count++;
				}
			}
		}
		else
		{
			//utrx(untr("%1% [%2%] already in standard format")) % fileType % newFilename;
		}
	}
	catch(ia_exception& e)
	{
		path oldRelPath = irel_path / iold_filename;
		path newRelPath = irel_path / inew_filename;

		utrx(untr("rename error [%1%]. failed to rename [%2%] to [%3%]")) % e.what() % path2string(oldRelPath)  % path2string(newRelPath);
	}
	catch(std::exception)
	{
		path oldRelPath = irel_path / iold_filename;
		path newRelPath = irel_path / inew_filename;

		utrx(untr("rename error. failed to rename [%1%] to [%2%]")) % path2string(oldRelPath)  % path2string(newRelPath);
	}
}

unicodestring rec_dir_op_std_fmt_rename::apply_standard_format(const unicodestring& ifilename, bool iis_directory)
{
	unicodestring filename = ifilename;

	unicodestring toReplace[] =
	{
        untr(" "), untr("-"), untr("."), untr(","), untr("_"), untr("{"),
        untr("}"), untr("("), untr(")"), untr("~"),
    };
	int toReplaceLength = sizeof(toReplace) / sizeof(unicodestring);

	unicodestring extension;
	unicodechar whyphen = untr('-');
	unicodestring hyphen(untr("-"));
	unicodestring dot(untr("."));
	std::locale loc;

	// position of the file extension dot
	int idx = -1;
	bool hasExtension = false;

	if(!iis_directory)
		// only regular files have extensions
	{
		idx = filename.find_last_of(dot);

		if(idx >= 0)
		{
			extension = filename.substr(idx);
			hasExtension = true;
		}
	}

	for(int k = 1; k < filename.length(); k++)
		// insert hyphens
	{
		unicodechar wc0 = filename[k - 1];
		unicodechar wc1 = filename[k];

		// both are letters and first is lower, next is upper
		bool ins = (std::isalpha(wc0, loc) && std::isalpha(wc1, loc)) && (std::islower(wc0, loc) && std::isupper(wc1, loc));

		if(!ins)
			// first is letter, next is digit
		{
			ins = std::isalpha(wc0, loc) && std::isdigit(wc1, loc);
		}

		if(!ins)
			// first is digit, next is letter
		{
			ins = std::isdigit(wc0, loc) && std::isalpha(wc1, loc);
		}

		if(ins)
		{
			filename.insert(k, hyphen);
		}
	}

	// convert filename and extension to lower case and replace all chars in the list with hyphens
	to_lower(filename);

	if(hasExtension)
		// new position of the file extension dot
	{
		idx = filename.find_last_of(dot);
		to_lower(extension);
	}

	for(int k = 0; k < toReplaceLength; k++)
	{
		replace_all(filename, toReplace[k], hyphen);
	}

	if(hasExtension && idx >= 0 && extension.length() > 0)
		// put back the file extension dot
	{
		int len = filename.length() - idx;

		filename.replace(idx, len, extension);
	}

	for(int k = 1; k < filename.length();)
		// remove redundant hyphens from body
	{
		unicodechar wc0 = filename[k - 1];
		unicodechar wc1 = filename[k];

		if(wc0 == whyphen && wc1 == whyphen)
		{
			filename.erase(k, 1);
		}
		else
		{
			k++;
		}
	}

	if(filename.length() > 1 && filename[0] == whyphen)
		// remove redundant first hyphen
	{
		filename.erase(0, 1);
	}

	if(hasExtension)
	{
		idx = filename.find_last_of(dot);

		if(idx > 0 && filename[idx - 1] == whyphen)
		{
			filename.erase(idx - 1, 1);
		}
	}

	return filename;
}

#endif
