/**
 * CLargs.c
 * Command-line argument parser
 *
 * Copyright (C) 2025 Alfio Tomarchio
 * All rights reserved.
 *
 * Use of this source code is governed by a MIT-style license that can be found
 * in the LICENSE file or at https://opensource.org/licenses/MIT.
 */

#include "CLargs.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Starting capacities for the growable arrays that store the values and options
#define DEFAULT_VALUE_CAP 9
#define DEFAULT_OPTIONS_CAP 9

// Default behaviour for argument parse error
void defaultParseErrorCallback(const char* flag, char* msg) {
    fprintf(stderr, "Argument error: %s: %s\n", flag, msg);
    exit(EXIT_FAILURE);
}

// Default behaviour for --help (if enabled in schema)
bool defaultHelpCallback(const CL_Schema schema) {
    // Compute the maximum spacing needed for aligning flag descriptions
    uint32_t maxSpacingLength = 0;
    for (size_t i = 0; schema[i].type != END; i++) {
        uint32_t argSpacingLength = strlen(schema[i].name) + 1;
        switch (schema[i].type) {
            case STRING:
                if (schema[i].strOptions.oneOf[0]) {
                    argSpacingLength += 2;  // (
                    for (int o = 0; schema[i].strOptions.oneOf[o]; o++) {
                        argSpacingLength += strlen(schema[i].strOptions.oneOf[o]) + 1;  // [value]/ + final )
                    }
                } else {
                    argSpacingLength += 8;  // (value)
                }
            case INT:
                if (schema[i].intOptions.minValue == 0 && schema[i].intOptions.maxValue == 0) {
                    argSpacingLength += 6;  // (int)
                } else {
                    argSpacingLength += snprintf(NULL, 0, " (%d..%d)", schema[i].intOptions.minValue, schema[i].intOptions.maxValue);
                }
                break;
            case DOUBLE:
                if (schema[i].doubleOptions.minValue == 0.0 && schema[i].doubleOptions.maxValue == 0.0) {
                    argSpacingLength += 6;  // (num)
                } else {
                    argSpacingLength += snprintf(NULL, 0, " (%.2f..%.2f)", schema[i].doubleOptions.minValue, schema[i].doubleOptions.maxValue);
                }
                break;
            case END:
            case HELP:
            case BOOLEAN:
                break;
        }
        if (argSpacingLength > maxSpacingLength) {
            maxSpacingLength = argSpacingLength;
        }
    }

    printf("Options:\n");
    for (size_t i = 0; schema[i].type != END; i++) {
        if (schema[i].abbr) {
            printf(" -%c, ", schema[i].abbr);
        } else {
            printf("     ");
        }
        // Print flag name
        int spaceNeeded = printf("--%s", schema[i].name) - 2;  // Keep track of space needed to reach max width
        // Print hint for flag value, if necessary
        switch (schema[i].type) {
            case STRING:
                if (schema[i].strOptions.oneOf[0]) {
                    spaceNeeded += printf(" (");
                    size_t o = 0;
                    while (schema[i].strOptions.oneOf[o + 1]) {
                        spaceNeeded += printf("%s/", schema[i].strOptions.oneOf[o++]);
                    }
                    spaceNeeded += printf("%s)", schema[i].strOptions.oneOf[o]);
                } else if (schema[i].strOptions.optional) {
                    spaceNeeded += printf(" [value]");
                } else {
                    spaceNeeded += printf(" (value)");
                };
                break;
            case INT:
                if (schema[i].intOptions.minValue == 0 && schema[i].intOptions.maxValue == 0) {
                    spaceNeeded += printf(" (int)");
                } else {
                    spaceNeeded += printf(" (%d..%d)", schema[i].intOptions.minValue, schema[i].intOptions.maxValue);
                }
                break;
            case DOUBLE:
                if (schema[i].doubleOptions.minValue == 0.0 && schema[i].doubleOptions.maxValue == 0.0) {
                    spaceNeeded += printf(" (num)");
                } else {
                    spaceNeeded += printf(" (%.2f..%.2f)", schema[i].doubleOptions.minValue, schema[i].doubleOptions.maxValue);
                }
            case END:
            case HELP:
            case BOOLEAN:
                break;
        }
        // Pad with space left and print description
        printf("%*s%s\n", maxSpacingLength - spaceNeeded, "", schema[i].description);
    }

    return true;
}

CL_ParseErrorCallback parseErrorCallback = defaultParseErrorCallback;
CL_HelpCallback helpCallback = defaultHelpCallback;

void CL_setParseErrorCallback(CL_ParseErrorCallback cb) {
    parseErrorCallback = cb;
}
void CL_setHelpCallback(CL_HelpCallback cb) {
    helpCallback = cb;
}

CL_Args CL_parse(int argc, char* argv[], const CL_Schema schema) {
    size_t value_cap = DEFAULT_VALUE_CAP;
    size_t option_cap = 0;

    bool schemaDefined = schema != NULL;

    // Count options in schema (if defined)
    if (schemaDefined) {
        while (schema[option_cap].type != END) {
            option_cap++;
        };
    } else {
        option_cap = DEFAULT_OPTIONS_CAP;
    }

    // Define args object to return
    CL_Args args = {
        .path = argc > 0 ? argv[0] : "",
        .option_count = 0,
        .value_count = 0,
        .options = calloc(option_cap, sizeof(CL_FlagOption)),
        .values = calloc(value_cap, sizeof(char*)),
    };

    if (!args.options || !args.values) {
        perror("[CLargs] malloc");
        abort();
    }

    // Add the schema options as unset in the args
    if (schemaDefined) {
        while (args.option_count < option_cap) {
            CL_FlagOption option = {
                .flag = schema[args.option_count].name,
            };
            switch (schema[args.option_count].type) {
                case HELP:
                case BOOLEAN:
                    option.value.boolean = false;
                    break;
                case STRING:
                    option.value.string = "";
                    break;
                case INT:
                    option.value.integer = INT32_MIN;
                    break;
                case DOUBLE:
                    option.value.number = NAN;
                    break;
                case END:  // Unreachable
                    break;
            }
            args.options[args.option_count++] = option;
        }
    }

    // Process user arguments
    for (int a = 1; a < argc; a++) {
        if (argv[a][0] == '-' && argv[a][1] != '\0' && (schemaDefined || argv[a][1] == '-')) {
            // Is a flag
            if (schemaDefined) {
                // Schema defined, find option in schema
                size_t flag_index = SIZE_MAX;
                // Check if the flag is in short mode
                if (argv[a][1] != '-') {
                    // If there is more than 1 short flag, look for them all
                    if (argv[a][2] != '\0') {
                        for (int f = 1; argv[a][f] != '\0'; f++) {
                            for (int i = 0; schema[i].type != END; i++) {
                                if (argv[a][f] == schema[i].abbr) {
                                    // Grouped flags must be boolean (which will then be set to true)
                                    if (schema[i].type != BOOLEAN) {
                                        char flagString[2] = {argv[a][f], '\0'};
                                        parseErrorCallback(flagString, "Grouped flag not a boolean option");
                                        continue;
                                    }
                                    args.options[i].value.boolean = true;
                                    break;
                                }
                            }
                        }
                        // Continue to next flag
                        continue;
                    } else {
                        // Just one flag, look for its abbreviation
                        for (int i = 0; schema[i].type != END; i++) {
                            if (argv[a][1] == schema[i].abbr) {
                                flag_index = i;
                                break;
                            }
                        }
                    }
                } else {
                    // Long mode, look for the flag using strcmp
                    for (int i = 0; schema[i].type != END; i++) {
                        if (strcmp(argv[a] + 2, schema[i].name) == 0) {
                            flag_index = i;
                            break;
                        }
                    }
                }
                // Check if the flag has been found
                if (flag_index == SIZE_MAX) {
                    parseErrorCallback(argv[a], "Unknown option");
                    continue;
                }
                // Process the flag based on its type
                char* string_value = "";
                switch (schema[flag_index].type) {
                    case HELP:
                        if (helpCallback(schema)) {
                            exit(0);
                        };
                        break;
                    case BOOLEAN:
                        args.options[flag_index].value.boolean = true;
                        break;
                    case STRING:
                        // If the next arg is not a flag, treat it as the value
                        if (a < argc - 1 && (argv[a + 1][0] != '-' || argv[a + 1][1] != '-')) {
                            string_value = argv[++a];
                        }
                        if (string_value[0] == 0 && !schema[flag_index].strOptions.optional) {
                            parseErrorCallback(schema[flag_index].name, "Expected value after flag");
                            continue;
                        }
                        if (schema[flag_index].strOptions.oneOf[0]) {
                            bool equalsOneOfOptions = false;
                            for (int o = 0; schema[flag_index].strOptions.oneOf[o]; o++) {
                                if (strcmp(string_value, schema[flag_index].strOptions.oneOf[o]) == 0) {
                                    equalsOneOfOptions = true;
                                    break;
                                }
                            }
                            if (!equalsOneOfOptions) {
                                parseErrorCallback(schema[flag_index].name, "invalid option");
                                continue;
                            }
                        }
                        args.options[flag_index].value.string = string_value;
                        break;
                    case INT:
                        // If the next arg is not a flag, treat it as the value
                        if (a < argc - 1 && (argv[a + 1][0] != '-' || argv[a + 1][1] != '-')) {
                            string_value = argv[++a];
                        }
                        if (string_value[0] == 0) {
                            parseErrorCallback(schema[flag_index].name, "Expected value after flag");
                            continue;
                        }
                        // Compute the specified base and sign
                        int base = 10;
                        int32_t sign = 1;
                        if (string_value[0] == '-') {
                            sign = -1;
                            string_value = string_value + 1;
                        }
                        if (string_value[0] == '0') {
                            switch (string_value[1]) {
                                case 'x':
                                case 'X':
                                    base = 16;
                                    string_value = string_value + 2;
                                    break;
                                case 'b':
                                case 'B':
                                    base = 2;
                                    string_value = string_value + 2;
                                    break;
                                case 'o':
                                case 'O':
                                    base = 8;
                                    string_value = string_value + 2;
                                    break;
                            }
                        }
                        // Convert the actual number
                        int32_t integer_value = (int32_t)strtol(string_value, NULL, base) * sign;
                        // Check if it is out of the range provided by the schema
                        if (schema[flag_index].intOptions.minValue != 0 || schema[flag_index].intOptions.maxValue != 0) {
                            if (integer_value < schema[flag_index].intOptions.minValue || integer_value > schema[flag_index].intOptions.maxValue) {
                                parseErrorCallback(schema[flag_index].name, "Value out of range");
                                continue;
                            }
                        }

                        args.options[flag_index].value.integer = integer_value;
                        break;
                    case DOUBLE:
                        // If the next arg is not a flag, treat it as the value
                        if (a < argc - 1 && (argv[a + 1][0] != '-' || argv[a + 1][1] != '-')) {
                            string_value = argv[++a];
                        }
                        if (string_value[0] == 0) {
                            parseErrorCallback(schema[flag_index].name, "Expected value after flag");
                            continue;
                        }
                        // Convert the actual number
                        double numeric_value = strtod(string_value, NULL);
                        // Check if it is invalid
                        if (!isfinite(numeric_value)) {
                            parseErrorCallback(schema[flag_index].name, "Invalid value");
                            continue;
                        }
                        // Check if it is out of the range provided by the schema
                        if (schema[flag_index].doubleOptions.minValue != 0.0 || schema[flag_index].doubleOptions.maxValue != 0.0) {
                            if (numeric_value < schema[flag_index].doubleOptions.minValue || numeric_value > schema[flag_index].doubleOptions.maxValue) {
                                parseErrorCallback(schema[flag_index].name, "Value out of range");
                                continue;
                            }
                        }
                        args.options[flag_index].value.number = numeric_value;
                        break;
                    case END:  // Unreachable
                        break;
                }
            } else {
                // No schema defined; create the option object with a string value
                CL_FlagOption option = {
                    .flag = argv[a] + 2,
                    .value.string = "",
                };
                // If the next argument is not an option flag, treat it as the value for this option
                if (a < argc - 1 && (argv[a + 1][0] != '-' || argv[a + 1][1] != '-')) {
                    option.value.string = argv[++a];
                }
                // Add to options (increasing capacity if needed)
                if (args.option_count == option_cap) {
                    size_t new_option_cap = option_cap + 2;
                    CL_FlagOption* new_options_ptr = reallocarray(args.options, new_option_cap, sizeof(CL_FlagOption));
                    if (!new_options_ptr) {
                        perror("[CLargs] malloc");
                        abort();
                    }
                    args.options = new_options_ptr;
                }
                args.options[args.option_count] = option;
                args.option_count++;
            }
        } else {
            // Not a flag, just a value, copy to the values (increasing capacity if needed)
            if (args.value_count == value_cap) {
                size_t new_value_cap = value_cap + 2;
                char** new_values_ptr = reallocarray(args.values, new_value_cap, sizeof(char*));
                if (!new_values_ptr) {
                    perror("[CLargs] malloc");
                    abort();
                }
                args.values = new_values_ptr;
            }
            args.values[args.value_count] = argv[a];
            args.value_count++;
        }
    }

    return args;
}

CL_FlagValue CL_flag(char* flag, CL_Args args) {
    for (uint32_t i = 0; i < args.option_count; i++) {
        if (strcmp(flag, args.options[i].flag) == 0) {
            return args.options[i].value;
        }
    }

    // Not found; either not defined in the schema, or no schema was provided when parsing
    // return empty string value
    return (CL_FlagValue){.string = ""};
}

void CL_free(CL_Args args) {
    free(args.options);
    free(args.values);
}