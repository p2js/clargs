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

#ifndef CLARGS_H
#define CLARGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// SCHEMA DEFINITIONS

// Maximum possibilities for a "one of" option value
#define CL_MAX_ONEOF_OPTIONS 0xFF
// Macro to format options into the proper schema (appending end)
#define CL_DEFINESCHEMA(...) \
    {                        \
        __VA_ARGS__, {       \
            .type = END,     \
        }                    \
    }
// Help option
#define OPTION_HELP() {.name = "help", .type = HELP, .description = "Display the help menu"}
// Boolean flag option (either there or not)
#define OPTION_BOOLEAN(_name, _abbr, _desc) \
    {                                       \
        .name = _name,                      \
        .abbr = _abbr,                      \
        .type = BOOLEAN,                    \
        .description = _desc,               \
    }
// Integer option (can have min and max)
#define OPTION_INT(_name, _abbr, _desc, _min, _max) \
    {                                               \
        .name = _name,                              \
        .abbr = _abbr,                              \
        .type = INT,                                \
        .description = _desc,                       \
        .intOptions = {                             \
            .minValue = _min,                       \
            .maxValue = _max,                       \
        }                                           \
    }
// Double option (can have min and max)
#define OPTION_DOUBLE(_name, _abbr, _desc, _min, _max) \
    {                                                  \
        .name = _name,                                 \
        .abbr = _abbr,                                 \
        .type = DOUBLE,                                \
        .description = _desc,                          \
        .doubleOptions = {                             \
            .minValue = _min,                          \
            .maxValue = _max,                          \
        }                                              \
    }
// Optional option (may optionally be followed by a value)
#define OPTION_OPTIONAL(_name, _abbr, _desc) \
    {                                        \
        .name = _name,                       \
        .abbr = _abbr,                       \
        .type = STRING,                      \
        .description = _desc,                \
        .strOptions = {                      \
            .optional = true,                \
            .oneOf = {NULL},                 \
        }                                    \
    }
// "One of" option (may be one of the provided choices)
#define OPTION_ONEOF(_name, _abbr, _desc, ...) \
    {                                          \
        .name = _name,                         \
        .abbr = _abbr,                         \
        .type = STRING,                        \
        .description = _desc,                  \
        .strOptions = {                        \
            .optional = false,                 \
            .oneOf = {__VA_ARGS__},            \
        }                                      \
    }
// String option
#define OPTION_STRING(_name, _abbr, _desc) \
    {                                      \
        .name = _name,                     \
        .abbr = _abbr,                     \
        .type = STRING,                    \
        .description = _desc,              \
        .strOptions = {                    \
            .optional = false,             \
            .oneOf = {NULL},               \
        }                                  \
    }
// Enum representing the different types of options
typedef enum {
    END,
    HELP,
    BOOLEAN,
    STRING,
    INT,
    DOUBLE,
} CL_OptionType;

// Enum representing an option definition in the schema
typedef struct {
    const char* name;
    const char abbr;
    const CL_OptionType type;
    const char* description;
    union {
        struct {
            int32_t minValue;
            int32_t maxValue;
        } intOptions;
        struct {
            double minValue;
            double maxValue;
        } doubleOptions;
        struct {
            bool optional;
            char* oneOf[CL_MAX_ONEOF_OPTIONS];
        } strOptions;
    };
} CL_Option;

// Type representing the schema (array of options)
typedef CL_Option CL_Schema[];

// PARSER

// Union representing the possible values of an option flag
typedef union {
    bool boolean;
    int32_t integer;
    char* string;
    double number;
} CL_FlagValue;

// Struct representing an option flag and its value
typedef struct {
    const char* flag;
    CL_FlagValue value;
} CL_FlagOption;

// Args struct returned by CL_parse
typedef struct {
    char* path;
    uint32_t option_count;
    uint32_t value_count;
    CL_FlagOption* options;
    char** values;
} CL_Args;

// Type representing parse error callback function
typedef void (*CL_ParseErrorCallback)(const char* flag, char* msg);
// Set a custom parse error callback function
void CL_setParseErrorCallback(CL_ParseErrorCallback cb);
// Type representing callback function for the help menu (given a schema).
//
// Returns true if the program should exit after displaying the help menu
typedef bool (*CL_HelpCallback)(const CL_Schema schema);
// Set a custom help menu callback
void CL_setHelpCallback(CL_HelpCallback cb);

// Parse command-line arguments
CL_Args CL_parse(int argc, char* argv[], const CL_Schema schema);
// Get the value of a CL_Args flag
CL_FlagValue CL_flag(char* flag, CL_Args args);
// Free the heap allocations of CL_Args object
void CL_freeArgs(CL_Args args);

#ifdef __cplusplus
}
#endif

#endif