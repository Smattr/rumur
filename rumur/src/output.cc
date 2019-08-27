#include "assume-statements-count.h"
#include <cstddef>
#include <fstream>
#include <iostream>
#include "generate.h"
#include "max-simple-width.h"
#include "options.h"
#include "resources.h"
#include <rumur/rumur.h>
#include <string>
#include <utility>
#include "ValueType.h"

using namespace rumur;

static std::ostream &operator<<(std::ostream &out, Color c) {
  switch (c) {

    case Color::OFF:
      out << "OFF";
      break;

    case Color::ON:
      out << "ON";
      break;

    case Color::AUTO:
      out << "AUTO";
      break;

  }

  return out;
}

static std::ostream &operator<<(std::ostream &out, DeadlockDetection d) {
  switch (d) {

    case DeadlockDetection::OFF:
      out << "DEADLOCK_DETECTION_OFF";
      break;

    case DeadlockDetection::STUCK:
      out << "DEADLOCK_DETECTION_STUCK";
      break;

    case DeadlockDetection::STUTTERING:
      out << "DEADLOCK_DETECTION_STUTTERING";
      break;

  }

  return out;
}

static std::ostream &operator<<(std::ostream &out, SymmetryReduction s) {
  switch (s) {

    case SymmetryReduction::OFF:
      out << "SYMMETRY_REDUCTION_OFF";
      break;

    case SymmetryReduction::HEURISTIC:
      out << "SYMMETRY_REDUCTION_HEURISTIC";
      break;

    case SymmetryReduction::EXHAUSTIVE:
      out << "SYMMETRY_REDUCTION_EXHAUSTIVE";
      break;

  }

  return out;
}

static std::ostream &operator<<(std::ostream &out, CounterexampleTrace c) {
  switch (c) {

    case CounterexampleTrace::OFF:
      out << "CEX_OFF";
      break;

    case CounterexampleTrace::DIFF:
      out << "DIFF";
      break;

    case CounterexampleTrace::FULL:
      out << "FULL";
      break;

  }

  return out;
}

int output_checker(const std::string &path, const Model &model,
    const std::pair<ValueType, ValueType> &value_types) {

  std::ofstream out(path);
  if (!out)
    return -1;

  if (options.log_level < LogLevel::DEBUG)
    out << "#define NDEBUG 1\n\n";

  out

    // #includes
    << std::string((const char*)resources_includes_c, resources_includes_c_len)
    << "\n"

    // Settings that are used in header.c
    << "enum { SET_CAPACITY = " << options.set_capacity << "ul };\n\n"
    << "enum { SET_EXPAND_THRESHOLD = " << options.set_expand_threshold << " };\n\n"
    << "static const enum { OFF, ON, AUTO } COLOR = " << options.color << ";\n\n"
    << "enum trace_category_t {\n"
    << "  TC_HANDLE_READS       = " << TC_HANDLE_READS << ",\n"
    << "  TC_HANDLE_WRITES      = " << TC_HANDLE_WRITES << ",\n"
    << "  TC_QUEUE              = " << TC_QUEUE << ",\n"
    << "  TC_SET                = " << TC_SET << ",\n"
    << "  TC_SYMMETRY_REDUCTION = " << TC_SYMMETRY_REDUCTION << ",\n"
    << "};\n"
    << "static const uint64_t TRACES_ENABLED = UINT64_C(" << options.traces << ");\n\n"
    << "static const enum {\n"
    << "  DEADLOCK_DETECTION_OFF,\n"
    << "  DEADLOCK_DETECTION_STUCK,\n"
    << "  DEADLOCK_DETECTION_STUTTERING,\n"
    << "} DEADLOCK_DETECTION = " << options.deadlock_detection << ";\n\n"
    << "static const enum {\n"
    << "  SYMMETRY_REDUCTION_OFF,\n"
    << "  SYMMETRY_REDUCTION_HEURISTIC,\n"
    << "  SYMMETRY_REDUCTION_EXHAUSTIVE,\n"
    << "} SYMMETRY_REDUCTION = " << options.symmetry_reduction << ";\n\n"
    << "enum { SANDBOX_ENABLED = " << options.sandbox_enabled << " };\n\n"
    << "enum { MAX_ERRORS = " << options.max_errors << "ul };\n\n"
    << "enum { THREADS = " << options.threads << "ul };\n\n"
    << "enum { STATE_SIZE_BITS = " << model.size_bits() << "ul };\n\n"
    << "enum { ASSUME_STATEMENTS_COUNT = " << assume_statements_count(model) << "ul };\n\n"
    << "#define LIVENESS_COUNT " << model.liveness_count() << "\n\n"
    << "#define CEX_OFF 0\n"
    << "#define DIFF 1\n"
    << "#define FULL 2\n"
    << "#define COUNTEREXAMPLE_TRACE " << options.counterexample_trace << "\n\n"
    << "enum { MACHINE_READABLE_OUTPUT = " << options.machine_readable_output
      << " };\n\n"
    << "enum { MAX_SIMPLE_WIDTH = " << max_simple_width(model) << " };\n\n"
    << "#define BOUND " << options.bound << "\n\n"
    << "typedef " << value_types.first.c_type << " value_t;\n"
    << "#define VALUE_MIN " << value_types.first.int_min << "\n"
    << "#define VALUE_MAX " << value_types.first.int_max << "\n"
    << "#define VALUE_C(x) " << value_types.first.int_c << "(x)\n"
    << "#define PRIVAL " << value_types.first.pri << "\n"
    << "typedef " << value_types.second.c_type << " raw_value_t;\n"
    << "#define RAW_VALUE_MIN " << value_types.second.int_min << "\n"
    << "#define RAW_VALUE_MAX " << value_types.second.int_max << "\n"
    << "#define PRIRAWVAL " << value_types.second.pri << "\n\n";

  generate_cover_array(out, model);

    // Static boiler plate code
  out
    << std::string((const char*)resources_header_c, resources_header_c_len)
    << "\n";

  // the model itself
  generate_model(out, model);

  return 0;
}
