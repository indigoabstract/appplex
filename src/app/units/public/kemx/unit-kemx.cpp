#include "stdafx.h"

#include "unit-kemx.hpp"

#ifdef UNIT_KEMX

#include "com/mws/mws-com.hpp"
#include "com/mws/mws-camera.hpp"
#include "data-sequence.hpp"
#include "node-info.hpp"
#include <exception>
#include <string>
#include <vector>

using std::string;
using std::vector;


template <class T> class checker
{
public:
   static bool compare_values(T v1, T v2, int idx = 0)
   {
      mws_assert(v1 == v2, "test failed");

      if (v1 != v2)
      {
         mws_print("test failed. values differ. idx: %d\n", idx);
         return false;
      }

      return true;
   }
};

unit_kemx::unit_kemx() : unit(mws_stringify(UNIT_KEMX))
{
   is_valid_expression = false;
}

mws_sp<unit_kemx> unit_kemx::nwi()
{
   return mws_sp<unit_kemx>(new unit_kemx());
}

void unit_kemx::init()
{
   auto kemx = pfm::filesystem::load_res_as_string("kemx-access.txt");
   int kemxSize = kemx->size();
   data = "[ " + *kemx + " ]";

   mws_sp<memory_data_sequence> mds1(new memory_data_sequence());
   data_sequence_writer dsw(mds1);
   int8 t1 = -1;
   uint8 t2 = 177;
   int16 t3 = -1133;
   uint16 t4 = 65535;
   int32 t5 = -2147483639;
   uint32 t6 = 4294967289;
   int64 t7 = -140737488355319LL;
   uint64 t8 = 240737488355319LL;
   real32 t9 = 1333.757845f;
   real64 t10 = (real64)76429.856330875;

   dsw.write_int8(t1);
   dsw.write_uint8(t2);
   dsw.write_int16(t3);
   dsw.write_uint16(t4);
   dsw.write_int32(t5);
   dsw.write_uint32(t6);
   dsw.write_int64(t7);
   dsw.write_uint64(t8);
   dsw.write_real32(t9);
   dsw.write_real64(t10);

   const uint8* s = mds1->get_data_as_byte_array();
   bool x = storage.store_unit_byte_array("ds-test", s, mds1->get_size());
   mws_print("store ds-test %d\n", (int)x);

   mws_sp<memory_data_sequence> mds2(new memory_data_sequence(s, mds1->get_size()));
   data_sequence_reader dsr(mds2);

   int8 s1 = dsr.read_int8();
   uint8 s2 = dsr.read_uint8();
   int16 s3 = dsr.read_int16();
   uint16 s4 = dsr.read_uint16();
   int32 s5 = dsr.read_int32();
   uint32 s6 = dsr.read_uint32();
   int64 s7 = dsr.read_int64();
   uint64 s8 = dsr.read_uint64();
   real32 s9 = dsr.read_real32();
   real64 s10 = dsr.read_real64();

   int idx = 0;
   checker<int8>::compare_values(t1, s1, idx++);
   checker<uint8>::compare_values(t2, s2, idx++);
   checker<int16>::compare_values(t3, s3, idx++);
   checker<uint16>::compare_values(t4, s4, idx++);
   checker<int32>::compare_values(t5, s5, idx++);
   checker<uint32>::compare_values(t6, s6, idx++);
   checker<int64>::compare_values(t7, s7, idx++);
   checker<uint64>::compare_values(t8, s8, idx++);
   checker<real32>::compare_values(t9, s9, idx++);
   checker<real64>::compare_values(t10, s10, idx++);

   //int64 l1 = -2147483639;
   //l1 *= 64;
   //uint64 l2 = 4294967289;
   //l2 *= 64;
   //uint32 x1 = (l1 >> 32) & 0xffffffff;
   //uint32 x2 = (l1) & 0xffffffff;
   //uint32 x3 = (l2 >> 32) & 0xffffffff;
   //uint32 x4 = (l2) & 0xffffffff;
   //tracenl("load ds-test [%d, %d, %d, %d, %d, %u, %f %Lf]", s1, s2, s3, s4, s5, s6, s9, s10);
}

void unit_kemx::init_mws()
{
   mws_sp<mws_page> up = mws_page::nwi(mws_root);
   mws_sp<mws_tree> tree = mws_tree::nwi(up);
   mws_sp<mws_tree_model> mwstm(new mws_tree_model());

   tree->set_id("kemxtree");
   tree->set_model(mwstm);
   mws_cam->clear_color = true;
}

void unit_kemx::load()
{
   mws_try
   {
      //string exp = "[ dw[ dw[ dws[ url[ img[ src[*_*.jpg] [xxx] ] ] ] url[ a [ href [*kami_*_*.php] ] ] ] url[xxx-url] ] ]";
      string exp = data;

      is_valid_expression = false;
      node_info::parseBlock(exp, node_list, 0);
      trn();
      node_info::printList(node_list, 0);
      trn();
      is_valid_expression = true;

      mws_sp<mws_tree_model_node> node(new mws_tree_model_node("root"));
      int length = 0;
      mws_sp<mws_tree> tree = static_pointer_cast<mws_tree>(mws_root->find_by_id("kemxtree"));
      mws_sp<mws_tree_model> treemodel = tree->get_model();

      create_mws_tree_model(node_list, node, length);
      treemodel->set_length(length);
      treemodel->set_root_node(node);
      treemodel->notify_update();
   }
      mws_catch(std::exception& e)
   {
      mws_print("exception %s\n", e.what());
   }
}

void unit_kemx::unload()
{
   node_list.clear();
}

void unit_kemx::create_mws_tree_model(const vector<mws_sp<node_info> >& list, mws_sp<mws_tree_model_node> node, int& length)
{
   int size = list.size();

   for (int k = 0; k < size; k++)
   {
      mws_sp<node_info> kv = list[k];
      mws_sp<mws_tree_model_node> nn(new mws_tree_model_node(kv->getName()));

      node->nodes.push_back(nn);
      length++;

      if (kv->getList().size() > 0)
      {
         create_mws_tree_model(kv->getList(), nn, length);
      }
   }
}

#endif
