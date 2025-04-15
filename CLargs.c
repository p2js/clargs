#include "CLargs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Starting capacities for the growable arrays that store the values and options
#define DEFAULT_VALUE_CAP 1
#define DEFAULT_OPTIONS_CAP 4

// Default behaviour for argument parse error
void defaultParseErrorCallback(const char* flag, char* msg) {
    fprintf(stderr, "Argument error: %s: %s\n", flag, msg);
    exit(EXIT_FAILURE);
}

// Default behaviour for --help (if enabled in schema)
void defaultHelpCallback(const CL_Schema schema) {
    // Compute the maximum spacing needed for aligning help commands
    uint32_t maxSpacingLength = 0;
    for (size_t i = 0; schema[i].type != END; i++) {
        uint32_t argSpacingLength = strlen(schema[i].name);
        switch (schema[i].type) {
            case STRING:
                if (schema[i].strOptions.oneOf[0]) {
                    argSpacingLength += 1;
                    for (int o = 0; schema[i].strOptions.oneOf[o]; o++) {
                        argSpacingLength += strlen(schema[i].strOptions.oneOf[o]) + 1;
                    }
                } else {
                    argSpacingLength += 8;
                }
            case INT:
                if (schema[i].intOptions.minValue == schema[i].intOptions.maxValue == 0) {
                    argSpacingLength = 6;
                } else {
                    argSpacingLength += snprintf(NULL, 0, "(%d..%d)", schema[i].intOptions.minValue, schema[i].intOptions.maxValue);
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
        int spaceNeeded = printf("\t--%s", schema[i].name) - 3;
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
                if (schema[i].intOptions.minValue == schema[i].intOptions.maxValue == 0) {
                    spaceNeeded += printf(" (num)");
                } else {
                    spaceNeeded += printf("(%d..%d)", schema[i].intOptions.minValue, schema[i].intOptions.maxValue);
                }
                break;
            case END:
            case HELP:
            case BOOLEAN:
                break;
        }
        printf("%*s", maxSpacingLength - spaceNeeded, "");
        printf("%s\n", schema[i].description);
    }
    exit(0);
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

    // Count options in schema (if defined)
    if (schema != NULL) {
        while (schema[option_cap].type != END) {
            option_cap++;
        };
    } else {
        option_cap = DEFAULT_OPTIONS_CAP;
    }

    // Define args object to return
    CL_Args args = {
        .path = argc > 0 ? argv[0] : "",
        .options = calloc(option_cap, sizeof(CL_FlagOption)),
        .values = calloc(value_cap, sizeof(char**)),
        .value_count = 0,
        .option_count = 0,
    };

    if (!args.options || !args.values) {
        perror("[CLargs] malloc");
        abort();
    }

    // Add the schema options as not set in the args
    if (schema != NULL) {
        for (; args.option_count < option_cap; args.option_count++) {
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
                    option.value.number = INT32_MIN;
                    break;
                case END:  // Unreachable
                    break;
            }
            args.options[args.option_count] = option;
        }
    }

    for (int a = 1; a < argc; a++) {
        if (argv[a][0] == '-' && argv[a][1] == '-') {
            // Is a flag
            if (schema != NULL) {
                size_t flag_index = -1;
                for (int i = 0; schema[i].type != END; i++) {
                    if (strcmp(argv[a] + 2, schema[i].name) == 0) {
                        flag_index = i;
                    }
                }
                if (flag_index == -1) {
                    parseErrorCallback(argv[a], "Unknown option");
                    continue;
                }
                char* string_value = "";
                switch (schema[flag_index].type) {
                    case HELP:
                        helpCallback(schema);
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
                            }
                        }
                        args.options[flag_index].value.string = string_value;
                        break;
                    case INT:
                        parseErrorCallback(argv[a], "todo");
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