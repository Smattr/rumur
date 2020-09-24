/* this code was generated by Murphi2C */

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* built-in boolean type */
typedef bool boolean;


static void failed_assertion_(const char *message) {
  fprintf(stderr, "failed assertion: %s\n", message);
  exit(EXIT_FAILURE);
}

void (*failed_assertion)(const char*) = failed_assertion_;

static void failed_assumption_(const char *message) {
  fprintf(stderr, "failed assumption: %s\n", message);
  exit(EXIT_FAILURE);
}

void (*failed_assumption)(const char*) = failed_assumption_;

static void error_(const char *message) {
  fprintf(stderr, "error: %s\n", message);
  exit(EXIT_FAILURE);
}

void (*error)(const char*) = error_;

static void cover_(const char *message __attribute__((unused))) {
}

void (*cover)(const char*) = cover_;

static void liveness_(const char *message __attribute__((unused))) {
}

void (*liveness)(const char*) = liveness_;

// various printf wrappers to deal with the user having passed --value-type
static __attribute__((unused)) void print_int     (int v)      { printf("%d",          v); }
static __attribute__((unused)) void print_unsigned(unsigned v) { printf("%u",          v); }
static __attribute__((unused)) void print_short   (short v)    { printf("%hd",         v); }
static __attribute__((unused)) void print_long    (long v)     { printf("%ld",         v); }
static __attribute__((unused)) void print_int8_t  (int8_t v)   { printf("%" PRId8 , v); }
static __attribute__((unused)) void print_uint8_t (uint8_t v)  { printf("%" PRIu8 , v); }
static __attribute__((unused)) void print_int16_t (int16_t v)  { printf("%" PRId16, v); }
static __attribute__((unused)) void print_uint16_t(uint16_t v) { printf("%" PRIu16, v); }
static __attribute__((unused)) void print_int32_t (int32_t v)  { printf("%" PRId32, v); }
static __attribute__((unused)) void print_uint32_t(uint32_t v) { printf("%" PRIu32, v); }
static __attribute__((unused)) void print_int64_t (int64_t v)  { printf("%" PRId64, v); }
static __attribute__((unused)) void print_uint64_t(uint64_t v) { printf("%" PRIu64, v); }

// wrappers for producing literal expressions of value type
#define int_VALUE_C(v)      (v)
#define unsigned_VALUE_C(v) (v ## u)
#define short_VALUE_C(v)    ((short)(v))
#define long_VALUE_C(v)     (v ## l)
#define int8_t_VALUE_C(v)   INT8_C(v)
#define uint8_t_VALUE_C(v)  UINT8_C(v)
#define int16_t_VALUE_C(v)  INT16_C(v)
#define uint16_t_VALUE_C(v) UINT16_C(v)
#define int32_t_VALUE_C(v)  INT32_C(v)
#define uint32_t_VALUE_C(v) UINT32_C(v)
#define int64_t_VALUE_C(v)  INT64_C(v)
#define uint64_t_VALUE_C(v) UINT64_C(v)


