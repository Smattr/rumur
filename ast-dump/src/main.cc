#include <cassert>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include "../../common/help.h"
#include <iostream>
#include <memory>
#include "resources.h"
#include <rumur/rumur.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include "XMLPrinter.h"

static std::string in_filename;
static std::shared_ptr<std::istream> in;
static std::shared_ptr<std::ostream> out;

// buffer the contents of stdin into a buffer we can seek on
static void buffer_stdin(void) {

  // read in all of stdin
  std::ostringstream buf;
  buf << std::cin.rdbuf();
  buf.flush();

  // put this into a buffer we can read from and seek
  in = std::make_shared<std::istringstream>(buf.str());
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
        help(doc_rumur_ast_dump_1, doc_rumur_ast_dump_1_len);
        exit(EXIT_SUCCESS);

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
    struct stat buf;
    if (stat(argv[optind], &buf) < 0) {
      std::cerr << "failed to open " << argv[optind] << ": " << strerror(errno) << "\n";
      exit(EXIT_FAILURE);
    }

    if (S_ISDIR(buf.st_mode)) {
      std::cerr << "failed to open " << argv[optind] << ": this is a directory\n";
      exit(EXIT_FAILURE);
    }

    auto i = std::make_shared<std::ifstream>(argv[optind]);
    if (!i->is_open()) {
      std::cerr << "failed to open " << argv[optind] << "\n";
      exit(EXIT_FAILURE);
    }
    in_filename = argv[optind];
    in = i;
  } else {
    // we are going to read data from stdin
    buffer_stdin();
  }
}

int main(int argc, char **argv) {

  // Parse command line options
  parse_args(argc, argv);

  assert(in != nullptr);

  // Parse input model
  rumur::Ptr<rumur::Model> m;
  try {
    m = rumur::parse(in.get());
    resolve_symbols(*m);
    validate(*m);
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
