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


class data_sequence_reader
{
public:
   data_sequence_reader() {}
   data_sequence_reader(mws_sp<data_sequence> isequence) : sequence(isequence) {}
   ~data_sequence_reader() {}

   mws_sp<data_sequence> get_data_sequence() { return sequence.lock(); }
   void set_data_sequence(mws_sp<data_sequence> isequence) { sequence = isequence; }

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
   std::string read_string();
   std::string read_line();
   template<class T> void read_pointer(T*& s);

   // sequence data versions. return the number of bytes read
   int read_int8(int8* s, int elem_count, int offset);
   int read_uint8(uint8* s, int elem_count, int offset);
   int read_int16(int16* s, int elem_count, int offset);
   int read_uint16(uint16* s, int elem_count, int offset);
   int read_int32(int32* s, int elem_count, int offset);
   int read_uint32(uint32* s, int elem_count, int offset);
   int read_int64(int64* s, int elem_count, int offset);
   int read_uint64(uint64* s, int elem_count, int offset);
   int read_real32(real32* s, int elem_count, int offset);
   int read_real64(real64* s, int elem_count, int offset);

private:
   mws_wp<data_sequence> sequence;
};


class data_sequence_writer
{
public:
   data_sequence_writer() {}
   data_sequence_writer(mws_sp<data_sequence> isequence) : sequence(isequence) {}
   ~data_sequence_writer() {}

   mws_sp<data_sequence> get_data_sequence() { return sequence.lock(); }
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
   mws_wp<data_sequence> sequence;
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


inline int8 data_sequence_reader::read_int8()
{
   int8 s;
   read_int8(&s, 1, 0);

   return s;
}

inline uint8 data_sequence_reader::read_uint8()
{
   return (uint8)read_int8();
}

inline int16 data_sequence_reader::read_int16()
{
   int8 s[2];
   read_int8(s, 2, 0);

   return *(int16*)s;
}

inline uint16 data_sequence_reader::read_uint16()
{
   return (uint16)read_int16();
}

inline int32 data_sequence_reader::read_int32()
{
   int8 s[4];
   read_int8(s, 4, 0);

   return *(int32*)s;
}

inline uint32 data_sequence_reader::read_uint32()
{
   return (uint32)read_int32();
}

inline int64 data_sequence_reader::read_int64()
{
   int8 s[8];
   read_int8(s, 8, 0);

   return *(int64*)s;
}

inline uint64 data_sequence_reader::read_uint64()
{
   return (uint64)read_int64();
}

inline real32 data_sequence_reader::read_real32()
{
   int32 r = read_int32();

   return *(real32*)&r;
}

inline real64 data_sequence_reader::read_real64()
{
   int64 r = read_int64();

   return *(real64*)&r;
}

inline std::string data_sequence_reader::read_string()
{
   int32 elem_count = read_int32();
   std::string text(elem_count, 0);
   read_int8((int8*)&text[0], elem_count, 0);

   return text;
}

inline std::string data_sequence_reader::read_line()
{
   std::string text;
   std::vector<char> line;
   char c = 0;
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

template<int> void read_pointer_helper(mws_sp<data_sequence> sequence, void* p);

// 32-bit systems
template<> inline void read_pointer_helper<4>(mws_sp<data_sequence> sequence, void* p)
{
   sequence->read_bytes((int8*)p, 4, 0);
}

// 64-bit systems
template<> inline void read_pointer_helper<8>(mws_sp<data_sequence> sequence, void* p)
{
   sequence->read_bytes((int8*)p, 8, 0);
}

template<class T> inline void data_sequence_reader::read_pointer(T*& s)
{
   read_pointer_helper<sizeof(void*)>(sequence.lock(), (void*)&s);
}

inline int data_sequence_reader::read_int8(int8* s, int elem_count, int offset)
{
   return sequence.lock()->read_bytes(s, elem_count, offset);
}

inline int data_sequence_reader::read_uint8(uint8* s, int elem_count, int offset)
{
   return read_int8((int8*)s, elem_count, offset);
}

inline int data_sequence_reader::read_int16(int16* s, int elem_count, int offset)
{
   return read_int8((int8*)s, elem_count * 2, offset * 2);
}

inline int data_sequence_reader::read_uint16(uint16* s, int elem_count, int offset)
{
   return read_int16((int16*)s, elem_count, offset);
}

inline int data_sequence_reader::read_int32(int32* s, int elem_count, int offset)
{
   return read_int8((int8*)s, elem_count * 4, offset * 4);
}

inline int data_sequence_reader::read_uint32(uint32* s, int elem_count, int offset)
{
   return read_int32((int32*)s, elem_count, offset);
}

inline int data_sequence_reader::read_int64(int64* s, int elem_count, int offset)
{
   return read_int8((int8*)s, elem_count * 8, offset * 8);
}

inline int data_sequence_reader::read_uint64(uint64* s, int elem_count, int offset)
{
   return read_int64((int64*)s, elem_count, offset);
}

inline int data_sequence_reader::read_real32(real32* s, int elem_count, int offset)
{
   return read_int32((int32*)s, elem_count, offset);
}

inline int data_sequence_reader::read_real64(real64* s, int elem_count, int offset)
{
   return read_int64((int64*)s, elem_count, offset);
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
   write_pointer_helper<sizeof(void*)>(sequence.lock(), (void*)&s);
}

inline void data_sequence_writer::write_int8(const int8* s, int elem_count, int offset)
{
   sequence.lock()->write_bytes(s, elem_count, offset);
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
