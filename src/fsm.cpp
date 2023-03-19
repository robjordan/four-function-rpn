#include <Arduino.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <assert.h>
#include "fsm.h"

#define __ASSERT_USE_STDERR

#ifndef min
    #define min(a,b) ((a) > (b) ? (b) : (a))
#endif
#ifndef max
    #define max(a,b) ((a) > (b) ? (a) : (b))
#endif

char str[32];
value s[STACKDEPTH];	/* the stack */

#ifdef UNITTEST
#include <ncurses.h>
void refresh_screen() {
	// sleep(1);
	clear();

	for (int i=STACKDEPTH-1; i>=0; i--) {
		if (s[i].type == FIX) {
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

	init_stack();
	initscr();			/* Start curses mode 		  */
	cbreak();			/* Unbuffer input; key-at-a-time */
	noecho(); 			/* what it says! */

	refresh_screen();

	while (1) {
		int ch = getch();			/* Wait for user input */
		(void)printf("0x%08x\n", ch);
		process_symbol((char)ch);
		refresh_screen();
	}
	endwin();			/* End curses mode		  */
	return 0;
}
#endif

void init_stack() {
	// Initialise stack with all integer zeroes
	for (int i=0; i<STACKDEPTH; i++) {
		s[i].type = FIX;
		s[i].value.nt = 0;
	}
}

// Print a float, preserving maximal precision in limited characters, using
// scientific notation if necessary.
// Given n digits field width:
// 1 digit is used if the value if negative
// The remaining n or n-1 digit space can accommodate a maximal integer value 
// of (10^n)-1, or (10^(n-1))-1 if negative. If the value is greater than that, 
// then we have to use scientific notation.
// For small fractional numbers, i.e. those between -0.1 and 0.1 we go 
// straight to scientific notation.
void float_to_string(float f, char *str) {
	double ipart, fpart;
	bool negative = false;
	int digits_avail = DISPDIGITS;
	int ipart_digits = 0;
	char *p = str;
	int e = 0;	// exponent

	if (f < 0.0) {
		// Negative number: first character will be '-', so reserve a digit
		digits_avail--;
	}
	// Split into integer and fractional part
	fpart = modf((double)f, &ipart);

	if (isnan(fpart)) {
		strncpy(str, "nan", sizeof str);
	} else {

		// How many digits does the integer part require?
		if (ipart == 0.0) {
			ipart_digits = 1;
		} else {
			ipart_digits = (int)(floor(log10(fabs(ipart)))) + 1;
		}

		if (ipart_digits > digits_avail) {
			// Integer part too large to be displayed in full => use scientific
			// reduce digits_avail by one char before dp, and 4 in exponent
			dtostre(f, str, digits_avail-5, DTOSTR_UPPERCASE);
		} else if (ipart == 0.0 && fabs(fpart) < 0.001) {
			// Small number - use scientific notation
			dtostre(f, str, digits_avail-5, DTOSTR_UPPERCASE);
		} else {
			// Print the whole integer part and as many decimal digits as fit
			if (ipart == 0.0) {
				// There's no integer part, just print the decimal
				dtostrf(fpart, 0, digits_avail-1, str);
			} else {
				// Print the whole original number
				dtostrf(f, 0, digits_avail-ipart_digits, str);
			}
		}
	}
}

char *get_stack_item_as_string(size_t i) {
	
	const long MAXDISPINT = (unsigned long)(pow(10.0, (float)DISPDIGITS))-1;

	if (s[i].type == FIX) {
		if (s[i].value.nt <= MAXDISPINT) {
			(void)sprintf(str, "%ld", s[i].value.nt);
		} else {
			// Large ints need to be displayed in scientific notation
			float_to_string((float)s[i].value.nt, str);
		}
	} else {	// FLOAT
		float_to_string((float)s[i].value.fl, str);	
	}
	return(str);
}

char *convert_stack_item_for_led(char *si, uint8_t *dp_mask) {
	static char outstr[16];
	char *p = outstr;
	size_t len = strlen(si);
	char *dp = strchr(si, '.');
	size_t i = 0;
	if (dp == nullptr) {
		// There isn't a decimal point. Left-pad string with spaces.
		for (i = 0; i < (8-len); i++)
			outstr[i] = ' ';
		strncpy(&outstr[i], si, len);
		*dp_mask = 0;
	} else {
		// We found a decimal point. We're going to assume there are 8 numeric
		// characters, because that's the contract of the previous function
		// Serial.print(len);
		// Serial.flush();
		// assert(len == 9);
		strncpy(outstr, si, dp-si);		// copy the bit before dp
		strcpy(&outstr[dp-si], dp+1);		// copy the bit after dp
		*dp_mask = (0x80>>(dp-si-1));
	}

	return outstr;
}


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
	return ((x.type == FIX) ? (float)(x.value.nt) : x.value.fl);
}

int multiply_overflow(long x, long y) {
	// return (((num_bin_digits(x)+num_bin_digits(y))>31) ? 1 : 0);
	return((fabs((float)x * (float)y)) > (float)LONG_MAX);
}

int add_overflow(long x, long y) {
	// return (((num_bin_digits(max(x, y)))>30) ? 1 : 0);
	return((fabs((float)x + (float)y)) > (float)LONG_MAX);
}

int add_digit_overflow(long x, char d) {
	return((fabs((float)x * 10.0 + (float)(d-'0'))) > (float)LONG_MAX);
}
 
void process_symbol(char ch) {
	static enum {
		POSTOP,
		POSTENTER,
		FILLING_INT,
		FILLING_DEC
	} state = POSTOP;
	static float dp = 0.0;

	switch (ch) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (state == POSTOP) {
				stack_up();
				s[0].type = FIX;
				s[0].value.nt = (ch-(int)'0');
				state = FILLING_INT;
			} else if (state == POSTENTER) {
				s[0].type = FIX;
				s[0].value.nt = (ch-(int)'0');
				state = FILLING_INT;
			} else if (state == FILLING_INT) {
				if (s[0].type == FIX) {
					if (!add_digit_overflow(s[0].value.nt, ch)) {
						s[0].value.nt = s[0].value.nt * 10L + (ch-(int)'0');
					} else {
						s[0].value.fl = (to_float(s[0]) * 10.0) + (float)(ch-(int)'0');
						s[0].type = FLOAT;
					}
				} else {				
					s[0].value.fl = s[0].value.fl * 10.0;
					s[0].value.fl = s[0].value.fl + (float)(ch-(int)'0');
				}

			} else if (state == FILLING_DEC) {
				s[0].value.fl = s[0].value.fl + dp * (float)(ch-(int)'0');
				dp /= 10.0;
			}
			break;

		case '.':
			if (state == POSTOP) {
				stack_up();	
				s[0].type = FLOAT;
				s[0].value.fl = 0.0;
				state = FILLING_DEC;
				dp = 0.1;
			} else if (state == POSTENTER) {
				s[0].type = FLOAT;
				s[0].value.fl = 0.0;
				state = FILLING_DEC;
				dp = 0.1;
			} else if (state == FILLING_INT) {
				state = FILLING_DEC;
				s[0].type = FLOAT;
				s[0].value.fl = (float)s[0].value.nt;
				dp = 0.1;
			} else if (state == FILLING_DEC) {
				/* dp when already in decimal is ignored */
			}
			break;

		case '/':
			if (s[0].type == FIX && s[1].type == FIX 
				&& s[1].value.nt % s[0].value.nt == 0) {
				s[0].value.nt = s[1].value.nt / s[0].value.nt;	
			} else {
				s[0].value.fl = to_float(s[1]) / to_float(s[0]);
				s[0].type = FLOAT;
			}
			stack_down();
			state = POSTOP;
			break;

		case '*':
			if (s[0].type == FIX && s[1].type == FIX
				&& !multiply_overflow(s[0].value.nt, s[1].value.nt)) {
				s[0].value.nt = s[0].value.nt * s[1].value.nt;
			} else {
				s[0].value.fl = to_float(s[1]) * to_float(s[0]);
				s[0].type = FLOAT;
			}
			stack_down();
			state = POSTOP;
			break;

		case '-':
			if (s[0].type == FIX && s[1].type == FIX) {
				s[0].value.nt = s[1].value.nt - s[0].value.nt;
			} else {
				s[0].value.fl = to_float(s[1]) - to_float(s[0]);
				s[0].type = FLOAT;
			}
			stack_down();
			state = POSTOP;
			break;

		case '+':
			if (s[0].type == FIX && s[1].type == FIX
				&& !add_overflow(s[0].value.nt, s[1].value.nt)) {
				s[0].value.nt = s[1].value.nt + s[0].value.nt;
			} else {
				s[0].value.fl = to_float(s[1]) + to_float(s[0]);
				s[0].type = FLOAT;
			}
			stack_down();
			state = POSTOP;
			break;

		case 0x0a:	/* linefeed */
		case 0x0d:	/* carriage return */
			stack_up();
			state = POSTENTER;
			break;

		default:
			/* ignore anything else */
			break;	
	} 
}




