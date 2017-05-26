#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_MX_LISP

//#define STANDALONE

#ifdef STANDALONE
	//#define MMIX 1
	#define bool int
	#define false 0
	#define true 1
	#define ia_assert(cond)
	#define ia_signal_error()
#else
	#include "pfm.hpp"
#endif

#include <stdio.h>


// Implementarea unui interpretor pentru limbajul LISP (pentru o masina MMIX)
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef signed long long int64;
typedef unsigned long long uint64;
typedef int64 numeric_dt;
struct mx_elem;
struct mx_env;

//
// global data
//

static bool global_done = false;
#define GLOBAL_INPUT_LENGTH 1024
static char global_input[GLOBAL_INPUT_LENGTH] = {0};
static char* global_output = 0;
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

int size_of_address()
{
	return sizeof(uint64*);
}

struct mx_mem_block
{
	uint16 type;
	uint16 elem_span;
	struct mx_mem_block* next;
};

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

// maximum 65535 memory partitions
#define MAX_ELEM_COUNT 65535
#define MIN_ELEM_SIZE 16
#define MX_CTX_ELEM_COUNT 16000
#define MX_CTX_ELEM_SIZE 128

static const uint16 FREE_BLOCK = 0;
static const uint16 FULL_BLOCK = 1;
static const uint16 INVL_BLOCK = 2;

static char heap_mem[MX_CTX_ELEM_SIZE * MAX_ELEM_COUNT] = {0};
// sizeof(mx_mem_block) = 2 + 2 + 4
static char block_list_mem[sizeof(struct mx_mem_block) * MAX_ELEM_COUNT] = {0};
static struct mx_mem_ctx mxmc_inst = {};



//
// memory management functions
//

void mx_mem_init_ctx();
void mx_mem_destroy_ctx();
void* mx_mem_alloc(int isize_in_bytes);
void* mx_mem_realloc(const void* iaddress, int isize_in_bytes);
void mx_mem_free(const void* iaddress);
bool mx_mem_is_valid_address(const void* iaddress);
void mx_mem_copy(const void* isrc, void* idst, int isize);
void mx_mem_clear(void* idst, int isize);



//
// reference counting functions
//

typedef struct
{
	void(*destructor_ptr)(void* iobj_ptr);
	int ref_count;
}
mx_smart_ptr;

void check_obj_status(const void* iobj_ptr);
void check_ref_count(const void* iobj_ptr);
void inc_ref_count(const void* iobj_ptr);
void dec_ref_count(const void* iobj_ptr);
void reset_ref_count(const void* iobj_ptr);
void assign_smart_ptr(void** iobj_ptr_left, const void* iobj_ptr_right);



//
// input/output functions
//

// error signaling function
void mx_signal_error(const char* ierror_msg);
// read a text line from the console into 'global_input'
void mx_read_text_line();
void mx_print_text(const char* itext);
void mx_print_indent(int ilevel);



//
// testing functions
//

void test_mx_mem();
void test_mx_text();
void test_mx_vect();
void test_mx_list();
void test_mx_htable();
void test_mx_lisp(struct mx_env* env);
void test_all(struct mx_env* env);



//
// byte string functions
//

int mx_char_length(const char* itext);
char* mx_char_copy_abc(const char* isrc, char* idst, int imax_length);
char* mx_char_copy(const char* isrc, char* idst);
char* mx_char_clone_ab(const char* isrc, int imax_length);
char* mx_char_clone(const char* isrc);
int mx_char_compare(const char* itext_left, const char* itext_right);
char* mx_char_append(const char* itext, const char* iappend);
// reverse string s in place
void mx_char_reverse(char* s);



//
// mx_text functions
//

typedef struct
{
	void(*destructor_ptr)(void* iobj_ptr);
	int ref_count;

	char* text;
}
mx_text;

int size_of_text();
mx_text* mx_text_ctor_ab(const char* itext, int imax_length);
mx_text* mx_text_ctor(const char* itext);
void mx_text_dtor(void* itext);
int mx_text_length(const mx_text* itext);
char mx_text_char_at(const mx_text* itext, int iidx);
mx_text* mx_text_erase(const mx_text* itext, int ipos);
mx_text* mx_text_append(const mx_text* itext, const mx_text* iappend);
mx_text* mx_text_append_string(const mx_text* itext, const char* iappend);
mx_text* mx_text_append_string_string(const char* itext, const char* iappend);
int mx_text_compare(const mx_text* itext_left, const mx_text* itext_right);
int mx_text_compare_string(const mx_text* itext_left, const char* itext_right);



//
// utility functions
//

mx_text* number_to_text(numeric_dt inr);
numeric_dt text_to_number(const mx_text* n);
bool is_digit(char c);



//
// mx_vect functions
//

typedef struct
{
	void(*destructor_ptr)(void* iobj_ptr);
	int ref_count;

	int capacity_increment;
	int elem_count;
	mx_smart_ptr** elem_vect;
	int elem_vect_length;
}
mx_vect;

int size_of_mx_vect();
mx_vect* mx_vect_ctor(int iinitial_capacity, int icapacity_increment);
void mx_vect_dtor(void* iv);
mx_smart_ptr* mx_vect_front(mx_vect* iv);
mx_smart_ptr* mx_vect_elem_at(const mx_vect* iv, int iindex);
int mx_vect_size(const mx_vect* iv);
bool mx_vect_is_empty(mx_vect* iv);
void mx_vect_add_elem(mx_vect* iv, mx_smart_ptr* ielem);
void mx_vect_del_all_elem(mx_vect* iv);
void mx_vect_del_elem_at(mx_vect* iv, int iindex);
void mx_vect_pop_front(mx_vect* iv);

static mx_vect* global_env_vect = 0;



//
// mx_htable functions
//

struct mx_ht_entry;
typedef struct
{
	void(*destructor_ptr)(void* iobj_ptr);
	int ref_count;

	struct mx_ht_entry** entries;
	int capacity;
}
mx_htable;

int size_of_mx_htable();
mx_htable* mx_htable_ctor(int icapacity);
void mx_htable_dtor(void* iht);
const void* mx_htable_put(mx_htable* iht, const mx_text* ikey, const void* ivalue);
const void* mx_htable_get(mx_htable* iht, const mx_text* ikey);
bool mx_htable_del(mx_htable* iht, const mx_text* ikey);



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
	void(*destructor_ptr)(void* iobj_ptr);
	int ref_count;

	elem_type type;
	mx_text* val;
	mx_vect* list;
	fun_type fun;
	struct mx_env* env;
};

int size_of_mx_elem()
{
	return sizeof(struct mx_elem);
}

mx_vect* elem_list_copy(const mx_vect* isrc);

char* get_elem_type(const struct mx_elem* ic)
{
	if (ic != 0)
	{
		elem_type t = ic->type;

		if (ic->type == t_function)
		{
			return "function elem";
		}
		else if (ic->type == t_lambda)
		{
			return "lambda elem";
		}
		else if (ic->type == t_list)
		{
			return "list elem";
		}
		else if (ic->type == t_number)
		{
			return "number elem";
		}
		else if (ic->type == t_symbol)
		{
			return "symbol elem";
		}
	}

	return "elem type is unavailable";
}

void mx_elem_dtor(void* ic)
{
	if (!ic)
	{
		return;
	}

	struct mx_elem* c = (struct mx_elem*)ic;

	c->destructor_ptr = 0;
	c->fun = 0;

	assign_smart_ptr((void**)&c->val, 0);
	assign_smart_ptr((void**)&c->list, 0);
	assign_smart_ptr((void**)&c->env, 0);

	mx_mem_free(c);
	elem_destroyed++;
}

struct mx_elem* mx_elem_ctor(elem_type itype)
{
	struct mx_elem* c = (struct mx_elem*)mx_mem_alloc(size_of_mx_elem());

	c->destructor_ptr = mx_elem_dtor;

	c->type = itype;
	c->val = 0;
	// member ptr
	c->list = 0;
	assign_smart_ptr((void**)&c->list, mx_vect_ctor(1, 1));
	c->fun = 0;
	c->env = 0;
	elem_created++;

	return c;
}

struct mx_elem* mx_elem_ctor_ab(elem_type itype, mx_text* ival)
{
	struct mx_elem* c = (struct mx_elem*)mx_mem_alloc(size_of_mx_elem());

	c->destructor_ptr = mx_elem_dtor;
	c->ref_count = 0;

	c->type = itype;
	// member ptr
	c->val = 0;
	assign_smart_ptr((void**)&c->val, ival);
	// member ptr
	c->list = 0;
	assign_smart_ptr((void**)&c->list, mx_vect_ctor(1, 1));
	c->fun = 0;
	c->env = 0;
	elem_created++;

	return c;
}

struct mx_elem* mx_elem_ctor_ab2(elem_type itype, const char* ival)
{
	struct mx_elem* c = (struct mx_elem*)mx_mem_alloc(size_of_mx_elem());

	c->destructor_ptr = mx_elem_dtor;
	c->ref_count = 0;

	c->type = itype;
	// member ptr
	c->val = 0;
	assign_smart_ptr((void**)&c->val, mx_text_ctor(ival));
	// member ptr
	c->list = 0;
	assign_smart_ptr((void**)&c->list, mx_vect_ctor(1, 1));
	c->fun = 0;
	c->env = 0;
	elem_created++;

	return c;
}

struct mx_elem* mx_elem_ctor_a2(fun_type ifun)
{
	struct mx_elem* c = (struct mx_elem*)mx_mem_alloc(size_of_mx_elem());

	c->destructor_ptr = mx_elem_dtor;
	c->ref_count = 0;

	c->type = t_function;
	c->val = 0;
	// member ptr
	c->list = 0;
	assign_smart_ptr((void**)&c->list, mx_vect_ctor(1, 1));
	c->fun = ifun;
	c->env = 0;
	elem_created++;

	return c;
}

struct mx_elem* elem_copy(const struct mx_elem* isrc)
{
	// return ptr
	struct mx_elem* copy = 0;

	if (isrc->val)
	{
		copy = mx_elem_ctor_ab2(isrc->type, isrc->val->text);
	}
	else
	{
		copy = mx_elem_ctor(isrc->type);
	}

	// member ptr
	assign_smart_ptr((void**)&copy->list, elem_list_copy(isrc->list));
	copy->fun = isrc->fun;
	assign_smart_ptr((void**)&copy->env, isrc->env);

	return copy;
}

mx_vect* elem_list_copy(const mx_vect* isrc)
{
	// return ptr
	mx_vect* copy = mx_vect_ctor(1, 1);
	int length = mx_vect_size(isrc);

	for (int k = 0; k < length; k++)
	{
		// local ptr
		struct mx_elem* c = (struct mx_elem*)mx_vect_elem_at(isrc, k);
		// local ptr
		struct mx_elem* cc = elem_copy(c);

		mx_vect_add_elem(copy, (mx_smart_ptr*)cc);
	}

	return copy;
}

void mx_elem_dbg_list_impl(const struct mx_elem* c, int ilevel)
{
	mx_print_indent(ilevel);

	if (c->type == t_symbol)
	{
		if (c->val)
		{
			mx_print_text("tsym: ");
			mx_print_text(c->val->text);
		}
	}
	else if (c->type == t_number)
	{
		if (c->val)
		{
			mx_print_text("tnum: ");
			mx_print_text(c->val->text);
		}
	}
	else if (c->type == t_list)
	{
		if (c->list)
		{
			int length = mx_vect_size(c->list);

			mx_print_text("tlist:\n");
			
			for (int k = 0; k < length; k++)
			{
				// local ptr
				const struct mx_elem* cc = (struct mx_elem*)mx_vect_elem_at(c->list, k);

				mx_elem_dbg_list_impl(cc, ilevel + 1);
			}
		}
	}
	else if (c->type == t_function)
	{
		mx_print_text("tfun: ");
	}
	else if (c->type == t_lambda)
	{
		int length = mx_vect_size(c->list);

		mx_print_text("tlambda:\n");

		for (int k = 0; k < length; k++)
		{
			// local ptr
			const struct mx_elem* cc = (struct mx_elem*)mx_vect_elem_at(c->list, k);

			mx_elem_dbg_list_impl(cc, ilevel + 1);
		}
	}

	mx_print_text("\n");
}

void mx_elem_dbg_list(const struct mx_elem* c)
{
	if (!c)
	{
		return;
	}

	mx_elem_dbg_list_impl(c, 0);
}



//
// mx_env functions
//


// a dictionary that (a) associates symbols with elems, and
// (b) can chain to an "outer" dictionary
struct mx_env
{
	void(*destructor_ptr)(void* iobj_ptr);
	int ref_count;

	mx_htable* ht_env; // inner symbol->elem mapping
	struct mx_env* ext_env; // next adjacent outer env, or 0 if there are no further environments
};

int size_of_mx_env()
{
	return sizeof(struct mx_env);
}

void mx_env_dbg_list(const struct mx_env* e);

void mx_env_dtor(void* ie)
{
	if (ie == 0)
	{
		return;
	}

	struct mx_env* e = (struct mx_env*)ie;

	// member ptr
	assign_smart_ptr((void**)&e->ht_env, 0);
	// member ptr
	assign_smart_ptr((void**)&e->ext_env, 0);
	mx_mem_free(e);
	env_destroyed++;
}

struct mx_env* mx_env_ctor(struct mx_env* iext_env)
{
	struct mx_env* e = (struct mx_env*)mx_mem_alloc(size_of_mx_env());

	e->destructor_ptr = mx_env_dtor;
	e->ref_count = 0;

	e->ht_env = 0;
	assign_smart_ptr((void**)&e->ht_env, mx_htable_ctor(10));
	e->ext_env = 0;
	// member ptr
	assign_smart_ptr((void**)&e->ext_env, iext_env);
	env_created++;

	return e;
}

struct mx_env* mx_env_ctor_abc(const mx_vect* parms, const mx_vect* args, struct mx_env* iext_env)
{
	struct mx_env* e = (struct mx_env*)mx_mem_alloc(size_of_mx_env());

	e->destructor_ptr = mx_env_dtor;
	e->ref_count = 0;

	e->ht_env = 0;
	assign_smart_ptr((void**)&e->ht_env, mx_htable_ctor(10));
	e->ext_env = 0;
	// member ptr
	assign_smart_ptr((void**)&e->ext_env, iext_env);

	int length = mx_vect_size(parms);

	for (int k = 0; k < length; k++)
	{
		// local ptr
		struct mx_elem* c = (struct mx_elem*)mx_vect_elem_at(parms, k);
		// local ptr
		struct mx_elem* c2 = (struct mx_elem*)mx_vect_elem_at(args, k);

		mx_htable_put(e->ht_env, c->val, c2);
	}

	env_created++;

	return e;
}

// return a reference to the innermost environment where 'var' appears
struct mx_elem* mx_env_get(struct mx_env* e, const mx_text* var)
{
	// return ptr
	struct mx_elem* cr = (struct mx_elem*)mx_htable_get(e->ht_env, var);

	if (cr)
		// the symbol exists in this environment
	{
		return cr;
	}

	if (e->ext_env)
	{
		return mx_env_get(e->ext_env, var); // attempt to find the symbol in some "outer" env
	}

	mx_text* tx = 0;
	assign_smart_ptr((void**)&tx, mx_text_append_string_string("unbound symbol ", var->text));
	mx_char_copy_abc(tx->text, global_input, GLOBAL_INPUT_LENGTH);
	assign_smart_ptr((void**)&tx, 0);

	// return ptr
	return mx_elem_ctor_ab2(t_symbol, global_input);
}

// return a reference to the elem associated with the given symbol 'var'
void mx_env_put(struct mx_env* e, const mx_text* var, const struct mx_elem* c)
{
	mx_htable_put(e->ht_env, var, c);
}


//
// built-in lisp functions
//

struct mx_elem* fun_append(const mx_vect* c)
{
	// return ptr
	struct mx_elem* result = mx_elem_ctor(t_list);
	// local ptr
	struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(c, 0);
	struct mx_elem* c1 = (struct mx_elem*)mx_vect_elem_at(c, 1);
	mx_vect* ls = c1->list;
	int length = mx_vect_size(ls);

	// member ptr
	assign_smart_ptr((void**)&result->list, elem_list_copy(c0->list));

	for (int k = 0; k < length; k++)
	{
		// local ptr
		struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(c1->list, k);
		struct mx_elem* cc = elem_copy(ck);

		mx_vect_add_elem(result->list, (mx_smart_ptr*)cc);
	}

	return result;
}

struct mx_elem* fun_car(const mx_vect* c)
{
	// local ptr
	struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(c, 0);
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

struct mx_elem* fun_cdr(const mx_vect* c)
{
	// local ptr
	struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(c, 0);
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

struct mx_elem* fun_cons(const mx_vect* c)
{
	// return ptr
	struct mx_elem* result = mx_elem_ctor(t_list);
	// local ptr
	struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(c, 0);
	struct mx_elem* c1 = (struct mx_elem*)mx_vect_elem_at(c, 1);
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

struct mx_elem* fun_div(const mx_vect* c)
{
	// local ptr
	struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(c, 0);
	numeric_dt n = text_to_number(c0->val);
	int length = mx_vect_size(c);

	for (int k = 1; k < length; k++)
	{
		// local ptr
		struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(c, k);

		n = n / text_to_number(ck->val);
	}

	// return ptr
	return mx_elem_ctor_ab(t_number, number_to_text(n));
}

struct mx_elem* fun_greater(const mx_vect* c)
{
	// local ptr
	struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(c, 0);
	numeric_dt n = text_to_number(c0->val);
	int length = mx_vect_size(c);

	for (int k = 1; k < length; k++)
	{
		// local ptr
		struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(c, k);

		if (n <= text_to_number(ck->val))
		{
			// return ptr
			return false_sym;
		}
	}

	// return ptr
	return true_sym;
}

struct mx_elem* fun_length(const mx_vect* c)
{
	// local ptr
	struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(c, 0);
	mx_vect* ls = c0->list;
	int length = mx_vect_size(ls);

	// return ptr
	return mx_elem_ctor_ab(t_number, number_to_text(length));
}

struct mx_elem* fun_less(const mx_vect* c)
{
	// local ptr
	struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(c, 0);
	numeric_dt n = text_to_number(c0->val);
	int length = mx_vect_size(c);

	for (int k = 1; k < length; k++)
	{
		// local ptr
		struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(c, k);

		if (n >= text_to_number(ck->val))
		{
			// return ptr
			return false_sym;
		}
	}

	// return ptr
	return true_sym;
}

struct mx_elem* fun_less_equal(const mx_vect* c)
{
	// local ptr
	struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(c, 0);
	numeric_dt n = text_to_number(c0->val);
	int length = mx_vect_size(c);

	for (int k = 1; k < length; k++)
	{
		// local ptr
		struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(c, k);

		if (n > text_to_number(ck->val))
		{
			// return ptr
			return false_sym;
		}
	}

	// return ptr
	return true_sym;
}

struct mx_elem* fun_list(const mx_vect* c)
{
	// return ptr
	struct mx_elem* result = mx_elem_ctor(t_list);

	// member ptr
	assign_smart_ptr((void**)&result->list, elem_list_copy(c));

	return result;
}

struct mx_elem* fun_mul(const mx_vect* c)
{
	numeric_dt n = 1;
	int length = mx_vect_size(c);

	for (int k = 0; k < length; k++)
	{
		// local ptr
		struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(c, k);

		n = n * text_to_number(ck->val);
	}

	// return ptr
	return mx_elem_ctor_ab(t_number, number_to_text(n));
}

struct mx_elem* fun_nullp(const mx_vect* c)
{
	// local ptr
	struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(c, 0);
	mx_vect* ls = c0->list;

	if (mx_vect_is_empty(ls))
	{
		// return ptr
		return true_sym;
	}

	// return ptr
	return false_sym;
}

struct mx_elem* fun_add(const mx_vect* c)
{
	// local ptr
	struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(c, 0);
	numeric_dt n = text_to_number(c0->val);
	int length = mx_vect_size(c);

	for (int k = 1; k < length; k++)
	{
		// local ptr
		struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(c, k);

		n = n + text_to_number(ck->val);
	}

	// return ptr
	return mx_elem_ctor_ab(t_number, number_to_text(n));
}

struct mx_elem* fun_sub(const mx_vect* c)
{
	// local ptr
	struct mx_elem* c0 = (struct mx_elem*)mx_vect_elem_at(c, 0);
	numeric_dt n = text_to_number(c0->val);
	int length = mx_vect_size(c);

	for (int k = 1; k < length; k++)
	{
		// local ptr
		struct mx_elem* ck = (struct mx_elem*)mx_vect_elem_at(c, k);

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
void add_globals(struct mx_env* env)
{
	mx_env_put(env, mx_text_ctor("nil"), nil_sym);
	mx_env_put(env, mx_text_ctor("#f"), false_sym);
	mx_env_put(env, mx_text_ctor("#t"), true_sym);

	for (int k = 0; k < 14; k++)
	{
		mx_text* t = mx_text_ctor(key_tab[k]);
		struct mx_elem* c = mx_elem_ctor_a2(fun_tab[k]);

		mx_env_put(env, t, c);
	}
}


// eval function forward declaration
const struct mx_elem* eval(const struct mx_elem* x, struct mx_env* env);

// return true if symbol is found, false otherwise
bool print_help(mx_text* ival, struct mx_env* ienv)
{
	bool exit = true;

	if (mx_text_compare_string(ival, "clear") == 0)
	{
		for (int k = 0; k < 100; k++)
		{
			mx_print_text("\n\n\n\n\n\n\n\n\n\n");
		}
	}
	else if (mx_text_compare_string(ival, "help") == 0)
	{
		mx_print_text("     available commands:\n     clear\n     exit\n     list-help\n     keywords-help\n     test-help\n     status-help\n");
	}
	else if (mx_text_compare_string(ival, "list-env") == 0)
	{
		mx_print_text("     listing global environment contents:\n");
		mx_env_dbg_list(global_env);
	}
	else if (mx_text_compare_string(ival, "list-help") == 0)
	{
		mx_print_text("     available commands:\n     list-env\n");
	}
	else if (mx_text_compare_string(ival, "keywords-help") == 0)
	{
			mx_print_text("     built-in keywords / operations:\n\n\
     quote - the special form quote returns its single argument, as written, without evaluating it. this provides a way to include constant symbols and lists, which are not self-evaluating objects, in a program. (it is not necessary to quote self-evaluating objects such as numbers, strings, and vectors)\n\n\
     if - if test-form then-form [else-form] => result. if allows the execution of a form to be dependent on a single test-form. first test-form is evaluated. if the result is true, then then-form is selected; otherwise else-form is selected. whichever form is selected is then evaluated\n\n\
     set! - assigns values to variables. (set! var form) is the simple variable assignment statement of Lisp. form1 is evaluated and the result is stored in the variable var\n\n\
     define - (define var exp). defun (short for 'define function') is a macro in the Lisp family of programming languages that defines a function in the global environment that uses the form: (define <function - name>(<parameter1><parameter2>...<parameterN>) functionbody)\n\n\
     lambda - (lambda (var*) exp). lambda is the symbol for an anonymous function, a function without a name. every time you use an anonymous function, you need to include its whole body. thus, (lambda(arg) (/ arg 50)) is a function definition that says 'return the value resulting from dividing whatever is passed to me as arg by 50'\n\n\
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
	}
	else if (mx_text_compare_string(ival, "test-help") == 0)
	{
		mx_print_text("     available commands:\n     test-mem\n     test-text\n     test-vect\n     test-list\n     test-htable\n     test-lisp\n     test-all\n");
	}
	else if (mx_text_compare_string(ival, "test-mem") == 0)
	{
		test_mx_mem();
	}
	else if (mx_text_compare_string(ival, "test-text") == 0)
	{
		test_mx_text();
	}
	else if (mx_text_compare_string(ival, "test-vect") == 0)
	{
		test_mx_vect();
	}
	else if (mx_text_compare_string(ival, "test-list") == 0)
	{
		test_mx_list();
	}
	else if (mx_text_compare_string(ival, "test-htable") == 0)
	{
		test_mx_htable();
	}
	else if (mx_text_compare_string(ival, "test-lisp") == 0)
	{
		test_mx_lisp(ienv);
	}
	else if (mx_text_compare_string(ival, "test-all") == 0)
	{
		test_all(ienv);
	}
	else if (mx_text_compare_string(ival, "status-help") == 0)
	{
		mx_print_text("     available commands:\n     status-memory\n     status-objects\n");
	}
	else if (mx_text_compare_string(ival, "status-memory") == 0)
	{
		mx_print_text("status-memory\n");
	}
	else if (mx_text_compare_string(ival, "status-objects") == 0)
	{
		mx_print_text("status-objects\n");
	}
	else
	{
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

const struct mx_elem* eval_symbol(const mx_text* tx, struct mx_env* env)
{
	// return ptr
	const struct mx_elem* xv = mx_env_get(env, tx);

	check_obj_status(xv);

	return xv;
}

const struct mx_elem* eval_number(const struct mx_elem* x, struct mx_env* env)
{
	check_obj_status(x);

	// return ptr
	return x;
}

const struct mx_elem* eval_quote(const struct mx_elem* x, struct mx_env* env)
{
	// (quote exp)
	// return ptr
	const struct mx_elem* xv = (struct mx_elem*)mx_vect_elem_at(x->list, 1);

	check_obj_status(xv);

	return xv;
}

const struct mx_elem* eval_if(const struct mx_elem* x, struct mx_env* env, int xls)
{
	// (if test conseq [alt])
	// local ptr
	const struct mx_elem* xv = (struct mx_elem*)mx_vect_elem_at(x->list, 1);
	const struct mx_elem* cc = 0;
	assign_smart_ptr((void**)&cc, eval(xv, env));
	struct mx_elem* cr = 0;

	check_obj_status(cc);
	if (cc->val && mx_text_compare_string(cc->val, "#f") == 0)
	{
		if (xls < 4)
		{
			cr = nil_sym;
		}
		else
		{
			cr = (struct mx_elem*)mx_vect_elem_at(x->list, 3);
		}
	}
	else
	{
		cr = (struct mx_elem*)mx_vect_elem_at(x->list, 2);
	}

	assign_smart_ptr((void**)&cc, 0);

	// return ptr
	const struct mx_elem* xr = eval(cr, env);

	check_obj_status(xr);

	return xr;
}

const struct mx_elem* eval_setq(const struct mx_elem* x, struct mx_env* env)
{
	// (set! var exp)
	// As a practical matter, you almost always quote the first argument to set.
	// The combination of set and a quoted first argument is so common that it has its own name: the special form set!.
	// This special form is just like set except that the first argument is quoted automatically, so you don't need to type the quote mark yourself.
	// Also, as an added convenience, set! permits you to set several different variables to different values, all in one expression.
	// local ptr
	const struct mx_elem* xl1 = (struct mx_elem*)mx_vect_elem_at(x->list, 1);
	const struct mx_elem* c2 = (struct mx_elem*)mx_vect_elem_at(x->list, 2);
	// return ptr
	const struct mx_elem* cc = eval(c2, env);

	mx_env_put(env, xl1->val, cc);

	check_obj_status(cc);

	return cc;
}

const struct mx_elem* eval_define(const struct mx_elem* x, struct mx_env* env)
{
	// (define var exp)
	// defun (short for "define function") is a macro in the Lisp family of programming languages that defines a function in the global environment that uses the form:
	// (defun <function - name>(<parameter1><parameter2>...<parameterN>) functionbody)
	// local ptr
	const struct mx_elem* c2 = (struct mx_elem*)mx_vect_elem_at(x->list, 2);
	const struct mx_elem* xl1 = (struct mx_elem*)mx_vect_elem_at(x->list, 1);
	// return ptr
	const struct mx_elem* cr = eval(c2, env);

	mx_env_put(env, xl1->val, cr);

	check_obj_status(cr);

	return cr;
}

const struct mx_elem* eval_lambda(const struct mx_elem* x, struct mx_env* env)
{
	// (lambda (var*) exp)
	// keep a reference to the environment that exists now (when the
	// lambda is being defined) because that's the outer environment
	// we'll need to use when the lambda is executed

	// return ptr
	struct mx_elem* xr = elem_copy(x);

	xr->type = t_lambda;
	assign_smart_ptr((void**)&xr->env, env);

	check_obj_status(xr);

	return xr;
}

const struct mx_elem* eval_begin(const struct mx_elem* x, struct mx_env* env, int xls)
{
	// (begin exp*)
	// "begin" is a special form that causes each of its arguments to be evaluated in sequence and then returns the value of the last one.
	// The preceding expressions are evaluated only for the side effects they perform.
	// The values produced by them are discarded.

	for (int i = 1; i < xls - 1; ++i)
	{
		// local ptr
		const struct mx_elem* xv = (struct mx_elem*)mx_vect_elem_at(x->list, i);
		const struct mx_elem* cc = 0;
		assign_smart_ptr((void**)&cc, eval(xv, env));

		assign_smart_ptr((void**)&cc, 0);
	}

	// local ptr
	const struct mx_elem* xv = (struct mx_elem*)mx_vect_elem_at(x->list, xls - 1);
	// return  ptr
	const struct mx_elem* xr = eval(xv, env);

	check_obj_status(xr);

	return xr;
}

void eval_init_fun_exps(struct mx_elem** funp, mx_vect** expsp, const struct mx_elem* x, int xls, const struct mx_elem* xl0, struct mx_env* env)
{
	// (fun exp*)
	// local ptr
	assign_smart_ptr((void**)funp, eval(xl0, env));
	assign_smart_ptr((void**)expsp, mx_vect_ctor(1, 1));

	for (int k = 1; k < xls; k++)
	{
		// local ptr
		const struct mx_elem* c = (struct mx_elem*)mx_vect_elem_at(x->list, k);
		const struct mx_elem* xc = eval(c, env);

		check_obj_status(xc);
		mx_vect_add_elem(*expsp, (mx_smart_ptr*)xc);
	}
}

const struct mx_elem* eval_fun_type_lambda(struct mx_elem** funp, mx_vect** expsp)
{
	// Create an environment for the execution of this lambda function
	// where the outer environment is the one that existed* at the time
	// the lambda was defined and the new inner associations are the
	// parameter names with the given arguments.
	// *Although the environment existed at the time the lambda was defined
	// it wasn't necessarily complete - it may have subsequently had
	// more symbols defined in that environment.
	// local ptr
	struct mx_elem* fun = *funp;
	mx_vect* exps = *expsp;
	const struct mx_elem* pl1 = (struct mx_elem*)mx_vect_elem_at(fun->list, 1);
	const struct mx_elem* pl2 = (struct mx_elem*)mx_vect_elem_at(fun->list, 2);
	struct mx_env* env = 0;
	assign_smart_ptr((void**)&env, mx_env_ctor_abc(pl1->list, exps, fun->env));
	// return ptr
	const struct mx_elem* xr = eval(pl2, env);

	check_obj_status(xr);
	assign_smart_ptr((void**)&env, 0);
	check_obj_status(xr);
	check_ref_count(exps);
	check_obj_status(xr);
	check_ref_count(fun);

	inc_ref_count(xr);
	assign_smart_ptr((void**)funp, 0);
	assign_smart_ptr((void**)expsp, 0);
	dec_ref_count(xr);
	check_obj_status(xr);

	return xr;
}

const struct mx_elem* eval_fun_type_function(struct mx_elem** funp, mx_vect** expsp)
{
	struct mx_elem* fun = *funp;
	mx_vect* exps = *expsp;

	for (int k = 0; k < mx_vect_size(exps); k++)
	{
		struct mx_elem* c = (struct mx_elem*)mx_vect_elem_at(exps, k);
		check_obj_status(c);
	}

	// return ptr
	const struct mx_elem* cr = fun->fun(exps);

	check_ref_count(exps);
	check_ref_count(fun);

	check_obj_status(cr);
	assign_smart_ptr((void**)&fun, 0);
	assign_smart_ptr((void**)&exps, 0);

	return cr;
}

const struct mx_elem* eval_undefined_symbol()
{
	mx_text* tx = 0;
	assign_smart_ptr((void**)&tx, mx_text_append_string_string("undefined symbol ", tx->text));
	mx_char_copy_abc(tx->text, global_input, GLOBAL_INPUT_LENGTH);
	assign_smart_ptr((void**)&tx, 0);

	// return ptr
	return mx_elem_ctor_ab2(t_symbol, global_input);
}

const struct mx_elem* eval_new_symbol(const struct mx_elem* x)
{
	mx_text* tx = 0;
	assign_smart_ptr((void**)&tx, mx_text_append_string_string("undefined symbol. elem type is: ", get_elem_type(x)));
	mx_char_copy_abc(tx->text, global_input, GLOBAL_INPUT_LENGTH);
	assign_smart_ptr((void**)&tx, 0);

	// return ptr
	return mx_elem_ctor_ab2(t_symbol, global_input);
}


// eval function body
const struct mx_elem* eval(const struct mx_elem* iel, struct mx_env* env)
{
	mx_text* tx = iel->val;

	if (tx && print_help(tx, env))
	{
		return mx_elem_ctor_ab2(t_symbol, "");
	}

	if (tx && mx_text_compare_string(tx, "exit") == 0)
	{
		return eval_exit();
	}

	if (iel->type == t_symbol)
	{
		return eval_symbol(tx, env);
	}

	if (iel->type == t_number)
	{
		return eval_number(iel, env);
	}

	if (mx_vect_is_empty(iel->list))
	{
		return nil_sym;
	}

	const struct mx_elem* xl0 = (struct mx_elem*)mx_vect_elem_at(iel->list, 0);
	int xls = mx_vect_size(iel->list);
	mx_text* xl0_tx = xl0->val;

	if (xl0->type == t_symbol && xl0_tx)
	{
		if (mx_text_compare_string(xl0_tx, "quote") == 0)
		{
			return eval_quote(iel, env);
		}

		if (mx_text_compare_string(xl0_tx, "if") == 0)
		{
			return eval_if(iel, env, xls);
		}

		if (mx_text_compare_string(xl0_tx, "set!") == 0)
		{
			return eval_setq(iel, env);
		}

		if (mx_text_compare_string(xl0_tx, "define") == 0)
		{
			return eval_define(iel, env);
		}

		if (mx_text_compare_string(xl0_tx, "lambda") == 0)
		{
			return eval_lambda(iel, env);
		}

		if (mx_text_compare_string(xl0_tx, "begin") == 0)
		{
			return eval_begin(iel, env, xls);
		}
	}

	// evaluate lambda and built-in functions
	{
		struct mx_elem* fun = 0;
		mx_vect* exps = 0;

		eval_init_fun_exps(&fun, &exps, iel, xls, xl0, env);

		if (fun->type == t_lambda)
		{
			return eval_fun_type_lambda(&fun, &exps);
		}

		if (fun->type == t_function)
		{
			return eval_fun_type_function(&fun, &exps);
		}

		// unreference fun & exps
		assign_smart_ptr((void**)&fun, 0);
		assign_smart_ptr((void**)&exps, 0);
	}

	if (iel->type == t_symbol)
	{
		return eval_undefined_symbol();
	}

	return eval_new_symbol(iel);
}


////////////////////// parse, read and user interaction

// convert given string to list of tokens
mx_vect* tokenize(const mx_text* str)
{
	mx_vect* tokens = mx_vect_ctor(1, 1);
	const char* s = str->text;

	while (*s)
	{
		while (*s == ' ')
		{
			++s;
		}

		if (*s == '(' || *s == ')')
		{
			mx_text* tx = 0;

			if (*s == '(')
			{
				tx = mx_text_ctor("(");
			}
			else
			{
				tx = mx_text_ctor(")");
			}

			mx_vect_add_elem(tokens, (mx_smart_ptr*)tx);
			s++;
		}
		else
		{
			const char* t = s;

			while (*t && *t != ' ' && *t != '(' && *t != ')')
			{
				++t;
			}

			int delta = t - s;
			mx_text* tx = mx_text_ctor_ab(s, delta);

			mx_vect_add_elem(tokens, (mx_smart_ptr*)tx);
			s = t;
		}
	}

	return tokens;
}

// numbers become Numbers; every other token is a Symbol
struct mx_elem* atom(mx_text* tokens)
{
	char* token = tokens->text;

	if (is_digit(token[0]) || (token[0] == '-' && is_digit(token[1])))
	{
		return mx_elem_ctor_ab(t_number, tokens);
	}

	return mx_elem_ctor_ab(t_symbol, tokens);
}

// return the lisp expression in the given tokens
struct mx_elem* read_from(mx_vect* tokens)
{
	mx_text* token = 0;
	assign_smart_ptr((void**)&token, (mx_text*)mx_vect_front(tokens));

	mx_vect_pop_front(tokens);

	if (mx_text_compare_string(token, "(") == 0)
	{
		// return ptr
		struct mx_elem* c = mx_elem_ctor(t_list);
		mx_text* token2 = (mx_text*)mx_vect_front(tokens);

		if (token2 == 0)
		{
			// cleanup
			assign_smart_ptr((void**)&token, 0);
			mx_elem_dtor(c);

			return 0;
		}

		while (mx_text_compare_string(token2, ")") != 0)
		{
			// local ptr
			struct mx_elem* c2 = read_from(tokens);

			if (c2 == 0)
			{
				// cleanup
				assign_smart_ptr((void**)&token, 0);
				mx_elem_dtor(c);

				return 0;
			}

			mx_vect_add_elem(c->list, (mx_smart_ptr*)c2);

			if (mx_vect_size(tokens) <= 0)
			{
				// cleanup
				assign_smart_ptr((void**)&token, 0);
				mx_elem_dtor(c);

				return 0;
			}

			token2 = (mx_text*)mx_vect_front(tokens);

			if (token2 == 0)
			{
				// cleanup
				assign_smart_ptr((void**)&token, 0);
				mx_elem_dtor(c);

				return 0;
			}
		}

		mx_vect_pop_front(tokens);
		// cleanup
		assign_smart_ptr((void**)&token, 0);

		return c;
	}

	// return ptr
	struct mx_elem* rc = atom(token);
	// cleanup
	assign_smart_ptr((void**)&token, 0);

	return rc;
}

// return the Lisp expression represented by the given string
struct mx_elem* read(const mx_text* s)
{
	mx_vect* tokens = 0;
	assign_smart_ptr((void**)&tokens, tokenize(s));

	if (mx_vect_size(tokens) > 0)
	{
		struct mx_elem* rc = read_from(tokens);
		assign_smart_ptr((void**)&tokens, 0);

		return rc;
	}

	assign_smart_ptr((void**)&tokens, 0);
	return 0;
}

// convert given elem to a Lisp-readable string
mx_text* to_string(const struct mx_elem* exp)
{
	if (exp->type == t_list)
	{
		mx_text* s = 0;
		assign_smart_ptr((void**)&s, mx_text_ctor("("));
		int length = mx_vect_size(exp->list);

		for (int k = 0; k < length; k++)
		{
			// local ptr
			const struct mx_elem* c = (const struct mx_elem*)mx_vect_elem_at(exp->list, k);
			mx_text* t = 0;

			//s += to_string(c) + ' ';
			assign_smart_ptr((void**)&t, to_string(c));
			assign_smart_ptr((void**)&s, mx_text_append(s, t));
			assign_smart_ptr((void**)&s, mx_text_append_string(s, " "));
			assign_smart_ptr((void**)&t, 0);
		}

		char cc = mx_text_char_at(s, mx_text_length(s) - 1);

		if (cc == ' ')
		{
			int pos = mx_text_length(s) - 1;
			//s.erase(pos);
			assign_smart_ptr((void**)&s, mx_text_erase(s, pos));
		}

		//return s + ')';
		assign_smart_ptr((void**)&s, mx_text_append_string(s, ")"));

		// returning a smart pointer. reset ref count
		reset_ref_count(s);

		return s;
	}
	else if (exp->type == t_lambda)
	{
		return mx_text_ctor("<lambda>");
	}
	else if (exp->type == t_function)
	{
		return mx_text_ctor("<function>");
	}

	return exp->val;
}



// the default read-eval-print-loop
void run_mx_lisp_repl()
{
	mx_print_text("initializing memory context... please wait\n");
	mx_mem_init_ctx();

	mx_print_text("starting lisp repl...\ntype 'help' to list available commands\n\n");
	global_env_vect = mx_vect_ctor(1, 1);

	assign_smart_ptr((void**)&false_sym, mx_elem_ctor_ab2(t_symbol, "#f"));
	assign_smart_ptr((void**)&true_sym, mx_elem_ctor_ab2(t_symbol, "#t"));
	assign_smart_ptr((void**)&nil_sym, mx_elem_ctor_ab2(t_symbol, "nil"));

	int size = mx_vect_size(global_env_vect);

	for (int k = 0; k < size; k++)
	{
		struct mx_env* e = (struct mx_env*)mx_vect_elem_at(global_env_vect, k);

		if (e->ref_count > 1)
		{
			int x = 3;
		}
		//reset_ref_count(e);
		e->ext_env = 0;
	}
	mx_vect_del_all_elem(global_env_vect);

	assign_smart_ptr((void**)&global_env, mx_env_ctor(0));
	char prompt[] = "mxl> ";
	char indent[] = "     ";

	add_globals(global_env);
	global_done = false;

	while (!global_done)
	{
		mx_print_text(prompt);
		mx_read_text_line();

		mx_text* linetx = 0;
		struct mx_elem* c = 0;
		struct mx_elem* ce = 0;
		mx_text* ce_text = 0;
		assign_smart_ptr((void**)&linetx, mx_text_ctor(global_input));
		assign_smart_ptr((void**)&c, read(linetx));

		if (c != 0)
		{
			assign_smart_ptr((void**)&ce, eval(c, global_env));
			assign_smart_ptr((void**)&ce_text, to_string(ce));

			mx_print_text(indent);
			mx_print_text(ce_text->text);
		}
		else
		{
			mx_print_text(indent);
			mx_print_text("invalid expression");
		}

		mx_print_text("\n");

		assign_smart_ptr((void**)&linetx, 0);
		assign_smart_ptr((void**)&c, 0);
		assign_smart_ptr((void**)&ce, 0);
		assign_smart_ptr((void**)&ce_text, 0);
	}

	assign_smart_ptr((void**)&false_sym, 0);
	assign_smart_ptr((void**)&true_sym, 0);
	assign_smart_ptr((void**)&nil_sym, 0);
	mx_env_dtor(global_env);

	mx_vect_dtor(global_env_vect);

	{
		mx_text* t1 = 0;
		mx_text* t2 = 0;
		mx_text* t3 = 0;
		mx_text* t4 = 0;
		assign_smart_ptr((void**)&t1, number_to_text(elem_created));
		assign_smart_ptr((void**)&t2, number_to_text(elem_destroyed));
		assign_smart_ptr((void**)&t3, number_to_text(env_created));
		assign_smart_ptr((void**)&t4, number_to_text(env_destroyed));

		mx_print_text("\nelements created: ");
		mx_print_text(t1->text);
		mx_print_text("\nelements destroyed: ");
		mx_print_text(t2->text);
		mx_print_text("\nenvironments created: ");
		mx_print_text(t3->text);
		mx_print_text("\nenvironments destroyed: ");
		mx_print_text(t4->text);
		mx_print_text("\n");

		assign_smart_ptr((void**)&t1, 0);
		assign_smart_ptr((void**)&t2, 0);
		assign_smart_ptr((void**)&t3, 0);
		assign_smart_ptr((void**)&t4, 0);
	}

	mx_mem_destroy_ctx();
}



//
// error signaling function impl
//

void mx_signal_error(const char* ierror_msg)
{
	mx_print_text("error-");
	mx_print_text(ierror_msg);
	mx_print_text("\n");

	ia_signal_error();
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
#else
	fgets(global_input, GLOBAL_INPUT_LENGTH, stdin);
#endif

	int i = mx_char_length(global_input) - 1;

	if (global_input[i] == '\n')
	{
		global_input[i] = 0;
	}
}

void mx_print_text(const char* itext)
{
#ifdef STANDALONE
	//global_output = (char*)itext;
	//asm
	//(
	//	"LDA $255,global_output\n\t"
	//	"TRAP 0,Fputs,StdOut\n\t"
	//);
	fputs(itext,stdout);
#else
	vprint(itext);
#endif
}

void mx_print_indent(int ilevel)
{
	for (int k = 0; k < ilevel; k++)
	{
		mx_print_text(" ");
	}
}



//
// memory management functions impl
//

void mx_mem_init_ctx()
{
	ia_assert(MX_CTX_ELEM_SIZE >= MIN_ELEM_SIZE);
	ia_assert(MIN_ELEM_SIZE % 8 == 0);
	mxmc_inst.elem_size = MX_CTX_ELEM_SIZE;
	ia_assert(MX_CTX_ELEM_COUNT > 0 && MX_CTX_ELEM_COUNT < MAX_ELEM_COUNT);
	mxmc_inst.elem_count = MX_CTX_ELEM_COUNT;
	mxmc_inst.total_size = mxmc_inst.elem_size * mxmc_inst.elem_count;
	ia_assert(mxmc_inst.total_size % 8 == 0);
	// align on 64 bits
	mxmc_inst.mem_start = heap_mem;

	mx_mem_clear(block_list_mem, sizeof(struct mx_mem_block) * MAX_ELEM_COUNT);
	mx_mem_clear(mxmc_inst.mem_start, mxmc_inst.elem_size * mxmc_inst.elem_count);

	// align on 64 bits
	ia_assert((mxmc_inst.elem_count * sizeof(struct mx_mem_block)) % 8 == 0);
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

void* mx_mem_alloc(int isize_in_bytes)
{
	if (isize_in_bytes <= 0)
	{
		mx_signal_error("mx_mem_alloc: allocated size must be positive");

		return 0;
	}

	// how many elements will this request take
	int elem_span = isize_in_bytes / mxmc_inst.elem_size;
	int elem_index = 0;
	struct mx_mem_block* block = mxmc_inst.block_list;

	if (isize_in_bytes % mxmc_inst.elem_size != 0)
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

void* mx_mem_realloc(const void* iaddress, int isize_in_bytes)
{
	if (iaddress == 0)
	{
		mx_signal_error("mx_mem_realloc: address is 0");

		return 0;
	}

	if (isize_in_bytes <= 0)
	{
		mx_signal_error("mx_mem_realloc: reallocated size must be positive");

		return 0;
	}

	if (iaddress < (void*)mxmc_inst.mem_start || (iaddress >= (void*)(mxmc_inst.mem_start + mxmc_inst.elem_size * mxmc_inst.elem_count)))
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

		if (current_address == iaddress)
			// find out how many bytes we have to copy
		{
			void* new_address = 0;
			int old_size = block->elem_span * mxmc_inst.elem_size;
			int bytes_to_copy = isize_in_bytes;

			if (old_size < bytes_to_copy)
			{
				bytes_to_copy = old_size;
			}

			new_address = mx_mem_alloc(isize_in_bytes);
			mx_mem_copy(iaddress, new_address, bytes_to_copy);
			mx_mem_free(iaddress);

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

void mx_mem_free(const void* iaddress)
{
	if (iaddress == 0)
	{
		mx_signal_error("mx_mem_free: address is 0");

		return;
	}

	int* ps = (int*)mxmc_inst.mem_start;
	int* pe = (int*)(mxmc_inst.mem_start + mxmc_inst.elem_size * mxmc_inst.elem_count);

	if (iaddress < (void*)mxmc_inst.mem_start || (iaddress >= (void*)(mxmc_inst.mem_start + mxmc_inst.elem_size * mxmc_inst.elem_count)))
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

		if (current_address == iaddress)
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

void mx_mem_copy(const void* isrc, void* idst, int isize)
{
	ia_assert((isrc != 0) && (idst != 0));

	char* src = (char*)isrc;
	char* dst = (char*)idst;

	for (int k = 0; k < isize; k++)
	{
		*dst++ = *src++;
	}
}

void mx_mem_clear(void* idst, int isize)
{
	ia_assert(idst != 0);
	ia_assert(isize % 8 == 0);

	int size = isize / 8;

	uint64* dst = (uint64*)idst;

	for (int k = 0; k < size; k++)
	{
		*dst++ = (uint64)0;
	}
}



//
// reference counting functions
//

void check_obj_status(const void* iobj_ptr)
{
	if (iobj_ptr != 0)
	{
		mx_smart_ptr* ptr = (mx_smart_ptr*)iobj_ptr;

		if (ptr->destructor_ptr == 0)
		{
			mx_signal_error("check_obj_status: obj destroyed");
		}
	}
}

void check_ref_count(const void* iobj_ptr)
{
	if (iobj_ptr != 0)
	{
		mx_smart_ptr* ptr = (mx_smart_ptr*)iobj_ptr;

		if (ptr->ref_count == 0)
			// destroy objects with 0 references
		{
			ptr->destructor_ptr(ptr);
		}
	}
}

void inc_ref_count(const void* iobj_ptr)
{
	if (iobj_ptr != 0)
	{
		mx_smart_ptr* ptr = (mx_smart_ptr*)iobj_ptr;

		ptr->ref_count++;
	}
}

void dec_ref_count(const void* iobj_ptr)
{
	if (iobj_ptr != 0)
	{
		mx_smart_ptr* ptr = (mx_smart_ptr*)iobj_ptr;

		if (ptr->ref_count > 0)
		{
			ptr->ref_count--;
		}
	}
}

void reset_ref_count(const void* iobj_ptr)
{
	if (iobj_ptr != 0)
	{
		mx_smart_ptr* ptr = (mx_smart_ptr*)iobj_ptr;

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

int mx_char_length(const char* itext)
{
	int length = 0;

	if (itext)
	{
		while (itext[length] != 0)
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

char* mx_char_copy_abc(const char* isrc, char* idst, int imax_length)
{
	ia_assert((isrc != 0) && (idst != 0));

	char* str = idst;
	int idx = 0;

	while ((*idst++ = *isrc++) != 0 && idx < imax_length)
	{
		idx++;
	}

	str[idx] = 0;

	return str;
}

char* mx_char_copy(const char* isrc, char* idst)
{
	ia_assert((isrc != 0) && (idst != 0));

	char* str = idst;

	while ((*idst++ = *isrc++) != 0);

	return str;
}

char* mx_char_clone_ab(const char* isrc, int imax_length)
{
	ia_assert(isrc != 0);

	int isrc_length = mx_char_length(isrc);
	int length = imax_length;

	if (length > isrc_length)
	{
		length = isrc_length;
	}

	char* clone = (char*)mx_mem_alloc(length + 1);

	mx_char_copy_abc(isrc, clone, length);

	return clone;
}

char* mx_char_clone(const char* isrc)
{
	ia_assert(isrc != 0);

	char* clone = (char*)mx_mem_alloc(mx_char_length(isrc) + 1);

	mx_char_copy(isrc, clone);

	return clone;
}


int mx_char_compare(const char* itext_left, const char* itext_right)
{
	ia_assert((itext_left != 0) && (itext_right != 0));

	while ((*itext_left != 0 && *itext_right != 0) && (*itext_left == *itext_right))
	{
		itext_left++;
		itext_right++;
	}

	return *itext_left - *itext_right;
}

char* mx_char_append(const char* itext, const char* iappend)
{
	int itext_length = mx_char_length(itext);
	int iappend_length = mx_char_length(iappend);
	char* result = (char*)mx_mem_alloc(itext_length + iappend_length + 1);
	char* insert = result + itext_length;

	mx_char_copy(itext, result);
	mx_char_copy(iappend, insert);

	return result;
}

void mx_char_reverse(char* s)
{
	int i = 0;
	int j = mx_char_length(s) - 1;

	for (; i < j; i++, j--)
	{
		char c = s[i];

		s[i] = s[j];
		s[j] = c;
	}
}



//
// mx_text impl
//

int size_of_text()
{
	return sizeof(mx_text);
}

mx_text* mx_text_ctor_ab(const char* itext, int imax_length)
{
	mx_text* tx = (mx_text*)mx_mem_alloc(size_of_text());

	tx->destructor_ptr = mx_text_dtor;
	tx->ref_count = 0;

	if (itext != 0 && imax_length > 0)
	{
		tx->text = mx_char_clone_ab(itext, imax_length);
	}
	else
	{
		tx->text = mx_char_clone("");
	}

	return tx;
}

mx_text* mx_text_ctor(const char* itext)
{
	return mx_text_ctor_ab(itext, mx_char_length(itext));
}

void mx_text_dtor(void* itext)
{
	if (itext == 0)
	{
		return;
	}

	mx_text* tx = (mx_text*)itext;

	mx_mem_free(tx->text);
	mx_mem_free(tx);
}

int mx_text_length(const mx_text* itext)
{
	return mx_char_length(itext->text);
}

char mx_text_char_at(const mx_text* itext, int iidx)
{
	ia_assert(iidx < mx_text_length(itext));

	return itext->text[iidx];
}

mx_text* mx_text_erase(const mx_text* itext, int ipos)
{
	int length = mx_text_length(itext);

	if (ipos >= length)
	{
		return 0;
	}

	return mx_text_ctor_ab(itext->text, ipos);
}

mx_text* mx_text_append(const mx_text* itext, const mx_text* iappend)
{
	return mx_text_append_string(itext, iappend->text);
}

mx_text* mx_text_append_string(const mx_text* itext, const char* iappend)
{
	return mx_text_append_string_string(itext->text, iappend);
}

mx_text* mx_text_append_string_string(const char* itext, const char* iappend)
{
	char* cat = mx_char_append(itext, iappend);
	mx_text* tx = mx_text_ctor(cat);

	mx_mem_free(cat);

	return tx;
}

int mx_text_compare(const mx_text* itext_left, const mx_text* itext_right)
{
	return mx_char_compare(itext_left->text, itext_right->text);
}

int mx_text_compare_string(const mx_text* itext_left, const char* itext_right)
{
	return mx_char_compare(itext_left->text, itext_right);
}



//
// utility functions impl
//

mx_text* number_to_text(numeric_dt inr)
{
	// max digits + 1 for 64 bits
	char* str = (char*)mx_mem_alloc(21);
	int i = 0;
	bool is_negative = false;

	// handle 0 explicitly
	if (inr == 0)
	{
		str[i++] = '0';
		str[i] = 0;
	}
	else
	{
		if (inr < 0)
		{
			is_negative = true;
			inr = -inr;
		}

		// funess individual digits
		while (inr != 0)
		{
			int rem = inr % 10;

			str[i++] = rem + '0';
			inr = inr / 10;
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


	mx_text* tx = mx_text_ctor(str);
	mx_mem_free(str);

	return tx;
}

numeric_dt text_to_number(const mx_text* n)
{
	const char* str = n->text;
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

bool is_digit(char c)
{
	return c >= '0' && c <= '9';
}



//
// mx_vect impl
//

int size_of_mx_vect()
{
	return sizeof(mx_vect);
}

mx_vect* mx_vect_ctor(int iinitial_capacity, int icapacity_increment)
{
	ia_assert(iinitial_capacity > 0);

	mx_vect* iv = (mx_vect*)mx_mem_alloc(size_of_mx_vect());

	iv->destructor_ptr = mx_vect_dtor;
	iv->ref_count = 0;

	iv->elem_vect_length = iinitial_capacity;
	iv->capacity_increment = icapacity_increment;
	iv->elem_count = 0;
	iv->elem_vect = (mx_smart_ptr**)mx_mem_alloc(iinitial_capacity * sizeof(uint64*));

	return iv;
}

void mx_vect_dtor(void* iv)
{
	if (iv == 0)
	{
		return;
	}

	mx_vect* v = (mx_vect*)iv;

	mx_vect_del_all_elem(v);
	mx_mem_free(v->elem_vect);
	mx_mem_free(v);
}

mx_smart_ptr* mx_vect_front(mx_vect* iv)
{
	if (iv->elem_count == 0)
	{
		mx_signal_error("mx_vect_first_elem: vector is empty");

		return 0;
	}
	else
	{
		return iv->elem_vect[0];
	}
}

mx_smart_ptr* mx_vect_elem_at(const mx_vect* iv, int iindex)
{
	if (iindex < 0 || iindex >= iv->elem_count)
	{
		mx_signal_error("mx_vect_elem_at: index out of bounds");

		return 0;
	}

	return iv->elem_vect[iindex];
}

int mx_vect_size(const mx_vect* iv)
{
	return iv->elem_count;
}

bool mx_vect_is_empty(mx_vect* iv)
{
	return iv->elem_count == 0;
}

void mx_vect_reallocate(mx_vect* iv)
{
	int new_size = iv->elem_count;

	if (iv->capacity_increment > 0)
	{
		new_size = new_size + iv->capacity_increment;
	}
	else
	{
		new_size = new_size + iv->elem_count;
	}

	iv->elem_vect_length = new_size;
	iv->elem_vect = (mx_smart_ptr**)mx_mem_realloc(iv->elem_vect, new_size * sizeof(uint64*));
}

void mx_vect_add_elem(mx_vect* iv, mx_smart_ptr* ielem)
{
	if (iv->elem_count >= iv->elem_vect_length)
	{
		mx_vect_reallocate(iv);
	}

	iv->elem_vect[iv->elem_count] = 0;
	assign_smart_ptr((void**)&iv->elem_vect[iv->elem_count], ielem);
	iv->elem_count++;
}

void mx_vect_del_all_elem(mx_vect* iv)
{
	for (int k = 0; k < iv->elem_count; k++)
	{
		assign_smart_ptr((void**)&iv->elem_vect[k], 0);
	}

	iv->elem_count = 0;
}

void mx_vect_del_elem_at(mx_vect* iv, int iindex)
{
	if (iindex < 0 || iindex >= iv->elem_count)
	{
		mx_signal_error("mx_vect_del_elem_at: index out of bounds");

		return;
	}

	// arraycopy(elem_vect, iindex + 1, elem_vect, iindex, elem_count - 1 - iindex);
	mx_smart_ptr** src = iv->elem_vect + iindex + 1;
	mx_smart_ptr** dst = iv->elem_vect + iindex;
	int size = (iv->elem_count - 1 - iindex) * sizeof(uint64*);

	assign_smart_ptr((void**)&iv->elem_vect[iindex], 0);
	mx_mem_copy(src, dst, size);
	iv->elem_count--;
}

void mx_vect_pop_front(mx_vect* iv)
{
	mx_vect_del_elem_at(iv, 0);
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

struct mx_ht_entry* mx_ht_entry_ctor(const mx_text* ikey, const void* ivalue);
void mx_ht_entry_dtor(struct mx_ht_entry* ihte);
const void* mx_ht_entry_put(struct mx_ht_entry* ihte, const mx_text* ikey, const void* ivalue);
const void* mx_ht_entry_get(struct mx_ht_entry* ihte, const mx_text* ikey);
bool mx_ht_entry_contains_key(struct mx_ht_entry* ihte, const mx_text* ikey);
bool mx_ht_entry_del(struct mx_ht_entry* ihte, const mx_text* ikey);

int size_of_mx_htable()
{
	return sizeof(mx_htable);
}

mx_htable* mx_htable_ctor(int icapacity)
{
	mx_htable* iht = (mx_htable*)mx_mem_alloc(size_of_mx_htable());

	iht->destructor_ptr = mx_htable_dtor;
	iht->ref_count = 0;

	iht->capacity = icapacity;
	iht->entries = (struct mx_ht_entry**)mx_mem_alloc(size_of_mx_ht_entry());

	for (int k = 0; k < iht->capacity; k++)
	{
		iht->entries[k] = 0;
	}

	return iht;
}

void mx_htable_dtor(void* iht)
{
	if (iht == 0)
	{
		return;
	}

	mx_htable* ht = (mx_htable*)iht;

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

int mx_htable_get_hash(const char* itext)
{
	int h = 5381;
	int length = mx_char_length(itext);

	for (int k = 0; k < length; k++)
	{
		h = ((h << 5) + h) + itext[k];
	}

	return h & 0x7fffffff;
}

/**
*
* @param key
* @param value
* @return previous value if it exists, null otherwise
*/
const void* mx_htable_put(mx_htable* iht, const mx_text* ikey, const void* ivalue)
{
	if (ikey == 0)
	{
		mx_signal_error("mx_htable_add: ikey is null");

		return 0;
	}

	int index = mx_htable_get_hash(ikey->text) % iht->capacity;
	struct mx_ht_entry* e = iht->entries[index];

	if (e == 0)
	{
		iht->entries[index] = mx_ht_entry_ctor(ikey, ivalue);
	}
	else
	{
		return mx_ht_entry_put(e, ikey, ivalue);
	}

	return 0;
}

/**
*
* @param key
* @return value if key exists, null otherwise
*/
const void* mx_htable_get(mx_htable* iht, const mx_text* ikey)
{
	if (ikey == 0)
	{
		mx_signal_error("mx_htable_get: ikey is null");

		return 0;
	}

	int index = mx_htable_get_hash(ikey->text) % iht->capacity;
	struct mx_ht_entry* e = iht->entries[index];

	if (e == 0)
	{
		return 0;
	}

	return mx_ht_entry_get(e, ikey);
}

/**
*
* @param ikey
* @return true if element corresponding to key has been deleted, false othewise
*/
bool mx_htable_del(mx_htable* iht, const mx_text* ikey)
{
	if (ikey == 0)
	{
		mx_signal_error("mx_htable_del: ikey is null");

		return 0;
	}

	int index = mx_htable_get_hash(ikey->text) % iht->capacity;
	struct mx_ht_entry* e = iht->entries[index];

	if (e != 0)
		// if we have to delete first entry from the list
	{
		if (mx_text_compare(ikey, e->key) == 0)
			// if this is the first element in the list, delete it and return
		{
			if (e->next == 0)
			{
				mx_ht_entry_dtor(iht->entries[index]);
				iht->entries[index] = 0;

				return true;
			}
			else
				// set the next entry to be head of the list and delete current head of the list
			{
				iht->entries[index] = e->next;
				mx_ht_entry_del(e, ikey);
			}
		}

		return mx_ht_entry_del(e, ikey);
	}

	return false;
}

/**
* implement a linked list for internal use by mx_hashtable
*/
struct mx_ht_entry* mx_ht_entry_ctor(const mx_text* ikey, const void* ivalue)
{
	struct mx_ht_entry* ihte = (struct mx_ht_entry*)mx_mem_alloc(size_of_mx_ht_entry());

	ihte->key = 0;
	assign_smart_ptr((void**)&ihte->key, ikey);
	ihte->value = 0;
	assign_smart_ptr((void**)&ihte->value, ivalue);
	ihte->next = 0;

	return ihte;
}

void mx_ht_entry_dtor(struct mx_ht_entry* ihte)
{
	while (ihte != 0)
	{
		struct mx_ht_entry* t = ihte->next;

		//mx_print_text("mx_ht_entry_dtor ");
		//mx_print_text(ihte->key->text);
		//mx_print_text("\n");

		assign_smart_ptr((void**)&ihte->key, 0);
		assign_smart_ptr((void**)&ihte->value, 0);
		mx_mem_free(ihte);
		ihte = t;
	}
}

/**
* add e at the end of list, if not already in the list. otherwise replace old entry with new one
* @param e
* @return previous value if it exists, null otherwise
*/
const void* mx_ht_entry_put(struct mx_ht_entry* ihte, const mx_text* ikey, const void* ivalue)
{
	struct mx_ht_entry* e = ihte;
	struct mx_ht_entry* p = e;

	do
	{
		if (mx_text_compare(e->key, ikey) == 0)
			// same key. replace
		{
			assign_smart_ptr((void**)&e->key, ikey);
			assign_smart_ptr((void**)&e->value, ivalue);

			return ivalue;
		}

		p = e;
		e = e->next;
	}
	while (e != 0);

	struct mx_ht_entry* new_entry = mx_ht_entry_ctor(ikey, ivalue);

	p->next = new_entry;
	new_entry->next = 0;

	return 0;
}

/**
* returns value corresponding to ikey
* @param ikey
* @return value corresponding to ikey, null if it doesn't exist
*/
const void* mx_ht_entry_get(struct mx_ht_entry* ihte, const mx_text* ikey)
{
	struct mx_ht_entry* e = ihte;

	do
	{
		if (mx_text_compare(ikey, e->key) == 0)
		{
			return e->value;
		}

		e = e->next;
	} while (e != 0);

	return 0;
}

/**
*
* @param ikey
* @return true if list contains ikey, false otherwise
*/
bool mx_ht_entry_contains_key(struct mx_ht_entry* ihte, const mx_text* ikey)
{
	return mx_ht_entry_get(ihte, ikey) != 0;
}

/**
* delete an element with key == ikey from the list (if it exists)
* assusmes the list contains AT LEAST 2 elements! (at least 1 element besides the head of the list)
* @param ikey
* @return true if element corresponding to ikey has been deleted, false otherwise
*/
bool mx_ht_entry_del(struct mx_ht_entry* ihte, const mx_text* ikey)
{
	struct mx_ht_entry* e = ihte;
	struct mx_ht_entry* p = 0;

	do
	{
		if (mx_text_compare(ikey, e->key) == 0)
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



void mx_env_dbg_list(const struct mx_env* e)
{
	mx_htable* h = e->ht_env;

	mx_print_text("env\n[\n");

	for (int k = 0; k < h->capacity; k++)
	{
		struct mx_ht_entry* t = h->entries[k];

		while (t)
		{
			const char* key = t->key->text;
			const struct mx_elem* c = (const struct mx_elem*)t->value;

			mx_print_text(" [ ");
			mx_print_text(key);
			mx_print_text(", ");

			mx_elem_dbg_list(c);

			mx_print_text("],\n");

			t = t->next;
		}
	}

	if (e->ext_env)
	{
		mx_env_dbg_list(e->ext_env);
	}

	mx_print_text("],\n");
}

// evaluate the given Lisp expression and compare the result against the given expected_result
void test_eq(const char* expression, const char* expected_result)
{
	// local ptr
	mx_text* expr = 0;
	assign_smart_ptr((void**)&expr, mx_text_ctor(expression));
	struct mx_elem* rc = 0;
	assign_smart_ptr((void**)&rc, read(expr));
	//mx_elem_dbg_list(rc);
	struct mx_elem* ec = 0;
	assign_smart_ptr((void**)&ec, eval(rc, test_env));
	//mx_elem_dbg_list(ec);
	mx_text* result = 0;
	assign_smart_ptr((void**)&result, to_string(ec));

	assign_smart_ptr((void**)&expr, 0);
	assign_smart_ptr((void**)&rc, 0);
	assign_smart_ptr((void**)&ec, 0);

	executed_test_count++;

	if (mx_char_compare(result->text, expected_result) != 0)
	{
		mx_print_text("\n");
		mx_print_text(expression);
		mx_print_text(" : expected ");
		mx_print_text(expected_result);
		mx_print_text(", got ");
		mx_print_text(result->text);
		mx_print_text("\n");

		failed_test_count++;
	}
	else
	{
		mx_print_text("\n");
		mx_print_text(expression);
		mx_print_text(" : ");
		mx_print_text(result->text);
		mx_print_text("\n");
	}

	assign_smart_ptr((void**)&result, 0);
	mx_read_text_line();
}

void test_mx_lisp(struct mx_env* env)
{
	executed_test_count = 0;
	failed_test_count = 0;
	test_env = env;

	// the 29 unit tests for lis.py
	test_eq("(quote (testing 1 (2.0) -3.14e159))", "(testing 1 (2.0) -3.14e159)");
	test_eq("(+ 2 2)", "4");
	test_eq("(+ (* 2 100) (* 1 10))", "210");
	test_eq("(if(> 6 5) (+ 1 1) (+ 2 2))", "2");
	test_eq("(if(< 6 5) (+ 1 1) (+ 2 2))", "4");
	test_eq("(define x 3)", "3");
	test_eq("x", "3");
	test_eq("(+ x x)", "6");
	test_eq("(begin (define x 1) (set! x (+ x 1)) (+ x 1))", "3");
	test_eq("((lambda (x) (+ x x)) 5)", "10");
	test_eq("(define twice (lambda (x) (* 2 x)))", "<lambda>");
	test_eq("(twice 5)", "10");
	test_eq("(define compose (lambda (f g) (lambda (x) (f (g x)))))", "<lambda>");
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
		"(lambda (x y)"
		"(if(null? x) (quote ())"
		"(f (list (car x) (car y))"
		"((combine f) (cdr x) (cdr y)))))))", "<lambda>");
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
	assign_smart_ptr((void**)&t1, number_to_text(executed_test_count));
	assign_smart_ptr((void**)&t2, number_to_text(failed_test_count));
	mx_print_text("\n\ntotal tests ");
	mx_print_text(t1->text);
	mx_print_text(", total failures ");
	mx_print_text(t2->text);
	mx_print_text("\n");

	//mx_env_dbg_list(global_env);
	assign_smart_ptr((void**)&t1, 0);
	assign_smart_ptr((void**)&t2, 0);
	test_env = 0;
}

void test_all(struct mx_env* env)
{
	test_mx_mem();
	test_mx_text();
	test_mx_vect();
	test_mx_list();
	test_mx_htable();
	test_mx_lisp(env);
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
		assign_smart_ptr((void**)&query, mx_text_ctor(key));
		mx_text* val = (mx_text*)mx_htable_get(ht, query);

		mx_print_text("get ");
		mx_print_text(key);
		mx_print_text(" - ");
		mx_print_text(val->text);
		mx_print_text("\n");
		assign_smart_ptr((void**)&query, 0);
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

#endif
