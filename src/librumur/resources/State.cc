#include <bitset>

struct State {
    std::bitset<STATE_SIZE_BITS> data;
    const State *previous;

  public:
    State(const State *s): data(s->data), previous(s) {
    }

    bool operator==(const State &other) const {
        return data == other.data;
    }
};
