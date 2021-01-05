#include "stdafx.hxx"

#include "mod-data-seqv-tst.hxx"
#include <data-seqv.hxx>


namespace
{
   void main_test(const mws_path& i_tmp_path)
   {
      {
         // write an int and a float to a memory sequence and then read them back
         rw_seqv sequence;
         sequence.w.write_i32(42);
         sequence.w.write_f32(666.66f);
         sequence.rewind();
         int32_t meaning_of_life = sequence.r.read_i32();
         assert(meaning_of_life == 42);
         float how_much_for_an_apple = sequence.r.read_f32();
         assert(how_much_for_an_apple == 666.66f);
         sequence.rewind();

         // write an int to a memory sequence and then read it back as a float
         sequence.w.write_u32(0x5f3759df);
         sequence.rewind();
         float magic_number = sequence.r.read_f32();
      }
      {
         // same steps, but with a file sequence now
         // and also showing the equivalent template versions for reading/writing
         mws_path file_path = i_tmp_path / "test.binary";
         rw_file_seqv sequence(std_file_wrapper(file_path.string(), "w+b"));
         sequence.w.write(42);
         sequence.w.write(666.66f);
         sequence.rewind();
         int32_t meaning_of_life = sequence.r.read<int>();
         assert(meaning_of_life == 42);
         float how_much_for_an_apple = sequence.r.read<float>();
         assert(how_much_for_an_apple == 666.66f);
         sequence.rewind();
         sequence.w.write(0x5f3759df);
         sequence.rewind();
         float magic_number = sequence.r.read_f32();
      }
      {
         // sequence readers and writers also work with pointers to data sequences
         mem_data_seqv* sequence_ptr = new mem_data_seqv();
         data_seqv_writer_ptr dsw_ptr(sequence_ptr);
         data_seqv_reader_ptr dsr_ptr(sequence_ptr);
         dsw_ptr.write_u32(0x5f3759df);
         dsr_ptr.data_sequence()->rewind(); // equivalent to sequence_ptr->rewind();
         float magic_number = dsr_ptr.read_f32();
         delete sequence_ptr;
      }
      {
         // and shared pointers
         std::shared_ptr<mem_data_seqv> sequence_sp = std::make_shared<mem_data_seqv>();
         data_seqv_writer_sp dsw_sp(sequence_sp);
         data_seqv_reader_sp dsr_sp(sequence_sp);
         dsw_sp.write_u32(0x5f3759df);
         dsr_sp.data_sequence()->rewind(); // equivalent to sequence_sp->rewind();
         float magic_number = dsr_sp.read_f32();
      }
      {
         const uint32_t nr = 0x12345678;
         rw_seqv seqv;
         seqv.w.write_u32(nr);
         seqv.w.write(nr);
         seqv.rewind();
         uint32_t t0 = seqv.r.read_u32();
         uint32_t t1 = seqv.r.read<uint32_t>();
         seqv.rewind();
         uint32_t t2 = seqv.r.read<uint32_t>();
         uint32_t t3 = seqv.r.read_u32();
         assert(t0 == nr);
         assert(t0 == t1);
         assert(t0 == t2);
         assert(t0 == t3);
         seqv.rewind();
         seqv.w.write_u32(nr);
         seqv.rewind();
         float f0 = seqv.r.read_f32();
         seqv.rewind();
         seqv.w.write_f32(f0);
         seqv.rewind();
         uint32_t t4 = seqv.r.read_u32();
         assert(t0 == t4);
      }

      mws_println_all_builds("all tests passed");
   }
}


mod_data_seqv_tst::mod_data_seqv_tst() : mws_mod(mws_stringify(MOD_DATA_SEQV_TST)) {}

mws_sp<mod_data_seqv_tst> mod_data_seqv_tst::nwi()
{
   return mws_sp<mod_data_seqv_tst>(new mod_data_seqv_tst());
}

void mod_data_seqv_tst::init()
{
   main_test(storage.tmp_dir());
   set_app_exit_on_next_run(true);
}
