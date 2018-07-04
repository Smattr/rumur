#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
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
  .traces = 0,
  .deadlock_detection = true,
  .symmetry_reduction = true,
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
      { "deadlock-detection", no_argument, 0, 131 },
      { "debug", no_argument, 0, 'd' },
      { "help", no_argument, 0, '?' },
      { "monopolise", no_argument, 0, 133 },
      { "monopolize", no_argument, 0, 133 },
      { "no-color", no_argument, 0, 129 },
      { "no-colour", no_argument, 0, 129 },
      { "no-deadlock-detection", no_argument, 0, 132 },
      { "output", required_argument, 0, 'o' },
      { "set-capacity", required_argument, 0, 's' },
      { "set-expand-threshold", required_argument, 0, 'e' },
      { "symmetry_reduction", required_argument, 0, 134 },
      { "threads", required_argument, 0, 't' },
      { "trace", required_argument, 0, 130 },
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

      case 128: // --colour
        output_options.color = rumur::ON;
        break;

      case 129: // --no-colour
        output_options.color = rumur::OFF;
        break;

      case 130: // --trace ...
        if (strcmp(optarg, "handle_reads") == 0) {
          output_options.traces |= rumur::TC_HANDLE_READS;
        } else if (strcmp(optarg, "handle_writes") == 0) {
          output_options.traces |= rumur::TC_HANDLE_WRITES;
        } else if (strcmp(optarg, "queue") == 0) {
          output_options.traces |= rumur::TC_QUEUE;
        } else if (strcmp(optarg, "set") == 0) {
          output_options.traces |= rumur::TC_SET;
        } else if (strcmp(optarg, "all") == 0) {
          output_options.traces = uint64_t(-1);
        } else {
          std::cerr
            << "invalid --trace argument \"" << optarg << "\"\n"
            << "valid arguments are \"handle_reads\", \"handle_writes\", "
              "\"queue\", and \"set\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 131: // --deadlock-detection
        output_options.deadlock_detection = true;
        break;

      case 132: // --no-deadlock-detection
        output_options.deadlock_detection = false;
        break;

      case 133: { // --monopolise

        long pagesize = sysconf(_SC_PAGESIZE);
        if (pagesize < 0) {
          perror("failed to retrieve page size");
          exit(EXIT_FAILURE);
        }

        long physpages = sysconf(_SC_PHYS_PAGES);
        if (physpages < 0) {
          perror("failed to retrieve physical pages");
          exit(EXIT_FAILURE);
        }

        /* Allocate a set that will eventually cover all of memory upfront. Note
         * that this will never actually reach 100% occupancy because memory
         * also needs to contain our code and data as well as the OS.
         */
        output_options.set_capacity = size_t(pagesize) * size_t(physpages);

        // Never expand the set.
        output_options.set_expand_threshold = 100;

        break;
      }

      case 134: // --symmetry-reduction ...
        if (strcmp(optarg, "on") == 0) {
          output_options.symmetry_reduction = true;
        } else if (strcmp(optarg, "off") == 0) {
          output_options.symmetry_reduction = false;
        } else {
          std::cerr << "invalid argument to --symmetry-reduction, \"" << optarg
            << "\"\n";
          exit(EXIT_FAILURE);
        }
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
