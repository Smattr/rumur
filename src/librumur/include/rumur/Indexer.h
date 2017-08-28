#pragma once

namespace rumur {

// Generic helper for generating unique IDs
class Indexer {

  private:
    unsigned long index = 0;

  public:
    unsigned long new_index();

};

}
