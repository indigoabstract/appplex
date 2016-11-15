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
	weak_ptr<dir_node> parent_dir;
	boost::uintmax_t file_size;
};


class dir_node
{
public:
	bfs::path abs_dir_path;
	bfs::path rel_dir_path;
	weak_ptr<dir_node> root;
	weak_ptr<dir_node> parent;
	std::vector<shared_ptr<dir_node> > directories;
	std::vector<shared_ptr<file_node> > files;
};


class recursive_dir_op
{
public:
	virtual void on_start(shared_ptr<dir_node> dir);
	virtual void on_finish(shared_ptr<dir_node> dir);
	virtual bool on_entering_dir(shared_ptr<dir_node> dir);
	virtual void on_leaving_dir(shared_ptr<dir_node> dir);
	virtual void apply_to_file(shared_ptr<file_node> file);
};


class rdo_list_files: public recursive_dir_op
{
public:
	rdo_list_files();
	void on_start(shared_ptr<dir_node> dir);
	void on_finish(shared_ptr<dir_node> dir);
	bool on_entering_dir(shared_ptr<dir_node> dir);
	void on_leaving_dir(shared_ptr<dir_node> dir);
	void apply_to_file(shared_ptr<file_node> file);

private:
	int file_count;
	int directory_count;
	int total_file_size;
};


class directory_tree
{
public:
	static shared_ptr<directory_tree> new_directory_tree(bfs::path& iroot_path, const std::vector<unicodestring>& iexclude_path = std::vector<unicodestring>());
	void recursive_apply(recursive_dir_op& rdo);
	shared_ptr<dir_node> get_root_node();

private:
	directory_tree();
	void recursive_apply(shared_ptr<dir_node> dtn, recursive_dir_op& rdo);
	void recursive_read(shared_ptr<directory_tree> dt, shared_ptr<dir_node> dtn, bfs::directory_iterator& di);

	std::vector<bfs::path> exclude_path;
	shared_ptr<dir_node> root_node;
};

#endif
