#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#ifndef min
    #define min(a,b) ((a) > (b) ? (b) : (a))
#endif
#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

#define STACKDEPTH 4
#define DISPDIGITS 8

typedef struct {
	enum {
		FLOATING,
		FIXED
	} value_type;
	union {
		float fl;
		int nt;
	} value;
} value;

char str[32];
value s[STACKDEPTH];	/* the stack */

// push each stack element up one step, to make space for a new top item
void stack_up() {
	for (int i=STACKDEPTH-1; i>0; i--) {
		s[i] = s[i-1];
	}
}

// pull each stack element, except the highest 2, down one step
void stack_down() {
	for (int i=2; i<STACKDEPTH; i++) {
		s[i-1] = s[i];
	}
}

float to_float(value x) {
	return ((x.value_type == FIXED) ? (float)x.value.nt : x.value.fl);
}

int num_bin_digits(int x) {
	if (x < 0) {
		x = -x;
	}
	int n = sizeof(int)*8;
	int mask = 1<<(n*8-1);
	while (!(x & mask) && (n>0)) {
		// (void)printf("x: 0x%08x, n: %d\n", x, n);
		x <<= 1;
		n--;
	}
	return (n);
}

int multiply_overflow(int x, int y) {
	return (((num_bin_digits(x)+num_bin_digits(y))>31) ? 1 : 0);
}

int addition_overflow(int x, int y) {
	return (((num_bin_digits(max(x, y)))>30) ? 1 : 0);
}
 
void process_symbol(int ch) {
	static enum {
		POSTOP,
		POSTENTER,
		FILLING_INT,
		FILLING_DEC
	} state = POSTOP;
	static float dp = 0.0;

	switch (ch) {
		case (int)'0':
		case (int)'1':
		case (int)'2':
		case (int)'3':
		case (int)'4':
		case (int)'5':
		case (int)'6':
		case (int)'7':
		case (int)'8':
		case (int)'9':
			if (state == POSTOP) {
				stack_up();
				s[0].value_type = FIXED;
				s[0].value.nt = (ch-(int)'0');
				state = FILLING_INT;
			} else if (state == POSTENTER) {
				s[0].value_type = FIXED;
				s[0].value.nt = (ch-(int)'0');
				state = FILLING_INT;
			} else if (state == FILLING_INT) {
				s[0].value.nt = s[0].value.nt * 10 + (ch-(int)'0');
			} else if (state == FILLING_DEC) {
				s[0].value.fl = s[0].value.fl + dp * (float)(ch-(int)'0');
				dp /= 10.0;
			}
			break;

		case (int)'.':
			if (state == POSTOP) {
				stack_up();	
				s[0].value_type = FLOATING;
				s[0].value.fl = 0.0;
				state = FILLING_DEC;
				dp = 0.1;
			} else if (state == POSTENTER) {
				s[0].value_type = FLOATING;
				s[0].value.fl = 0.0;
				state = FILLING_DEC;
				dp = 0.1;
			} else if (state == FILLING_INT) {
				state = FILLING_DEC;
				s[0].value_type = FLOATING;
				s[0].value.fl = (float)s[0].value.nt;
				dp = 0.1;
			} else if (state == FILLING_DEC) {
				/* dp when already in decimal is ignored */
			}
			break;

		case (int)'/':
			if (s[0].value_type == FIXED && s[1].value_type == FIXED 
				&& s[1].value.nt % s[0].value.nt == 0) {
				s[0].value.nt = s[1].value.nt / s[0].value.nt;	
			} else {
				s[0].value.fl = to_float(s[1]) / to_float(s[0]);
				s[0].value_type = FLOATING;
			}
			stack_down();
			state = POSTOP;
			break;

		case (int)'*':
			if (s[0].value_type == FIXED && s[1].value_type == FIXED
				&& !multiply_overflow(s[0].value.nt, s[1].value.nt)) {
				s[0].value.nt = s[0].value.nt * s[1].value.nt;
			} else {
				s[0].value.fl = to_float(s[1]) * to_float(s[0]);
				s[0].value_type = FLOATING;
			}
			stack_down();
			state = POSTOP;
			break;

		case (int)'-':
			if (s[0].value_type == FIXED && s[1].value_type == FIXED) {
				s[0].value.nt = s[1].value.nt - s[0].value.nt;
			} else {
				s[0].value.fl = to_float(s[1]) - to_float(s[0]);
				s[0].value_type = FLOATING;
			}
			stack_down();
			state = POSTOP;
			break;

		case (int)'+':
			if (s[0].value_type == FIXED && s[1].value_type == FIXED
				&& !addition_overflow(s[0].value.nt, s[1].value.nt)) {
				s[0].value.nt = s[1].value.nt + s[0].value.nt;
			} else {
				s[0].value.fl = to_float(s[1]) + to_float(s[0]);
				s[0].value_type = FLOATING;
			}
			stack_down();
			state = POSTOP;
			break;

		case (int)0x0a:	/* enter */
			stack_up();
			state = POSTENTER;
			break;

		default:
			/* ignore anything else */
			break;	
	} 
}

void refresh_screen() {
	// sleep(1);
	clear();

	for (int i=STACKDEPTH-1; i>=0; i--) {
		if (s[i].value_type == FIXED) {
			(void)sprintf(str, "%d", s[i].value.nt);
			if (strlen(str) > DISPDIGITS) {
				// We'll need to display in Scientific notation
				int precision = DISPDIGITS - ((s[i].value.nt<0) ? 7 : 6);
				(void)sprintf(str, "%.*e", precision, (float)s[i].value.nt);
			}
		}
		else {
			(void)sprintf(str, "%g", s[i].value.fl);
			if (strlen(str) > DISPDIGITS) {
				// We'll need to display in Scientific notation
				int precision = DISPDIGITS - ((s[i].value.fl<0) ? 7 : 6);
				(void)sprintf(str, "%.*e", precision, s[i].value.fl);
			}			
		}
		char prefix[5];
		sprintf(prefix, "%d: ", i);
		mvaddstr(STACKDEPTH-i, 0, prefix);
		mvaddstr(STACKDEPTH-i, 4, str);
	}
	refresh();			/* Print it on to the real screen */
}

int main(void){	

	// (void)printf("0xf000, %d\n", num_bin_digits(0xf000));
	// (void)printf("0x0f000000, %d\n", num_bin_digits(0x0f000000));
	// (void)printf("0x0f00, %d\n", num_bin_digits(0x0f00));
	// (void)printf("0x00f0, %d\n", num_bin_digits(0x00f0));
	// (void)printf("99999999, %d\n", num_bin_digits(99999999));
	// (void)printf("-9999999, %d\n", num_bin_digits(-9999999));
	// exit(0);

	// Initialise stack with all integer zeroes
	for (int i=0; i<STACKDEPTH; i++) {
		s[i].value_type = FIXED;
		s[i].value.nt = 0;
	}

	initscr();			/* Start curses mode 		  */
	cbreak();			/* Unbuffer input; key-at-a-time */
	noecho(); 			/* what it says! */

	refresh_screen();

	while (1) {
		int ch = getch();			/* Wait for user input */
		(void)printf("0x%08x\n", ch);
		process_symbol(ch);
		refresh_screen();
	}
	endwin();			/* End curses mode		  */
	return 0;
}


