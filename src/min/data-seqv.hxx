#pragma once

#include "pfm.hxx"
#include <cstdint>
#include <memory>
#include <vector>
#include <string>


class data_seqv
{
public:
   data_seqv();
   virtual ~data_seqv();
   virtual bool reached_end_of_sequence();
   void close();
   virtual const uint8_t* data_as_byte_array() = 0;
   // return total number of bytes in the sequence
   virtual uint64_t size()const = 0;
   virtual void reset() = 0;
   uint64_t read_position()const { return read_position_v; }
   uint64_t write_position()const { return write_position_v; }
   uint64_t total_bytes_read()const { return total_bytes_read_v; }
   uint64_t total_bytes_written()const { return total_bytes_written_v; }
   virtual void rewind() = 0;
   // return number of bytes read
   int read_bytes(int8_t* i_seqv, int i_elem_count, int i_offset);
   void write_bytes(const int8_t* i_seqv, int i_elem_count, int i_offset);

protected:
   // return number of bytes read/written
   virtual int read_int8(int8_t* i_seqv, int i_elem_count, int i_offset) = 0;
   virtual int write_int8(const int8_t* i_seqv, int i_elem_count, int i_offset) = 0;

   uint64_t read_position_v;
   uint64_t write_position_v;

private:
   uint64_t total_bytes_read_v;
   uint64_t total_bytes_written_v;
};


// doesn't copy the input data, but uses it directly. be very careful with this.
class ro_mem_seqv : public data_seqv
{
public:
   ro_mem_seqv(const uint8_t* i_seqv, uint64_t i_elem_count) : seqv(i_seqv), size_v(i_elem_count) {}
   virtual ~ro_mem_seqv() {}

   const uint8_t* data_as_byte_array() override { return seqv; }
   virtual uint64_t size() const { return size_v; }
   virtual void reset() { rewind(); }
   virtual void rewind() override { set_read_position(0); }

   std::shared_ptr<std::vector<uint8_t>> data_as_byte_vector();
   void set_read_position(uint64_t i_position);
   void set_write_position(uint64_t i_position);

protected:
   virtual int read_int8(int8_t* i_seqv, int i_elem_count, int i_offset) override;
   virtual int write_int8(const int8_t* i_seqv, int i_elem_count, int i_offset) override;

private:
   const uint8_t* seqv = nullptr;
   uint64_t size_v = 0;
};


class mem_data_seqv : public data_seqv
{
public:
   mem_data_seqv();
   mem_data_seqv(const uint8_t* i_seqv, int i_elem_count);
   virtual ~mem_data_seqv();

   virtual uint64_t size()const { return seqv.size(); }
   virtual void reset() { rewind(); seqv.clear(); }
   const uint8_t* data_as_byte_array() override;
   std::shared_ptr<std::vector<uint8_t>> data_as_byte_vector();
   virtual void rewind() override { set_read_position(0); set_write_position(0); }
   void set_read_position(uint64_t i_position);
   void set_write_position(uint64_t i_position);

protected:
   int read_int8(int8_t* i_seqv, int i_elem_count, int i_offset);
   int write_int8(const int8_t* i_seqv, int i_elem_count, int i_offset);

private:
   std::vector<int8_t> seqv;
};


class file_data_seqv : public data_seqv
{
public:
   file_data_seqv(std::shared_ptr<mws_file> i_file, bool i_is_writable);
   virtual ~file_data_seqv() {}

   bool reached_end_of_sequence() override;
   void close();
   virtual const uint8_t* data_as_byte_array() override { return nullptr; }
   virtual uint64_t size()const;
   virtual void reset() override;
   virtual void rewind() override;
   virtual void seek(uint64_t i_pos);

protected:
   int read_int8(int8_t* i_seqv, int i_elem_count, int i_offset);
   int write_int8(const int8_t* i_seqv, int i_elem_count, int i_offset);

private:
   std::shared_ptr<mws_file> file;
   uint64_t file_size = 0;
   bool is_writable = false;
};


template<class seqv_ptr> class data_seqv_reader
{
public:
   data_seqv_reader() {}
   data_seqv_reader(seqv_ptr i_seqv) : seqv(i_seqv) {}
   ~data_seqv_reader() {}

   seqv_ptr get_data_sequence() { return seqv; }
   void set_data_sequence(seqv_ptr i_seqv) { seqv = i_seqv; }

   // single data versions
   int8_t read_int8()
   {
      int8_t sq;
      read_int8(&sq, 1, 0);

      return sq;
   }

   uint8_t read_uint8()
   {
      return (uint8_t)read_int8();
   }

   int16_t read_int16()
   {
      int8_t sq[2];
      read_int8(sq, 2, 0);

      return *(int16_t*)sq;
   }

   uint16_t read_uint16()
   {
      return (uint16_t)read_int16();
   }

   int32_t read_int32()
   {
      int8_t sq[4];
      read_int8(sq, 4, 0);

      return *(int32_t*)sq;
   }

   uint32_t read_uint32()
   {
      return (uint32_t)read_int32();
   }

   int64_t read_int64()
   {
      int8_t sq[8];
      read_int8(sq, 8, 0);

      return *(int64_t*)sq;
   }

   uint64_t read_uint64()
   {
      return (uint64_t)read_int64();
   }

   float read_fltp32()
   {
      int32_t r = read_int32();

      return *(float*)&r;
   }

   double read_fltp64()
   {
      int64_t r = read_int64();

      return *(double*)&r;
   }

   std::string read_string()
   {
      uint32_t elem_count = read_uint32();
      std::string text(elem_count, 0);
      read_int8((int8_t*)text.data(), elem_count, 0);

      return text;
   }

   std::string read_line()
   {
      std::string text;
      std::vector<char> line;
      int8_t c = 0;
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
      int8_t* p = (int8_t*)&i_seqv;
      seqv->read_bytes(p, sizeof(p), 0);
   }

   // seqv data versions. return the number of bytes read
   int read_int8(int8_t* i_seqv, int i_elem_count, int i_offset)
   {
      return seqv->read_bytes(i_seqv, i_elem_count, i_offset);
   }

   int read_uint8(uint8_t* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int8((int8_t*)i_seqv, i_elem_count, i_offset);
   }

   int read_int16(int16_t* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int8((int8_t*)i_seqv, i_elem_count * 2, i_offset * 2);
   }

   int read_uint16(uint16_t* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int16((int16_t*)i_seqv, i_elem_count, i_offset);
   }

   int read_int32(int32_t* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int8((int8_t*)i_seqv, i_elem_count * 4, i_offset * 4);
   }

   int read_uint32(uint32_t* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int32((int32_t*)i_seqv, i_elem_count, i_offset);
   }

   int read_int64(int64_t* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int8((int8_t*)i_seqv, i_elem_count * 8, i_offset * 8);
   }

   int read_uint64(uint64_t* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int64((int64_t*)i_seqv, i_elem_count, i_offset);
   }

   int read_fltp32(float* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int32((int32_t*)i_seqv, i_elem_count, i_offset);
   }

   int read_fltp64(double* i_seqv, int i_elem_count, int i_offset)
   {
      return read_int64((int64_t*)i_seqv, i_elem_count, i_offset);
   }

private:
   seqv_ptr seqv = nullptr;
};


using data_sequence_reader = data_seqv_reader<std::shared_ptr<data_seqv>>;
using data_seq_rdr_ptr = data_seqv_reader<data_seqv*>;


class data_seqv_writer
{
public:
   data_seqv_writer() {}
   data_seqv_writer(std::shared_ptr<data_seqv> i_seqv) : seqv(i_seqv) {}
   ~data_seqv_writer() {}

   std::shared_ptr<data_seqv> get_data_sequence() { return seqv; }
   void set_data_sequence(std::shared_ptr<data_seqv> i_seqv) { seqv = i_seqv; }

   // single data versions
   void write_int8(int8_t d);
   void write_uint8(uint8_t d);
   void write_int16(int16_t d);
   void write_uint16(uint16_t d);
   void write_int32(int32_t d);
   void write_uint32(uint32_t d);
   void write_int64(int64_t d);
   void write_uint64(uint64_t d);
   void write_fltp32(float d);
   void write_fltp64(double d);
   void write_string(const std::string& i_text);
   void write_line(const std::string& i_text, bool inew_line = true);
   template<class T> void write_pointer(T* const i_seqv);

   // seqv data versions
   void write_int8(const int8_t* i_seqv, int i_elem_count, int i_offset);
   void write_uint8(const uint8_t* i_seqv, int i_elem_count, int i_offset);
   void write_int16(const int16_t* i_seqv, int i_elem_count, int i_offset);
   void write_uint16(const uint16_t* i_seqv, int i_elem_count, int i_offset);
   void write_int32(const int32_t* i_seqv, int i_elem_count, int i_offset);
   void write_uint32(const uint32_t* i_seqv, int i_elem_count, int i_offset);
   void write_int64(const int64_t* i_seqv, int i_elem_count, int i_offset);
   void write_uint64(const uint64_t* i_seqv, int i_elem_count, int i_offset);
   void write_fltp32(const float* i_seqv, int i_elem_count, int i_offset);
   void write_fltp64(const double* i_seqv, int i_elem_count, int i_offset);

private:
   std::shared_ptr<data_seqv> seqv;
};


class rw_seqv : public mem_data_seqv
{
public:
   ~rw_seqv() {}

   static std::shared_ptr<rw_seqv> nwi()
   {
      std::shared_ptr<rw_seqv> inst(new rw_seqv());
      inst->r.set_data_sequence(inst);
      inst->w.set_data_sequence(inst);
      return inst;
   }

   data_sequence_reader r;
   data_seqv_writer w;

private:
   rw_seqv() {}
};


class rw_file_seqv : public file_data_seqv
{
public:
   virtual ~rw_file_seqv() {}

   static std::shared_ptr<rw_file_seqv> nwi(std::shared_ptr<mws_file> i_file, bool i_is_writable)
   {
      std::shared_ptr<rw_file_seqv> inst(new rw_file_seqv(i_file, i_is_writable));
      inst->r.set_data_sequence(inst);
      if (i_is_writable) { inst->w.set_data_sequence(inst); }
      return inst;
   }

   data_sequence_reader r;
   data_seqv_writer w;

private:
   rw_file_seqv(std::shared_ptr<mws_file> i_file, bool i_is_writable) : file_data_seqv(i_file, i_is_writable) {}
};



// implementation // implementation // implementation // implementation // implementation



inline data_seqv::data_seqv()
{
   read_position_v = 0;
   write_position_v = 0;
   total_bytes_read_v = 0;
   total_bytes_written_v = 0;
}

inline data_seqv::~data_seqv()
{
}

inline bool data_seqv::reached_end_of_sequence()
{
   return read_position() >= size();
}

inline void data_seqv::close()
{
}

inline int data_seqv::read_bytes(int8_t* i_seqv, int i_elem_count, int i_offset)
{
   int bytes_read = read_int8(i_seqv, i_elem_count, i_offset);

   read_position_v += bytes_read;
   total_bytes_read_v += bytes_read;

   return bytes_read;
}

inline void data_seqv::write_bytes(const int8_t* i_seqv, int i_elem_count, int i_offset)
{
   int bytes_written = write_int8(i_seqv, i_elem_count, i_offset);

   write_position_v += bytes_written;
   total_bytes_written_v += bytes_written;
}


inline std::shared_ptr<std::vector<uint8_t>> ro_mem_seqv::data_as_byte_vector()
{
   std::shared_ptr<std::vector<uint8_t>> sq;
   size_t sz = static_cast<size_t>(size());

   if (sz > 0)
   {
      sq = std::shared_ptr<std::vector<uint8_t>>(new std::vector<uint8_t>(sz));
      memcpy(sq->data(), seqv, sz);
   }

   return sq;
}

inline void ro_mem_seqv::set_read_position(uint64_t i_position)
{
   if (i_position >= 0 && i_position <= size())
   {
      read_position_v = i_position;
   }
}

inline void ro_mem_seqv::set_write_position(uint64_t)
{
   mws_throw mws_exception("n/a");
}

inline int ro_mem_seqv::read_int8(int8_t* i_seqv, int i_elem_count, int i_offset)
{
   int bytes_to_read = 0;

   if (i_elem_count > 0 && read_position() < size())
   {
      bytes_to_read = (int)std::min((uint64_t)i_elem_count, size() - read_position());
      memcpy(&i_seqv[i_offset], &seqv[(size_t)read_position()], bytes_to_read);
   }

   return bytes_to_read;
}

inline int ro_mem_seqv::write_int8(const int8_t*, int, int)
{
   mws_throw mws_exception("n/a");

   return -1;
}


inline mem_data_seqv::mem_data_seqv()
{
}

inline mem_data_seqv::mem_data_seqv(const uint8_t* i_seqv, int i_elem_count)
{
   seqv.resize(i_elem_count);
   memcpy(&seqv[0], i_seqv, i_elem_count);
}

inline mem_data_seqv::~mem_data_seqv()
{
}

inline const uint8_t* mem_data_seqv::data_as_byte_array()
{
   return (const uint8_t*)seqv.data();
}

inline std::shared_ptr<std::vector<uint8_t>> mem_data_seqv::data_as_byte_vector()
{
   std::shared_ptr<std::vector<uint8_t>> sq;
   size_t sz = static_cast<size_t>(size());

   if (sz > 0)
   {
      sq = std::make_shared<std::vector<uint8_t>>(sz);
      memcpy(sq->data(), seqv.data(), sz);
   }

   return sq;
}

inline void mem_data_seqv::set_read_position(uint64_t i_position)
{
   if (i_position >= 0 && i_position <= size())
   {
      read_position_v = i_position;
   }
}

inline void mem_data_seqv::set_write_position(uint64_t i_position)
{
   if (i_position >= 0 && i_position <= size())
   {
      write_position_v = i_position;
   }
}

inline int mem_data_seqv::read_int8(int8_t* i_seqv, int i_elem_count, int i_offset)
{
   int bytes_to_read = 0;

   if (i_elem_count > 0 && read_position() < size())
   {
      bytes_to_read = (int)std::min((uint64_t)i_elem_count, size() - read_position());
      memcpy(&i_seqv[i_offset], &seqv[(size_t)read_position()], bytes_to_read);
   }

   return bytes_to_read;
}

inline int mem_data_seqv::write_int8(const int8_t* i_seqv, int i_elem_count, int i_offset)
{
   if (i_elem_count > 0)
   {
      if (size() - write_position() < i_elem_count)
      {
         seqv.resize((size_t)size() + i_elem_count);
      }

      memcpy(&seqv[(size_t)write_position()], &i_seqv[i_offset], i_elem_count);
   }

   return i_elem_count;
}


inline file_data_seqv::file_data_seqv(std::shared_ptr<mws_file> i_file, bool i_is_writable)
{
   mws_assert(i_file != nullptr);
   mws_assert(i_file->is_open());
   mws_assert((i_is_writable) ? i_file->is_writable() : true);
   file = i_file;
   is_writable = i_is_writable;
}

inline bool file_data_seqv::reached_end_of_sequence()
{
   if (file_size == 0)
   {
      file_size = size();
   }

   if (write_position() > file_size)
   {
      return read_position() >= write_position();
   }

   return read_position() >= file_size;
   //return file->io.reached_eof();
}

inline void file_data_seqv::close()
{
   file->io.close();
}

inline uint64_t file_data_seqv::size()const
{
   return file->length();
}

inline void file_data_seqv::reset()
{
   rewind();
   file->io.seek(0);
}

inline void file_data_seqv::rewind()
{
   seek(0);
}

inline void file_data_seqv::seek(uint64_t i_pos)
{
   file->io.seek(i_pos);
   read_position_v = i_pos;
   write_position_v = i_pos;
}

inline int file_data_seqv::read_int8(int8_t* i_seqv, int i_elem_count, int i_offset)
{
   int bytes_read = 0;

   if (i_elem_count > 0 && read_position() < size())
   {
      bytes_read = file->io.read((uint8_t*)i_seqv, i_elem_count);
   }

   if (bytes_read != i_elem_count)
   {
      mws_throw mws_exception("reached end of file");
   }

   return bytes_read;
}

inline int file_data_seqv::write_int8(const int8_t* i_seqv, int i_elem_count, int i_offset)
{
   int bytes_written = 0;

   if (i_elem_count > 0)
   {
      bytes_written = file->io.write((uint8_t*)i_seqv, i_elem_count);
   }

   return bytes_written;
}


inline void data_seqv_writer::write_int8(int8_t i_seqv)
{
   write_int8(&i_seqv, 1, 0);
}

inline void data_seqv_writer::write_uint8(uint8_t i_seqv)
{
   write_int8(i_seqv);
}

inline void data_seqv_writer::write_int16(int16_t i_seqv)
{
   write_int8((int8_t*)&i_seqv, 2, 0);
}

inline void data_seqv_writer::write_uint16(uint16_t i_seqv)
{
   write_int16(i_seqv);
}

inline void data_seqv_writer::write_int32(int32_t i_seqv)
{
   write_int8((int8_t*)&i_seqv, 4, 0);
}

inline void data_seqv_writer::write_uint32(uint32_t i_seqv)
{
   write_int32(i_seqv);
}

inline void data_seqv_writer::write_int64(int64_t i_seqv)
{
   write_int8((int8_t*)&i_seqv, 8, 0);
}

inline void data_seqv_writer::write_uint64(uint64_t i_seqv)
{
   write_int64(i_seqv);
}

inline void data_seqv_writer::write_fltp32(float i_seqv)
{
   write_int8((int8_t*)&i_seqv, 4, 0);
}

inline void data_seqv_writer::write_fltp64(double i_seqv)
{
   write_int8((int8_t*)&i_seqv, 8, 0);
}

inline void data_seqv_writer::write_string(const std::string& i_text)
{
   write_uint32(i_text.length());
   write_int8((int8_t*)i_text.data(), i_text.length(), 0);
}

inline void data_seqv_writer::write_line(const std::string& i_text, bool inew_line)
{
   write_int8((int8_t*)&i_text[0], i_text.length(), 0);

   if (inew_line)
   {
      write_int8('\n');
   }
}

template<int> void write_pointer_helper(std::shared_ptr<data_seqv> seqv, void* i_seqv);

// 32-bit systems
template<> inline void write_pointer_helper<4>(std::shared_ptr<data_seqv> seqv, void* i_seqv)
{
   seqv->write_bytes((int8_t*)i_seqv, 4, 0);
}

// 64-bit systems
template<> inline void write_pointer_helper<8>(std::shared_ptr<data_seqv> seqv, void* i_seqv)
{
   seqv->write_bytes((int8_t*)i_seqv, 8, 0);
}

template<class T> inline void data_seqv_writer::write_pointer(T* const i_seqv)
{
   write_pointer_helper<sizeof(void*)>(seqv, (void*)&i_seqv);
}

inline void data_seqv_writer::write_int8(const int8_t* i_seqv, int i_elem_count, int i_offset)
{
   seqv->write_bytes(i_seqv, i_elem_count, i_offset);
}

inline void data_seqv_writer::write_uint8(const uint8_t* i_seqv, int i_elem_count, int i_offset)
{
   write_int8((int8_t*)i_seqv, i_elem_count, i_offset);
}

inline void data_seqv_writer::write_int16(const int16_t* i_seqv, int i_elem_count, int i_offset)
{
   write_int8((int8_t*)i_seqv, i_elem_count * 2, i_offset * 2);
}

inline void data_seqv_writer::write_uint16(const uint16_t* i_seqv, int i_elem_count, int i_offset)
{
   write_int16((int16_t*)i_seqv, i_elem_count, i_offset);
}

inline void data_seqv_writer::write_int32(const int32_t* i_seqv, int i_elem_count, int i_offset)
{
   write_int8((int8_t*)i_seqv, i_elem_count * 4, i_offset * 4);
}

inline void data_seqv_writer::write_uint32(const uint32_t* i_seqv, int i_elem_count, int i_offset)
{
   write_int32((int32_t*)i_seqv, i_elem_count, i_offset);
}

inline void data_seqv_writer::write_int64(const int64_t* i_seqv, int i_elem_count, int i_offset)
{
   write_int8((int8_t*)i_seqv, i_elem_count * 8, i_offset * 8);
}

inline void data_seqv_writer::write_uint64(const uint64_t* i_seqv, int i_elem_count, int i_offset)
{
   write_int64((int64_t*)i_seqv, i_elem_count, i_offset);
}

inline void data_seqv_writer::write_fltp32(const float* i_seqv, int i_elem_count, int i_offset)
{
   write_int32((int32_t*)i_seqv, i_elem_count, i_offset);
}

inline void data_seqv_writer::write_fltp64(const double* i_seqv, int i_elem_count, int i_offset)
{
   write_int64((int64_t*)i_seqv, i_elem_count, i_offset);
}




class data_seqv_reader_big_endian
{
public:
   data_seqv_reader_big_endian(data_seqv* i_seqv) : seqv(i_seqv) {}
   ~data_seqv_reader_big_endian() {}

   // single data versions
   int8_t read_int8();
   uint8_t read_uint8();
   int16_t read_int16();
   uint16_t read_uint16();
   int32_t read_int32();
   uint32_t read_uint32();
   int64_t read_int64();
   uint64_t read_uint64();
   float read_fltp32();
   double read_fltp64();

   // seqv data versions
   void read_int8(int8_t* i_seqv, int i_elem_count, int i_offset);
   void read_uint8(uint8_t* i_seqv, int i_elem_count, int i_offset);
   //void read_int16(int16_t* i_seqv, int i_elem_count, int i_offset);
   //void read_uint16(uint16_t* i_seqv, int i_elem_count, int i_offset);
   //void read_int32(int32_t* i_seqv, int i_elem_count, int i_offset);
   //void read_uint32(uint32_t* i_seqv, int i_elem_count, int i_offset);
   //void read_int64(int64_t* i_seqv, int i_elem_count, int i_offset);
   //void read_uint64(uint64_t* i_seqv, int i_elem_count, int i_offset);
   //void read_fltp32(float* i_seqv, int i_elem_count, int i_offset);
   //void read_fltp64(double* i_seqv, int i_elem_count, int i_offset);

private:
   data_seqv* seqv;
};


inline int8_t data_seqv_reader_big_endian::read_int8()
{
   int8_t sq;
   read_int8(&sq, 1, 0);

   return sq;
}

inline uint8_t data_seqv_reader_big_endian::read_uint8()
{
   return (uint8_t)read_int8();
}

inline int16_t data_seqv_reader_big_endian::read_int16()
{
   int8_t sq[2];
   read_int8(sq, 2, 0);
   int16_t r = ((sq[1] & 0xff) | (sq[0] & 0xff) << 8);

   return r;
}

inline uint16_t data_seqv_reader_big_endian::read_uint16()
{
   return (uint16_t)read_int16();
}

inline int32_t data_seqv_reader_big_endian::read_int32()
{
   int8_t sq[4];
   read_int8(sq, 4, 0);
   int32_t r = (sq[3] & 0xff) | ((sq[2] & 0xff) << 8) | ((sq[1] & 0xff) << 16) | ((sq[0] & 0xff) << 24);

   return r;
}

inline uint32_t data_seqv_reader_big_endian::read_uint32()
{
   return (uint32_t)read_int32();
}

inline int64_t data_seqv_reader_big_endian::read_int64()
{
   int8_t sq[8];
   read_int8(sq, 8, 0);
   int64_t r = ((int64_t)(sq[7] & 0xff) | ((int64_t)(sq[6] & 0xff) << 8) | ((int64_t)(sq[5] & 0xff) << 16) | ((int64_t)(sq[4] & 0xff) << 24) |
      ((int64_t)(sq[3] & 0xff) << 32) | ((int64_t)(sq[2] & 0xff) << 40) | ((int64_t)(sq[1] & 0xff) << 48) | ((int64_t)(sq[0] & 0xff) << 56));

   return r;
}

inline uint64_t data_seqv_reader_big_endian::read_uint64()
{
   return (uint64_t)read_int64();
}

inline float data_seqv_reader_big_endian::read_fltp32()
{
   int32_t r = read_int32();

   return *(float*)&r;
}

inline double data_seqv_reader_big_endian::read_fltp64()
{
   int64_t r = read_int64();

   return *(double*)&r;
}

inline void data_seqv_reader_big_endian::read_int8(int8_t* i_seqv, int i_elem_count, int i_offset)
{
   seqv->read_bytes(i_seqv, i_elem_count, i_offset);
}

inline void data_seqv_reader_big_endian::read_uint8(uint8_t* i_seqv, int i_elem_count, int i_offset)
{
   read_int8((int8_t*)i_seqv, i_elem_count, i_offset);
}