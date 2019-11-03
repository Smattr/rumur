#pragma once

// global options set by command line flags
struct Options {

  // add Rumur-optional semicolons for compatibility with CMurphi?
  bool explicit_semicolons = false;
};

extern Options options;
