#pragma once

#include <iostream>

namespace rumur {

/* Logging interface. Some other parts of librumur expect one of these to be
 * provided and will use it for logging messages. Errors are not logged using
 * one of these, but are instead thrown as exceptions.
 */
class Log {

 public:
  enum LogLevel {
    SILENT,
    WARNINGS,
    INFORMATIONAL,
    DEBUG,
  };

  std::ostream &warn;
  std::ostream &info;
  std::ostream &debug;

  Log(std::ostream &out, LogLevel level);
};

/* A logger that discards all messages. Use this if you do not need logging
 * diagnostics.
 */
extern Log null_log;

}
