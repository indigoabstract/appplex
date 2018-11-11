/*
 *  gap_buffer.h
 *
 *  Author: Hsin Tsao (stsao@lazyhacker.com)
 *  Version: 1.0 (June 12, 2003)
 *
 *  A text buffer class using the buffer-gap technique for managing
 *  the text stored in the buffer.
 *
 *  Portions of this work derived from Joseph Allen's usenet
 *  postings on comp.editor that was released to the public
 *  domain.
 *
 *
 *  There are no restrictions on the use of this code other
 *  than to include my name in any derived work.  There are
 *  no warranty for this obviously, but you are welcomed
 *  to modify, correct, or adapt the code to your needs.  The
 *  author appreciates if you are willing to submit corrections
 *  or suggestions for improvement.
 *  
 *
 *  http://www.lazyhacker.com
 */

#include "appplex-conf.hpp"

#ifdef UNIT_TEXXED

#include "pfm.hpp"
#include "com/mws/mws-com.hpp"
#include <stdio.h>
#include <string>


class text_area_model_ro : public text_area_model
{
public:
   text_area_model_ro();
   bool get_word_wrap() override;
   void set_word_wrap(bool i_word_wrap) override;
   int get_line_count() override;
   std::string get_line_at(int i_idx, bool i_keep_line_break = true) override;
   std::vector<std::string> get_lines_at(int i_idx, int i_line_count, bool i_keep_line_break = true) override;
   void push_back(const char* i_text, int i_length) override;
   void push_front(const char* i_text, int i_length) override;
   void set_text(const std::string& i_text) override;
   void set_text(const char* i_text, int i_length) override;
   void set_size(int i_width, int i_height) override;
   void set_font(mws_sp<mws_font> i_font) override;
   int get_char_at_pixel(float i_x, float i_y) override;

private:
   void update_back_added_line_offsets(const std::string& i_new_text);
   void update_front_added_line_offsets(const std::string& i_new_text);
   void update_line_offsets();

   std::string text;
   bool word_wrap = false;
   std::vector<uint32> line_offsets;
   int width = 0;
   int height = 0;
   mws_sp<mws_font> font;
};


class text_area_model_rw : public text_area_model
{
public:
   bool get_word_wrap() override;
   void set_word_wrap(bool i_word_wrap) override;
   int get_line_count() override;
   std::string get_line_at(int i_idx, bool i_keep_line_break = true) override;
   std::vector<std::string> get_lines_at(int i_idx, int i_line_count, bool i_keep_line_break = true) override;
   void push_back(const char* i_text, int i_length) override;
   void set_text(const std::string& i_text) override;
   void set_text(const char* i_text, int i_length) override;
   void set_size(int i_width, int i_height) override;
   void set_font(mws_sp<mws_font> i_font) override;
   int get_char_at_pixel(float i_x, float i_y) override;
};


class GapBuffer : public text_area_model
{

    char *point;                    // location pointer into buffer
    char *buffer;                   // start of text buffer
    char *bufend;                   // first location outside buffer
    char *gapstart;                 // start of gap
    char *gapend;                   // first location after end of gap
	// offset into 'buffer' for each line
	std::vector<uint32> nl_offsets;
    
    unsigned int GAP_SIZE;          // expand GAP by this value

    int InitBuffer(unsigned int size);

    /*
     * Copy the characters from one location to another.  We have
     * to write our own instead of using memcopy because we are
     * working within a single linear buffer and thus can have
     * overlap between the source and destination.
     */
    int CopyBytes(char *destination, char *source, unsigned int length);

    /*
     *  Expand the size of the buffer.
     */
    void ExpandBuffer(unsigned int size);

    /*
     *  Expand the size of the gap.
     */
    void ExpandGap(unsigned int size);
    
public:

    static const int DEFAULT_GAP_SIZE=20;

    /* Constructor with default gap size. */
    GapBuffer(int gsize=DEFAULT_GAP_SIZE);

    /* Constructor with instantiating with an existing file. */
    GapBuffer(FILE *file, int gsize=DEFAULT_GAP_SIZE);

	/* Constructor with instantiating from a std::string. */
	GapBuffer(const std::string& itext, int gsize = DEFAULT_GAP_SIZE);

	/* Copy constructor to deal with our pointer members. */
    GapBuffer(const GapBuffer& tb);

    ~GapBuffer();

	virtual int get_line_count();
	virtual std::string get_line_at(int iidx, bool i_keep_line_break = true);
	virtual std::vector<std::string> get_lines_at(int iidx, int iline_count, bool i_keep_line_break = true);
   virtual void push_back(const char* i_text, int i_length) override;

	/*
     *  Returns the size of the buffer minus the gap.
     */
    int BufferSize();

    /*
     *  Move the gap to the current position of the point.
     */
    void MoveGapToPoint();

    /*
     *  Set point to offset from start of buffer.
     */
    void SetPoint(unsigned int offset);

    /*
     *  Returns the current size of the gap.
     */
    int SizeOfGap();    

    /*
     *  Returns offset from point to start of buffer.
     */
    unsigned int PointOffset();

    /*
     * Return character that point is pointing to.
     * If point is inside the gap, then return the
     * the first character outside the gap.
     */
    char GetChar();

    /*
     * Return the previous character and
     * move point back one position.
     */
    char PreviousChar();

    /*
     *  Replace the character of point. Does
     *  not move the gap.
     */
    void ReplaceChar(char ch);

    /*
     *  Get the next character and increment point.
     */
    char NextChar();

    /*
     *  Inserts a character at point location
     *  and advance the point.
     */
    void PutChar(char ch);

    /*
     *  Insert character at point position, but
     *  does NOT advance the point.
     */
    void InsertChar(char ch);

    /*
     *  Delete "size" number of characters.
     */
    void DeleteChars(unsigned int size);

    /*
     *  Inserts a length size string 
     *  at point.
     */
    void InsertString(const char *string, unsigned int length);

    /*
     *  Prints out the current buffer from start
     *  to end.
     */
    void PrintBuffer();

    /*
     * Saves to file the number of bytes starting from
     * the point.
     */
    int SaveBufferToFile(FILE *file, unsigned int bytes);

    
    virtual bool get_word_wrap() { return false; }
    virtual void set_word_wrap(bool i_word_wrap) {}
    void set_text(const std::string& i_text) override {}
    virtual void set_text(const char* i_text, int i_length) {}
    virtual void set_size(int i_width, int i_height) {}
    virtual void set_font(mws_sp<mws_font> i_font) {}
    virtual int get_char_at_pixel(float i_x, float i_y) { return 0; }
};

#endif
