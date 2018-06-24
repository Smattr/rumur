#include <algorithm>
#include <cassert>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include "resources_manpage.h"
#include <rumur/rumur.h>
#include <string>
#include <unistd.h>

static std::istream *in;
static std::string *out;
static rumur::OutputOptions output_options = {
  .overflow_checks = true,
  .threads = 0,
  .debug = false,
  .set_capacity = 8 * 1024 * 1024,
  .set_expand_threshold = 65,
  .color = rumur::AUTO,
};

static void help(const char *arg0) {

  // Construct a path to where we expect the manpage to be.
  std::string const cmd(arg0);
  auto pos = cmd.rfind('/');
  std::string manpage;
  if (pos != std::string::npos)
    manpage += cmd.substr(0, pos + 1);
  manpage += "rumur.1";

  // If it is indeed there, try to run `man`.
  if (access(manpage.c_str(), R_OK) == 0) {

    char const *args[] = { "man",
#ifndef __APPLE__
      "--local-file",
#endif
      manpage.c_str(), nullptr };
    execvp(args[0], const_cast<char**>(args));

    // If we failed to exec, fall through to the option below.
  }

  // Fall back option: just print the pre-formatted manpage data.
  std::cout << std::string((const char*)resources_manpage_text,
    (size_t)resources_manpage_text_len);

  exit(EXIT_SUCCESS);
}

static void parse_args(int argc, char **argv) {

  for (;;) {
    static struct option options[] = {
      { "color", no_argument, 0, 128 },
      { "colour", no_argument, 0, 128 },
      { "debug", no_argument, 0, 'd' },
      { "help", no_argument, 0, '?' },
      { "no-color", no_argument, 0, 129 },
      { "no-colour", no_argument, 0, 129 },
      { "output", required_argument, 0, 'o' },
      { "set-capacity", required_argument, 0, 's' },
      { "set-expand-threshold", required_argument, 0, 'e' },
      { "threads", required_argument, 0, 't' },
      { 0, 0, 0, 0 },
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "de:o:s:t:?", options, &option_index);

    if (c == -1)
      break;

    switch (c) {

      case 'd':
        output_options.debug = true;
        break;

      case 'e':
        try {
          output_options.set_expand_threshold = std::stoul(optarg);
        } catch (std::exception) {
          std::cerr << "invalid --set-expand-threshold argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        if (output_options.set_expand_threshold < 1 ||
            output_options.set_expand_threshold > 100) {
          std::cerr << "invalid --set-expand-threshold argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 'o':
        if (out != nullptr)
          delete out;
        out = new std::string(optarg);
        break;

      case 's':
        try {
          output_options.set_capacity = std::stoul(optarg);
        } catch (std::exception) {
          std::cerr << "invalid --set-capacity argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
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
        help(argv[0]);
        __builtin_unreachable();

      case 128:
        output_options.color = rumur::ON;
        break;

      case 129:
        output_options.color = rumur::OFF;
        break;

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

template<typename T, typename U>
static bool contains(const T &container, U predicate) {
  return std::find_if(container.begin(), container.end(), predicate)
    != container.end();
}

static bool has_start_state(const rumur::Rule *r) {
  if (dynamic_cast<const rumur::StartState*>(r))
    return true;
  if (auto s = dynamic_cast<const rumur::Ruleset*>(r))
    return contains(s->rules, has_start_state);
  return false;
}

static void validate(const rumur::Model &m) {

  // Check whether we have a start state.
  if (!contains(m.rules, has_start_state))
    std::cerr << "warning: model has no start state\n";

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

  assert(m != nullptr);
  validate(*m);

  assert(out != nullptr);
  if (rumur::output_checker(*out, *m, output_options) != 0)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
