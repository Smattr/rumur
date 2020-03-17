#pragma once

// global options set by command line flags
struct Options {

  // add Rumur-optional semicolons for compatibility with CMurphi?
  bool explicit_semicolons = false;

  // remove liveness invariants and statements?
  bool remove_liveness = false;

  // turn switch statements into if statements?
  bool switch_to_if = false;
};

extern Options options;
