/*
 * Utils.h
 *
 *  Created on: 21 août 2015
 *      Author: warole
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>

class Utils {
public:
	static char *floatToCharArray(char *a, double f, int precision) {
		long p[] = { 0, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
				100000000 };
		char *ret = a;
		long heiltal = (long) f;
		itoa(heiltal, a, 10);
		while (*a != '\0')
			a++;
		*a++ = '.';
		long desimal = abs((long )((f - heiltal) * p[precision]));
		itoa(desimal, a, 10);
		return ret;
	}
};

#endif /* UTILS_H_ */
