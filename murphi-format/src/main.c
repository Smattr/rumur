#include "../../common/help.h"
#include "debug.h"
#include "format.h"
#include "resources.h"
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <rumur/version.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool debug;

static bool in_place;

static const char **inputs;
static size_t n_inputs;

static const char *output;

static void parse_args(int argc, char **argv) {

  while (true) {
    static struct option opts[] = {
        {"debug", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {"in-place", no_argument, 0, 'i'},
        {"output", required_argument, 0, 'o'},
        {"version", no_argument, 0, 129},
        {0},
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "dio:", opts, &option_index);

    if (c == -1)
      break;

    switch (c) {

    case 'd': // --debug
      debug = true;
      break;

    case 'h': // --help
      help(doc_murphi_format_1, doc_murphi_format_1_len);
      exit(EXIT_SUCCESS);

    case 'i': // --in-place
      in_place = true;
      break;

    case 'o': // --output
      output = optarg;
      break;

    case 129: // --version
      printf("murphi-format version %s\n", rumur_get_version());
      exit(EXIT_SUCCESS);

    default:
      fprintf(stderr, "unexpected error\n");
      exit(EXIT_FAILURE);
    }
  }

  if (output != NULL && in_place) {
    fprintf(stderr, "--in-place and --output are mutually incompatible\n");
    exit(EXIT_FAILURE);
  }

  if (optind < argc) {
    n_inputs = (size_t)(argc - optind);
    inputs = calloc(n_inputs, sizeof(inputs[0]));
    if (inputs == NULL) {
      fprintf(stderr, "out of memory\n");
      exit(EXIT_FAILURE);
    }
    for (int i = optind; i < argc; ++i)
      inputs[i - optind] = argv[i];
  }

  if (in_place && n_inputs == 0) {
    fprintf(stderr, "--in-place requires a file to be provided\n");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char **argv) {

  parse_args(argc, argv);

  char *buffer = NULL;
  size_t buffer_size = 0;
  int rc = 0;

  FILE *out;
  if (in_place) {
    out = open_memstream(&buffer, &buffer_size);
    if (out == NULL) {
      fprintf(stderr, "failed to open in-memory stream: %s\n", strerror(errno));
      rc = EXIT_FAILURE;
      goto done;
    }
  } else if (output == NULL) {
    out = stdout;
  } else {
    out = fopen(output, "w");
    if (out == NULL) {
      fprintf(stderr, "failed to open %s: %s\n", output, strerror(errno));
      rc = EXIT_FAILURE;
      goto done;
    }
  }

  for (size_t i = 0; i < n_inputs || i == 0; ++i) {

    FILE *in;
    if (n_inputs == 0) {
      in = stdin;
    } else {
      in = fopen(inputs[i], "r");
      if (in == NULL) {
        fprintf(stderr, "failed to open %s: %s\n", inputs[i], strerror(errno));
        rc = EXIT_FAILURE;
        goto done;
      }
    }

    rc = format(out, in);

    (void)fclose(in);

    if (rc != 0) {
      fprintf(stderr, "failed to reformat %s: %s\n",
              n_inputs == 0 ? "stdin" : inputs[i], strerror(rc));
      goto done;
    }

    if (in_place) {
      if (fflush(out) < 0) {
        fprintf(stderr, "failed to flush in-memory buffer: %s\n",
                strerror(errno));
        rc = EXIT_FAILURE;
        goto done;
      }
      assert(inputs != NULL);
      FILE *wb = fopen(inputs[i], "w");
      if (wb == NULL) {
        fprintf(stderr, "failed to open %s for write back: %s\n", inputs[i],
                strerror(errno));
        rc = EXIT_FAILURE;
        goto done;
      }
      if (fwrite(buffer, buffer_size, 1, wb) != 1) {
        fprintf(stderr, "failed to write back %s: %s\n", inputs[i],
                strerror(errno));
        (void)fclose(wb);
        rc = EXIT_FAILURE;
        goto done;
      }
      (void)fclose(wb);
      memset(buffer, '\0', buffer_size);
    }
  }

done:
  if (out != NULL)
    (void)fclose(out);
  free(buffer);
  free(inputs);

  return rc;
}
