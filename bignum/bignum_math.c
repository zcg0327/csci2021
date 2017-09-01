/*
 * bignum_math.c -- an outline for CLab1
 *
 * orginially written by Andy Exley
 * modified by Emery Mizero
 * completed and updated by Brady Olson
 **************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bignum_math.h"


/*
 * Returns true if the given char is a digit from 0 to 9
 */
bool is_digit(char c) {
	return c >= '0' && c <= '9';
}

/*
 * Returns true if lower alphabetic character
 */
bool is_lower_alphabetic(char c) {
	return c >= 'a' && c <= 'z';
}

/*
 * Returns true if upper alphabetic character
 */
bool is_upper_alphabetic(char c) {
	return c >= 'A' && c <= 'Z';
}

/*
 * Convert a string to an integer
 * returns 0 if it cannot be converted.
 */
int string_to_integer(char* input) {
	int result = 0;
	int length = strlen(input);
    int num_digits = length;
	int sign = 1;
	
	int i = 0;
    int factor = 1;

    if (input[0] == '-') {
		num_digits--;
		sign = -1;
    }

	for (i = 0; i < num_digits; i++, length--) {
		if (!is_digit(input[length-1])) {
			return 0;
		}
		if (i > 0) factor*=10;
		result += (input[length-1] - '0') * factor;
	}

    return sign * result;
}

/*
 * Returns true if the given base is valid.
 * that is: integers between 2 and 36
 */
bool valid_base(int base) {
	if(!(base >= 2 && base <= 36)) { 
		return false; 
	}
	return true;
}

/*
 * Returns true if the given string (char array) is a valid input,
 * that is: digits 0-9, letters A-Z, a-z.
 * Does not handle negative numbers or symbols.
 */
bool valid_input(char* input, int base) {
	bool result = true;
	int t;
	
	// loop through every char in the string until a null char found
	// also, stop loop if an invalid char is found
	for (int i=0; input[i] != '\0' && result; i++) {
		if (is_digit(input[i])) {
			// checks if the number is in the valid range [0,base] inclusive
			result = ((input[i] - '0') < base && (input[i] - '0') >= 0);;
		} else if (is_lower_alphabetic(input[i])) {
			// checks if the letter is in valid range
			result = ((input[i] - 'a' + 10) < base);
		} else if (is_upper_alphabetic(input[i])) {
			// checks if the letter is in valid range
			result = ((input[i] - 'A' + 10) < base);
		} else { result = false; } // should not be reached
	}
	
	return result;
}

/*
 * converts from an array of characters (string) to an array of integers
 */
int* string_to_integer_array(char* str) {
	int* result;
	int i, str_offset = 0;
		result = malloc((strlen(str) + 1) * sizeof(int));
		result[strlen(str)] = -1;
	for(i = str_offset; str[i] != '\0'; i++) {
		if(is_digit(str[i])) { 
			result[i - str_offset] = str[i] - '0';
		} else if (is_lower_alphabetic(str[i])) {
			result[i - str_offset] = str[i] - 'a' + 10;
		} else if (is_upper_alphabetic(str[i])) {
			result[i - str_offset] = str[i] - 'A' + 10;
		} else {
			printf("I don't know how got to this point!\n");
		}
	}
	return result;
}

/*
 * finds the length of a bignum... 
 * simply traverses the bignum until a negative number is found.
 */
int bignum_length(int* num) {
	int len = 0;
	while(num[len] >= 0) { len++; }
	return len;
}

/*
 * prints out a bignum using digits and lowercase characters
 * prints a negative sign appropriately
 * does not print leading zeros
 */
void bignum_print(int* num) {
	int i;
	if(num == NULL) { return; }
	
	/* Print negative sign appropriately */
	i = bignum_length(num);
	// if the terminating entry of the array is not -1 (see subtract)
	if (num[i] < -1) { printf("-"); }
	
	/* Then, print each digit */
	bool printed = false;   // tells if ANYTHING has been printed
							// to be used if result is 0
	bool lead_zero = true;  // tells if only zeros have been found in num
	for (i=0; num[i] >= 0; i++) {
		if (num[i] < 10) { 
			// only print a zero if it is a non-leading zero
			if (num[i] == 0 && !lead_zero) { 
				printf("%d", num[i]);
				printed = true;
			}
			else if (num[i] > 0) { 
				printf("%d", num[i]);  // once a nonzero digit is printed
				lead_zero = false;     // allow zeroes to be printed
				printed = true;
			}
		}
		else { 
			printf("%c", 'a'-(10-num[i]));  // once a nonzero digit is printed
			lead_zero = false;				// allow zeroes to be printed
			printed = true;
		}
	}
	if (!printed) { printf("0"); }
}

/*
 *	Helper for reversing the result that we built backward.
 *  see add(...) below
 */
void reverse(int* num) {
	int i, len = bignum_length(num);
	for(i = 0; i < len/2; i++) {
		int temp = num[i];
		num[i] = num[len-i-1];
		num[len-i-1] = temp;
	}
}


/*
 * used to add two numbers with the same sign
 * GIVEN FOR GUIDANCE
 */
int* add(int* input1, int* input2, int base) {
	int len1 = bignum_length(input1);
	int len2 = bignum_length(input2);
	int resultlength = ((len1 > len2)? len1 : len2) + 2;
	int* result = (int*) malloc (sizeof(int) * resultlength);
	int r = 0;
	int carry = 0;
	int sign = -1;

	len1--;
	len2--;

	while(len1 >= 0 || len2 >= 0) {
		int num1 = (len1 >= 0)? input1[len1] : 0;
		int num2 = (len2 >= 0)? input2[len2] : 0;
		result[r] = (num1 + num2 + carry) % base;
		carry = (num1 + num2 + carry) / base;
		len1--;
		len2--;
		r++;
	}
	if(carry > 0) { result[r] = carry; r++; }
	result[r] = sign;
	reverse(result);
	return result;
}

int* subtract(int* input1, int* input2, int base) {
	int len1 = bignum_length(input1);
	int len2 = bignum_length(input2);
	int resultlength = ((len1 > len2)? len1 : len2) + 2;
	int* result = (int*) malloc (sizeof(int) * resultlength);
	int r = 0;
	int carry = 0;
	bool switched = false;	

	/*
	 *  check if input2 is a larger number than input1
	 * if input2 is larger, the order needs to be reversed
	 */
	
	// first checks if input2 is longer, then it is obviously larger
	if (len1 < len2) { switched = true; }
	else if (len1 == len2) {
		// compare each place until input1 and input2 differ
		for (int i=0; i < len1; i++) {
			if (input1[i] < input2[i]) {
				switched = true; 
				i = len1; 
			}
			else if (input1[i] > input2[i]) { i = len1; }
		}
		// note: if the numbers are equal, they do not get switched
		// this maintains a positive zero
	}

	// if the order was switched, the result should be negative -> pass -2
	// otherwise, pass -1 as the terminating entry regularly
	int sign = (switched)? -2 : -1;
	
	len1--;
	len2--;
	
	int num1, num2, temp;
	while (len1 >= 0 || len2 >= 0) {
		// make num2 the "top" number if order was switched
		if (!switched) {
			num1 = (len1 >=0)? input1[len1] : 0;
			num2 = (len2 >=0)? input2[len2] : 0;
		} else {
			num1 = (len2 >=0)? input2[len2] : 0;
			num2 = (len1 >=0)? input1[len1] : 0;
		}
		temp = (num1 - num2 - carry) % base;
		// if the temporary result is < 0 (since % is not true modulo)
		// make temp positive and borrow from the next place
		if (temp < 0) {
			temp = temp + base;
			carry = 1;
		} else { carry = 0; } // if temp >= 0, no borrowing
		result[r] = temp;
		len1--;
		len2--;
		r++;
	}
	result[r] = sign;
	reverse(result);
	return result;
}


/*
 * simply calls the appropriate operation function
 */
int* perform_math(int* input1, int* input2, char op, int base) {
	if(op == '+') {
		return add(input1, input2, base);
	}
	return subtract(input1, input2, base);
}

/*
 * Print to "stderr" and exit program
 */
void print_usage(char* name) {
	fprintf(stderr, "----------------------------------------------------\n");
	fprintf(stderr, "Usage: %s base input1 operation input2\n", name);
	fprintf(stderr, "base must be number between 2 and 36, inclusive\n");
	fprintf(stderr, "input1 and input2 are arbitrary-length integers\n");
	fprintf(stderr, "Two operations are allowed '+' and '-'\n");
	fprintf(stderr, "----------------------------------------------------\n");
	exit(1);
}


/*
 * MAIN: Run the program and tests your functions.
 * sample command: ./bignum 4 12 + 13
 * Result: 31
 */
int main(int argc, char** argv) {
	int input_base;

    int* input1;
    int* input2;
    int* result;
    float input_1, input_2, result_float;

	if(argc != 5) { 
		print_usage(argv[0]); 
	}

	input_base = string_to_integer(argv[1]);

	if(!valid_base(input_base)) { 
		fprintf(stderr, "Invalid base: %s\n", argv[1]);
		print_usage(argv[0]);
	}

	if(!valid_input(argv[2], input_base)) { 
		fprintf(stderr, "Invalid input1: %s\n", argv[2]);
		print_usage(argv[0]);
	}
	
	if(!valid_input(argv[4], input_base)) { 
		fprintf(stderr, "Invalid input2: %s\n", argv[4]);
		print_usage(argv[0]);
	}

	if(argv[3][0] != '-' && argv[3][0] != '+') {
		fprintf(stderr, "Invalid operation: %s\n", argv[3]);
		print_usage(argv[0]);
	}
	
	input1 = string_to_integer_array(argv[2]);
	input2 = string_to_integer_array(argv[4]);

	result = perform_math(input1, input2, argv[3][0], input_base);

	printf("Result: ");
	bignum_print(result);

	printf("\n");
	exit(0);
}
