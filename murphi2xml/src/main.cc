#include "../../common/help.h"
#include "XMLPrinter.h"
#include "resources.h"
#include <cassert>
#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

static std::string in_filename = "<stdin>";
static std::shared_ptr<std::istream> in;
static std::shared_ptr<std::istream> in_replay;
static std::shared_ptr<std::ostream> out;

// buffer the contents of stdin so we can read it twice
static void buffer_stdin() {

  // read in all of stdin
  std::ostringstream buf;
  buf << std::cin.rdbuf();
  buf.flush();

  // put this into two buffers we can read from
  in = std::make_shared<std::istringstream>(buf.str());
  in_replay = std::make_shared<std::istringstream>(buf.str());
}

static void parse_args(int argc, char **argv) {

  for (;;) {
    static struct option options[] = {
        {"help", no_argument, 0, '?'},
        {"output", required_argument, 0, 'o'},
        {"version", no_argument, 0, 128},
        {0, 0, 0, 0},
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "o:", options, &option_index);

    if (c == -1)
      break;

    switch (c) {

    case '?':
      help(doc_murphi2xml_1, doc_murphi2xml_1_len);
      exit(EXIT_SUCCESS);

    case 'o': {
      auto o = std::make_shared<std::ofstream>(optarg);
      if (!o->is_open()) {
        std::cerr << "failed to open " << optarg << '\n';
        exit(EXIT_FAILURE);
      }
      out = o;
      break;
    }

    case 128: // --version
      std::cout << "Rumur version " << rumur::get_version() << '\n';
      exit(EXIT_SUCCESS);

    default:
      std::cerr << "unexpected error\n";
      exit(EXIT_FAILURE);
    }
  }

  if (optind == argc - 1) {
    struct stat buf;
    if (stat(argv[optind], &buf) < 0) {
      std::cerr << "failed to open " << argv[optind] << ": " << strerror(errno)
                << '\n';
      exit(EXIT_FAILURE);
    }

    if (S_ISDIR(buf.st_mode)) {
      std::cerr << "failed to open " << argv[optind]
                << ": this is a directory\n";
      exit(EXIT_FAILURE);
    }

    in_filename = argv[optind];

    auto i = std::make_shared<std::ifstream>(in_filename);
    if (!i->is_open()) {
      std::cerr << "failed to open " << in_filename << '\n';
      exit(EXIT_FAILURE);
    }
    in = i;

    // open the input again that we need for replay during XML output
    auto i2 = std::make_shared<std::ifstream>(in_filename);
    if (!i2->is_open()) {
      std::cerr << "failed to open " << in_filename << '\n';
      exit(EXIT_FAILURE);
    }
    in_replay = i2;
  } else {
    // we are going to read data from stdin
    buffer_stdin();
  }
}

int main(int argc, char **argv) {

  // Parse command line options
  parse_args(argc, argv);

  assert(in != nullptr);

  // parse input model
  rumur::Ptr<rumur::Model> m;
  try {
    m = rumur::parse_model(*in);
  } catch (rumur::Error &e) {
    std::cerr << e.loc << ":" << e.what() << '\n';
    return EXIT_FAILURE;
  }

  // re-index the model to make sure AST node identifiers are ready for symbol
  // resolution below
  m->reindex();

  // resolve symbolic references and validate the model
  try {
    resolve_symbols(*m);
    validate(*m);
  } catch (rumur::Error &e) {
    std::cerr << e.loc << ":" << e.what() << '\n';
    return EXIT_FAILURE;
  }

  assert(m != nullptr);

  {
    XMLPrinter p(in_filename, *in_replay, out == nullptr ? std::cout : *out);
    p.dispatch(*m);
  }

  return EXIT_SUCCESS;
}
