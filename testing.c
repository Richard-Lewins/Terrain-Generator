#include <stdio.h>
#include <stdbool.h>
#include "testing.h"

// Checks the test condition and outputs an appropriate message to the appropriate
// output stream. If the output stream is NULL, does not output anything.
void assert_test(bool test, char* testName, FILE* outOK, FILE* outFAIL) {
    if (test) {
        if (outOK != NULL) {
            fprintf(outOK, "T: %s - %-50s\n", testName, "OK");
        }
    } else {
        if (outFAIL != NULL) {
            fprintf(outFAIL, "T: %s - %-50s\n", testName, "FAIL");
        }
    }
}

