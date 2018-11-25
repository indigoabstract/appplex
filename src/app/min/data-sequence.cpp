#include "stdafx.h"

#include "data-sequence.hpp"
#include "min.hpp"


data_sequence::data_sequence()
{
   read_position = 0;
   write_position = 0;
   total_bytes_read = 0;
   total_bytes_written = 0;
}

data_sequence::~data_sequence()
{
}

bool data_sequence::reached_end_of_sequence()
{
   uint64 read_pos = get_read_position();
   uint64 size = get_size();

   return read_pos >= size;
}

void data_sequence::close()
{
}

int data_sequence::read_bytes(int8* s, int elem_count, int offset)
{
   int bytes_read = read_int8(s, elem_count, offset);

   read_position += bytes_read;
   total_bytes_read += bytes_read;

   return bytes_read;
}

void data_sequence::write_bytes(const int8* s, int elem_count, int offset)
{
   int bytes_written = write_int8(s, elem_count, offset);

   write_position += bytes_written;
   total_bytes_written += bytes_written;
}


memory_data_sequence::memory_data_sequence()
{
}

memory_data_sequence::memory_data_sequence(const uint8* s, int elem_count)
{
   sequence.resize(elem_count);
   memcpy(&sequence[0], s, elem_count);
}

memory_data_sequence::~memory_data_sequence()
{
}

const uint8* memory_data_sequence::get_data_as_byte_array()
{
   return (const uint8*)begin_ptr(sequence);
}

shared_ptr<std::vector<uint8> > memory_data_sequence::get_data_as_byte_vector()
{
   shared_ptr<std::vector<uint8> > s;

   if (get_size() > 0)
   {
      s = shared_ptr<std::vector<uint8> >(new std::vector<uint8>(get_size()));
      memcpy(begin_ptr(s), begin_ptr(sequence), get_size());
   }

   return s;
}

void memory_data_sequence::set_read_position(uint64 position)
{
   if (position >= 0 && position <= get_size())
   {
      read_position = position;
   }
}

void memory_data_sequence::set_write_position(uint64 position)
{
   if (position >= 0 && position <= get_size())
   {
      write_position = position;
   }
}

int memory_data_sequence::read_int8(int8* s, int elem_count, int offset)
{
   int bytesToRead = 0;

   if (elem_count > 0 && get_read_position() < get_size())
   {
      bytesToRead = std::min((uint64)elem_count, get_size() - get_read_position());
      memcpy(&s[offset], &sequence[get_read_position()], bytesToRead);
   }

   return bytesToRead;
}

int memory_data_sequence::write_int8(const int8* s, int elem_count, int offset)
{
   if (elem_count > 0)
   {
      if (get_size() - get_write_position() < elem_count)
      {
         sequence.resize(get_size() + elem_count);
      }

      memcpy(&sequence[get_write_position()], &s[offset], elem_count);
   }

   return elem_count;
}


file_data_sequence::file_data_sequence(shared_ptr<pfm_file> ifile)
{
   file = ifile;
}

file_data_sequence::~file_data_sequence()
{
}

bool file_data_sequence::reached_end_of_sequence()
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

void file_data_sequence::close()
{
   file->io.close();
}

uint64 file_data_sequence::get_size()const
{
   return file->length();
}

void file_data_sequence::reset()
{
   rewind();
   file->io.seek(0);
}

void file_data_sequence::rewind()
{
   seek(0);
}

void file_data_sequence::seek(uint64 i_pos)
{
   file->io.seek(i_pos);
   read_position = i_pos;
   write_position = i_pos;
}

int file_data_sequence::read_int8(int8* s, int elem_count, int offset)
{
   int bytes_read = 0;

   if (elem_count > 0 && get_read_position() < get_size())
   {
      bytes_read = file->io.read((uint8*)s, elem_count);
   }

   if (bytes_read != elem_count)
   {
      mws_throw ia_exception("reached end of file");
   }

   return bytes_read;
}

int file_data_sequence::write_int8(const int8* s, int elem_count, int offset)
{
   int bytes_written = 0;

   if (elem_count > 0)
   {
      bytes_written = file->io.write((uint8*)s, elem_count);
   }

   return bytes_written;
}
