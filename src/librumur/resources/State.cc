/* The state type is a specialisation of this class. */
template<uint64_t SIZE_BITS>
class StateBase {

    std::bitset<SIZE_BITS> data;

};

using State = StateBase<STATE_SIZE_BITS>;
