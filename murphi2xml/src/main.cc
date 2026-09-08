#include "../../common/help.h"
#include "XMLPrinter.h"
#include "resources.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <memory>
#include <rumur/rumur.h>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

static const char *in_filename = "<stdin>";
static std::shared_ptr<std::istream> in;
static std::shared_ptr<std::ostream> out;

/// use colour in error messages?
static enum { AUTO, ON, OFF } color = AUTO;

// buffer the contents of stdin so we can read it twice
static void buffer_stdin() {

  // read in all of stdin
  std::ostringstream buf;
  buf << std::cin.rdbuf();
  buf.flush();

  // put this into a buffer we can read from
  in = std::make_shared<std::istringstream>(buf.str());
}

static void parse_args(int argc, char **argv) {

  for (;;) {
    static struct option options[] = {
        {"color", required_argument, 0, 129},
        {"colour", required_argument, 0, 129},
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
      std::cout << "Rumur version " << rumur_get_version() << '\n';
      exit(EXIT_SUCCESS);

    case 129: // --color, --colour
      if (strcmp(optarg, "auto") == 0) {
        color = AUTO;
      } else if (strcmp(optarg, "on") == 0) {
        color = ON;
      } else if (strcmp(optarg, "off") == 0) {
        color = OFF;
      } else {
        std::cerr << "invalid --colour argument \"" << optarg << "\"\n"
                  << "valid arguments are \"auto\", \"off\", and \"on\"\n";
        exit(EXIT_FAILURE);
      }
      break;

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
  } else {
    // we are going to read data from stdin
    buffer_stdin();
  }
}

static bool use_colors() {
  if (color == AUTO)
    color = isatty(STDERR_FILENO) ? ON : OFF;
  return color == ON;
}

static const char *bold() {
  if (use_colors())
    return "\033[1m";
  return "";
}

static const char *green() {
  if (use_colors())
    return "\033[32m";
  return "";
}

static const char *red() {
  if (use_colors())
    return "\033[31m";
  return "";
}

static const char *reset() {
  if (use_colors())
    return "\033[0m";
  return "";
}

static const char *white() {
  if (use_colors())
    return "\033[37m";
  return "";
}

static void print_location(std::istream &src, const rumur::location &location) {

  // the type of position.line and position.column changes across Bison
  // releases, so avoid some -Wsign-compare warnings by casting them in advance
  auto loc_line = static_cast<unsigned long>(location.begin.line);
  auto loc_col = static_cast<unsigned long>(location.begin.column);

  std::string line;
  unsigned long lineno = 0;
  while (lineno < loc_line) {
    if (!std::getline(src, line))
      return;
    lineno++;
  }

  // print the line, and construct an underline indicating the column location
  std::ostringstream buf;
  unsigned long col = 1;
  for (const char &c : line) {
    if (col == loc_col) {
      buf << green() << bold() << '^' << reset();
    } else if (col < loc_col) {
      if (c == '\t') {
        buf << '\t';
      } else {
        buf << ' ';
      }
    }
    std::cerr << c;
    col++;
  }
  std::cerr << '\n';

  std::cerr << buf.str() << '\n';
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
    std::cerr << white() << bold() << in_filename << ':' << e.loc << ':'
              << reset() << ' ' << red() << bold() << "error:" << reset() << ' '
              << white() << bold() << e.what() << reset() << '\n';
    in->seekg(0);
    print_location(*in, e.loc);
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
    std::cerr << white() << bold() << in_filename << ':' << e.loc << ':'
              << reset() << ' ' << red() << bold() << "error:" << reset() << ' '
              << white() << bold() << e.what() << reset() << '\n';
    in->seekg(0);
    print_location(*in, e.loc);
    return EXIT_FAILURE;
  }

  assert(m != nullptr);

  in->seekg(0);
  {
    XMLPrinter p(in_filename, *in, out == nullptr ? std::cout : *out);
    p.dispatch(*m);
  }

  return EXIT_SUCCESS;
}
