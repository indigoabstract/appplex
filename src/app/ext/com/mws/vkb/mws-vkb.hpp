#include "appplex-conf.hpp"

#if defined MOD_MWS && defined MOD_VKB

#include "../mws.hpp"


class mws_vkb : public mws_virtual_keyboard
{
public:
   virtual ~mws_vkb() {}
   virtual void set_target(mws_sp<mws_text_box> i_txe) override {};
   void load(std::string i_filename);

protected:
   mws_vkb() {}
};

#endif
