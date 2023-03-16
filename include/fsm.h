
#define STACKDEPTH 4
#define DISPDIGITS 8

typedef enum {FLOAT, FIX} value_type;
typedef struct {
	value_type type;
	union {
		float fl;
		long nt;
	} value;
} value;

void init_stack();
void float_to_string(float f, char *str);
char *get_stack_item_as_string(size_t i);
void stack_up();
void stack_down();
float to_float(value x);
int num_bin_digits(long x);
int multiply_overflow(long x, long y);
int add_overflow(long x, long y);
void process_symbol(char ch);