#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "cmd-line-arg.hpp"
#include "min.hpp"
#include "com/util/unicode/boost-program-options-util.hpp"
#include "com/util/unicode/conversions-util.hpp"
#include "mod/rdo-call-ffmpeg.hpp"
#include "mod/rdo-directory-statistics.hpp"
#include "mod/rdo-std-fmt-rename.hpp"
#include "mod/rdo-add-stdafx-include.hpp"
#include "mod/rdo-recursive-copy.hpp"
#include "mod/appplex-conf/mod-appplex-conf.hpp"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <fmt/ostream.h>
#include <unordered_map>

using namespace boost::program_options;
using std::string;
using std::vector;

using smap = std::unordered_map<std::string, mws_sp<mod_cmd> >;


const string HELP			= "help";
const string HELP_MODULE	= "help-module";
const string MODULE			= "module";


class dispatcher
{
public:
	// appplex --module=start-process --process-path="C:\Users\indigoabstract\Desktop\ff mpeg\ffmpeg\bin\ffmpeg.exe" --process-arguments="-i "[source-path]" -ab 128k -ac 2 -ar 44100 "[destination-path]""\
	// --source-path="C:\\Users\\indigoabstract\\Desktop\\t s t\\JUN FUKAMACHI and AKIRA WADA--DIGIT CAFE 2005" --destination-path="to-mp3"
	// appplex --module=standard-format-filenames --source-path="C:\Users\indigoabstract\Desktop\tmedia" --destination-path="C:\Users\indigoabstract\Desktop\tmedia\std-fmt"
	// appplex --module=standard-format-filenames --source-path="C:\Users\indigoabstract\Desktop\tmedia" --destination-path="std-fmt"
	static mws_sp<long_operation> test_run(const vector<unicodestring>& args)
	{
		vector<unicodestring>* v = 0;
		vector<unicodestring> v1;
		v = &v1;
		//v->push_back(untr("--help"));
		//v->push_back(untr("--help-module=start-process"));
		//v->push_back(untr("--help-module=standard-format-filenames"));
		v->push_back(untr("--module=start-process"));
		v->push_back(untr("--process-path=C:\\Users\\indigoabstract\\Desktop\\ff mpeg\\ffmpeg\\bin\\ffmpeg.exe"));
		v->push_back(untr("--process-arguments=-i '[source-path]' -ab 128k -ac 2 -ar 44100 '[destination-path]'"));
		v->push_back(untr("--source-path=C:\\Users\\indigoabstract\\Desktop\\tmusic\\unprocessed"));
		v->push_back(untr("--destination-path=to-mp3"));
		v->push_back(untr("--wait-time=5"));

		vector<unicodestring> v2;
		v = &v2;
		v->push_back(untr("--module=standard-format-filenames"));
		v->push_back(untr("--source-path=C:\\Users\\indigoabstract\\Desktop\\tmusic"));
		//v2.push_back(untr("--destination-path=std-fmt"));

		vector<unicodestring> v3;
		v = &v3;
		v->push_back(untr("--module"));
		v->push_back(untr("standard-format-filenames"));
		v->push_back(untr("--source-path"));
		v->push_back(untr("C:\\Users\\indigoabstract\\Desktop\\tmedia"));
		v->push_back(untr("--destination-path"));
		v->push_back(untr("std-fmt"));

        vector<unicodestring> v4;
		v = &v4;
        v->push_back(untr("--module=directory-statistics"));
        v->push_back(untr("--source-path=/home/developer"));

		vector<unicodestring> v5;
		v = &v5;
		v->push_back(untr("--module=add-stdafx-include"));
		v->push_back(untr("--source-path=c:\\data\\devel\\appplex\\src"));
		v->push_back(untr("--exclude-path=lib\\boost"));
		v->push_back(untr("lib\\boost\\libs"));

		vector<unicodestring> v6;
		v = &v6;
		v->push_back(untr("--module=sherlock"));
		v->push_back(untr("-t"));
		v->push_back(untr("0"));
		v->push_back(untr("-z"));
		v->push_back(untr("3"));
		v->push_back(untr("-n"));
		v->push_back(untr("2"));
		v->push_back(untr("-o"));
		v->push_back(untr("results.txt"));
		v->push_back(untr("cCtrlServ.java"));
		v->push_back(untr("Dialogs.java"));

		vector<unicodestring> v7;
		v = &v7;
		v->push_back(untr("--module=appplex-conf"));
		v->push_back(untr("--source-path=c:\\data\\devel\\appplex\\src"));
		v->push_back(untr("--exclude-path=ext\\lib"));
		v->push_back(untr("units\\test-ffmpeg"));
		v->push_back(untr("units\\test-vplayer-android"));
		v->push_back(untr("units\\test-vplayer-ios"));

		return dispatcher::run(v7);
	}

	static mws_sp<long_operation> run(const vector<unicodestring>& args)
	{
		init_module_map();

		options_description general("available main options");
		general.add_options()
			(HELP.c_str(), "produce help message")
			(HELP_MODULE.c_str(), value<string>(), (trs("produce a help for a given module. valid modules are [{}]", get_module_list())).c_str())
			(MODULE.c_str(), value<string>(), (trs("module name. valid modules are [{}]", get_module_list())).c_str())
			;

		variables_map vm;
		unicode_parsed_options parsed = unicode_command_line_parser(args).options(general).allow_unregistered().run();
		store(parsed, vm);
		notify(vm);

		if(vm.count(HELP))
		{
			trx("{}", general);

			return mws_sp<long_operation>();
		}

		if(vm.count(HELP_MODULE))
		{
			string modNameVal = vm[HELP_MODULE].as<string>();

			if(modules.find(modNameVal) != modules.end())
			{
				trx("{}", modules[modNameVal]->get_options_description());
			}
			else
			{
            mws_throw mws_exception(trs("module [{0}] does not exist. valid modules [{1}]", modNameVal, get_module_list()));
			}

			return mws_sp<long_operation>();
		}

		if(vm.count(MODULE))
		{
			string mod_name_val = vm[MODULE].as<string>();

			if(modules.find(mod_name_val) != modules.end())
			{
				vector<unicodestring> argsCopy = args;
				unicodestring umodule = string2unicodestring(MODULE);
            unicodestring moduleEq = utrs(untr("{}="), umodule);

				for(int k = 0; k < argsCopy.size(); k++)
					// remove module and module-name from the argument list
				{
					if(argsCopy[k].find(umodule) != unicodestring::npos)
					{
						vector<unicodestring>::iterator position = argsCopy.begin() + k;

						if(argsCopy[k].find(moduleEq) != unicodestring::npos)
							// in the form of --module=module-name
						{
							argsCopy.erase(position);
						}
						else
							// in the form of --module module-name
						{
							argsCopy.erase(position, position + 2);
						}

						break;
					}
				}

				unicodestring squote = untr("'");
				unicodestring dquote = untr("\"");

				for(int k = 0; k < argsCopy.size(); k++)
					// replace all single quotes with double quotes
				{
					boost::algorithm::replace_all(argsCopy[k], squote, dquote);
				}

				return modules[mod_name_val]->run(argsCopy);
			}
			else
			{
            mws_throw mws_exception(trs("module [{0}] does not exist. valid modules [{1}]", mod_name_val, get_module_list()));
			}
		}
		else
		{
         mws_throw mws_exception("module was not set. call with --help for more information");
		}

		return mws_sp<long_operation>();
	}

	static string get_module_list()
	{
		string s;
		int k = 0;
		int lastIdx = modules.size() - 1;

		for(smap::value_type i : modules)
		{
			s += i.first;

			if(k != lastIdx)
			{
				s += ", ";
			}

			k++;
		}

		return s;
	}

private:
	static void add_module(mws_sp<mod_cmd> module)
	{
        const string& moduleName = module->get_module_name();

		if(modules.find(moduleName) != modules.end())
		{
         mws_throw mws_exception(trs("dispatcher::addModule. module [{}] has already been added", moduleName));
		}

		modules[moduleName] = module;
	}

	static void init_module_map()
	{
		if(!is_init)
		{
			is_init = true;

			add_module(mws_sp<mod_cmd>(new mod_cmd_start_process()));
			add_module(mws_sp<mod_cmd>(new mod_cmd_dir_statistics()));
			add_module(mws_sp<mod_cmd>(new mod_cmd_std_fmt_filenames()));
			add_module(mws_sp<mod_cmd>(new mod_cmd_add_stdafx_include()));
			add_module(mws_sp<mod_cmd>(new mod_cmd_recursive_copy()));
			add_module(mws_sp<mod_cmd>(new mod_cmd_appplex_conf()));
			//add_module(mws_sp<mod_cmd>(new mod_cmd_sherlock()));
		}
	}

	static smap modules;
	static bool is_init;
};

smap dispatcher::modules;
bool dispatcher::is_init = false;


mws_sp<long_operation> cmd_line_arg::run(const vector<unicodestring>& args)
{
    //return dispatcher::test_run(args);
    return dispatcher::run(args);
}

#endif
