#ifndef __CLARGS_H__
#define __CLARGS_H__

#ifdef __cplusplus
extern "C" {
#endif

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
#define OPTION_HELP() {.name = "help", .type = HELP, .description = "display the help menu"}
// Boolean flag option (either there or not)
#define OPTION_BOOLEAN(_name, _desc) \
    {                                \
        .name = _name,               \
        .type = BOOLEAN,             \
        .description = _desc,        \
    }
// Integer option (can have min and max)
#define OPTION_INT(_name, _desc, _min, _max) \
    {                                        \
        .name = _name,                       \
        .type = INT,                         \
        .description = _desc,                \
        .intOptions = {                      \
            .minValue = _min,                \
            .maxValue = _max,                \
        }                                    \
    }
// Optional option (may optionally be followed by a value)
#define OPTION_OPTIONAL(_name, _desc) \
    {                                 \
        .name = _name,                \
        .type = STRING,               \
        .description = _desc,         \
        .strOptions = {               \
            .optional = true,         \
            .oneOf = {NULL},          \
        }                             \
    }
// "One of" option (may be one of the provided choices)
#define OPTION_ONEOF(_name, _desc, ...) \
    {                                   \
        .name = _name,                  \
        .type = STRING,                 \
        .description = _desc,           \
        .strOptions = {                 \
            .optional = false,          \
            .oneOf = {__VA_ARGS__},     \
        }                               \
    }
// String option
#define OPTION_STRING(_name, _desc) \
    {                               \
        .name = _name,              \
        .type = STRING,             \
        .description = _desc,       \
        .strOptions = {             \
            .optional = false,      \
            .oneOf = {NULL},        \
        }                           \
    }
typedef enum {
    END,
    HELP,
    BOOLEAN,
    STRING,
    INT,
} CL_OptionType;

typedef struct {
    const char* name;
    const CL_OptionType type;
    const char* description;
    union {
        struct {
            int32_t minValue;
            int32_t maxValue;
        } intOptions;
        struct {
            bool optional;
            char* oneOf[CL_MAX_ONEOF_OPTIONS];
        } strOptions;
    };
} CL_Option;

typedef CL_Option CL_Schema[];

// PARSER

// Helper to check if the flag value has not been set in the args
#define CL_NOTSET(x) _Generic((x), bool: !x, char*: x[0] == '\0', int32_t: x == INT32_MIN)

// Union representing the possible values of an option flag
typedef union {
    bool boolean;
    int32_t number;
    char* string;
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
// Function to set a custom parse error callback function
void CL_setParseErrorCallback(CL_ParseErrorCallback cb);
// Type representing callback function for the help menu (given a schema)
typedef void (*CL_HelpCallback)(const CL_Schema schema);
// Function to set a custom help menu callback
void CL_setHelpCallback(CL_HelpCallback cb);

// Parse command-line arguments
CL_Args CL_parse(int argc, char* argv[], const CL_Schema schema);
// Get the value of a CL_Args flag
CL_FlagValue CL_flag(char* flag, CL_Args args);
// Free a CL_Args object
void CL_freeArgs(CL_Args args);

#ifdef __cplusplus
}
#endif

#endif