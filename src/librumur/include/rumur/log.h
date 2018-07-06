#pragma once

#include <iostream>

namespace rumur {

/* Logging interface. This is expected to be used through a singleton instance,
 * declared below. Errors are not logged using this, but are instead thrown as
 * exceptions.
 */
class Log {

 public:
  enum Level {
    SILENT,
    WARNING,
    INFO,
    DEBUG,
  };

  std::ostream *warn;
  std::ostream *info;
  std::ostream *debug;

  Log(std::ostream &stream = std::cerr, Level level_ = WARNING);

  // Change the current log level.
  void set_level(Level level_);

  // Change the stream log messages are written to.
  void set_stream(std::ostream &stream);

 private:
  std::ostream *out;
  Level level;

  void update_streams(void);
};

// The log itself that should be used.
extern Log log;

}
