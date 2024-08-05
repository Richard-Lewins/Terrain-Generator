#ifndef TESTING_H
#define TESTING_H

#include <stdbool.h>
#include <stdio.h>

// Takes in a boolean expression, a test name (should be less than 50 characters) and
// output streams to write the test results to.
extern void assert_test(bool, char*, FILE*, FILE*);

#endif
