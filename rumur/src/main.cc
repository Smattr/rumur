#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <fstream>
#include "generate.h"
#include <getopt.h>
#include "help.h"
#include <iostream>
#include <memory>
#include "options.h"
#include "resources.h"
#include <rumur/rumur.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include "utils.h"
#include "version.h"

static std::shared_ptr<std::istream> in;
static std::shared_ptr<std::string> out;

static void parse_args(int argc, char **argv) {

  for (;;) {
    static struct option opts[] = {
      { "color", required_argument, 0, 128 },
      { "colour", required_argument, 0, 128 },
      { "counterexample-trace", required_argument, 0, 137 },
      { "deadlock-detection", required_argument, 0, 131 },
      { "debug", no_argument, 0, 'd' },
      { "help", no_argument, 0, 'h' },
      { "max-errors", required_argument, 0, 136 },
      { "monopolise", no_argument, 0, 133 },
      { "monopolize", no_argument, 0, 133 },
      { "output", required_argument, 0, 'o' },
      { "output-format", required_argument, 0, 138 },
      { "quiet", no_argument, 0, 'q' },
      { "sandbox", required_argument, 0, 135 },
      { "set-capacity", required_argument, 0, 's' },
      { "set-expand-threshold", required_argument, 0, 'e' },
      { "symmetry-reduction", required_argument, 0, 134 },
      { "threads", required_argument, 0, 't' },
      { "trace", required_argument, 0, 130 },
      { "verbose", no_argument, 0, 'v' },
      { "version", no_argument, 0, 139 },
      { 0, 0, 0, 0 },
    };

    int option_index = 0;
    int c = getopt_long(argc, argv, "de:ho:qs:t:v", opts, &option_index);

    if (c == -1)
      break;

    switch (c) {

      case 'd':
        options.log_level = DEBUG;
        break;

      case 'e':
        try {
          options.set_expand_threshold = std::stoul(optarg);
        } catch (std::exception&) {
          std::cerr << "invalid --set-expand-threshold argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        if (options.set_expand_threshold < 1 ||
            options.set_expand_threshold > 100) {
          std::cerr << "invalid --set-expand-threshold argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 'h': // --help
        help();
        exit(EXIT_SUCCESS);

      case 'o':
        out = std::make_shared<std::string>(optarg);
        break;

      case 'q': // --quiet
        options.log_level = SILENT;
        break;

      case 's':
        try {
          options.set_capacity = std::stoul(optarg);
        } catch (std::exception&) {
          std::cerr << "invalid --set-capacity argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 't':
        try {
          options.threads = std::stoul(optarg);
        } catch (std::exception&) {
          std::cerr << "invalid --threads argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 'v': // --verbose
        options.log_level = INFO;
        break;

      case '?':
        std::cerr << "run `" << argv[0] << " --help` to see available options\n";
        exit(EXIT_FAILURE);

      case 128: // --colour
        if (strcmp(optarg, "auto") == 0) {
          options.color = AUTO;
        } else if (strcmp(optarg, "on") == 0) {
          if (options.machine_readable_output) {
            std::cerr << "colour is not supported in combination with "
              << "--output-format \"machine readable\"\n";
            exit(EXIT_FAILURE);
          }
          options.color = ON;
        } else if (strcmp(optarg, "off") == 0) {
          options.color = OFF;
        } else {
          std::cerr
            << "invalid --colour argument \"" << optarg << "\"\n"
            << "valid arguments are \"auto\", \"off\", and \"on\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 130: // --trace ...
        if (strcmp(optarg, "handle_reads") == 0) {
          options.traces |= TC_HANDLE_READS;
        } else if (strcmp(optarg, "handle_writes") == 0) {
          options.traces |= TC_HANDLE_WRITES;
        } else if (strcmp(optarg, "queue") == 0) {
          options.traces |= TC_QUEUE;
        } else if (strcmp(optarg, "set") == 0) {
          options.traces |= TC_SET;
        } else if (strcmp(optarg, "symmetry_reduction") == 0) {
          options.traces |= TC_SYMMETRY_REDUCTION;
        } else if (strcmp(optarg, "all") == 0) {
          options.traces = uint64_t(-1);
        } else {
          std::cerr
            << "invalid --trace argument \"" << optarg << "\"\n"
            << "valid arguments are \"handle_reads\", \"handle_writes\", "
              "\"queue\", \"set\", and \"symmetry_reduction\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 131: // --deadlock-detection ...
        if (strcmp(optarg, "off") == 0) {
          options.deadlock_detection = DEADLOCK_DETECTION_OFF;
        } else if (strcmp(optarg, "stuck") == 0) {
          options.deadlock_detection = DEADLOCK_DETECTION_STUCK;
        } else if (strcmp(optarg, "stuttering") == 0) {
          options.deadlock_detection = DEADLOCK_DETECTION_STUTTERING;
        } else {
          std::cerr << "invalid argument to --deadlock-detection, \"" << optarg
            << "\"\n";
          exit(EXIT_FAILURE);
        }
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
        options.set_capacity = size_t(pagesize) * size_t(physpages);

        // Never expand the set.
        options.set_expand_threshold = 100;

        break;
      }

      case 134: // --symmetry-reduction ...
        if (strcmp(optarg, "off") == 0) {
          options.symmetry_reduction = SYMMETRY_REDUCTION_OFF;
        } else if (strcmp(optarg, "heuristic") == 0) {
          options.symmetry_reduction = SYMMETRY_REDUCTION_HEURISTIC;
        } else if (strcmp(optarg, "exhaustive") == 0) {
          options.symmetry_reduction = SYMMETRY_REDUCTION_EXHAUSTIVE;
        } else {
          std::cerr << "invalid argument to --symmetry-reduction, \"" << optarg
            << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 135: // --sandbox ...
        if (strcmp(optarg, "on") == 0) {
          options.sandbox_enabled = true;
        } else if (strcmp(optarg, "off") == 0) {
          options.sandbox_enabled = false;
        } else {
          std::cerr << "invalid argument to --sandbox, \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 136: // --max-errors ...
        try {
          options.max_errors = std::stoul(optarg);
        } catch (std::exception&) {
          std::cerr << "invalid --max-errors argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        if (options.max_errors == 0) {
          std::cerr << "invalid --max-errors argument \"" << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 137: // --counterexample-trace ...
        if (strcmp(optarg, "full") == 0) {
          options.counterexample_trace = FULL;
        } else if (strcmp(optarg, "diff") == 0) {
          options.counterexample_trace = DIFF;
        } else if (strcmp(optarg, "off") == 0) {
          options.counterexample_trace = CEX_OFF;
        } else {
          std::cerr << "invalid argument to --counterexample-trace, \""
            << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 138: // --output-format ...
        if (strcmp(optarg, "machine-readable") == 0) {
          options.machine_readable_output = true;
          // Disable colour that would interfere with XML
          options.color = OFF;
        } else if (strcmp(optarg, "human-readable") == 0) {
          options.machine_readable_output = false;
        } else {
          std::cerr << "invalid argument to --output-format, \""
            << optarg << "\"\n";
          exit(EXIT_FAILURE);
        }
        break;

      case 139: // --version
        std::cout << "Rumur version " << VERSION << "\n";
        exit(EXIT_SUCCESS);

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

    auto inf = std::make_shared<std::ifstream>(argv[optind]);
    if (!inf->is_open()) {
      std::cerr << "failed to open " << argv[optind] << "\n";
      exit(EXIT_FAILURE);
    }
    in = inf;
  }

  if (out == nullptr) {
    std::cerr << "output file is required\n";
    exit(EXIT_FAILURE);
  }

  if (options.threads == 0) {
    // automatic
    long r = sysconf(_SC_NPROCESSORS_ONLN);
    if (r < 1) {
      options.threads = 1;
    } else {
      options.threads = static_cast<unsigned long>(r);
    }
  }
}

template<typename T, typename U>
static bool contains(const T &container, U predicate) {
  return std::find_if(container.begin(), container.end(), predicate)
    != container.end();
}

static bool has_start_state(const rumur::Ptr<rumur::Rule> &r) {
  if (isa<rumur::StartState>(r))
    return true;
  if (auto a = dynamic_cast<const rumur::AliasRule*>(r.get()))
    return contains(a->rules, has_start_state);
  if (auto s = dynamic_cast<const rumur::Ruleset*>(r.get()))
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
  rumur::Ptr<rumur::Model> m;
  try {
    m = rumur::parse(in == nullptr ? &std::cin : in.get());
    resolve_symbols(*m);
    validate_model(*m);
    m->reindex();
  } catch (rumur::Error &e) {
    std::cerr << e.loc << ":" << e.what() << "\n";
    return EXIT_FAILURE;
  }

  assert(m != nullptr);
  validate(*m);

  assert(out != nullptr);
  if (output_checker(*out, *m) != 0)
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
