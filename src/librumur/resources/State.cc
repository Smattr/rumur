/* The state type is a specialisation of this class. */
template<uint64_t SIZE_BITS>
class StateBase {

  public:
    std::bitset<SIZE_BITS> data;
    const StateBase<SIZE_BITS> *previous = nullptr;

    StateBase<SIZE_BITS> *duplicate() const {
        auto *s = new StateBase<SIZE_BITS>(*this);
        s->previous = this;
        return s;
    }

    bool operator==(const StateBase<SIZE_BITS> &other) const {
        return data == other.data;
    }

    bool operator!=(const StateBase<SIZE_BITS> &other) const {
        return !(*this == other);
    }

};

using State = StateBase<STATE_SIZE_BITS>;

/* A suitable implementation of hash for storing States in associated
 * containers.
 */
struct state_hash {

  private:
    std::hash<std::bitset<STATE_SIZE_BITS>> hasher;

  public:
    std::size_t operator()(const State *s) const {
        return hasher(s->data);
    }
};
