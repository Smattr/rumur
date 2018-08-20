#include <cassert>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <string>
#include <unistd.h>
#include "XMLPrinter.h"

static std::string in_filename;
static std::shared_ptr<std::istream> in;
static std::shared_ptr<std::ostream> out;

static void help(const char *arg0) {
  std::cout
    << arg0 << " [--output FILE | -o FILE] INPUT_FILE\n"
    << "\n"
    << "Print the abstract syntax tree of a parsed Murphi model.\n";
  exit(EXIT_SUCCESS);
}

static void parse_args(int argc, char **argv) {

  for (;;) {
    static struct option options[] = {
      { "help", no_argument, 0, '?' },
      { "output", required_argument, 0, 'o' },
      { 0, 0, 0, 0 },
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "o:", options, &option_index);

    if (c == -1)
      break;

    switch (c) {

      case '?':
        help(argv[0]);
        __builtin_unreachable();

      case 'o': {
        auto o = std::make_shared<std::ofstream>(optarg);
        if (!o->is_open()) {
          std::cerr << "failed to open " << optarg << "\n";
          exit(EXIT_FAILURE);
        }
        out = o;
        break;
      }

      default:
        std::cerr << "unexpected error\n";
        exit(EXIT_FAILURE);

    }
  }

  if (optind == argc - 1) {
    auto i = std::make_shared<std::ifstream>(argv[optind]);
    if (!i->is_open()) {
      std::cerr << "failed to open " << argv[optind] << "\n";
      exit(EXIT_FAILURE);
    }
    in_filename = argv[optind];
    in = i;
  }
}

int main(int argc, char **argv) {

  // Parse command line options
  parse_args(argc, argv);

  // Parse input model
  std::shared_ptr<rumur::Model> m;
  try {
    m = rumur::parse(in == nullptr ? &std::cin : in.get());
  } catch (rumur::Error &e) {
    std::cerr << e.loc << ":" << e.what() << "\n";
    return EXIT_FAILURE;
  }

  assert(m != nullptr);

  {
    XMLPrinter p(in_filename, out == nullptr ? std::cout : *out);
    p.dispatch(*m);
  }

  return EXIT_SUCCESS;
}
