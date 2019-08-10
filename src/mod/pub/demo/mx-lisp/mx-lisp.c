#include "stdafx.hxx"

// this means running the bare bones console aplication
//#define STANDALONE

#ifdef STANDALONE

//#define MMIX 1
#define bool int
#define false 0
#define true 1
#define mws_assert(cond)
#define mws_signal_error()
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed long long int64;
typedef unsigned long long uint64;

#else

#include "pfm-def.h"
#include <stdlib.h>
#include <string.h>

#endif

#include <stdio.h>


// Implementarea unui interpretor pentru limbajul LISP (pentru o masina MMIX)
typedef int64 numeric_dt;
struct mx_elem;
struct mx_env;

typedef enum
{
   e_invalid,
   e_quote,
   e_if,
   e_setq,
   e_define,
   e_lambda,
   e_begin,
   e_set_vkb_enabled,
   e_set_scr_brightness,
   e_use_sq_brackets_as_parenths,

   e_exit,
   e_clear,
   e_help,
   e_list_help,
   e_list_env,
   e_keywords_help,
   e_test_help,
   e_test_mem,
   e_test_text,
   e_test_vect,
   e_test_list,
   e_test_htable,
   e_test_lisp,
   e_test_all,
   e_status_help,
   e_status_memory,
   e_status_objects,
}
eval_fnct_type;
typedef struct
{
   const char* name;
   eval_fnct_type id;
}
mx_eval_fnct_id;
static mx_eval_fnct_id eval_fncts_list[] =
{
   {"quote", e_quote},
   {"if", e_if},
   {"set!", e_setq},
   {"define", e_define},
   {"lambda", e_lambda},
   {"begin", e_begin},
   {"mx-set-vkb-enabled", e_set_vkb_enabled},
   {"mx-set-scr-brightness", e_set_scr_brightness},
   {"mx-use-sq-brackets-as-parenths", e_use_sq_brackets_as_parenths},

   {"exit", e_exit},
   {"clear", e_clear},
   {"help", e_help},
   {"list-env", e_list_env},
   {"list-help", e_list_help},
   {"keywords-help", e_keywords_help},
   {"test-help", e_test_help},
   {"test-mem", e_test_mem},
   {"test-text", e_test_text},
   {"test-vect", e_test_vect},
   {"test-list", e_test_list},
   {"test-htable", e_test_htable},
   {"test-lisp", e_test_lisp},
   {"test-all", e_test_all},
   {"status-help", e_status_help},
   {"status-memory", e_status_memory},
   {"status-objects", e_status_objects},
};
const int eval_fncts_list_size = sizeof(eval_fncts_list) / sizeof(mx_eval_fnct_id);
static eval_fnct_type mx_fnct_id_by_name(const char* i_name);
static void (*outputf)(const char* i_text) = 0;


struct mx_mem_block
{
   uint16 type;
   uint16 elem_span;
   struct mx_mem_block* next;
};


//
// global data
//

// maximum 65535 memory partitions
#define MAX_ELEM_COUNT 65535
#define MIN_ELEM_SIZE 16
#define MX_CTX_ELEM_COUNT 16000
#define MX_CTX_ELEM_SIZE 128
#define GLOBAL_INPUT_LENGTH 1024

static const uint16 FREE_BLOCK = 0;
static const uint16 FULL_BLOCK = 1;
static const uint16 INVL_BLOCK = 2;

#ifdef STANDALONE

static char heap_mem[MX_CTX_ELEM_SIZE * MAX_ELEM_COUNT] = { 0 };
// sizeof(mx_mem_block) = 2 + 2 + 4
static char block_list_mem[sizeof(struct mx_mem_block) * MAX_ELEM_COUNT] = { 0 };
static char global_input[GLOBAL_INPUT_LENGTH] = { 0 };

void mx_lisp_set_vkb_enabled(int i_enabled) {}
float mx_lisp_set_scr_brightness(int i_brightness) { return 0.f; }
void mx_lisp_use_sq_brackets_as_parenths(int i_enabled) {}

#else

static char* heap_mem = 0;
static char* block_list_mem = 0;
static char* global_input = 0;
void mx_lisp_set_vkb_enabled(int i_enabled);
float mx_lisp_set_scr_brightness(int i_brightness);
void mx_lisp_use_sq_brackets_as_parenths(int i_enabled);

#endif

static char* global_output = 0;
static bool global_done = false;
static struct mx_elem* false_sym = 0;
static struct mx_elem* true_sym = 0;
static struct mx_elem* nil_sym = 0;
static struct mx_env* global_env = 0;
static struct mx_env* test_env = 0;

static int executed_test_count = 0;      // count of number of unit tests executed
static int failed_test_count = 0;     // count of number of unit tests that fail
static int elem_created = 0;
static int elem_destroyed = 0;
static int env_created = 0;
static int env_destroyed = 0;

int size_of_address() { return sizeof(uint64*); }


struct mx_mem_ctx
   // size always refers to the size in bytes, unless noted otherwise
{
   // this is the minimum memory partition size
   int elem_size;
   // total number of partitions
   int elem_count;
   int total_size;
   int avg_request_size;
   int avg_block_size;
   char* mem_start;
   struct mx_mem_block* block_list;
   int block_list_length;
};

static struct mx_mem_ctx mxmc_inst = { 0 };



//
// memory management functions
//

void mx_mem_init_ctx();
void mx_mem_destroy_ctx();
void* mx_mem_alloc(int i_size_in_bytes);
void* mx_mem_realloc(const void* i_address, int i_size_in_bytes);
void mx_mem_free(const void* i_address);
bool mx_mem_is_valid_address(const void* i_address);
void mx_mem_copy(const void* i_src, void* i_dst, int i_size);
void mx_mem_clear(void* i_dst, int i_size);



//
// reference counting functions
//

typedef struct
{
   void(*destructor_ptr)(void* i_obj_ptr);
   int ref_count;
}
mx_smart_ptr;

void check_obj_status(const void* i_obj_ptr);
void check_ref_count(const void* i_obj_ptr);
void inc_ref_count(const void* i_obj_ptr);
void dec_ref_count(const void* i_obj_ptr);
void reset_ref_count(const void* i_obj_ptr);
void assign_smart_ptr(void** iobj_ptr_left, const void* iobj_ptr_right);



//
// input/output functions
//

// error signaling function
void mx_signal_error(const char* i_error_msg);
// read a text line from the console into 'global_input'
void mx_read_text_line();
void mx_print_text(const char* i_text);
void mx_print_indent(int i_level);



//
// testing functions
//

void test_mx_mem();
void test_mx_text();
void test_mx_vect();
void test_mx_list();
void test_mx_htable();
void test_mx_lisp(struct mx_env* i_env);
void test_all(struct mx_env* i_env);



//
// byte string functions
//

int mx_char_length(const char* i_text);
char* mx_char_copy_abc(const char* i_src, char* i_dst, int i_max_length);
char* mx_char_copy(const char* i_src, char* i_dst);
char* mx_char_clone_ab(const char* i_src, int i_max_length);
char* mx_char_clone(const char* i_src);
int mx_char_compare(const char* i_text_left, const char* i_text_right);
char* mx_char_append(const char* i_text, const char* i_append);
// reverse string i_s in place
void mx_char_reverse(char* i_s);



//
// mx_text functions
//

typedef struct
{
   void(*destructor_ptr)(void* i_obj_ptr);
   int ref_count;

   char* text;
}
mx_text;

int size_of_text();
mx_text* mx_text_ctor_ab(const char* i_text, int i_max_length);
mx_text* mx_text_ctor(const char* i_text);
void mx_text_dtor(void* i_text);
int mx_text_length(const mx_text* i_text);
char mx_text_char_at(const mx_text* i_text, int iidx);
mx_text* mx_text_erase(const mx_text* i_text, int ipos);
mx_text* mx_text_append(const mx_text* i_text, const mx_text* i_append);
mx_text* mx_text_append_string(const mx_text* i_text, const char* i_append);
mx_text* mx_text_append_string_string(const char* i_text, const char* i_append);
int mx_text_compare(const mx_text* i_text_left, const mx_text* i_text_right);
int mx_text_compare_string(const mx_text* i_text_left, const char* i_text_right);



//
// utility functions
//

mx_text* number_to_text(numeric_dt i_nr);
numeric_dt text_to_number(const mx_text* i_n);
bool is_digit(char i_c);



//
// mx_vect functions
//

typedef struct
{
   void(*destructor_ptr)(void* i_obj_ptr);
   int ref_count;

   int capacity_increment;
   int elem_count;
   mx_smart_ptr** elem_vect;
   int elem_vect_length;
}
mx_vect;

int size_of_mx_vect();
mx_vect* mx_vect_ctor(int i_initial_capacity, int i_capacity_increment);
void mx_vect_dtor(void* i_v);
mx_smart_ptr* mx_vect_front(mx_vect* i_v);
mx_smart_ptr* mx_vect_elem_at(const mx_vect* i_v, int i_index);
int mx_vect_size(const mx_vect* i_v);
bool mx_vect_is_empty(mx_vect* i_v);
void mx_vect_add_elem(mx_vect* i_v, mx_smart_ptr* i_elem);
void mx_vect_del_all_elem(mx_vect* i_v);
void mx_vect_del_elem_at(mx_vect* i_v, int i_index);
void mx_vect_pop_front(mx_vect* i_v);

static mx_vect* global_env_vect = 0;



//
// mx_htable functions
//

struct mx_ht_entry;
typedef struct
{
   void(*destructor_ptr)(void* i_obj_ptr);
   int ref_count;

   struct mx_ht_entry** entries;
   int capacity;
}
mx_htable;

int size_of_mx_htable();
mx_htable* mx_htable_ctor(int i_capacity);
void mx_htable_dtor(void* i_ht);
const void* mx_htable_put(mx_htable* i_ht, const mx_text* i_key, const void* i_value);
const void* mx_htable_get(mx_htable* i_ht, const mx_text* i_key);
bool mx_htable_del(mx_htable* i_ht, const mx_text* i_key);



//
// mx_elem functions
//

typedef enum
{
   t_symbol,
   t_number,
   t_list,
   t_function,
   t_lambda,
}
elem_type;

struct mx_env;
typedef struct mx_elem* (*fun_type)(const mx_vect*);

// a variant that can hold any kind of lisp value
struct mx_elem
{
   void(*destructor_ptr)(void* i_obj_ptr);
   int ref_count;

   elem_type type;
   mx_text* val;
   mx_vect* list;
   fun_type fun;
   struct mx_env* i_env;
};

int size_of_mx_elem()
{
   return sizeof(struct mx_elem);
}

mx_vect* elem_list_copy(const mx_vect* i_src);

char* get_elem_type(const struct mx_elem* i_c)
{
   if (i_c != 0)
   {
      elem_type t = i_c->type;

      if (i_c->type == t_function)
      {
         return "function elem";
      }
      else if (i_c->type == t_lambda)
      {
         return "lambda elem";
      }
      else if (i_c->type == t_list)
      {
         return "list elem";
      }
      else if (i_c->type == t_number)
      {
         return "number elem";
      }
      else if (i_c->type == t_symbol)
      {
         return "symbol elem";
      }
   }

   return "elem type is unavailable";
}

void mx_elem_dtor(void* i_c)
{
   if (!i_c)
   {
      return;
   }

   struct mx_elem* c = (struct mx_elem*)i_c;

   c->destructor_ptr = 0;
   c->fun = 0;

   assign_smart_ptr(&c->val, 0);
   assign_smart_ptr(&c->list, 0);
   assign_smart_ptr(&c->i_env, 0);

   mx_mem_free(c);
   elem_destroyed++;
}

struct mx_elem* mx_elem_ctor(elem_type i_type)
{
   struct mx_elem* i_c = (struct mx_elem*)mx_mem_alloc(size_of_mx_elem());

   i_c->destructor_ptr = mx_elem_dtor;

   i_c->type = i_type;
   i_c->val = 0;
   // member ptr
   i_c->list = 0;
   assign_smart_ptr(&i_c->list, mx_vect_ctor(1, 1));
   i_c->fun = 0;
   i_c->i_env = 0;
   elem_created++;

   return i_c;
}

struct mx_elem* mx_elem_ctor_ab(elem_type i_type, mx_text* i_val)
{
   struct mx_elem* c = (struct mx_elem*)mx_mem_alloc(size_of_mx_elem());

   c->destructor_ptr = mx_elem_dtor;
   c->ref_count = 0;

   c->type = i_type;
   // member ptr
   c->val = 0;
   assign_smart_ptr(&c->val, i_val);
   // member ptr
   c->list = 0;
   assign_smart_ptr(&c->list, mx_vect_ctor(1, 1));
   c->fun = 0;
   c->i_env = 0;
   elem_created++;

   return c;
}

struct mx_elem* mx_elem_ctor_ab2(elem_type i_type, const char* i_val)
{
   struct mx_elem* c = (struct mx_elem*)mx_mem_alloc(size_of_mx_elem());

   c->destructor_ptr = mx_elem_dtor;
   c->ref_count = 0;

   c->type = i_type;
   // member ptr
   c->val = 0;
   assign_smart_ptr(&c->val, mx_text_ctor(i_val));
   // member ptr
   c->list = 0;
   assign_smart_ptr(&c->list, mx_vect_ctor(1, 1));
   c->fun = 0;
   c->i_env = 0;
   elem_created++;

   return c;
}

struct mx_elem* mx_elem_ctor_a2(fun_type i_fun)
{
   struct mx_elem* c = (struct mx_elem*)mx_mem_alloc(size_of_mx_elem());

   c->destructor_ptr = mx_elem_dtor;
   c->ref_count = 0;

   c->type = t_function;
   c->val = 0;
   // member ptr
   c->list = 0;
   assign_smart_ptr(&c->list, mx_vect_ctor(1, 1));
   c->fun = i_fun;
   c->i_env = 0;
   elem_created++;

   return c;
}

struct mx_elem* elem_copy(const struct mx_elem* i_src)
{
   // return ptr
   struct mx_elem* copy = 0;

   if (i_src->val)
   {
      copy = mx_elem_ctor_ab2(i_src->type, i_src->val->text);
   }
   else
   {
      copy = mx_elem_ctor(i_src->type);
   }

   // member ptr
   assign_smart_ptr(&copy->list, elem_list_copy(i_src->list));
   copy->fun = i_src->fun;
   assign_smart_ptr(&copy->i_env, i_src->i_env);

   return copy;
}

mx_vect* elem_list_copy(const mx_vect* i_src)
{
   // return ptr
   mx_vect* copy = mx_vect_ctor(1, 1);
   int length = mx_vect_size(i_src);

   for (int k = 0; k < length; k++)
   {
      // local ptr
      struct mx_elem* i_c = (struct mx_elem*)mx_vect_elem_at(i_src, k);
      // local ptr
      struct mx_elem* cc = elem_copy(i_c);

      mx_vect_add_elem(copy, (mx_smart_ptr*)cc);
   }

   return copy;
}

void mx_elem_dbg_list_impl(const struct mx_elem* i_c, int i_level)
{
   mx_print_indent(i_level);

   switch (i_c->type)
   {
   case t_symbol:
      if (i_c->val)
      {
         mx_print_text("tsym: ");
         mx_print_text(i_c->val->text);
      }
      break;

   case t_number:
      if (i_c->val)
      {
         mx_print_text("tnum: ");
         mx_print_text(i_c->val->text);
      }
      break;

   case t_list:
      if (i_c->list)
      {
         int length = mx_vect_size(i_c->list);

         mx_print_text("tlist:\n");

         for (int k = 0; k < length; k++)
         {
            // local ptr
            const struct mx_elem* cc = (struct mx_elem*)mx_vect_elem_at(i_c->list, k);

            mx_elem_dbg_list_impl(cc, i_level + 1);
         }
      }
      break;

   case t_function:
      mx_print_text("tfun: ");
      break;

   case t_lambda:
   {
      int length = mx_vect_size(i_c->list);

      mx_print_text("tlambda:\n");

      for (int k = 0; k < length; k++)
      {
         // local ptr
         const struct mx_elem* cc = (struct mx_elem*)mx_vect_elem_at(i_c->list, k);

         mx_elem_dbg_list_impl(cc, i_level + 1);
      }
      break;
   }
   }

   mx_print_text("\n");
}

void mx_elem_dbg_list(const struct mx_elem* i_c)
{
   if (!i_c)
   {
      return;
   }

   mx_elem_dbg_list_impl(i_c, 0);
}



//
// mx_env functions
//


// a dictionary that (a) associates symbols with elems, and
// (b) can chain to an "outer" dictionary
struct mx_env
{
   void(*destructor_ptr)(void* i_obj_ptr);
   int ref_count;

   mx_htable* ht_env; // inner symbol->elem mapping
   struct mx_env* ext_env; // next adjacent outer i_env, or 0 if there are no further environments
};

int size_of_mx_env()
{
   return sizeof(struct mx_env);
}

void mx_env_dbg_list(const struct mx_env* i_e);

void mx_env_dtor(void* i_e)
{
   if (i_e == 0)
   {
      return;
   }

   struct mx_env* e = (struct mx_env*)i_e;

   // member ptr
   assign_smart_ptr(&e->ht_env, 0);
   // member ptr
   assign_smart_ptr(&e->ext_env, 0);
   mx_mem_free(e);
   env_destroyed++;
}

struct mx_env* mx_env_ctor(struct mx_env* i_ext_env)
{
   struct mx_env* e = (struct mx_env*)mx_mem_alloc(size_of_mx_env());

   e->destructor_ptr = mx_env_dtor;
   e->ref_count = 0;

   e->ht_env = 0;
   assign_smart_ptr(&e->ht_env, mx_htable_ctor(10));
   e->ext_env = 0;
   // member ptr
   assign_smart_ptr(&e->ext_env, i_ext_env);
   env_created++;

   return e;
}

struct mx_env* mx_env_ctor_abc(const mx_vect* i_parms, const mx_vect* i_args, struct mx_env* i_ext_env)
{
   struct mx_env* e = (struct mx_env*)mx_mem_alloc(size_of_mx_env());

   e->destructor_ptr = mx_env_dtor;
   e->ref_count = 0;

   e->ht_env = 0;
   assign_smart_ptr(&e->ht_env, mx_htable_ctor(10));
   e->ext_env = 0;
   // member ptr
   assign_smart_ptr(&e->ext_env, i_ext_env);

   int length = mx_vect_size(i_parms);

   for (int k = 0; k < length; k++)
   {
      // local ptr
      struct mx_elem* i_c = (struct mx_elem*)mx_vect_elem_at(i_parms, k);
      // local ptr
      struct mx_elem* c2 = (struct mx_elem*)mx_vect_elem_at(i_args, k);

      mx_htable_put(e->ht_env, i_c->val, c2);
   }

   env_created++;

   return e;
}

// return a reference to the innermost environment where 'i_var' appears
struct mx_elem* mx_env_get(struct mx_env* i_e, const mx_text* i_var)
{
   // return ptr
   struct mx_elem* cr = (struct mx_elem*)mx_htable_get(i_e->ht_env, i_var);

   if (cr)
      // the symbol exists in this environment
   {
      return cr;
   }

   if (i_e->ext_env)
   {
      return mx_env_get(i_e->ext_env, i_var); // attempt to find the symbol in some "outer" i_env
   }

   mx_text* i_tx = 0;
   assign_smart_ptr(&i_tx, mx_text_append_string_string("unbound symbol ", i_var->text));
   mx_char_copy_abc(i_tx->text, global_input, GLOBAL_INPUT_LENGTH);
   assign_smart_ptr(&i_tx, 0);

   // return ptr
   return mx_elem_ctor_ab2(t_symbol, global_input);
}

// return a reference to the elem associated with the given symbol 'i_var'
void mx_env_put(struct mx_env* i_e, const mx_text* i_var, const struct mx_elem* i_c)
{
   mx_htable_put(i_e->ht_env, i_var, i_c);
}


//
// built-in lisp functions
//

struct mx_elem* fun_append(const mx_vect* i_c)
{
   // return ptr
   struct mx_elem* result = mx_elem_ctor(t_list);
   // local ptr
   struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(i_c, 0);
   struct mx_elem* c1 = (struct mx_elem*)mx_vect_elem_at(i_c, 1);
   mx_vect* ls = c1->list;
   int length = mx_vect_size(ls);

   // member ptr
   assign_smart_ptr(&result->list, elem_list_copy(c0->list));

   for (int k = 0; k < length; k++)
   {
      // local ptr
      struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(c1->list, k);
      struct mx_elem* cc = elem_copy(ck);

      mx_vect_add_elem(result->list, (mx_smart_ptr*)cc);
   }

   return result;
}

struct mx_elem* fun_car(const mx_vect* i_c)
{
   // local ptr
   struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(i_c, 0);
   mx_vect* ls = c0->list;

   if (mx_vect_is_empty(ls))
   {
      // return ptr
      return nil_sym;
   }

   // return ptr
   struct mx_elem* cr = (struct mx_elem*)mx_vect_elem_at(ls, 0);

   return cr;
}

struct mx_elem* fun_cdr(const mx_vect* i_c)
{
   // local ptr
   struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(i_c, 0);
   mx_vect* ls = c0->list;
   int length = mx_vect_size(ls);

   if (length < 2)
   {
      // return ptr
      return nil_sym;
   }

   // return ptr
   struct mx_elem* result = elem_copy(c0);
   mx_vect_del_elem_at(result->list, 0);

   return result;
}

struct mx_elem* fun_cons(const mx_vect* i_c)
{
   // return ptr
   struct mx_elem* result = mx_elem_ctor(t_list);
   // local ptr
   struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(i_c, 0);
   struct mx_elem* c1 = (struct mx_elem*)mx_vect_elem_at(i_c, 1);
   mx_vect* ls = c1->list;
   int length = mx_vect_size(ls);

   mx_vect_add_elem(result->list, (mx_smart_ptr*)c0);

   for (int k = 0; k < length; k++)
   {
      // local ptr
      struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(c1->list, k);
      struct mx_elem* cc = elem_copy(ck);

      mx_vect_add_elem(result->list, (mx_smart_ptr*)cc);
   }

   return result;
}

struct mx_elem* fun_div(const mx_vect* i_c)
{
   // local ptr
   struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(i_c, 0);
   numeric_dt n = text_to_number(c0->val);
   int length = mx_vect_size(i_c);

   for (int k = 1; k < length; k++)
   {
      // local ptr
      struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(i_c, k);

      n = n / text_to_number(ck->val);
   }

   // return ptr
   return mx_elem_ctor_ab(t_number, number_to_text(n));
}

struct mx_elem* fun_greater(const mx_vect* i_c)
{
   // local ptr
   struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(i_c, 0);
   numeric_dt n = text_to_number(c0->val);
   int length = mx_vect_size(i_c);

   for (int k = 1; k < length; k++)
   {
      // local ptr
      struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(i_c, k);

      if (n <= text_to_number(ck->val))
      {
         // return ptr
         return false_sym;
      }
   }

   // return ptr
   return true_sym;
}

struct mx_elem* fun_length(const mx_vect* i_c)
{
   // local ptr
   struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(i_c, 0);
   mx_vect* ls = c0->list;
   int length = mx_vect_size(ls);

   // return ptr
   return mx_elem_ctor_ab(t_number, number_to_text(length));
}

struct mx_elem* fun_less(const mx_vect* i_c)
{
   // local ptr
   struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(i_c, 0);
   numeric_dt n = text_to_number(c0->val);
   int length = mx_vect_size(i_c);

   for (int k = 1; k < length; k++)
   {
      // local ptr
      struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(i_c, k);

      if (n >= text_to_number(ck->val))
      {
         // return ptr
         return false_sym;
      }
   }

   // return ptr
   return true_sym;
}

struct mx_elem* fun_less_equal(const mx_vect* i_c)
{
   // local ptr
   struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(i_c, 0);
   numeric_dt n = text_to_number(c0->val);
   int length = mx_vect_size(i_c);

   for (int k = 1; k < length; k++)
   {
      // local ptr
      struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(i_c, k);

      if (n > text_to_number(ck->val))
      {
         // return ptr
         return false_sym;
      }
   }

   // return ptr
   return true_sym;
}

struct mx_elem* fun_list(const mx_vect* i_c)
{
   // return ptr
   struct mx_elem* result = mx_elem_ctor(t_list);

   // member ptr
   assign_smart_ptr(&result->list, elem_list_copy(i_c));

   return result;
}

struct mx_elem* fun_mul(const mx_vect* i_c)
{
   numeric_dt n = 1;
   int length = mx_vect_size(i_c);

   for (int k = 0; k < length; k++)
   {
      // local ptr
      struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(i_c, k);

      n = n * text_to_number(ck->val);
   }

   // return ptr
   return mx_elem_ctor_ab(t_number, number_to_text(n));
}

struct mx_elem* fun_nullp(const mx_vect* i_c)
{
   // local ptr
   struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(i_c, 0);
   mx_vect* ls = c0->list;

   if (mx_vect_is_empty(ls))
   {
      // return ptr
      return true_sym;
   }

   // return ptr
   return false_sym;
}

struct mx_elem* fun_add(const mx_vect* i_c)
{
   // local ptr
   struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(i_c, 0);
   numeric_dt n = text_to_number(c0->val);
   int length = mx_vect_size(i_c);

   for (int k = 1; k < length; k++)
   {
      // local ptr
      struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(i_c, k);

      n = n + text_to_number(ck->val);
   }

   // return ptr
   return mx_elem_ctor_ab(t_number, number_to_text(n));
}

struct mx_elem* fun_sub(const mx_vect* i_c)
{
   // local ptr
   struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(i_c, 0);
   numeric_dt n = text_to_number(c0->val);
   int length = mx_vect_size(i_c);

   for (int k = 1; k < length; k++)
   {
      // local ptr
      struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(i_c, k);

      n = n - text_to_number(ck->val);
   }

   // return ptr
   return mx_elem_ctor_ab(t_number, number_to_text(n));
}


static char* key_tab[] =
{
   "append",
   "car",
   "cdr",
   "cons",
   "length",
   "list",
   "null?",
   "+",
   "-",
   "*",
   "/",
   ">",
   "<",
   "<=",
};

static fun_type fun_tab[] =
{
   &fun_append,
   &fun_car,
   &fun_cdr,
   &fun_cons,
   &fun_length,
   &fun_list,
   &fun_nullp,
   &fun_add,
   &fun_sub,
   &fun_mul,
   &fun_div,
   &fun_greater,
   &fun_less,
   &fun_less_equal,
};


// define the bare minimum set of primitives necessary to pass the unit tests
void add_globals(struct mx_env* i_env)
{
   mx_env_put(i_env, mx_text_ctor("nil"), nil_sym);
   mx_env_put(i_env, mx_text_ctor("#f"), false_sym);
   mx_env_put(i_env, mx_text_ctor("#t"), true_sym);

   for (int k = 0; k < 14; k++)
   {
      mx_text* t = mx_text_ctor(key_tab[k]);
      struct mx_elem* i_c = mx_elem_ctor_a2(fun_tab[k]);

      mx_env_put(i_env, t, i_c);
   }
}


// eval function forward declaration
const struct mx_elem* eval(const struct mx_elem* i_x, struct mx_env* i_env);

// return true if symbol is found, false otherwise
bool print_help(mx_text* i_val, struct mx_env* i_env)
{
   bool exit = true;
   eval_fnct_type fnct_id = mx_fnct_id_by_name(i_val->text);

   switch (fnct_id)
   {
   case e_clear:
      for (int k = 0; k < 100; k++)
      {
         mx_print_text("\n\n\n\n\n\n\n\n\n\n");
      }
      break;
   case e_help:
      mx_print_text("     available commands:\n     clear\n     exit\n     list-help\n     keywords-help\n     test-help\n     status-help\n");
      break;
   case e_list_env:
      mx_print_text("     listing global environment contents:\n");
      mx_env_dbg_list(global_env);
      break;
   case e_list_help:
      mx_print_text("     available commands:\n     list-i_env\n");
      break;
   case e_keywords_help:
      mx_print_text("     built-in keywords / operations:\n\n\
     quote - the special form quote returns its single argument, as written, without evaluating it. this provides a way to include constant symbols and lists, which are not self-evaluating objects, in a program. (it is not necessary to quote self-evaluating objects such as numbers, strings, and vectors)\n\n\
     if - if test-form then-form [else-form] => result. if allows the execution of a form to be dependent on a single test-form. first test-form is evaluated. if the result is true, then then-form is selected; otherwise else-form is selected. whichever form is selected is then evaluated\n\n\
     set! - assigns values to variables. (set! i_var form) is the simple variable assignment statement of Lisp. form1 is evaluated and the result is stored in the variable i_var\n\n\
     define - (define i_var exp). defun (short for 'define function') is a macro in the Lisp family of programming languages that defines a function in the global environment that uses the form: (define <function - name>(<parameter1><parameter2>...<parameterN>) functionbody)\n\n\
     lambda - (lambda (i_var*) exp). lambda is the symbol for an anonymous function, a function without a name. every time you use an anonymous function, you need to include its whole body. thus, (lambda(arg) (/ arg 50)) is a function definition that says 'return the value resulting from dividing whatever is passed to me as arg by 50'\n\n\
     begin - (begin exp*). 'begin' is a special form that causes each of its arguments to be evaluated in sequence and then returns the value of the last one. the preceding expressions are evaluated only for the side effects they perform. the values produced by them are discarded\n\n\
     #f - false\n\n\
     #t - the boolean representing true, and the canonical generalized boolean representing true. although any object other than nil is considered true, #t is generally used when there is no special reason to prefer one such object over another\n\n\
     nil - nil represents both boolean (and generalized boolean) false and the empty list\n\n\
     append - the append function attaches one list to another\n\n\
     car - the car of a list is, quite simply, the first item in the list. thus the car of the list (rose violet daisy buttercup) is rose\n\n\
     cdr - the cdr of a list is the rest of the list, that is, the cdr function returns the part of the list that follows the first item. thus, while the car of the list '(rose violet daisy buttercup) is rose, the rest of the list, the value returned by the cdr function, is (violet daisy buttercup)\n\n\
     cons - the cons function constructs lists; it is the inverse of car and cdr. for example, cons can be used to make a four element list from the three element list, (fir oak maple): (cons 'pine '(fir oak maple))\n\n\
     length - you can find out how many elements there are in a list by using the Lisp function length\n\n\
     list - clones the list received as parameter\n\n\
     null? - returns #t if object is the empty list; otherwise, returns #f.\n\n\
     + - addition\n\n\
     - - substraction\n\n\
     * - multiplication\n\n\
     / - division\n\n\
     > - greater than\n\n\
     < - less than\n\n\
     <= - less than or equal\n");
      break;
   case e_test_help:
      mx_print_text("     available commands:\n     test-mem\n     test-text\n     test-vect\n     test-list\n     test-htable\n     test-lisp\n     test-all\n");
      break;
   case e_test_mem:
      test_mx_mem();
      break;
   case e_test_text:
      test_mx_text();
      break;
   case e_test_vect:
      test_mx_vect();
      break;
   case e_test_list:
      test_mx_list();
      break;
   case e_test_htable:
      test_mx_htable();
      break;
   case e_test_lisp:
      test_mx_lisp(i_env);
      break;
   case e_test_all:
      test_all(i_env);
      break;
   case e_status_help:
      mx_print_text("     available commands:\n     status-memory\n     status-objects\n");
      break;
   case e_status_memory:
      mx_print_text("status-memory\n");
      break;
   case e_status_objects:
      mx_print_text("status-objects\n");
      break;
   default:
      exit = false;
   }

   return exit;
}

const struct mx_elem* eval_exit()
{
   global_done = true;

   // return ptr
   return mx_elem_ctor_ab2(t_symbol, "\nprogram finished\nfreeing memory... please wait");
}

const struct mx_elem* eval_symbol(const mx_text* i_tx, struct mx_env* i_env)
{
   // return ptr
   const struct mx_elem* xv = mx_env_get(i_env, i_tx);

   check_obj_status(xv);

   return xv;
}

const struct mx_elem* eval_number(const struct mx_elem* i_x, struct mx_env* i_env)
{
   check_obj_status(i_x);

   // return ptr
   return i_x;
}

const struct mx_elem* eval_quote(const struct mx_elem* i_x, struct mx_env* i_env)
{
   // (quote exp)
   // return ptr
   const struct mx_elem* xv = (struct mx_elem*)mx_vect_elem_at(i_x->list, 1);

   check_obj_status(xv);

   return xv;
}

const struct mx_elem* eval_if(const struct mx_elem* i_x, struct mx_env* i_env, int i_xls)
{
   // (if test conseq [alt])
   // local ptr
   const struct mx_elem* xv = (struct mx_elem*)mx_vect_elem_at(i_x->list, 1);
   struct mx_elem* cc = 0;
   assign_smart_ptr(&cc, eval(xv, i_env));
   struct mx_elem* cr = 0;

   check_obj_status(cc);
   if (cc->val && mx_text_compare_string(cc->val, "#f") == 0)
   {
      if (i_xls < 4)
      {
         cr = nil_sym;
      }
      else
      {
         cr = (struct mx_elem*)mx_vect_elem_at(i_x->list, 3);
      }
   }
   else
   {
      cr = (struct mx_elem*)mx_vect_elem_at(i_x->list, 2);
   }

   assign_smart_ptr(&cc, 0);

   // return ptr
   const struct mx_elem* xr = eval(cr, i_env);

   check_obj_status(xr);

   return xr;
}

const struct mx_elem* eval_setq(const struct mx_elem* i_x, struct mx_env* i_env)
{
   // (set! i_var exp)
   // As a practical matter, you almost always quote the first argument to set.
   // The combination of set and a quoted first argument is so common that it has its own name: the special form set!.
   // This special form is just like set except that the first argument is quoted automatically, so you don't need to type the quote mark yourself.
   // Also, as an added convenience, set! permits you to set several different variables to different values, all in one expression.
   // local ptr
   const struct mx_elem* xl1 = (struct mx_elem*)mx_vect_elem_at(i_x->list, 1);
   const struct mx_elem* c2 = (struct mx_elem*)mx_vect_elem_at(i_x->list, 2);
   // return ptr
   const struct mx_elem* cc = eval(c2, i_env);

   mx_env_put(i_env, xl1->val, cc);

   check_obj_status(cc);

   return cc;
}

const struct mx_elem* eval_define(const struct mx_elem* i_x, struct mx_env* i_env)
{
   // (define i_var exp)
   // defun (short for "define function") is a macro in the Lisp family of programming languages that defines a function in the global environment that uses the form:
   // (defun <function - name>(<parameter1><parameter2>...<parameterN>) functionbody)
   // local ptr
   const struct mx_elem* c2 = (struct mx_elem*)mx_vect_elem_at(i_x->list, 2);
   const struct mx_elem* xl1 = (struct mx_elem*)mx_vect_elem_at(i_x->list, 1);
   // return ptr
   const struct mx_elem* cr = eval(c2, i_env);

   mx_env_put(i_env, xl1->val, cr);

   check_obj_status(cr);

   return cr;
}

const struct mx_elem* eval_lambda(const struct mx_elem* i_x, struct mx_env* i_env)
{
   // (lambda (i_var*) exp)
   // keep a reference to the environment that exists now (when the
   // lambda is being defined) because that'i_s the outer environment
   // we'll need to use when the lambda is executed

   // return ptr
   struct mx_elem* xr = elem_copy(i_x);

   xr->type = t_lambda;
   assign_smart_ptr(&xr->i_env, i_env);

   check_obj_status(xr);

   return xr;
}

const struct mx_elem* eval_begin(const struct mx_elem* i_x, struct mx_env* i_env, int i_xls)
{
   // (begin exp*)
   // "begin" is a special form that causes each of its arguments to be evaluated in sequence and then returns the value of the last one.
   // The preceding expressions are evaluated only for the side effects they perform.
   // The values produced by them are discarded.

   for (int i = 1; i < i_xls - 1; ++i)
   {
      // local ptr
      const struct mx_elem* xv = (struct mx_elem*)mx_vect_elem_at(i_x->list, i);
      struct mx_elem* cc = 0;
      assign_smart_ptr(&cc, eval(xv, i_env));

      assign_smart_ptr(&cc, 0);
   }

   // local ptr
   const struct mx_elem* xv = (struct mx_elem*)mx_vect_elem_at(i_x->list, i_xls - 1);
   // return  ptr
   const struct mx_elem* xr = eval(xv, i_env);

   check_obj_status(xr);

   return xr;
}

const struct mx_elem* eval_set_vkb_enabled(const struct mx_elem* i_x, struct mx_env* i_env)
{
   // return ptr
   const struct mx_elem* xv = (struct mx_elem*)mx_vect_elem_at(i_x->list, 1);
   mx_text* i_tx = 0;

   check_obj_status(xv);

   if (mx_text_compare(xv->val, true_sym->val) == 0)
   {
      assign_smart_ptr(&i_tx, mx_text_ctor("the virtual keyboard is now enabled"));
      mx_lisp_set_vkb_enabled(true);
   }
   else if (mx_text_compare(xv->val, false_sym->val) == 0 || mx_text_compare(xv->val, nil_sym->val) == 0)
   {
      assign_smart_ptr(&i_tx, mx_text_ctor("the virtual keyboard is now disabled"));
      mx_lisp_set_vkb_enabled(false);
   }
   else
   {
      assign_smart_ptr(&i_tx, mx_text_append_string_string(xv->val->text, " is invalid as parameter. please put #t, #f or nil."));
   }

   mx_char_copy_abc(i_tx->text, global_input, GLOBAL_INPUT_LENGTH);
   assign_smart_ptr(&i_tx, 0);

   return mx_elem_ctor_ab2(t_symbol, global_input);
}

const struct mx_elem* eval_set_scr_brightness(const struct mx_elem* i_x, struct mx_env* i_env)
{
   // return ptr
   mx_text* i_tx = 0;
   const struct mx_elem* xv = (struct mx_elem*)mx_vect_elem_at(i_x->list, 1);
   check_obj_status(xv);
   const struct mx_elem* num = eval_number(xv, i_env);
   check_obj_status(num);

   if (num->type == t_number)
   {
      int brightness = (int)text_to_number(num->val);
      mx_lisp_set_scr_brightness(brightness);
      assign_smart_ptr(&i_tx, mx_text_append_string_string("set the screen brigtness to ", num->val->text));
   }
   else
   {
      assign_smart_ptr(&i_tx, mx_text_append_string_string(xv->val->text, " is invalid as parameter. please put a number."));
   }

   mx_char_copy_abc(i_tx->text, global_input, GLOBAL_INPUT_LENGTH);
   assign_smart_ptr(&i_tx, 0);

   return mx_elem_ctor_ab2(t_symbol, global_input);
}

const struct mx_elem* eval_use_sq_brackets_as_parenths(const struct mx_elem* i_x, struct mx_env* i_env)
{
   // return ptr
   const struct mx_elem* xv = (struct mx_elem*)mx_vect_elem_at(i_x->list, 1);
   mx_text* i_tx = 0;

   check_obj_status(xv);

   if (mx_text_compare(xv->val, true_sym->val) == 0)
   {
      assign_smart_ptr(&i_tx, mx_text_ctor("square brackets can now be used as parentheses"));
      mx_lisp_use_sq_brackets_as_parenths(true);
   }
   else if (mx_text_compare(xv->val, false_sym->val) == 0 || mx_text_compare(xv->val, nil_sym->val) == 0)
   {
      assign_smart_ptr(&i_tx, mx_text_ctor("square brackets cannot be used as parentheses anymore"));
      mx_lisp_use_sq_brackets_as_parenths(false);
   }
   else
   {
      assign_smart_ptr(&i_tx, mx_text_append_string_string(xv->val->text, " is invalid as parameter. please put #t, #f or nil."));
   }

   mx_char_copy_abc(i_tx->text, global_input, GLOBAL_INPUT_LENGTH);
   assign_smart_ptr(&i_tx, 0);

   return mx_elem_ctor_ab2(t_symbol, global_input);
}

void eval_init_fun_exps(struct mx_elem** i_funp, mx_vect** i_expsp, const struct mx_elem* i_x, int i_xls, const struct mx_elem* i_xl0, struct mx_env* i_env)
{
   // (fun exp*)
   // local ptr
   assign_smart_ptr(i_funp, eval(i_xl0, i_env));
   assign_smart_ptr(i_expsp, mx_vect_ctor(1, 1));

   for (int k = 1; k < i_xls; k++)
   {
      // local ptr
      const struct mx_elem* i_c = (struct mx_elem*)mx_vect_elem_at(i_x->list, k);
      const struct mx_elem* xc = eval(i_c, i_env);

      check_obj_status(xc);
      mx_vect_add_elem(*i_expsp, (mx_smart_ptr*)xc);
   }
}

const struct mx_elem* eval_fun_type_lambda(struct mx_elem** i_funp, mx_vect** i_expsp)
{
   // Create an environment for the execution of this lambda function
   // where the outer environment is the one that existed* at the time
   // the lambda was defined and the new inner associations are the
   // parameter names with the given arguments.
   // *Although the environment existed at the time the lambda was defined
   // it wasn't necessarily complete - it may have subsequently had
   // more symbols defined in that environment.
   // local ptr
   struct mx_elem* fun = *i_funp;
   mx_vect* exps = *i_expsp;
   const struct mx_elem* pl1 = (struct mx_elem*)mx_vect_elem_at(fun->list, 1);
   const struct mx_elem* pl2 = (struct mx_elem*)mx_vect_elem_at(fun->list, 2);
   struct mx_env* i_env = 0;
   assign_smart_ptr(&i_env, mx_env_ctor_abc(pl1->list, exps, fun->i_env));
   // return ptr
   const struct mx_elem* xr = eval(pl2, i_env);

   check_obj_status(xr);
   assign_smart_ptr(&i_env, 0);
   check_obj_status(xr);
   check_ref_count(exps);
   check_obj_status(xr);
   check_ref_count(fun);

   inc_ref_count(xr);
   assign_smart_ptr(i_funp, 0);
   assign_smart_ptr(i_expsp, 0);
   dec_ref_count(xr);
   check_obj_status(xr);

   return xr;
}

const struct mx_elem* eval_fun_type_function(struct mx_elem** i_funp, mx_vect** i_expsp)
{
   struct mx_elem* fun = *i_funp;
   mx_vect* exps = *i_expsp;

   for (int k = 0; k < mx_vect_size(exps); k++)
   {
      struct mx_elem* i_c = (struct mx_elem*)mx_vect_elem_at(exps, k);
      check_obj_status(i_c);
   }

   // return ptr
   const struct mx_elem* cr = fun->fun(exps);

   check_ref_count(exps);
   check_ref_count(fun);

   check_obj_status(cr);
   assign_smart_ptr(&fun, 0);
   assign_smart_ptr(&exps, 0);

   return cr;
}

const struct mx_elem* eval_undefined_symbol()
{
   mx_text* i_tx = 0;
   assign_smart_ptr(&i_tx, mx_text_append_string_string("undefined symbol ", i_tx->text));
   mx_char_copy_abc(i_tx->text, global_input, GLOBAL_INPUT_LENGTH);
   assign_smart_ptr(&i_tx, 0);

   // return ptr
   return mx_elem_ctor_ab2(t_symbol, global_input);
}

const struct mx_elem* eval_new_symbol(const struct mx_elem* i_x)
{
   mx_text* i_tx = 0;
   assign_smart_ptr(&i_tx, mx_text_append_string_string("undefined symbol. elem type is: ", get_elem_type(i_x)));
   mx_char_copy_abc(i_tx->text, global_input, GLOBAL_INPUT_LENGTH);
   assign_smart_ptr(&i_tx, 0);

   // return ptr
   return mx_elem_ctor_ab2(t_symbol, global_input);
}


// eval function body
const struct mx_elem* eval(const struct mx_elem* i_el, struct mx_env* i_env)
{
   mx_text* i_tx = i_el->val;

   if (i_tx && print_help(i_tx, i_env))
   {
      return mx_elem_ctor_ab2(t_symbol, "");
   }

   if (i_tx && mx_text_compare_string(i_tx, "exit") == 0)
   {
      return eval_exit();
   }

   if (i_el->type == t_symbol)
   {
      return eval_symbol(i_tx, i_env);
   }

   if (i_el->type == t_number)
   {
      return eval_number(i_el, i_env);
   }

   if (mx_vect_is_empty(i_el->list))
   {
      return nil_sym;
   }

   const struct mx_elem* i_xl0 = (struct mx_elem*)mx_vect_elem_at(i_el->list, 0);
   int i_xls = mx_vect_size(i_el->list);
   mx_text* xl0_tx = i_xl0->val;

   if (i_xl0->type == t_symbol && xl0_tx)
   {
      eval_fnct_type fnct_id = mx_fnct_id_by_name(xl0_tx->text);

      switch (fnct_id)
      {
      case e_quote:
         return eval_quote(i_el, i_env);
      case e_if:
         return eval_if(i_el, i_env, i_xls);
      case e_setq:
         return eval_setq(i_el, i_env);
      case e_define:
         return eval_define(i_el, i_env);
      case e_lambda:
         return eval_lambda(i_el, i_env);
      case e_begin:
         return eval_begin(i_el, i_env, i_xls);
      case e_set_vkb_enabled:
         return eval_set_vkb_enabled(i_el, i_env);
      case e_set_scr_brightness:
         return eval_set_scr_brightness(i_el, i_env);
      case e_use_sq_brackets_as_parenths:
         return eval_use_sq_brackets_as_parenths(i_el, i_env);
      }
   }

   // evaluate lambda and built-in functions
   {
      struct mx_elem* fun = 0;
      mx_vect* exps = 0;

      eval_init_fun_exps(&fun, &exps, i_el, i_xls, i_xl0, i_env);

      if (fun->type == t_lambda)
      {
         return eval_fun_type_lambda(&fun, &exps);
      }

      if (fun->type == t_function)
      {
         return eval_fun_type_function(&fun, &exps);
      }

      // unreference fun & exps
      assign_smart_ptr(&fun, 0);
      assign_smart_ptr(&exps, 0);
   }

   if (i_el->type == t_symbol)
   {
      return eval_undefined_symbol();
   }

   return eval_new_symbol(i_el);
}


////////////////////// parse, read and user interaction

// convert given string to list of tokens
mx_vect* tokenize(const mx_text* i_str)
{
   mx_vect* tokens = mx_vect_ctor(1, 1);
   const char* i_s = i_str->text;

   while (*i_s)
   {
      while (*i_s == ' ')
      {
         ++i_s;
      }

      if (*i_s == '(' || *i_s == ')')
      {
         mx_text* i_tx = 0;

         if (*i_s == '(')
         {
            i_tx = mx_text_ctor("(");
         }
         else
         {
            i_tx = mx_text_ctor(")");
         }

         mx_vect_add_elem(tokens, (mx_smart_ptr*)i_tx);
         i_s++;
      }
      else
      {
         const char* t = i_s;

         while (*t && *t != ' ' && *t != '(' && *t != ')')
         {
            ++t;
         }

         int delta = t - i_s;
         mx_text* i_tx = mx_text_ctor_ab(i_s, delta);

         mx_vect_add_elem(tokens, (mx_smart_ptr*)i_tx);
         i_s = t;
      }
   }

   return tokens;
}

// numbers become Numbers; every other token is a Symbol
struct mx_elem* atom(mx_text* i_tokens)
{
   char* token = i_tokens->text;

   if (is_digit(token[0]) || (token[0] == '-' && is_digit(token[1])))
   {
      return mx_elem_ctor_ab(t_number, i_tokens);
   }

   return mx_elem_ctor_ab(t_symbol, i_tokens);
}

// return the lisp expression in the given tokens
struct mx_elem* read_from(mx_vect* i_tokens)
{
   mx_text* token = 0;
   assign_smart_ptr(&token, (mx_text*)mx_vect_front(i_tokens));

   mx_vect_pop_front(i_tokens);

   if (mx_text_compare_string(token, "(") == 0)
   {
      // return ptr
      struct mx_elem* i_c = mx_elem_ctor(t_list);
      mx_text* token2 = (mx_text*)mx_vect_front(i_tokens);

      if (token2 == 0)
      {
         // cleanup
         assign_smart_ptr(&token, 0);
         mx_elem_dtor(i_c);

         return 0;
      }

      while (mx_text_compare_string(token2, ")") != 0)
      {
         // local ptr
         struct mx_elem* c2 = read_from(i_tokens);

         if (c2 == 0)
         {
            // cleanup
            assign_smart_ptr(&token, 0);
            mx_elem_dtor(i_c);

            return 0;
         }

         mx_vect_add_elem(i_c->list, (mx_smart_ptr*)c2);

         if (mx_vect_size(i_tokens) <= 0)
         {
            // cleanup
            assign_smart_ptr(&token, 0);
            mx_elem_dtor(i_c);

            return 0;
         }

         token2 = (mx_text*)mx_vect_front(i_tokens);

         if (token2 == 0)
         {
            // cleanup
            assign_smart_ptr(&token, 0);
            mx_elem_dtor(i_c);

            return 0;
         }
      }

      mx_vect_pop_front(i_tokens);
      // cleanup
      assign_smart_ptr(&token, 0);

      return i_c;
   }

   // return ptr
   struct mx_elem* rc = atom(token);
   // cleanup
   assign_smart_ptr(&token, 0);

   return rc;
}

// return the Lisp expression represented by the given string
struct mx_elem* read_exp(const mx_text* i_s)
{
   mx_vect* tokens = 0;
   assign_smart_ptr(&tokens, tokenize(i_s));

   if (mx_vect_size(tokens) > 0)
   {
      struct mx_elem* rc = read_from(tokens);
      assign_smart_ptr(&tokens, 0);

      return rc;
   }

   assign_smart_ptr(&tokens, 0);
   return 0;
}

// convert given elem to a Lisp-readable string
mx_text* to_string(const struct mx_elem* i_exp)
{
   if (i_exp->type == t_list)
   {
      mx_text* i_s = 0;
      assign_smart_ptr(&i_s, mx_text_ctor("("));
      int length = mx_vect_size(i_exp->list);

      for (int k = 0; k < length; k++)
      {
         // local ptr
         const struct mx_elem* i_c = (const struct mx_elem*)mx_vect_elem_at(i_exp->list, k);
         mx_text* t = 0;

         //i_s += to_string(i_c) + ' ';
         assign_smart_ptr(&t, to_string(i_c));
         assign_smart_ptr(&i_s, mx_text_append(i_s, t));
         assign_smart_ptr(&i_s, mx_text_append_string(i_s, " "));
         assign_smart_ptr(&t, 0);
      }

      char cc = mx_text_char_at(i_s, mx_text_length(i_s) - 1);

      if (cc == ' ')
      {
         int pos = mx_text_length(i_s) - 1;
         //i_s.erase(pos);
         assign_smart_ptr(&i_s, mx_text_erase(i_s, pos));
      }

      //return i_s + ')';
      assign_smart_ptr(&i_s, mx_text_append_string(i_s, ")"));

      // returning a smart pointer. reset ref count
      reset_ref_count(i_s);

      return i_s;
   }
   else if (i_exp->type == t_lambda)
   {
      return mx_text_ctor("<lambda>");
   }
   else if (i_exp->type == t_function)
   {
      return mx_text_ctor("<function>");
   }

   return i_exp->val;
}



void mx_set_outputf(void (*i_outputf)(const char* i_text))
{
   outputf = i_outputf;
}

void mx_lisp_init_ctx()
{
   mx_print_text("starting lisp repl...\n");
   mx_mem_init_ctx();

   global_env_vect = mx_vect_ctor(1, 1);

   assign_smart_ptr(&false_sym, mx_elem_ctor_ab2(t_symbol, "#f"));
   assign_smart_ptr(&true_sym, mx_elem_ctor_ab2(t_symbol, "#t"));
   assign_smart_ptr(&nil_sym, mx_elem_ctor_ab2(t_symbol, "nil"));

   int size = mx_vect_size(global_env_vect);

   for (int k = 0; k < size; k++)
   {
      struct mx_env* e = (struct mx_env*)mx_vect_elem_at(global_env_vect, k);

      if (e->ref_count > 1)
      {
         int i_x = 3;
      }
      //reset_ref_count(e);
      e->ext_env = 0;
   }
   mx_vect_del_all_elem(global_env_vect);

   assign_smart_ptr(&global_env, mx_env_ctor(0));
   add_globals(global_env);
   global_done = false;

   mx_print_text("type 'help' to list available commands\n\n");
}

void mx_lisp_eval_line(const char* i_line, int i_line_length)
{
   if (global_done)
   {
      return;
   }

   mx_char_copy_abc(i_line, global_input, i_line_length);

   mx_text* linetx = 0;
   struct mx_elem* i_c = 0;
   struct mx_elem* ce = 0;
   mx_text* ce_text = 0;
   assign_smart_ptr(&linetx, mx_text_ctor(global_input));
   assign_smart_ptr(&i_c, read_exp(linetx));

   if (i_c != 0)
   {
      assign_smart_ptr(&ce, eval(i_c, global_env));
      assign_smart_ptr(&ce_text, to_string(ce));

      mx_print_text(ce_text->text);
   }
   else
   {
      mx_print_text("invalid expression");
   }

   mx_print_text("\n");

   assign_smart_ptr(&linetx, 0);
   assign_smart_ptr(&i_c, 0);
   assign_smart_ptr(&ce, 0);
   assign_smart_ptr(&ce_text, 0);
}

void mx_lisp_destroy_ctx()
{
   assign_smart_ptr(&false_sym, 0);
   assign_smart_ptr(&true_sym, 0);
   assign_smart_ptr(&nil_sym, 0);
   mx_env_dtor(global_env);

   mx_vect_dtor(global_env_vect);

   {
      mx_text* t1 = 0;
      mx_text* t2 = 0;
      mx_text* t3 = 0;
      mx_text* t4 = 0;
      assign_smart_ptr(&t1, number_to_text(elem_created));
      assign_smart_ptr(&t2, number_to_text(elem_destroyed));
      assign_smart_ptr(&t3, number_to_text(env_created));
      assign_smart_ptr(&t4, number_to_text(env_destroyed));

      mx_print_text("\nelements created: ");
      mx_print_text(t1->text);
      mx_print_text("\nelements destroyed: ");
      mx_print_text(t2->text);
      mx_print_text("\nenvironments created: ");
      mx_print_text(t3->text);
      mx_print_text("\nenvironments destroyed: ");
      mx_print_text(t4->text);
      mx_print_text("\n");

      assign_smart_ptr(&t1, 0);
      assign_smart_ptr(&t2, 0);
      assign_smart_ptr(&t3, 0);
      assign_smart_ptr(&t4, 0);
   }

   mx_mem_destroy_ctx();
}

// the default read-eval-print-loop
void run_mx_lisp_repl()
{
   mx_print_text("initializing memory context... please wait\n");
   mx_mem_init_ctx();

   mx_print_text("starting lisp repl...\ntype 'help' to list available commands\n\n");
   global_env_vect = mx_vect_ctor(1, 1);

   assign_smart_ptr(&false_sym, mx_elem_ctor_ab2(t_symbol, "#f"));
   assign_smart_ptr(&true_sym, mx_elem_ctor_ab2(t_symbol, "#t"));
   assign_smart_ptr(&nil_sym, mx_elem_ctor_ab2(t_symbol, "nil"));

   int size = mx_vect_size(global_env_vect);

   for (int k = 0; k < size; k++)
   {
      struct mx_env* e = (struct mx_env*)mx_vect_elem_at(global_env_vect, k);

      if (e->ref_count > 1)
      {
         int i_x = 3;
      }
      //reset_ref_count(e);
      e->ext_env = 0;
   }
   mx_vect_del_all_elem(global_env_vect);

   assign_smart_ptr(&global_env, mx_env_ctor(0));
   char prompt[] = "mxl> ";
   char indent[] = "     ";

   add_globals(global_env);
   global_done = false;

   while (!global_done)
   {
      mx_print_text(prompt);
      mx_read_text_line();

      mx_text* linetx = 0;
      struct mx_elem* i_c = 0;
      struct mx_elem* ce = 0;
      mx_text* ce_text = 0;
      assign_smart_ptr(&linetx, mx_text_ctor(global_input));
      assign_smart_ptr(&i_c, read_exp(linetx));

      if (i_c != 0)
      {
         assign_smart_ptr(&ce, eval(i_c, global_env));
         assign_smart_ptr(&ce_text, to_string(ce));

         mx_print_text(indent);
         mx_print_text(ce_text->text);
      }
      else
      {
         mx_print_text(indent);
         mx_print_text("invalid expression");
      }

      mx_print_text("\n");

      assign_smart_ptr(&linetx, 0);
      assign_smart_ptr(&i_c, 0);
      assign_smart_ptr(&ce, 0);
      assign_smart_ptr(&ce_text, 0);
   }

   assign_smart_ptr(&false_sym, 0);
   assign_smart_ptr(&true_sym, 0);
   assign_smart_ptr(&nil_sym, 0);
   mx_env_dtor(global_env);

   mx_vect_dtor(global_env_vect);

   {
      mx_text* t1 = 0;
      mx_text* t2 = 0;
      mx_text* t3 = 0;
      mx_text* t4 = 0;
      assign_smart_ptr(&t1, number_to_text(elem_created));
      assign_smart_ptr(&t2, number_to_text(elem_destroyed));
      assign_smart_ptr(&t3, number_to_text(env_created));
      assign_smart_ptr(&t4, number_to_text(env_destroyed));

      mx_print_text("\nelements created: ");
      mx_print_text(t1->text);
      mx_print_text("\nelements destroyed: ");
      mx_print_text(t2->text);
      mx_print_text("\nenvironments created: ");
      mx_print_text(t3->text);
      mx_print_text("\nenvironments destroyed: ");
      mx_print_text(t4->text);
      mx_print_text("\n");

      assign_smart_ptr(&t1, 0);
      assign_smart_ptr(&t2, 0);
      assign_smart_ptr(&t3, 0);
      assign_smart_ptr(&t4, 0);
   }

   mx_mem_destroy_ctx();
}



//
// error signaling function impl
//

void mx_signal_error(const char* i_error_msg)
{
   mx_print_text("error-");
   mx_print_text(i_error_msg);
   mx_print_text("\n");

   mws_signal_error(i_error_msg);
}

void mx_read_text_line()
{
#ifdef STANDALONE

   //asm
   //(
   //	"LDA $255,global_input\n\t"
   //	"TRAP 0,Fgets,StdIn\n\t"
   //);
   fgets(global_input, GLOBAL_INPUT_LENGTH, stdin);

#endif

   int i = mx_char_length(global_input) - 1;

   if (global_input[i] == '\n')
   {
      global_input[i] = 0;
   }
}

void mx_print_text(const char* i_text)
{
   if (outputf)
   {
      (*outputf)(i_text);
   }
   else
   {
#ifdef STANDALONE

      //global_output = (char*)i_text;
      //asm
      //(
      //	"LDA $255,global_output\n\t"
      //	"TRAP 0,Fputs,StdOut\n\t"
      //);
      fputs(i_text, stdout);

#else

      mws_print(i_text);

#endif
}
}

void mx_print_indent(int i_level)
{
   for (int k = 0; k < i_level; k++)
   {
      mx_print_text(" ");
   }
}



//
// memory management functions impl
//

eval_fnct_type mx_fnct_id_by_name(const char* i_name)
{
   for (int k = 0; k < eval_fncts_list_size; k++)
   {
      if (mx_char_compare(i_name, eval_fncts_list[k].name) == 0)
      {
         return eval_fncts_list[k].id;
      }
   }

   return e_invalid;
}

void mx_mem_init_ctx()
{
   mws_assert(MX_CTX_ELEM_SIZE >= MIN_ELEM_SIZE);
   mws_assert(MIN_ELEM_SIZE % 8 == 0);

#ifndef STANDALONE

   // static allocation of large arrays fails with emscripten, so use malloc in this case
   heap_mem = (char*)malloc(MX_CTX_ELEM_SIZE * MAX_ELEM_COUNT);
   memset(heap_mem, 0, MX_CTX_ELEM_SIZE * MAX_ELEM_COUNT);
   // sizeof(mx_mem_block) = 2 + 2 + 4
   block_list_mem = (char*)malloc(sizeof(struct mx_mem_block) * MAX_ELEM_COUNT);
   memset(block_list_mem, 0, sizeof(struct mx_mem_block) * MAX_ELEM_COUNT);
   global_input = (char*)malloc(GLOBAL_INPUT_LENGTH);
   memset(global_input, 0, GLOBAL_INPUT_LENGTH);
   memset(&mxmc_inst, 0, sizeof(struct mx_mem_ctx));

#endif

   mxmc_inst.elem_size = MX_CTX_ELEM_SIZE;
   mws_assert(MX_CTX_ELEM_COUNT > 0 && MX_CTX_ELEM_COUNT < MAX_ELEM_COUNT);
   mxmc_inst.elem_count = MX_CTX_ELEM_COUNT;
   mxmc_inst.total_size = mxmc_inst.elem_size * mxmc_inst.elem_count;
   mws_assert(mxmc_inst.total_size % 8 == 0);
   // align on 64 bits
   mxmc_inst.mem_start = heap_mem;

   mx_mem_clear(block_list_mem, sizeof(struct mx_mem_block) * MAX_ELEM_COUNT);
   mx_mem_clear(mxmc_inst.mem_start, mxmc_inst.elem_size * mxmc_inst.elem_count);

   // align on 64 bits
   mws_assert((mxmc_inst.elem_count * sizeof(struct mx_mem_block)) % 8 == 0);
   struct mx_mem_block* block_list = (struct mx_mem_block*)block_list_mem;
   mxmc_inst.block_list = block_list;
   block_list->type = FREE_BLOCK;
   block_list->elem_span = mxmc_inst.elem_count;
   block_list->next = 0;
   mxmc_inst.block_list_length = 1;
}

void mx_mem_destroy_ctx()
{
   struct mx_mem_ctx* m = &mxmc_inst;

   if (!(m->block_list_length == 1 && m->block_list->type == FREE_BLOCK))
   {
      mx_signal_error("mx_mem_destroy_ctx: memory leaks detected");
   }

   m->mem_start = 0;
   m->block_list = 0;
}

void* mx_mem_alloc(int i_size_in_bytes)
{
   if (i_size_in_bytes <= 0)
   {
      mx_signal_error("mx_mem_alloc: allocated size must be positive");

      return 0;
   }

   // how many elements will this request take
   int elem_span = i_size_in_bytes / mxmc_inst.elem_size;
   int elem_index = 0;
   struct mx_mem_block* block = mxmc_inst.block_list;

   if (i_size_in_bytes % mxmc_inst.elem_size != 0)
   {
      elem_span++;
   }

   while (block != 0)
      // find the first large enough block
   {
      if (block->type == FREE_BLOCK && block->elem_span >= elem_span)
         // found it
      {
         if (block->elem_span > elem_span)
            // split the current block
         {
            struct mx_mem_block* next_block = mxmc_inst.block_list + elem_index + elem_span;

            next_block->type = FREE_BLOCK;
            next_block->elem_span = block->elem_span - elem_span;
            next_block->next = block->next;
            block->elem_span = elem_span;
            block->next = next_block;
            mxmc_inst.block_list_length++;
         }

         block->type = FULL_BLOCK;
         break;
      }
      else
         // keep looking
      {
         elem_index = elem_index + block->elem_span;
         block = block->next;
      }
   }

   if (block == 0)
      // out of memory
   {
      mx_signal_error("mx_mem_alloc: out of memory");

      return 0;
   }

   return mxmc_inst.mem_start + elem_index * mxmc_inst.elem_size;
}

void* mx_mem_realloc(const void* i_address, int i_size_in_bytes)
{
   if (i_address == 0)
   {
      mx_signal_error("mx_mem_realloc: address is 0");

      return 0;
   }

   if (i_size_in_bytes <= 0)
   {
      mx_signal_error("mx_mem_realloc: reallocated size must be positive");

      return 0;
   }

   if (i_address < (void*)mxmc_inst.mem_start || (i_address >= (void*)(mxmc_inst.mem_start + mxmc_inst.elem_size * mxmc_inst.elem_count)))
   {
      mx_signal_error("mx_mem_realloc: address is invalid");

      return 0;
   }

   int elem_index = 0;
   struct mx_mem_block* block = mxmc_inst.block_list;

   while (block != 0)
      // find the corresponding block for this address, if it exists
   {
      void* current_address = mxmc_inst.mem_start + elem_index * mxmc_inst.elem_size;

      if (current_address == i_address)
         // find out how many bytes we have to copy
      {
         void* new_address = 0;
         int old_size = block->elem_span * mxmc_inst.elem_size;
         int bytes_to_copy = i_size_in_bytes;

         if (old_size < bytes_to_copy)
         {
            bytes_to_copy = old_size;
         }

         new_address = mx_mem_alloc(i_size_in_bytes);
         mx_mem_copy(i_address, new_address, bytes_to_copy);
         mx_mem_free(i_address);

         return new_address;
      }
      else
         // keep looking
      {
         elem_index = elem_index + block->elem_span;
         block = block->next;
      }
   }

   mx_signal_error("mx_mem_realloc: cannot find a memory block for the given address");

   return 0;
}

void mx_mem_free(const void* i_address)
{
   if (i_address == 0)
   {
      mx_signal_error("mx_mem_free: address is 0");

      return;
   }

   int* ps = (int*)mxmc_inst.mem_start;
   int* pe = (int*)(mxmc_inst.mem_start + mxmc_inst.elem_size * mxmc_inst.elem_count);

   if (i_address < (void*)mxmc_inst.mem_start || (i_address >= (void*)(mxmc_inst.mem_start + mxmc_inst.elem_size * mxmc_inst.elem_count)))
   {
      mx_signal_error("mx_mem_free: address is invalid");

      return;
   }

   int elem_index = 0;
   struct mx_mem_block* block = mxmc_inst.block_list;
   struct mx_mem_block* prev_block = 0;

   while (block != 0)
      // find the corresponding block for this address, if it exists
   {
      void* current_address = mxmc_inst.mem_start + elem_index * mxmc_inst.elem_size;

      if (current_address == i_address)
      {
         int size = block->elem_span * mxmc_inst.elem_size;

         // clear freed block to 0
         mx_mem_clear(current_address, size);
         break;
      }
      else
         // keep looking
      {
         elem_index = elem_index + block->elem_span;
         prev_block = block;
         block = block->next;
      }
   }

   if (block != 0)
      // if any of the next or prev blocks are free, concatenate them with this one
   {
      block->type = FREE_BLOCK;

      if (prev_block != 0 && prev_block->type == FREE_BLOCK)
         // prev block will absorb this one
      {
         prev_block->elem_span = prev_block->elem_span + block->elem_span;
         prev_block->next = block->next;
         block->type = INVL_BLOCK;
         block = prev_block;
         mxmc_inst.block_list_length--;
      }

      struct mx_mem_block* next_block = block->next;

      if (next_block != 0 && next_block->type == FREE_BLOCK)
         // this block will absorb the next one
      {
         block->elem_span = block->elem_span + next_block->elem_span;
         block->next = next_block->next;
         next_block->type = INVL_BLOCK;
         mxmc_inst.block_list_length--;
      }
   }
   else
   {
      mx_signal_error("mx_mem_free: cannot find a memory block for the given address");
   }
}

void mx_mem_copy(const void* i_src, void* i_dst, int i_size)
{
   mws_assert((i_src != 0) && (i_dst != 0));

   char* src = (char*)i_src;
   char* dst = (char*)i_dst;

   for (int k = 0; k < i_size; k++)
   {
      *dst++ = *src++;
   }
}

void mx_mem_clear(void* i_dst, int i_size)
{
   mws_assert(i_dst != 0);
   mws_assert(i_size % 8 == 0);

   int size = i_size / 8;

   uint64* dst = (uint64*)i_dst;

   for (int k = 0; k < size; k++)
   {
      *dst++ = (uint64)0;
   }
}



//
// reference counting functions
//

void check_obj_status(const void* i_obj_ptr)
{
   if (i_obj_ptr != 0)
   {
      mx_smart_ptr* ptr = (mx_smart_ptr*)i_obj_ptr;

      if (ptr->destructor_ptr == 0)
      {
         mx_signal_error("check_obj_status: obj destroyed");
      }
   }
}

void check_ref_count(const void* i_obj_ptr)
{
   if (i_obj_ptr != 0)
   {
      mx_smart_ptr* ptr = (mx_smart_ptr*)i_obj_ptr;

      if (ptr->ref_count == 0)
         // destroy objects with 0 references
      {
         ptr->destructor_ptr(ptr);
      }
   }
}

void inc_ref_count(const void* i_obj_ptr)
{
   if (i_obj_ptr != 0)
   {
      mx_smart_ptr* ptr = (mx_smart_ptr*)i_obj_ptr;

      ptr->ref_count++;
   }
}

void dec_ref_count(const void* i_obj_ptr)
{
   if (i_obj_ptr != 0)
   {
      mx_smart_ptr* ptr = (mx_smart_ptr*)i_obj_ptr;

      if (ptr->ref_count > 0)
      {
         ptr->ref_count--;
      }
   }
}

void reset_ref_count(const void* i_obj_ptr)
{
   if (i_obj_ptr != 0)
   {
      mx_smart_ptr* ptr = (mx_smart_ptr*)i_obj_ptr;

      ptr->ref_count = 0;
   }
}

void assign_smart_ptr(void** iobj_ptr_left, const void* iobj_ptr_right)
{
   if (*iobj_ptr_left == iobj_ptr_right)
   {
      return;
   }

   if (iobj_ptr_right != 0)
   {
      if (*iobj_ptr_left != 0)
      {
         dec_ref_count(*iobj_ptr_left);
         check_ref_count(*iobj_ptr_left);
      }

      *iobj_ptr_left = (void*)iobj_ptr_right;
      inc_ref_count(iobj_ptr_right);
   }
   else
   {
      if (*iobj_ptr_left != 0)
      {
         dec_ref_count(*iobj_ptr_left);
         check_ref_count(*iobj_ptr_left);
      }

      *iobj_ptr_left = 0;
   }
}



//
// text / string functions impl
//

int mx_char_length(const char* i_text)
{
   int length = 0;

   if (i_text)
   {
      while (i_text[length] != 0)
      {
         length++;
      }
   }
   else
   {
      mx_signal_error("mx_text_length: param is null");
   }

   return length;
}

char* mx_char_copy_abc(const char* i_src, char* i_dst, int i_max_length)
{
   mws_assert((i_src != 0) && (i_dst != 0));

   char* str = i_dst;
   int idx = 0;

   while ((*i_dst++ = *i_src++) != 0 && idx < i_max_length)
   {
      idx++;
   }

   str[idx] = 0;

   return str;
}

char* mx_char_copy(const char* i_src, char* i_dst)
{
   mws_assert((i_src != 0) && (i_dst != 0));

   char* str = i_dst;

   while ((*i_dst++ = *i_src++) != 0);

   return str;
}

char* mx_char_clone_ab(const char* i_src, int i_max_length)
{
   mws_assert(i_src != 0);

   int isrc_length = mx_char_length(i_src);
   int length = i_max_length;

   if (length > isrc_length)
   {
      length = isrc_length;
   }

   char* clone = (char*)mx_mem_alloc(length + 1);

   mx_char_copy_abc(i_src, clone, length);

   return clone;
}

char* mx_char_clone(const char* i_src)
{
   mws_assert(i_src != 0);

   char* clone = (char*)mx_mem_alloc(mx_char_length(i_src) + 1);

   mx_char_copy(i_src, clone);

   return clone;
}


int mx_char_compare(const char* i_text_left, const char* i_text_right)
{
   mws_assert((i_text_left != 0) && (i_text_right != 0));

   while ((*i_text_left != 0 && *i_text_right != 0) && (*i_text_left == *i_text_right))
   {
      i_text_left++;
      i_text_right++;
   }

   return *i_text_left - *i_text_right;
}

char* mx_char_append(const char* i_text, const char* i_append)
{
   int itext_length = mx_char_length(i_text);
   int iappend_length = mx_char_length(i_append);
   char* result = (char*)mx_mem_alloc(itext_length + iappend_length + 1);
   char* insert = result + itext_length;

   mx_char_copy(i_text, result);
   mx_char_copy(i_append, insert);

   return result;
}

void mx_char_reverse(char* i_s)
{
   int i = 0;
   int j = mx_char_length(i_s) - 1;

   for (; i < j; i++, j--)
   {
      char i_c = i_s[i];

      i_s[i] = i_s[j];
      i_s[j] = i_c;
   }
}



//
// mx_text impl
//

int size_of_text()
{
   return sizeof(mx_text);
}

mx_text* mx_text_ctor_ab(const char* i_text, int i_max_length)
{
   mx_text* i_tx = (mx_text*)mx_mem_alloc(size_of_text());

   i_tx->destructor_ptr = mx_text_dtor;
   i_tx->ref_count = 0;

   if (i_text != 0 && i_max_length > 0)
   {
      i_tx->text = mx_char_clone_ab(i_text, i_max_length);
   }
   else
   {
      i_tx->text = mx_char_clone("");
   }

   return i_tx;
}

mx_text* mx_text_ctor(const char* i_text)
{
   return mx_text_ctor_ab(i_text, mx_char_length(i_text));
}

void mx_text_dtor(void* i_text)
{
   if (i_text == 0)
   {
      return;
   }

   mx_text* i_tx = (mx_text*)i_text;

   mx_mem_free(i_tx->text);
   mx_mem_free(i_tx);
}

int mx_text_length(const mx_text* i_text)
{
   return mx_char_length(i_text->text);
}

char mx_text_char_at(const mx_text* i_text, int iidx)
{
   mws_assert(iidx < mx_text_length(i_text));

   return i_text->text[iidx];
}

mx_text* mx_text_erase(const mx_text* i_text, int ipos)
{
   int length = mx_text_length(i_text);

   if (ipos >= length)
   {
      return 0;
   }

   return mx_text_ctor_ab(i_text->text, ipos);
}

mx_text* mx_text_append(const mx_text* i_text, const mx_text* i_append)
{
   return mx_text_append_string(i_text, i_append->text);
}

mx_text* mx_text_append_string(const mx_text* i_text, const char* i_append)
{
   return mx_text_append_string_string(i_text->text, i_append);
}

mx_text* mx_text_append_string_string(const char* i_text, const char* i_append)
{
   char* cat = mx_char_append(i_text, i_append);
   mx_text* i_tx = mx_text_ctor(cat);

   mx_mem_free(cat);

   return i_tx;
}

int mx_text_compare(const mx_text* i_text_left, const mx_text* i_text_right)
{
   return mx_char_compare(i_text_left->text, i_text_right->text);
}

int mx_text_compare_string(const mx_text* i_text_left, const char* i_text_right)
{
   return mx_char_compare(i_text_left->text, i_text_right);
}



//
// utility functions impl
//

mx_text* number_to_text(numeric_dt i_nr)
{
   // max digits + 1 for 64 bits
   char* str = (char*)mx_mem_alloc(21);
   int i = 0;
   bool is_negative = false;

   // handle 0 explicitly
   if (i_nr == 0)
   {
      str[i++] = '0';
      str[i] = 0;
   }
   else
   {
      if (i_nr < 0)
      {
         is_negative = true;
         i_nr = -i_nr;
      }

      // funess individual digits
      while (i_nr != 0)
      {
         int rem = i_nr % 10;

         str[i++] = rem + '0';
         i_nr = i_nr / 10;
      }

      // if number is negative, append '-'
      if (is_negative)
      {
         str[i++] = '-';
      }

      // append string terminator
      str[i] = 0;

      // reverse the string
      mx_char_reverse(str);
   }


   mx_text* i_tx = mx_text_ctor(str);
   mx_mem_free(str);

   return i_tx;
}

numeric_dt text_to_number(const mx_text* i_n)
{
   const char* str = i_n->text;
   numeric_dt number = 0;
   int sign = 1;
   int i = 0;

   // if number is negative, then update sign
   if (str[0] == '-')
   {
      sign = -1;
      // also update index of first digit
      i++;
   }

   // iterate through all digits and update the result
   for (; str[i] != 0; i++)
   {
      number = number * 10 + str[i] - '0';
   }

   // return result with sign
   return sign * number;
}

bool is_digit(char i_c)
{
   return i_c >= '0' && i_c <= '9';
}



//
// mx_vect impl
//

int size_of_mx_vect()
{
   return sizeof(mx_vect);
}

mx_vect* mx_vect_ctor(int i_initial_capacity, int i_capacity_increment)
{
   mws_assert(i_initial_capacity > 0);

   mx_vect* i_v = (mx_vect*)mx_mem_alloc(size_of_mx_vect());

   i_v->destructor_ptr = mx_vect_dtor;
   i_v->ref_count = 0;

   i_v->elem_vect_length = i_initial_capacity;
   i_v->capacity_increment = i_capacity_increment;
   i_v->elem_count = 0;
   i_v->elem_vect = (mx_smart_ptr * *)mx_mem_alloc(i_initial_capacity * sizeof(uint64*));

   return i_v;
}

void mx_vect_dtor(void* i_v)
{
   if (i_v == 0)
   {
      return;
   }

   mx_vect* v = (mx_vect*)i_v;

   mx_vect_del_all_elem(v);
   mx_mem_free(v->elem_vect);
   mx_mem_free(v);
}

mx_smart_ptr* mx_vect_front(mx_vect* i_v)
{
   if (i_v->elem_count == 0)
   {
      mx_signal_error("mx_vect_first_elem: vector is empty");

      return 0;
   }
   else
   {
      return i_v->elem_vect[0];
   }
}

mx_smart_ptr* mx_vect_elem_at(const mx_vect* i_v, int i_index)
{
   if (i_index < 0 || i_index >= i_v->elem_count)
   {
      mx_signal_error("mx_vect_elem_at: index out of bounds");

      return 0;
   }

   return i_v->elem_vect[i_index];
}

int mx_vect_size(const mx_vect* i_v)
{
   return i_v->elem_count;
}

bool mx_vect_is_empty(mx_vect* i_v)
{
   return i_v->elem_count == 0;
}

void mx_vect_reallocate(mx_vect* i_v)
{
   int new_size = i_v->elem_count;

   if (i_v->capacity_increment > 0)
   {
      new_size = new_size + i_v->capacity_increment;
   }
   else
   {
      new_size = new_size + i_v->elem_count;
   }

   i_v->elem_vect_length = new_size;
   i_v->elem_vect = (mx_smart_ptr * *)mx_mem_realloc(i_v->elem_vect, new_size * sizeof(uint64*));
}

void mx_vect_add_elem(mx_vect* i_v, mx_smart_ptr* i_elem)
{
   if (i_v->elem_count >= i_v->elem_vect_length)
   {
      mx_vect_reallocate(i_v);
   }

   i_v->elem_vect[i_v->elem_count] = 0;
   assign_smart_ptr(&i_v->elem_vect[i_v->elem_count], i_elem);
   i_v->elem_count++;
}

void mx_vect_del_all_elem(mx_vect* i_v)
{
   for (int k = 0; k < i_v->elem_count; k++)
   {
      assign_smart_ptr(&i_v->elem_vect[k], 0);
   }

   i_v->elem_count = 0;
}

void mx_vect_del_elem_at(mx_vect* i_v, int i_index)
{
   if (i_index < 0 || i_index >= i_v->elem_count)
   {
      mx_signal_error("mx_vect_del_elem_at: index out of bounds");

      return;
   }

   // arraycopy(elem_vect, i_index + 1, elem_vect, i_index, elem_count - 1 - i_index);
   mx_smart_ptr** src = i_v->elem_vect + i_index + 1;
   mx_smart_ptr** dst = i_v->elem_vect + i_index;
   int size = (i_v->elem_count - 1 - i_index) * sizeof(uint64*);

   assign_smart_ptr(&i_v->elem_vect[i_index], 0);
   mx_mem_copy(src, dst, size);
   i_v->elem_count--;
}

void mx_vect_pop_front(mx_vect* i_v)
{
   mx_vect_del_elem_at(i_v, 0);
}



//
// mx_htable impl
//

struct mx_ht_entry
{
   const mx_text* key;
   const void* value;
   struct mx_ht_entry* next;
};

int size_of_mx_ht_entry()
{
   return sizeof(struct mx_ht_entry);
}

struct mx_ht_entry* mx_ht_entry_ctor(const mx_text* i_key, const void* i_value);
void mx_ht_entry_dtor(struct mx_ht_entry* i_hte);
const void* mx_ht_entry_put(struct mx_ht_entry* i_hte, const mx_text* i_key, const void* i_value);
const void* mx_ht_entry_get(struct mx_ht_entry* i_hte, const mx_text* i_key);
bool mx_ht_entry_contains_key(struct mx_ht_entry* i_hte, const mx_text* i_key);
bool mx_ht_entry_del(struct mx_ht_entry* i_hte, const mx_text* i_key);

int size_of_mx_htable()
{
   return sizeof(mx_htable);
}

mx_htable* mx_htable_ctor(int i_capacity)
{
   mx_htable* i_ht = (mx_htable*)mx_mem_alloc(size_of_mx_htable());

   i_ht->destructor_ptr = mx_htable_dtor;
   i_ht->ref_count = 0;

   i_ht->capacity = i_capacity;
   i_ht->entries = (struct mx_ht_entry**)mx_mem_alloc(size_of_mx_ht_entry());

   for (int k = 0; k < i_ht->capacity; k++)
   {
      i_ht->entries[k] = 0;
   }

   return i_ht;
}

void mx_htable_dtor(void* i_ht)
{
   if (i_ht == 0)
   {
      return;
   }

   mx_htable* ht = (mx_htable*)i_ht;

   for (int k = 0; k < ht->capacity; k++)
   {
      struct mx_ht_entry* hte = ht->entries[k];

      if (hte != 0)
      {
         mx_ht_entry_dtor(hte);
      }
   }

   mx_mem_free(ht->entries);
   mx_mem_free(ht);
}

int mx_htable_get_hash(const char* i_text)
{
   int h = 5381;
   int length = mx_char_length(i_text);

   for (int k = 0; k < length; k++)
   {
      h = ((h << 5) + h) + i_text[k];
   }

   return h & 0x7fffffff;
}

/**
*
* @param key
* @param value
* @return previous value if it exists, null otherwise
*/
const void* mx_htable_put(mx_htable* i_ht, const mx_text* i_key, const void* i_value)
{
   if (i_key == 0)
   {
      mx_signal_error("mx_htable_add: i_key is null");

      return 0;
   }

   int index = mx_htable_get_hash(i_key->text) % i_ht->capacity;
   struct mx_ht_entry* e = i_ht->entries[index];

   if (e == 0)
   {
      i_ht->entries[index] = mx_ht_entry_ctor(i_key, i_value);
   }
   else
   {
      return mx_ht_entry_put(e, i_key, i_value);
   }

   return 0;
}

/**
*
* @param key
* @return value if key exists, null otherwise
*/
const void* mx_htable_get(mx_htable* i_ht, const mx_text* i_key)
{
   if (i_key == 0)
   {
      mx_signal_error("mx_htable_get: i_key is null");

      return 0;
   }

   int index = mx_htable_get_hash(i_key->text) % i_ht->capacity;
   struct mx_ht_entry* e = i_ht->entries[index];

   if (e == 0)
   {
      return 0;
   }

   return mx_ht_entry_get(e, i_key);
}

/**
*
* @param i_key
* @return true if element corresponding to key has been deleted, false othewise
*/
bool mx_htable_del(mx_htable* i_ht, const mx_text* i_key)
{
   if (i_key == 0)
   {
      mx_signal_error("mx_htable_del: i_key is null");

      return 0;
   }

   int index = mx_htable_get_hash(i_key->text) % i_ht->capacity;
   struct mx_ht_entry* e = i_ht->entries[index];

   if (e != 0)
      // if we have to delete first entry from the list
   {
      if (mx_text_compare(i_key, e->key) == 0)
         // if this is the first element in the list, delete it and return
      {
         if (e->next == 0)
         {
            mx_ht_entry_dtor(i_ht->entries[index]);
            i_ht->entries[index] = 0;

            return true;
         }
         else
            // set the next entry to be head of the list and delete current head of the list
         {
            i_ht->entries[index] = e->next;
            mx_ht_entry_del(e, i_key);
         }
      }

      return mx_ht_entry_del(e, i_key);
   }

   return false;
}

/**
* implement a linked list for internal use by mx_hashtable
*/
struct mx_ht_entry* mx_ht_entry_ctor(const mx_text* i_key, const void* i_value)
{
   struct mx_ht_entry* i_hte = (struct mx_ht_entry*)mx_mem_alloc(size_of_mx_ht_entry());

   i_hte->key = 0;
   assign_smart_ptr((void**)& i_hte->key, i_key);
   i_hte->value = 0;
   assign_smart_ptr((void**)& i_hte->value, i_value);
   i_hte->next = 0;

   return i_hte;
}

void mx_ht_entry_dtor(struct mx_ht_entry* i_hte)
{
   while (i_hte != 0)
   {
      struct mx_ht_entry* t = i_hte->next;

      //mx_print_text("mx_ht_entry_dtor ");
      //mx_print_text(i_hte->key->text);
      //mx_print_text("\n");

      assign_smart_ptr((void**)& i_hte->key, 0);
      assign_smart_ptr((void**)& i_hte->value, 0);
      mx_mem_free(i_hte);
      i_hte = t;
   }
}

/**
* add e at the end of list, if not already in the list. otherwise replace old entry with new one
* @param e
* @return previous value if it exists, null otherwise
*/
const void* mx_ht_entry_put(struct mx_ht_entry* i_hte, const mx_text* i_key, const void* i_value)
{
   struct mx_ht_entry* e = i_hte;
   struct mx_ht_entry* p = e;

   do
   {
      if (mx_text_compare(e->key, i_key) == 0)
         // same key. replace
      {
         assign_smart_ptr((void**)& e->key, i_key);
         assign_smart_ptr((void**)& e->value, i_value);

         return i_value;
      }

      p = e;
      e = e->next;
   } while (e != 0);

   struct mx_ht_entry* new_entry = mx_ht_entry_ctor(i_key, i_value);

   p->next = new_entry;
   new_entry->next = 0;

   return 0;
}

/**
* returns value corresponding to i_key
* @param i_key
* @return value corresponding to i_key, null if it doesn't exist
*/
const void* mx_ht_entry_get(struct mx_ht_entry* i_hte, const mx_text* i_key)
{
   struct mx_ht_entry* e = i_hte;

   do
   {
      if (mx_text_compare(i_key, e->key) == 0)
      {
         return e->value;
      }

      e = e->next;
   } while (e != 0);

   return 0;
}

/**
*
* @param i_key
* @return true if list contains i_key, false otherwise
*/
bool mx_ht_entry_contains_key(struct mx_ht_entry* i_hte, const mx_text* i_key)
{
   return mx_ht_entry_get(i_hte, i_key) != 0;
}

/**
* delete an element with key == i_key from the list (if it exists)
* assusmes the list contains AT LEAST 2 elements! (at least 1 element besides the head of the list)
* @param i_key
* @return true if element corresponding to i_key has been deleted, false otherwise
*/
bool mx_ht_entry_del(struct mx_ht_entry* i_hte, const mx_text* i_key)
{
   struct mx_ht_entry* e = i_hte;
   struct mx_ht_entry* p = 0;

   do
   {
      if (mx_text_compare(i_key, e->key) == 0)
      {
         if (e->next == 0)
         {
            p->next = 0;
         }
         else
         {
            p->next = e->next;
         }

         e->next = 0;
         mx_ht_entry_dtor(e);

         return true;
      }

      p = e;
      e = e->next;
   } while (e != 0);

   return false;
}



void mx_env_dbg_list(const struct mx_env* i_e)
{
   mx_htable* h = i_e->ht_env;

   mx_print_text("i_env\n[\n");

   for (int k = 0; k < h->capacity; k++)
   {
      struct mx_ht_entry* t = h->entries[k];

      while (t)
      {
         const char* key = t->key->text;
         const struct mx_elem* i_c = (const struct mx_elem*)t->value;

         mx_print_text(" [ ");
         mx_print_text(key);
         mx_print_text(", ");

         mx_elem_dbg_list(i_c);

         mx_print_text("],\n");

         t = t->next;
      }
   }

   if (i_e->ext_env)
   {
      mx_env_dbg_list(i_e->ext_env);
   }

   mx_print_text("],\n");
}

// evaluate the given Lisp expression and compare the result against the given i_expected_result
void test_eq(const char* i_expression, const char* i_expected_result)
{
   // local ptr
   mx_text* expr = 0;
   assign_smart_ptr(&expr, mx_text_ctor(i_expression));
   struct mx_elem* rc = 0;
   assign_smart_ptr(&rc, read_exp(expr));
   //mx_elem_dbg_list(rc);
   struct mx_elem* ec = 0;
   assign_smart_ptr(&ec, eval(rc, test_env));
   //mx_elem_dbg_list(ec);
   mx_text* result = 0;
   assign_smart_ptr(&result, to_string(ec));

   assign_smart_ptr(&expr, 0);
   assign_smart_ptr(&rc, 0);
   assign_smart_ptr(&ec, 0);

   executed_test_count++;

   if (mx_char_compare(result->text, i_expected_result) != 0)
   {
      mx_print_text("\n");
      mx_print_text(i_expression);
      mx_print_text(" : expected ");
      mx_print_text(i_expected_result);
      mx_print_text(", got ");
      mx_print_text(result->text);
      mx_print_text("\n");

      failed_test_count++;
   }
   else
   {
      mx_print_text("\n");
      mx_print_text(i_expression);
      mx_print_text(" : ");
      mx_print_text(result->text);
      mx_print_text("\n");
   }

   assign_smart_ptr(&result, 0);
   mx_read_text_line();
}

void test_mx_lisp(struct mx_env* i_env)
{
   executed_test_count = 0;
   failed_test_count = 0;
   test_env = i_env;

   // the 29 unit tests for lis.py
   test_eq("(quote (testing 1 (2.0) -3.14e159))", "(testing 1 (2.0) -3.14e159)");
   test_eq("(+ 2 2)", "4");
   test_eq("(+ (* 2 100) (* 1 10))", "210");
   test_eq("(if(> 6 5) (+ 1 1) (+ 2 2))", "2");
   test_eq("(if(< 6 5) (+ 1 1) (+ 2 2))", "4");
   test_eq("(define i_x 3)", "3");
   test_eq("i_x", "3");
   test_eq("(+ i_x i_x)", "6");
   test_eq("(begin (define i_x 1) (set! i_x (+ i_x 1)) (+ i_x 1))", "3");
   test_eq("((lambda (i_x) (+ i_x i_x)) 5)", "10");
   test_eq("(define twice (lambda (i_x) (* 2 i_x)))", "<lambda>");
   test_eq("(twice 5)", "10");
   test_eq("(define compose (lambda (f g) (lambda (i_x) (f (g i_x)))))", "<lambda>");
   test_eq("((compose list twice) 5)", "(10)");
   test_eq("(define repeat (lambda (f) (compose f f)))", "<lambda>");
   test_eq("((repeat twice) 5)", "20");
   test_eq("((repeat (repeat twice)) 5)", "80");
   test_eq("(define fact (lambda (n) (if(<= n 1) 1 (* n (fact (- n 1))))))", "<lambda>");
   test_eq("(fact 3)", "6");
   test_eq("(fact 12)", "479001600"); // max for 32 bits
   test_eq("(fact 20)", "2432902008176640000"); // max for 64 bits
   test_eq("(define abs (lambda (n) ((if(> n 0) + -) 0 n)))", "<lambda>");
   test_eq("(list (abs -3) (abs 0) (abs 3))", "(3 0 3)");
   test_eq("(define combine (lambda (f)"
      "(lambda (i_x y)"
      "(if(null? i_x) (quote ())"
      "(f (list (car i_x) (car y))"
      "((combine f) (cdr i_x) (cdr y)))))))", "<lambda>");
   test_eq("(define zip (combine cons))", "<lambda>");
   test_eq("(zip (list 1 2 3 4) (list 5 6 7 8))", "((1 5) (2 6) (3 7) (4 8))");
   test_eq("(define riff-shuffle (lambda (deck) (begin"
      "(define take (lambda (n seq) (if(<= n 0) (quote ()) (cons (car seq) (take (- n 1) (cdr seq))))))"
      "(define drop (lambda (n seq) (if(<= n 0) seq (drop (- n 1) (cdr seq)))))"
      "(define mid (lambda (seq) (/ (length seq) 2)))"
      "((combine append) (take (mid deck) deck) (drop (mid deck) deck)))))", "<lambda>");
   test_eq("(riff-shuffle (list 1 2))", "(1 2)");
   //test_eq("(riff-shuffle (list 1 2 3 4 5 6 7 8))", "(1 5 2 6 3 7 4 8)");
   //test_eq("((repeat riff-shuffle) (list 1 2 3 4 5 6 7 8))", "(1 3 5 7 2 4 6 8)");
   //test_eq("(riff-shuffle (riff-shuffle (riff-shuffle (list 1 2 3 4 5 6 7 8))))", "(1 2 3 4 5 6 7 8)");
   test_eq("(define sum (lambda (n) (if(<= n 1) 1 (+ n (sum (- n 1))))))", "<lambda>");
   test_eq("(sum 101)", "5151");

   mx_text* t1 = 0;
   mx_text* t2 = 0;
   assign_smart_ptr(&t1, number_to_text(executed_test_count));
   assign_smart_ptr(&t2, number_to_text(failed_test_count));
   mx_print_text("\n\ntotal tests ");
   mx_print_text(t1->text);
   mx_print_text(", total failures ");
   mx_print_text(t2->text);
   mx_print_text("\n");

   //mx_env_dbg_list(global_env);
   assign_smart_ptr(&t1, 0);
   assign_smart_ptr(&t2, 0);
   test_env = 0;
}

void test_all(struct mx_env* i_env)
{
   test_mx_mem();
   test_mx_text();
   test_mx_vect();
   test_mx_list();
   test_mx_htable();
   test_mx_lisp(i_env);
}

void test_mx_mem()
{
   mx_print_text("--- testing memory functions ---\n");

   char* s1 = (char*)mx_mem_alloc(23);
   char* s2 = (char*)mx_mem_alloc(256);
   char* s3 = (char*)mx_mem_alloc(377);
   char* s4 = (char*)mx_mem_alloc(153);

   mx_char_copy("abcd-xxx!", s1);
   mx_char_copy("a,voe[W@[fd]fd]", s2);
   mx_char_copy("this iss a tesssxxxxssst", s3);
   mx_char_copy("a loooooong texxxxst !", s4);

   s1 = (char*)mx_mem_realloc(s1, 34);
   s2 = (char*)mx_mem_realloc(s2, 7);
   s2[6] = 0;
   s3 = (char*)mx_mem_realloc(s3, 11);
   s3[10] = 0;
   s4 = (char*)mx_mem_realloc(s4, 757);

   mx_print_text("s1: ");
   mx_print_text(s1);
   mx_print_text("\ns2: ");
   mx_print_text(s2);
   mx_print_text("\ns3: ");
   mx_print_text(s3);
   mx_print_text("\ns4: ");
   mx_print_text(s4);

   mx_mem_free(s1);
   mx_mem_free(s2);
   mx_mem_free(s4);
   mx_mem_free(s3);

   mx_print_text("\n\n");
}

void test_mx_text()
{

}

void test_mx_vect()
{

}

void test_mx_list()
{

}

void test_mx_htable()
{
   mx_print_text("--- testing hash-table functions ---\n");

   mx_htable* ht = mx_htable_ctor(10);
   char* l[] = { "a", "a", "abc", "ab", "abcd", "dcba", "acbd", "acdb", "bcad", "bacd", "bcda", "bdac", "bdca", };
   char* v[] = { "1", "2","3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "bada55", };
   int length = sizeof(l) / sizeof(char*);

   mx_print_text("--- testing put ---\n");
   for (int k = 0, m = length - 1; k < length; k++, m--)
   {
      mx_htable_put(ht, mx_text_ctor(l[k]), mx_text_ctor(v[m]));

      mx_print_text("put ");
      mx_print_text(l[k]);
      mx_print_text(" - ");
      mx_print_text(v[m]);
      mx_print_text("\n");
   }

   mx_print_text("--- testing get ---\n");
   for (int k = 0; k < length; k++)
   {
      char* key = l[k];
      mx_text* query = 0;
      assign_smart_ptr(&query, mx_text_ctor(key));
      mx_text* val = (mx_text*)mx_htable_get(ht, query);

      mx_print_text("get ");
      mx_print_text(key);
      mx_print_text(" - ");
      mx_print_text(val->text);
      mx_print_text("\n");
      assign_smart_ptr(&query, 0);
   }

   mx_htable_dtor(ht);
   ht = 0;

   mx_print_text("\n");
}

#ifdef STANDALONE

int main()
{
   run_mx_lisp_repl();

   return 0;
}

#endif
