#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_SND_GENERATORS

#include "unit.hpp"


class unit_test_snd_generators : public unit
{
public:
	static std::shared_ptr<unit_test_snd_generators> nwi();

    virtual void init();
	virtual void init_mws();
    virtual void load();

private:
	unit_test_snd_generators();
};

#endif // UNIT_TEST_SND_GENERATORS
