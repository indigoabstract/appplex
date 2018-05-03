#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_FFMPEG

#include "unit.hpp"


class unit_test_ffmpeg : public unit
{
public:
	static shared_ptr<unit_test_ffmpeg> nwi();

    virtual void init();
    virtual void load();
	virtual bool update();

private:
	unit_test_ffmpeg();
	virtual void receive(shared_ptr<iadp> idp);
};

#endif
