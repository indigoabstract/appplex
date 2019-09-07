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
   int read_bytes(int8* s, int elem_count, int offset);
   void write_bytes(const int8* s, int elem_count, int offset);

protected:
   // return number of bytes read/written
   virtual int read_int8(int8* s, int elem_count, int offset) = 0;
   virtual int write_int8(const int8* s, int elem_count, int offset) = 0;

   uint64 read_position;
   uint64 write_position;

private:
   uint64 total_bytes_read;
   uint64 total_bytes_written;
};


class mws_ro_mem_seq : public data_sequence
{
public:
   mws_ro_mem_seq(const uint8* i_data, uint64 i_elem_count) : seq(i_data), seq_size(i_elem_count) {}
   ~mws_ro_mem_seq() {}

   const uint8* get_data_as_byte_array()
   {
      return seq;
   }

   virtual uint64 get_size() const { return seq_size; }
   virtual void reset() { rewind(); }
   virtual void rewind() override { set_read_position(0); }

   mws_sp<std::vector<uint8> > get_data_as_byte_vector()
   {
      mws_sp<std::vector<uint8> > s;
      size_t size = static_cast<size_t>(get_size());

      if (size > 0)
      {
         s = mws_sp<std::vector<uint8> >(new std::vector<uint8>(size));
         memcpy(s->data(), seq, size);
      }

      return s;
   }

   void set_read_position(uint64 position)
   {
      if (position >= 0 && position <= get_size())
      {
         read_position = position;
      }
   }

   void set_write_position(uint64 position)
   {
      mws_throw mws_exception("n/a");
   }

protected:
   int read_int8(int8* s, int i_elem_count, int offset)
   {
      int bytes_to_read = 0;

      if (i_elem_count > 0 && get_read_position() < get_size())
      {
         bytes_to_read = (int)std::min((uint64)i_elem_count, get_size() - get_read_position());
         memcpy(&s[offset], &seq[(size_t)get_read_position()], bytes_to_read);
      }

      return bytes_to_read;
   }

   int write_int8(const int8* s, int i_elem_count, int offset)
   {
      mws_throw mws_exception("n/a");

      return -1;
   }

private:
   const uint8* seq;
   uint64 seq_size;
};


class memory_data_sequence : public data_sequence
{
public:
   memory_data_sequence();
   memory_data_sequence(const uint8* s, int elem_count);
   ~memory_data_sequence();
   virtual uint64 get_size()const { return sequence.size(); }
   virtual void reset() { rewind(); sequence.clear(); }
   const uint8* get_data_as_byte_array();
   mws_sp<std::vector<uint8> > get_data_as_byte_vector();
   virtual void rewind() override { set_read_position(0); set_write_position(0); }
   void set_read_position(uint64 position);
   void set_write_position(uint64 position);

protected:
   int read_int8(int8* s, int elem_count, int offset);
   int write_int8(const int8* s, int elem_count, int offset);

private:
   std::vector<int8> sequence;
};


class file_data_sequence : public data_sequence
{
public:
   file_data_sequence(mws_sp<pfm_file> ifile);
   ~file_data_sequence();
   bool reached_end_of_sequence() override;
   void close();
   virtual uint64 get_size()const;
   virtual void reset() override;
   virtual void rewind() override;
   virtual void seek(uint64 ipos);

protected:
   int read_int8(int8* s, int elem_count, int offset);
   int write_int8(const int8* s, int elem_count, int offset);

private:
   mws_sp<pfm_file> file;
   uint64 file_size = 0;
};


template<class seq_ptr> class data_seq_rdr
{
public:
   data_seq_rdr() : sequence(nullptr) {}
   data_seq_rdr(seq_ptr isequence) : sequence(isequence) {}
   ~data_seq_rdr() {}

   seq_ptr get_data_sequence() { return sequence; }
   void set_data_sequence(seq_ptr isequence) { sequence = isequence; }

   // single data versions
   int8 read_int8()
   {
      int8 s;
      read_int8(&s, 1, 0);

      return s;
   }

   uint8 read_uint8()
   {
      return (uint8)read_int8();
   }

   int16 read_int16()
   {
      int8 s[2];
      read_int8(s, 2, 0);

      return *(int16*)s;
   }

   uint16 read_uint16()
   {
      return (uint16)read_int16();
   }

   int32 read_int32()
   {
      int8 s[4];
      read_int8(s, 4, 0);

      return *(int32*)s;
   }

   uint32 read_uint32()
   {
      return (uint32)read_int32();
   }

   int64 read_int64()
   {
      int8 s[8];
      read_int8(s, 8, 0);

      return *(int64*)s;
   }

   uint64 read_uint64()
   {
      return (uint64)read_int64();
   }

   real32 read_real32()
   {
      int32 r = read_int32();

      return *(real32*)& r;
   }

   real64 read_real64()
   {
      int64 r = read_int64();

      return *(real64*)& r;
   }

   std::string read_string()
   {
      int32 elem_count = read_int32();
      std::string text(elem_count, 0);
      read_int8((int8*)& text[0], elem_count, 0);

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

   template<class T> void read_pointer(T*& s)
   {
      int8* p = (int8*)& s;
      sequence->read_bytes(p, sizeof(p), 0);
   }

   // sequence data versions. return the number of bytes read
   int read_int8(int8* s, int elem_count, int offset)
   {
      return sequence->read_bytes(s, elem_count, offset);
   }

   int read_uint8(uint8* s, int elem_count, int offset)
   {
      return read_int8((int8*)s, elem_count, offset);
   }

   int read_int16(int16* s, int elem_count, int offset)
   {
      return read_int8((int8*)s, elem_count * 2, offset * 2);
   }

   int read_uint16(uint16* s, int elem_count, int offset)
   {
      return read_int16((int16*)s, elem_count, offset);
   }

   int read_int32(int32* s, int elem_count, int offset)
   {
      return read_int8((int8*)s, elem_count * 4, offset * 4);
   }

   int read_uint32(uint32* s, int elem_count, int offset)
   {
      return read_int32((int32*)s, elem_count, offset);
   }

   int read_int64(int64* s, int elem_count, int offset)
   {
      return read_int8((int8*)s, elem_count * 8, offset * 8);
   }

   int read_uint64(uint64* s, int elem_count, int offset)
   {
      return read_int64((int64*)s, elem_count, offset);
   }

   int read_real32(real32* s, int elem_count, int offset)
   {
      return read_int32((int32*)s, elem_count, offset);
   }

   int read_real64(real64* s, int elem_count, int offset)
   {
      return read_int64((int64*)s, elem_count, offset);
   }

private:
   seq_ptr sequence;
};


using data_sequence_reader = data_seq_rdr<mws_sp<data_sequence>>;
using data_seq_rdr_ptr = data_seq_rdr<data_sequence*>;


class data_sequence_writer
{
public:
   data_sequence_writer() {}
   data_sequence_writer(mws_sp<data_sequence> isequence) : sequence(isequence) {}
   ~data_sequence_writer() {}

   mws_sp<data_sequence> get_data_sequence() { return sequence; }
   void set_data_sequence(mws_sp<data_sequence> isequence) { sequence = isequence; }

   // single data versions
   void write_int8(int8 d);
   void write_uint8(uint8 d);
   void write_int16(int16 d);
   void write_uint16(uint16 d);
   void write_int32(int32 d);
   void write_uint32(uint32 d);
   void write_int64(int64 d);
   void write_uint64(uint64 d);
   void write_real32(real32 d);
   void write_real64(real64 d);
   void write_string(const std::string& itext);
   void write_line(const std::string& itext, bool inew_line = true);
   template<class T> void write_pointer(T* const s);

   // sequence data versions
   void write_int8(const int8* s, int elem_count, int offset);
   void write_uint8(const uint8* s, int elem_count, int offset);
   void write_int16(const int16* s, int elem_count, int offset);
   void write_uint16(const uint16* s, int elem_count, int offset);
   void write_int32(const int32* s, int elem_count, int offset);
   void write_uint32(const uint32* s, int elem_count, int offset);
   void write_int64(const int64* s, int elem_count, int offset);
   void write_uint64(const uint64* s, int elem_count, int offset);
   void write_real32(const real32* s, int elem_count, int offset);
   void write_real64(const real64* s, int elem_count, int offset);

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
   ~rw_file_sequence() {}

   static mws_sp<rw_file_sequence> nwi(mws_sp<pfm_file> ifile)
   {
      mws_sp<rw_file_sequence> inst(new rw_file_sequence(ifile));
      inst->r.set_data_sequence(inst);
      inst->w.set_data_sequence(inst);
      return inst;
   }

   data_sequence_reader r;
   data_sequence_writer w;

private:
   rw_file_sequence(mws_sp<pfm_file> ifile) : file_data_sequence(ifile) {}
};


// implementation

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

inline int data_sequence::read_bytes(int8* s, int elem_count, int offset)
{
   int bytes_read = read_int8(s, elem_count, offset);

   read_position += bytes_read;
   total_bytes_read += bytes_read;

   return bytes_read;
}

inline void data_sequence::write_bytes(const int8* s, int elem_count, int offset)
{
   int bytes_written = write_int8(s, elem_count, offset);

   write_position += bytes_written;
   total_bytes_written += bytes_written;
}


inline memory_data_sequence::memory_data_sequence()
{
}

inline memory_data_sequence::memory_data_sequence(const uint8* s, int elem_count)
{
   sequence.resize(elem_count);
   memcpy(&sequence[0], s, elem_count);
}

inline memory_data_sequence::~memory_data_sequence()
{
}

inline const uint8* memory_data_sequence::get_data_as_byte_array()
{
   return (const uint8*)sequence.data();
}

inline mws_sp<std::vector<uint8> > memory_data_sequence::get_data_as_byte_vector()
{
   mws_sp<std::vector<uint8> > s;
   size_t size = static_cast<size_t>(get_size());

   if (size > 0)
   {
      s = mws_sp<std::vector<uint8> >(new std::vector<uint8>(size));
      memcpy(s->data(), sequence.data(), size);
   }

   return s;
}

inline void memory_data_sequence::set_read_position(uint64 position)
{
   if (position >= 0 && position <= get_size())
   {
      read_position = position;
   }
}

inline void memory_data_sequence::set_write_position(uint64 position)
{
   if (position >= 0 && position <= get_size())
   {
      write_position = position;
   }
}

inline int memory_data_sequence::read_int8(int8* s, int elem_count, int offset)
{
   int bytes_to_read = 0;

   if (elem_count > 0 && get_read_position() < get_size())
   {
      bytes_to_read = (int)std::min((uint64)elem_count, get_size() - get_read_position());
      memcpy(&s[offset], &sequence[(size_t)get_read_position()], bytes_to_read);
   }

   return bytes_to_read;
}

inline int memory_data_sequence::write_int8(const int8 * s, int elem_count, int offset)
{
   if (elem_count > 0)
   {
      if (get_size() - get_write_position() < elem_count)
      {
         sequence.resize((size_t)get_size() + elem_count);
      }

      memcpy(&sequence[(size_t)get_write_position()], &s[offset], elem_count);
   }

   return elem_count;
}


inline file_data_sequence::file_data_sequence(mws_sp<pfm_file> ifile)
{
   file = ifile;
}

inline file_data_sequence::~file_data_sequence()
{
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

inline int file_data_sequence::read_int8(int8 * s, int elem_count, int offset)
{
   int bytes_read = 0;

   if (elem_count > 0 && get_read_position() < get_size())
   {
      bytes_read = file->io.read((uint8*)s, elem_count);
   }

   if (bytes_read != elem_count)
   {
      mws_throw mws_exception("reached end of file");
   }

   return bytes_read;
}

inline int file_data_sequence::write_int8(const int8 * s, int elem_count, int offset)
{
   int bytes_written = 0;

   if (elem_count > 0)
   {
      bytes_written = file->io.write((uint8*)s, elem_count);
   }

   return bytes_written;
}


inline void data_sequence_writer::write_int8(int8 s)
{
   write_int8(&s, 1, 0);
}

inline void data_sequence_writer::write_uint8(uint8 s)
{
   write_int8(s);
}

inline void data_sequence_writer::write_int16(int16 s)
{
   write_int8((int8*)&s, 2, 0);
}

inline void data_sequence_writer::write_uint16(uint16 s)
{
   write_int16(s);
}

inline void data_sequence_writer::write_int32(int32 s)
{
   write_int8((int8*)&s, 4, 0);
}

inline void data_sequence_writer::write_uint32(uint32 s)
{
   write_int32(s);
}

inline void data_sequence_writer::write_int64(int64 s)
{
   write_int8((int8*)&s, 8, 0);
}

inline void data_sequence_writer::write_uint64(uint64 s)
{
   write_int64(s);
}

inline void data_sequence_writer::write_real32(real32 s)
{
   write_int8((int8*)&s, 4, 0);
}

inline void data_sequence_writer::write_real64(real64 s)
{
   write_int8((int8*)&s, 8, 0);
}

inline void data_sequence_writer::write_string(const std::string& itext)
{
   write_int32(itext.length());
   write_int8((int8*)&itext[0], itext.length(), 0);
}

inline void data_sequence_writer::write_line(const std::string& itext, bool inew_line)
{
   write_int8((int8*)&itext[0], itext.length(), 0);

   if (inew_line)
   {
      write_int8('\n');
   }
}

template<int> void write_pointer_helper(mws_sp<data_sequence> sequence, void* s);

// 32-bit systems
template<> inline void write_pointer_helper<4>(mws_sp<data_sequence> sequence, void* s)
{
   sequence->write_bytes((int8*)s, 4, 0);
}

// 64-bit systems
template<> inline void write_pointer_helper<8>(mws_sp<data_sequence> sequence, void* s)
{
   sequence->write_bytes((int8*)s, 8, 0);
}

template<class T> inline void data_sequence_writer::write_pointer(T* const s)
{
   write_pointer_helper<sizeof(void*)>(sequence, (void*)&s);
}

inline void data_sequence_writer::write_int8(const int8* s, int elem_count, int offset)
{
   sequence->write_bytes(s, elem_count, offset);
}

inline void data_sequence_writer::write_uint8(const uint8* s, int elem_count, int offset)
{
   write_int8((int8*)s, elem_count, offset);
}

inline void data_sequence_writer::write_int16(const int16* s, int elem_count, int offset)
{
   write_int8((int8*)s, elem_count * 2, offset * 2);
}

inline void data_sequence_writer::write_uint16(const uint16* s, int elem_count, int offset)
{
   write_int16((int16*)s, elem_count, offset);
}

inline void data_sequence_writer::write_int32(const int32* s, int elem_count, int offset)
{
   write_int8((int8*)s, elem_count * 4, offset * 4);
}

inline void data_sequence_writer::write_uint32(const uint32* s, int elem_count, int offset)
{
   write_int32((int32*)s, elem_count, offset);
}

inline void data_sequence_writer::write_int64(const int64* s, int elem_count, int offset)
{
   write_int8((int8*)s, elem_count * 8, offset * 8);
}

inline void data_sequence_writer::write_uint64(const uint64* s, int elem_count, int offset)
{
   write_int64((int64*)s, elem_count, offset);
}

inline void data_sequence_writer::write_real32(const real32* s, int elem_count, int offset)
{
   write_int32((int32*)s, elem_count, offset);
}

inline void data_sequence_writer::write_real64(const real64* s, int elem_count, int offset)
{
   write_int64((int64*)s, elem_count, offset);
}




class data_sequence_reader_big_endian
{
public:
   data_sequence_reader_big_endian(data_sequence* isequence) : sequence(isequence) {}
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
   real32 read_real32();
   real64 read_real64();

   // sequence data versions
   void read_int8(int8* s, int elem_count, int offset);
   void read_uint8(uint8* s, int elem_count, int offset);
   //void read_int16(int16* s, int elem_count, int offset);
   //void read_uint16(uint16* s, int elem_count, int offset);
   //void read_int32(int32* s, int elem_count, int offset);
   //void read_uint32(uint32* s, int elem_count, int offset);
   //void read_int64(int64* s, int elem_count, int offset);
   //void read_uint64(uint64* s, int elem_count, int offset);
   //void read_real32(real32* s, int elem_count, int offset);
   //void read_real64(real64* s, int elem_count, int offset);

private:
   data_sequence * sequence;
};


inline int8 data_sequence_reader_big_endian::read_int8()
{
   int8 s;
   read_int8(&s, 1, 0);

   return s;
}

inline uint8 data_sequence_reader_big_endian::read_uint8()
{
   return (uint8)read_int8();
}

inline int16 data_sequence_reader_big_endian::read_int16()
{
   int8 s[2];
   read_int8(s, 2, 0);
   int16 r = ((s[1] & 0xff) | (s[0] & 0xff) << 8);

   return r;
}

inline uint16 data_sequence_reader_big_endian::read_uint16()
{
   return (uint16)read_int16();
}

inline int32 data_sequence_reader_big_endian::read_int32()
{
   int8 s[4];
   read_int8(s, 4, 0);
   int32 r = (s[3] & 0xff) | ((s[2] & 0xff) << 8) | ((s[1] & 0xff) << 16) | ((s[0] & 0xff) << 24);

   return r;
}

inline uint32 data_sequence_reader_big_endian::read_uint32()
{
   return (uint32)read_int32();
}

inline int64 data_sequence_reader_big_endian::read_int64()
{
   int8 s[8];
   read_int8(s, 8, 0);
   int64 r = ((int64)(s[7] & 0xff) | ((int64)(s[6] & 0xff) << 8) | ((int64)(s[5] & 0xff) << 16) | ((int64)(s[4] & 0xff) << 24) |
      ((int64)(s[3] & 0xff) << 32) | ((int64)(s[2] & 0xff) << 40) | ((int64)(s[1] & 0xff) << 48) | ((int64)(s[0] & 0xff) << 56));

   return r;
}

inline uint64 data_sequence_reader_big_endian::read_uint64()
{
   return (uint64)read_int64();
}

inline real32 data_sequence_reader_big_endian::read_real32()
{
   int32 r = read_int32();

   return *(real32*)&r;
}

inline real64 data_sequence_reader_big_endian::read_real64()
{
   int64 r = read_int64();

   return *(real64*)&r;
}

inline void data_sequence_reader_big_endian::read_int8(int8* s, int elem_count, int offset)
{
   sequence->read_bytes(s, elem_count, offset);
}

inline void data_sequence_reader_big_endian::read_uint8(uint8* s, int elem_count, int offset)
{
   read_int8((int8*)s, elem_count, offset);
}
