#include <cassert>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <rumur/rumur.h>
#include <string>
#include <unistd.h>

static std::istream *in;
static std::string *out;
static rumur::OutputOptions output_options = {
  .overflow_checks = true,
  .threads = 0,
  .debug = false,
};

static void parse_args(int argc, char **argv) {

  for (;;) {
    static struct option options[] = {
      { "debug", no_argument, 0, 'd' },
      { "help", no_argument, 0, '?' },
      { "output", required_argument, 0, 'o' },
      { "threads", required_argument, 0, 't' },
      { 0, 0, 0, 0 },
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "do:t:?", options, &option_index);

    if (c == -1)
      break;

    switch (c) {

      case 'd':
        output_options.debug = true;
        break;

      case 'o':
        if (out != nullptr)
          delete out;
        out = new std::string(optarg);
        break;

      case 't':
        try {
          output_options.threads = std::stoul(optarg);
        } catch (std::exception) {
          std::cerr << "invalid --threads argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case '?':
        std::cerr << "usage: " << argv[0] << " --output FILE [FILE]\n";
        exit(EXIT_FAILURE);

      default:
        std::cerr << "unexpected error\n";
        exit(EXIT_FAILURE);

    }
  }

  if (optind == argc - 1) {
    auto inf = new std::ifstream(argv[optind]);
    if (!inf->is_open()) {
      std::cerr << "failed to open " << argv[optind] << "\n";
      exit(EXIT_FAILURE);
    }
    in = inf;
  } else {
    in = &std::cin;
  }

  if (out == nullptr) {
    std::cerr << "output file is required\n";
    exit(EXIT_FAILURE);
  }

  if (output_options.threads == 0) {
    // automatic
    long r = sysconf(_SC_NPROCESSORS_ONLN);
    if (r < 1) {
      output_options.threads = 1;
    } else {
      output_options.threads = static_cast<unsigned long>(r);
    }
  }
}

int main(int argc, char **argv) {

  // Parse command line options
  parse_args(argc, argv);

  // Parse input model
  rumur::Model *m;
  try {
    m = rumur::parse(in);
  } catch (rumur::Error &e) {
    std::cerr << e.loc << ":" << e.what() << "\n";
    return EXIT_FAILURE;
  }

  assert(out != nullptr);
  assert(m != nullptr);
  if (rumur::output_checker(*out, *m, output_options) != 0)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
