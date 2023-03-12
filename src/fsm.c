#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#ifndef min
    #define min(a,b) ((a) > (b) ? (b) : (a))
#endif
#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif


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

char xstr[32], ystr[32], zstr[32], tstr[32];
value x, y, z, t;

// void format_float(char *dst, int n, char *prefix, float f) {
// 	int prefix_len = min(strlen(prefix), n);
// 	strncpy(dst, prefix, prefix_len, n));
// 	n -= prefix_len;
// 	dst += prefix_len
// 	if (f < 0.0) {
// 		strncpy(dst, "-", n);
// 		n - 1;
// 		dst++;
// 		f *= (-1);
// 	}

// 	if (f >= 0) {
// 		/* is scientific notation needed? */


// 	} else /* f < 0*/ {

// 	}

// }

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
				t = z;
				z = y;
				y = x;
				x.value_type = FIXED;
				x.value.nt = (ch-(int)'0');
				state = FILLING_INT;
			} else if (state == POSTENTER) {
				x.value_type = FIXED;
				x.value.nt = (ch-(int)'0');
				state = FILLING_INT;
			} else if (state == FILLING_INT) {
				x.value.nt = x.value.nt * 10 + (ch-(int)'0');
			} else if (state == FILLING_DEC) {
				x.value.fl = x.value.fl + dp * (float)(ch-(int)'0');
				dp /= 10.0;
			}
			break;

		case (int)'.':
			if (state == POSTOP) {
				t = z;
				z = y;
				y = x;				
				x.value_type = FLOATING;
				x.value.fl = 0.0;
				state = FILLING_DEC;
				dp = 0.1;
			} else if (state == POSTENTER) {
				x.value_type = FLOATING;
				x.value.fl = 0.0;
				state = FILLING_DEC;
				dp = 0.1;
			} else if (state == FILLING_INT) {
				state = FILLING_DEC;
				x.value_type = FLOATING;
				x.value.fl = (float)x.value.nt;
				dp = 0.1;
			} else if (state == FILLING_DEC) {
				/* dp when already in decimal is ignored */
			}
			break;

		case (int)'/':
			if (x.value_type == FIXED && y.value_type == FIXED 
				&& y.value.nt % x.value.nt == 0) {
				x.value.nt = y.value.nt / x.value.nt;	
			} else {
				x.value.fl = to_float(y) / to_float(x);
				x.value_type = FLOATING;
			}
			y = z;
			z = t;
			state = POSTOP;
			break;

		case (int)'*':
			if (x.value_type == FIXED && y.value_type == FIXED
				&& !multiply_overflow(x.value.nt, y.value.nt)) {
				x.value.nt = x.value.nt * y.value.nt;
			} else {
				x.value.fl = to_float(y) * to_float(x);
				x.value_type = FLOATING;
			}
			y = z;
			z = t;
			state = POSTOP;
			break;

		case (int)'-':
			if (x.value_type == FIXED && y.value_type == FIXED) {
				x.value.nt = y.value.nt - x.value.nt;
			} else {
				x.value.fl = to_float(y) - to_float(x);
				x.value_type = FLOATING;
			}
			y = z;
			z = t;
			state = POSTOP;
			break;

		case (int)'+':
			if (x.value_type == FIXED && y.value_type == FIXED
				&& !addition_overflow(x.value.nt, y.value.nt)) {
				x.value.nt = y.value.nt + x.value.nt;
			} else {
				x.value.fl = to_float(y) + to_float(x);
				x.value_type = FLOATING;
			}
			y = z;
			z = t;
			state = POSTOP;
			break;

		case (int)0x0a:	/* enter */
			t = z;
			z = y;
			y = x;
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
	if (x.value_type == FIXED)
		(void)sprintf(xstr, "x(i): %d", x.value.nt);
	else
		(void)sprintf(xstr, "x(f): %g", x.value.fl);
	
	if (y.value_type == FIXED)
		(void)sprintf(ystr, "y: %d", y.value.nt);
	else
		(void)sprintf(ystr, "y: %g", y.value.fl);
	
	if (z.value_type == FIXED)
		(void)sprintf(zstr, "z: %d", z.value.nt);
	else
		(void)sprintf(zstr, "z: %g", z.value.fl);
	
	if (t.value_type == FIXED)
		(void)sprintf(tstr, "t: %d", t.value.nt);
	else
		(void)sprintf(tstr, "t: %g", t.value.fl);
	mvaddstr(0, 0, tstr);
	mvaddstr(1, 0, zstr);
	mvaddstr(2, 0, ystr);
	mvaddstr(3, 0, xstr);
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

	x.value_type = FIXED;
	x.value.nt = 0;
	y = x;
	z = x;
	t = x;

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


