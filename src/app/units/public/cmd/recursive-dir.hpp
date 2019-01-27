#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "pfm.hpp"
#include <boost/cstdint.hpp>
#include <boost/filesystem.hpp>
#include <vector>

namespace bfs = ::boost::filesystem;


class dir_node;

class file_node
{
public:
	bfs::path abs_file_path;
	bfs::path rel_file_path;
	mws_wp<dir_node> parent_dir;
	boost::uintmax_t file_size;
};


class dir_node
{
public:
	bfs::path abs_dir_path;
	bfs::path rel_dir_path;
	mws_wp<dir_node> root;
	mws_wp<dir_node> parent;
	std::vector<mws_sp<dir_node> > directories;
	std::vector<mws_sp<file_node> > files;
};


class recursive_dir_op
{
public:
	virtual void on_start(mws_sp<dir_node> dir);
	virtual void on_finish(mws_sp<dir_node> dir);
	virtual bool on_entering_dir(mws_sp<dir_node> dir);
	virtual void on_leaving_dir(mws_sp<dir_node> dir);
	virtual void apply_to_file(mws_sp<file_node> file);
};


class rdo_list_files: public recursive_dir_op
{
public:
	rdo_list_files();
	void on_start(mws_sp<dir_node> dir);
	void on_finish(mws_sp<dir_node> dir);
	bool on_entering_dir(mws_sp<dir_node> dir);
	void on_leaving_dir(mws_sp<dir_node> dir);
	void apply_to_file(mws_sp<file_node> file);

private:
	int file_count;
	int directory_count;
	int total_file_size;
};


class directory_tree
{
public:
	static mws_sp<directory_tree> new_directory_tree(bfs::path& iroot_path, const std::vector<unicodestring>& iexclude_path = std::vector<unicodestring>());
	void recursive_apply(recursive_dir_op& rdo);
	mws_sp<dir_node> get_root_node();

private:
	directory_tree();
	void recursive_apply(mws_sp<dir_node> dtn, recursive_dir_op& rdo);
	void recursive_read(mws_sp<directory_tree> dt, mws_sp<dir_node> dtn, bfs::directory_iterator& di);

	std::vector<bfs::path> exclude_path;
	mws_sp<dir_node> root_node;
};

#endif
