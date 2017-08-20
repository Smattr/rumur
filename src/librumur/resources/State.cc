/* The state type is a specialisation of this class. */
template<uint64_t SIZE_BITS>
class StateBase {

  public:
    std::bitset<SIZE_BITS> data;
    StateBase<SIZE_BITS> *previous = nullptr;

    StateBase<SIZE_BITS> *duplicate() const {
        auto *s = new StateBase<SIZE_BITS>(*this);
        s->previous = this;
        return s;
    }

};

using State = StateBase<STATE_SIZE_BITS>;
