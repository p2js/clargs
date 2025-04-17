/**
 * arithmetic.c
 *
 * Example program for CLargs (parsing with schema, custom help callback)
 * Try calling the program with different options to test out the parser's response!
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../CLargs.h"

const CL_Schema schema = CL_DEFINESCHEMA(
    OPTION_BOOLEAN("verbose", 'v', "enable verbose output"),
    OPTION_BOOLEAN("round", 'r', "round final value before output"),
    OPTION_ONEOF("mode", 0, "Operation to perform", "add", "sub", "mul", "div"),  // 0 (or '\0') for no shorthand abbreviation
    OPTION_DOUBLE("xValue", 'x', "First value of operation", 0, 0),
    OPTION_DOUBLE("yValue", 'y', "Second value of operation", 0, 0),
    OPTION_INT("power", 'p', "Power to raise the final result to before output", 0, 10),
    OPTION_HELP());

// You can incorporate the default help menu into your custom callback by declaring it like so!
extern bool defaultHelpCallback(const CL_Schema schema);

bool customHelpCallback(const CL_Schema schema) {
    printf("Usage: arithmetic [options] x y\n");
    printf("  Or : arithmetic -x (x) -y (y) [otherOptions]\n\n");
    defaultHelpCallback(schema);
    printf("\nExample program for the CLargs library.\nPerforms arithmetic operations on the provided floating point numbers.\n");
    return true;
}

typedef enum {
    ADD,
    SUB,
    MUL,
    DIV,
} Mode;

Mode getMode(char* mode_string) {
    if (strcmp(mode_string, "sub") == 0) {
        return SUB;
    }
    if (strcmp(mode_string, "mul") == 0) {
        return MUL;
    }
    if (strcmp(mode_string, "div") == 0) {
        return DIV;
    }
    // either "add" or "", either way want add
    return ADD;
}

int main(int argc, char* argv[]) {
    CL_setHelpCallback(customHelpCallback);
    CL_Args args = CL_parse(argc, argv, schema);

    double x = CL_flag("xValue", args).number;
    double y = CL_flag("yValue", args).number;
    // Set the values of x and y to the values if the args were not provided
    if (isnan(x) && isnan(y)) {
        if (args.value_count < 2) {
            fprintf(stderr, "Expected a value for x and y\n");
            exit(EXIT_FAILURE);
        } else {
            x = strtod(args.values[0], NULL);
            y = strtod(args.values[1], NULL);
        }
    } else if (isnan(x)) {
        if (args.value_count < 1) {
            fprintf(stderr, "Expected a value for x\n");
            exit(EXIT_FAILURE);
        } else {
            x = strtod(args.values[0], NULL);
        }
    } else if (isnan(y)) {
        if (args.value_count < 1) {
            fprintf(stderr, "Expected a value for y\n");
            exit(EXIT_FAILURE);
        } else {
            y = strtod(args.values[0], NULL);
        }
    }

    Mode mode = getMode(CL_flag("mode", args).string);
    int32_t power = CL_flag("power", args).integer;
    if (power == INT32_MIN) {
        power = 1;
    }

    bool roundResult = CL_flag("round", args).boolean;
    bool verbose = CL_flag("verbose", args).boolean;

    // Free args after processing
    CL_free(args);

    if (verbose) {
        char symbol;
        switch (mode) {
            case ADD:
                symbol = '+';
                break;
            case SUB:
                symbol = '-';
                break;
            case MUL:
                symbol = '*';
                break;
            case DIV:
                symbol = '/';
                break;
        }
        printf("VERBOSE MODE\nPerforming %f %c %f\n", x, symbol, y);
        if (power != 1) {
            printf("Then raising by the power of %d\n", power);
        }
        if (roundResult) {
            printf("Then rounding.\n");
        }
    }

    double result = 0;
    switch (mode) {
        case ADD:
            result = x + y;
            break;
        case SUB:
            result = x - y;
            break;
        case MUL:
            result = x * y;
            break;
        case DIV:
            result = x / y;
            break;
    }

    if (verbose && (power != 1 || roundResult)) {
        printf("Operation result (before power/round): %f\nTotal result: ", result);
    }

    result = round(pow(result, (double)power));

    printf("%f\n", result);
}