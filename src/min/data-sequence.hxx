#pragma once

#include "pfm.hxx"
#include <vector>
#include <string>


class data_sequence
{
public:
   data_sequence();
   virtual ~data_sequence();
   virtual bool reached_end_of_sequence();
   void close();
   virtual uint64 get_size()const = 0;
   virtual void reset() = 0;
   uint64 get_read_position()const { return read_position; }
   uint64 get_write_position()const { return write_position; }
   uint64 get_total_bytes_read()const { return total_bytes_read; }
   uint64 get_total_bytes_written()const { return total_bytes_written; }
   virtual void rewind() = 0;
   // return number of bytes read
   int read_bytes(int8* i_seqv, int i_elem_count, int i_offset);
   void write_bytes(const int8* i_seqv, int i_elem_count, int i_offset);

protected:
   // return number of bytes read/written
   virtual int read_int8(int8* i_seqv, int i_elem_count, int i_offset) = 0;
   virtual int write_int8(const int8* i_seqv, int i_elem_count, int i_offset) = 0;

   uint64 read_position;
   uint64 write_position;

private:
   uint64 total_bytes_read;
   uint64 total_bytes_written;
};


// doesn't copy the input data, but uses it directly. be very careful with this.
class mws_ro_mem_seq : public data_sequence
{
public:
   mws_ro_mem_seq(const uint8* i_seqv, uint64 i_elem_count) : seq(i_seqv), seq_size(i_elem_count) {}
   virtual ~mws_ro_mem_seq() {}

   const uint8* get_data_as_byte_array() { return seq; }
   virtual uint64 get_size() const { return seq_size; }
   virtual void reset() { rewind(); }
   virtual void rewind() override { set_read_position(0); }

   mws_sp<std::vector<uint8>> get_data_as_byte_vector();
   void set_read_position(uint64 i_position);
   void set_write_position(uint64 i_position);

protected:
   virtual int read_int8(int8* i_seqv, int i_elem_count, int i_offset) override;
   virtual int write_int8(const int8* i_seqv, int i_elem_count, int i_offset) override;

private:
   const uint8* seq = nullptr;
   uint64 seq_size = 0;
};


class memory_data_sequence : public data_sequence
{
public:
   memory_data_sequence();
   memory_data_sequence(const uint8* i_seqv, int i_elem_count);
   virtual ~memory_data_sequence();

   virtual uint64 get_size()const { return sequence.size(); }
   virtual void reset() { rewind(); sequence.clear(); }
   const uint8* get_data_as_byte_array();
   mws_sp<std::vector<uint8>> get_data_as_byte_vector();
   virtual void rewind() override { set_read_position(0); set_write_position(0); }
   void set_read_position(uint64 i_position);
   void set_write_position(uint64 i_position);

protected:
   int read_int8(int8* i_seqv, int i_elem_count, int i_offset);
   int write_int8(const int8* i_seqv, int i_elem_count, int i_offset);

private:
   std::vector<int8> sequence;
};


class file_data_sequence : public data_sequence
{
public:
   file_data_sequence(mws_sp<mws_file> i_file, bool i_is_writable);
   virtual ~file_data_sequence() {}

   bool reached_end_of_sequence() override;
   void close();
   virtual uint64 get_size()const;
   virtual void reset() override;
   virtual void rewind() override;
   virtual void seek(uint64 i_pos);

protected:
   int read_int8(int8* i_seqv, int i_elem_count, int i_offset);
   int write_int8(const int8* i_seqv, int i_elem_count, int i_offset);

private:
   mws_sp<mws_file> file;
   uint64 file_size = 0;
   bool is_writable = false;
};


template<class seq_ptr> class data_seq_rdr
{
public:
   data_seq_rdr() {}
   data_seq_rdr(seq_ptr i_seqv) : sequence(i_seqv) {}
   ~data_seq_rdr() {}

   seq_ptr get_data_sequence() { return sequence; }
   void set_data_sequence(seq_ptr i_seqv) { sequence = i_seqv; }

   // single data versions
   int8 read_int8()
   {
      int8 seqv;
      read_int8(&seqv, 1, 0);

      return seqv;
   }

   uint8 read_uint8()
   {
      return (uint8)read_int8();
   }

   int16 read_int16()
   {
      int8 seqv[2];
      read_int8(seqv, 2, 0);

      return *(int16*)seqv;
   }

   uint16 read_uint16()
   {
      return (uint16)read_int16();
   }

   int32 read_int32()
   {
      int8 seqv[4];
      read_int8(seqv, 4, 0);

      return *(int32*)seqv;
   }

   uint32 read_uint32()
   {
      return (uint32)read_int32();
   }

   int64 read_int64()
   {
      int8 seqv[8];
      read_int8(seqv, 8, 0);

      return *(int64*)seqv;
   }

   uint64 read_uint64()
   {
      return (uint64)read_int64();
   }

   fltp32 read_fltp32()
   {
      int32 r = read_int32();

      return *(fltp32*)&r;
   }

   fltp64 read_fltp64()
   {
      int64 r = read_int64();

      return *(fltp64*)&r;
   }

   std::string read_string()
   {
      uint32 elem_count = read_uint32();
      std::string text(elem_count, 0);
      read_int8((int8*)text.data(), elem_count, 0);

      return text;
   }

   std::string read_line()
   {
      std::string text;
      std::vector<char> line;
      int8 c = 0;
      line.reserve(256);

      while (true)
      {
         int bytes_read = read_int8(&c, 1, 0);

         if (bytes_read <= 0 || (c == '\n') || (c == '\r'))
         {
            break;
         }

         line.push_back(c);
      }

      if (!line.empty())
      {
         text = std::string(line.data(), line.size());
      }

      return text;
   }

   template<class T> void read_pointer(T*& i_seqv)
   {
      int8* p = (int8*)&i_seqv;
      sequence->read_bytes(p, sizeof(p), 0);
   }

   // sequence data versions. return the number of bytes read
   int read_int8(int8* i_seqv, int i_elem_count, int i_offset)
   {
      return sequence->read_bytes(i_seqv, i_elem_count, i_offset);
   }

   int read_uint8(uint8* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int8((int8*)i_seqv, i_elem_count, i_offset);
   }

   int read_int16(int16* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int8((int8*)i_seqv, i_elem_count * 2, i_offset * 2);
   }

   int read_uint16(uint16* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int16((int16*)i_seqv, i_elem_count, i_offset);
   }

   int read_int32(int32* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int8((int8*)i_seqv, i_elem_count * 4, i_offset * 4);
   }

   int read_uint32(uint32* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int32((int32*)i_seqv, i_elem_count, i_offset);
   }

   int read_int64(int64* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int8((int8*)i_seqv, i_elem_count * 8, i_offset * 8);
   }

   int read_uint64(uint64* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int64((int64*)i_seqv, i_elem_count, i_offset);
   }

   int read_fltp32(fltp32* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int32((int32*)i_seqv, i_elem_count, i_offset);
   }

   int read_fltp64(fltp64* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int64((int64*)i_seqv, i_elem_count, i_offset);
   }

private:
   seq_ptr sequence = nullptr;
};


using data_sequence_reader = data_seq_rdr<mws_sp<data_sequence>>;
using data_seq_rdr_ptr = data_seq_rdr<data_sequence*>;


class data_sequence_writer
{
public:
   data_sequence_writer() {}
   data_sequence_writer(mws_sp<data_sequence> i_seqv) : sequence(i_seqv) {}
   ~data_sequence_writer() {}

   mws_sp<data_sequence> get_data_sequence() { return sequence; }
   void set_data_sequence(mws_sp<data_sequence> i_seqv) { sequence = i_seqv; }

   // single data versions
   void write_int8(int8 d);
   void write_uint8(uint8 d);
   void write_int16(int16 d);
   void write_uint16(uint16 d);
   void write_int32(int32 d);
   void write_uint32(uint32 d);
   void write_int64(int64 d);
   void write_uint64(uint64 d);
   void write_fltp32(fltp32 d);
   void write_fltp64(fltp64 d);
   void write_string(const std::string& i_text);
   void write_line(const std::string& i_text, bool inew_line = true);
   template<class T> void write_pointer(T* const i_seqv);

   // sequence data versions
   void write_int8(const int8* i_seqv, int i_elem_count, int i_offset);
   void write_uint8(const uint8* i_seqv, int i_elem_count, int i_offset);
   void write_int16(const int16* i_seqv, int i_elem_count, int i_offset);
   void write_uint16(const uint16* i_seqv, int i_elem_count, int i_offset);
   void write_int32(const int32* i_seqv, int i_elem_count, int i_offset);
   void write_uint32(const uint32* i_seqv, int i_elem_count, int i_offset);
   void write_int64(const int64* i_seqv, int i_elem_count, int i_offset);
   void write_uint64(const uint64* i_seqv, int i_elem_count, int i_offset);
   void write_fltp32(const fltp32* i_seqv, int i_elem_count, int i_offset);
   void write_fltp64(const fltp64* i_seqv, int i_elem_count, int i_offset);

private:
   mws_sp<data_sequence> sequence;
};


class rw_sequence : public memory_data_sequence
{
public:
   ~rw_sequence() {}

   static mws_sp<rw_sequence> nwi()
   {
      mws_sp<rw_sequence> inst(new rw_sequence());
      inst->r.set_data_sequence(inst);
      inst->w.set_data_sequence(inst);
      return inst;
   }

   data_sequence_reader r;
   data_sequence_writer w;

private:
   rw_sequence() {}
};


class rw_file_sequence : public file_data_sequence
{
public:
   virtual ~rw_file_sequence() {}

   static mws_sp<rw_file_sequence> nwi(mws_sp<mws_file> i_file, bool i_is_writable)
   {
      mws_sp<rw_file_sequence> inst(new rw_file_sequence(i_file, i_is_writable));
      inst->r.set_data_sequence(inst);
      if (i_is_writable) { inst->w.set_data_sequence(inst); }
      return inst;
   }

   data_sequence_reader r;
   data_sequence_writer w;

private:
   rw_file_sequence(mws_sp<mws_file> i_file, bool i_is_writable) : file_data_sequence(i_file, i_is_writable) {}
};



// implementation // implementation // implementation // implementation // implementation



inline data_sequence::data_sequence()
{
   read_position = 0;
   write_position = 0;
   total_bytes_read = 0;
   total_bytes_written = 0;
}

inline data_sequence::~data_sequence()
{
}

inline bool data_sequence::reached_end_of_sequence()
{
   uint64 read_pos = get_read_position();
   uint64 size = get_size();

   return read_pos >= size;
}

inline void data_sequence::close()
{
}

inline int data_sequence::read_bytes(int8* i_seqv, int i_elem_count, int i_offset)
{
   int bytes_read = read_int8(i_seqv, i_elem_count, i_offset);

   read_position += bytes_read;
   total_bytes_read += bytes_read;

   return bytes_read;
}

inline void data_sequence::write_bytes(const int8* i_seqv, int i_elem_count, int i_offset)
{
   int bytes_written = write_int8(i_seqv, i_elem_count, i_offset);

   write_position += bytes_written;
   total_bytes_written += bytes_written;
}


inline mws_sp<std::vector<uint8>> mws_ro_mem_seq::get_data_as_byte_vector()
{
   mws_sp<std::vector<uint8>> seqv;
   size_t size = static_cast<size_t>(get_size());

   if (size > 0)
   {
      seqv = mws_sp<std::vector<uint8>>(new std::vector<uint8>(size));
      memcpy(seqv->data(), seq, size);
   }

   return seqv;
}

inline void mws_ro_mem_seq::set_read_position(uint64 i_position)
{
   if (i_position >= 0 && i_position <= get_size())
   {
      read_position = i_position;
   }
}

inline void mws_ro_mem_seq::set_write_position(uint64)
{
   mws_throw mws_exception("n/a");
}

inline int mws_ro_mem_seq::read_int8(int8* i_seqv, int i_elem_count, int i_offset)
{
   int bytes_to_read = 0;

   if (i_elem_count > 0 && get_read_position() < get_size())
   {
      bytes_to_read = (int)std::min((uint64)i_elem_count, get_size() - get_read_position());
      memcpy(&i_seqv[i_offset], &seq[(size_t)get_read_position()], bytes_to_read);
   }

   return bytes_to_read;
}

inline int mws_ro_mem_seq::write_int8(const int8*, int, int)
{
   mws_throw mws_exception("n/a");

   return -1;
}


inline memory_data_sequence::memory_data_sequence()
{
}

inline memory_data_sequence::memory_data_sequence(const uint8* i_seqv, int i_elem_count)
{
   sequence.resize(i_elem_count);
   memcpy(&sequence[0], i_seqv, i_elem_count);
}

inline memory_data_sequence::~memory_data_sequence()
{
}

inline const uint8* memory_data_sequence::get_data_as_byte_array()
{
   return (const uint8*)sequence.data();
}

inline mws_sp<std::vector<uint8>> memory_data_sequence::get_data_as_byte_vector()
{
   mws_sp<std::vector<uint8>> seqv;
   size_t size = static_cast<size_t>(get_size());

   if (size > 0)
   {
      seqv = mws_sp<std::vector<uint8>>(new std::vector<uint8>(size));
      memcpy(seqv->data(), sequence.data(), size);
   }

   return seqv;
}

inline void memory_data_sequence::set_read_position(uint64 i_position)
{
   if (i_position >= 0 && i_position <= get_size())
   {
      read_position = i_position;
   }
}

inline void memory_data_sequence::set_write_position(uint64 i_position)
{
   if (i_position >= 0 && i_position <= get_size())
   {
      write_position = i_position;
   }
}

inline int memory_data_sequence::read_int8(int8* i_seqv, int i_elem_count, int i_offset)
{
   int bytes_to_read = 0;

   if (i_elem_count > 0 && get_read_position() < get_size())
   {
      bytes_to_read = (int)std::min((uint64)i_elem_count, get_size() - get_read_position());
      memcpy(&i_seqv[i_offset], &sequence[(size_t)get_read_position()], bytes_to_read);
   }

   return bytes_to_read;
}

inline int memory_data_sequence::write_int8(const int8* i_seqv, int i_elem_count, int i_offset)
{
   if (i_elem_count > 0)
   {
      if (get_size() - get_write_position() < i_elem_count)
      {
         sequence.resize((size_t)get_size() + i_elem_count);
      }

      memcpy(&sequence[(size_t)get_write_position()], &i_seqv[i_offset], i_elem_count);
   }

   return i_elem_count;
}


inline file_data_sequence::file_data_sequence(mws_sp<mws_file> i_file, bool i_is_writable)
{
   mws_assert(i_file != nullptr);
   mws_assert(i_file->is_open());
   mws_assert((i_is_writable) ? i_file->is_writable() : true);
   file = i_file;
   is_writable = i_is_writable;
}

inline bool file_data_sequence::reached_end_of_sequence()
{
   if (file_size == 0)
   {
      file_size = get_size();
   }

   if (get_write_position() > file_size)
   {
      return get_read_position() >= get_write_position();
   }

   return get_read_position() >= file_size;
   //return file->io.reached_eof();
}

inline void file_data_sequence::close()
{
   file->io.close();
}

inline uint64 file_data_sequence::get_size()const
{
   return file->length();
}

inline void file_data_sequence::reset()
{
   rewind();
   file->io.seek(0);
}

inline void file_data_sequence::rewind()
{
   seek(0);
}

inline void file_data_sequence::seek(uint64 i_pos)
{
   file->io.seek(i_pos);
   read_position = i_pos;
   write_position = i_pos;
}

inline int file_data_sequence::read_int8(int8* i_seqv, int i_elem_count, int i_offset)
{
   int bytes_read = 0;

   if (i_elem_count > 0 && get_read_position() < get_size())
   {
      bytes_read = file->io.read((uint8*)i_seqv, i_elem_count);
   }

   if (bytes_read != i_elem_count)
   {
      mws_throw mws_exception("reached end of file");
   }

   return bytes_read;
}

inline int file_data_sequence::write_int8(const int8* i_seqv, int i_elem_count, int i_offset)
{
   int bytes_written = 0;

   if (i_elem_count > 0)
   {
      bytes_written = file->io.write((uint8*)i_seqv, i_elem_count);
   }

   return bytes_written;
}


inline void data_sequence_writer::write_int8(int8 i_seqv)
{
   write_int8(&i_seqv, 1, 0);
}

inline void data_sequence_writer::write_uint8(uint8 i_seqv)
{
   write_int8(i_seqv);
}

inline void data_sequence_writer::write_int16(int16 i_seqv)
{
   write_int8((int8*)&i_seqv, 2, 0);
}

inline void data_sequence_writer::write_uint16(uint16 i_seqv)
{
   write_int16(i_seqv);
}

inline void data_sequence_writer::write_int32(int32 i_seqv)
{
   write_int8((int8*)&i_seqv, 4, 0);
}

inline void data_sequence_writer::write_uint32(uint32 i_seqv)
{
   write_int32(i_seqv);
}

inline void data_sequence_writer::write_int64(int64 i_seqv)
{
   write_int8((int8*)&i_seqv, 8, 0);
}

inline void data_sequence_writer::write_uint64(uint64 i_seqv)
{
   write_int64(i_seqv);
}

inline void data_sequence_writer::write_fltp32(fltp32 i_seqv)
{
   write_int8((int8*)&i_seqv, 4, 0);
}

inline void data_sequence_writer::write_fltp64(fltp64 i_seqv)
{
   write_int8((int8*)&i_seqv, 8, 0);
}

inline void data_sequence_writer::write_string(const std::string& i_text)
{
   write_uint32(i_text.length());
   write_int8((int8*)i_text.data(), i_text.length(), 0);
}

inline void data_sequence_writer::write_line(const std::string& i_text, bool inew_line)
{
   write_int8((int8*)&i_text[0], i_text.length(), 0);

   if (inew_line)
   {
      write_int8('\n');
   }
}

template<int> void write_pointer_helper(mws_sp<data_sequence> sequence, void* i_seqv);

// 32-bit systems
template<> inline void write_pointer_helper<4>(mws_sp<data_sequence> sequence, void* i_seqv)
{
   sequence->write_bytes((int8*)i_seqv, 4, 0);
}

// 64-bit systems
template<> inline void write_pointer_helper<8>(mws_sp<data_sequence> sequence, void* i_seqv)
{
   sequence->write_bytes((int8*)i_seqv, 8, 0);
}

template<class T> inline void data_sequence_writer::write_pointer(T* const i_seqv)
{
   write_pointer_helper<sizeof(void*)>(sequence, (void*)&i_seqv);
}

inline void data_sequence_writer::write_int8(const int8* i_seqv, int i_elem_count, int i_offset)
{
   sequence->write_bytes(i_seqv, i_elem_count, i_offset);
}

inline void data_sequence_writer::write_uint8(const uint8* i_seqv, int i_elem_count, int i_offset)
{
   write_int8((int8*)i_seqv, i_elem_count, i_offset);
}

inline void data_sequence_writer::write_int16(const int16* i_seqv, int i_elem_count, int i_offset)
{
   write_int8((int8*)i_seqv, i_elem_count * 2, i_offset * 2);
}

inline void data_sequence_writer::write_uint16(const uint16* i_seqv, int i_elem_count, int i_offset)
{
   write_int16((int16*)i_seqv, i_elem_count, i_offset);
}

inline void data_sequence_writer::write_int32(const int32* i_seqv, int i_elem_count, int i_offset)
{
   write_int8((int8*)i_seqv, i_elem_count * 4, i_offset * 4);
}

inline void data_sequence_writer::write_uint32(const uint32* i_seqv, int i_elem_count, int i_offset)
{
   write_int32((int32*)i_seqv, i_elem_count, i_offset);
}

inline void data_sequence_writer::write_int64(const int64* i_seqv, int i_elem_count, int i_offset)
{
   write_int8((int8*)i_seqv, i_elem_count * 8, i_offset * 8);
}

inline void data_sequence_writer::write_uint64(const uint64* i_seqv, int i_elem_count, int i_offset)
{
   write_int64((int64*)i_seqv, i_elem_count, i_offset);
}

inline void data_sequence_writer::write_fltp32(const fltp32* i_seqv, int i_elem_count, int i_offset)
{
   write_int32((int32*)i_seqv, i_elem_count, i_offset);
}

inline void data_sequence_writer::write_fltp64(const fltp64* i_seqv, int i_elem_count, int i_offset)
{
   write_int64((int64*)i_seqv, i_elem_count, i_offset);
}




class data_sequence_reader_big_endian
{
public:
   data_sequence_reader_big_endian(data_sequence* i_seqv) : sequence(i_seqv) {}
   ~data_sequence_reader_big_endian() {}

   // single data versions
   int8 read_int8();
   uint8 read_uint8();
   int16 read_int16();
   uint16 read_uint16();
   int32 read_int32();
   uint32 read_uint32();
   int64 read_int64();
   uint64 read_uint64();
   fltp32 read_fltp32();
   fltp64 read_fltp64();

   // sequence data versions
   void read_int8(int8* i_seqv, int i_elem_count, int i_offset);
   void read_uint8(uint8* i_seqv, int i_elem_count, int i_offset);
   //void read_int16(int16* i_seqv, int i_elem_count, int i_offset);
   //void read_uint16(uint16* i_seqv, int i_elem_count, int i_offset);
   //void read_int32(int32* i_seqv, int i_elem_count, int i_offset);
   //void read_uint32(uint32* i_seqv, int i_elem_count, int i_offset);
   //void read_int64(int64* i_seqv, int i_elem_count, int i_offset);
   //void read_uint64(uint64* i_seqv, int i_elem_count, int i_offset);
   //void read_fltp32(fltp32* i_seqv, int i_elem_count, int i_offset);
   //void read_fltp64(fltp64* i_seqv, int i_elem_count, int i_offset);

private:
   data_sequence* sequence;
};


inline int8 data_sequence_reader_big_endian::read_int8()
{
   int8 seqv;
   read_int8(&seqv, 1, 0);

   return seqv;
}

inline uint8 data_sequence_reader_big_endian::read_uint8()
{
   return (uint8)read_int8();
}

inline int16 data_sequence_reader_big_endian::read_int16()
{
   int8 seqv[2];
   read_int8(seqv, 2, 0);
   int16 r = ((seqv[1] & 0xff) | (seqv[0] & 0xff) << 8);

   return r;
}

inline uint16 data_sequence_reader_big_endian::read_uint16()
{
   return (uint16)read_int16();
}

inline int32 data_sequence_reader_big_endian::read_int32()
{
   int8 seqv[4];
   read_int8(seqv, 4, 0);
   int32 r = (seqv[3] & 0xff) | ((seqv[2] & 0xff) << 8) | ((seqv[1] & 0xff) << 16) | ((seqv[0] & 0xff) << 24);

   return r;
}

inline uint32 data_sequence_reader_big_endian::read_uint32()
{
   return (uint32)read_int32();
}

inline int64 data_sequence_reader_big_endian::read_int64()
{
   int8 seqv[8];
   read_int8(seqv, 8, 0);
   int64 r = ((int64)(seqv[7] & 0xff) | ((int64)(seqv[6] & 0xff) << 8) | ((int64)(seqv[5] & 0xff) << 16) | ((int64)(seqv[4] & 0xff) << 24) |
      ((int64)(seqv[3] & 0xff) << 32) | ((int64)(seqv[2] & 0xff) << 40) | ((int64)(seqv[1] & 0xff) << 48) | ((int64)(seqv[0] & 0xff) << 56));

   return r;
}

inline uint64 data_sequence_reader_big_endian::read_uint64()
{
   return (uint64)read_int64();
}

inline fltp32 data_sequence_reader_big_endian::read_fltp32()
{
   int32 r = read_int32();

   return *(fltp32*)&r;
}

inline fltp64 data_sequence_reader_big_endian::read_fltp64()
{
   int64 r = read_int64();

   return *(fltp64*)&r;
}

inline void data_sequence_reader_big_endian::read_int8(int8* i_seqv, int i_elem_count, int i_offset)
{
   sequence->read_bytes(i_seqv, i_elem_count, i_offset);
}

inline void data_sequence_reader_big_endian::read_uint8(uint8* i_seqv, int i_elem_count, int i_offset)
{
   read_int8((int8*)i_seqv, i_elem_count, i_offset);
}
