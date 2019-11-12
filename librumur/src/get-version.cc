#include <rumur/get-version.h>
#include <string>

// this symbol is generated
extern const char *RUMUR_VERSION;

namespace rumur {

std::string get_version() {
  return RUMUR_VERSION;
}

}
