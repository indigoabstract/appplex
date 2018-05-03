#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_FREEIMAGE

#include "unit.hpp"
#include <string>
#include <vector>


class unit_test_freeimage : public unit
{
public:
	static shared_ptr<unit_test_freeimage> nwi();

    virtual void init();
	virtual void init_mws();
    virtual void load();

	static void save_image(std::string ifilename, shared_ptr<std::vector<uint32> > ibgra);

private:
	unit_test_freeimage();
};

#endif
