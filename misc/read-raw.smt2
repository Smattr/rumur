; proof of correctness of read_raw()

; To prove read_raw() correct, we wish to prove that its implementation
; corresponds to a specification:
;
;   spec(h.base, h.offset, h.width) == impl(h)
;
; Its specification is straightforward to define:
;
;   spec(h_base, h_offset, h_width)
;     = ((*(integer*)h_base) >> h_offset) & ((((integer)1) << h_width) - 1)
;
; where “integer” is an infinite precision integer type. Note however that we
; can relax this constraint. Offsets are always within the first byte (i.e. they
; are bit 0 - bit 7) and the maximum supported width is 64 bits. Therefore we
; can treat the base as pointing to a 71-bit vector, instead of to an infinite
; precision number.
;
; The implementation of read_raw() can be viewed in ../rumur/resources/header.c.
; To put it into a more suitable form for SMT reasoning, we transform it into a
; branchless Static Single Assignment (SSA) form:
;
;   // inline handle_align()
;   size_t width = h_width + h_offset;
;   size_t aligned_width = width % 8 != 0 ? width + 8 - width % 8 : width;
;
;   size_t low_size = aligned_width / 8;
;   size_t low_size2 = low_size > sizeof(uint64_t) ? sizeof(uint64_t) : low_size;
;
;   // inline copy_out64()
;   uint64_t low = 0;
;   memcpy(&low, h_base, low_size2);
;
;   uint64_t low2 = low >> h_offset;
;
;   size_t high_size = aligned_width / 8 - low_size2;
;
;   // inline copy_out64()
;   uint64_t high = 0;
;   high_size != 0 ? memcpy(&high, h_base + sizeof(low2), high_size) : (void);
;   uint64_t high2 = high_size != 0 ? high << (sizeof(low2) * 8 - h_offset : 0;
;
;   uint64_t low3 = high_size != 0 ? low2 | high2 : low2;
;
;   uint64_t mask = h_width < sizeof(low3) * 8 ? (UINT64_C(1) << h_width) - 1 : 0;
;   uint64_t low4 = h_width < sizeof(low3) * 8 ? low3 & mask : low3;
;
;   uint64_t ret2 = h_width == 0 ? 0 : low4;
;
; Now we can construct a correctness proof by transliterating both the
; specification and implementation into SMT, and then proving that they are
; equal for all valid inputs.
;
; TODO: also represent uint128_t extraction option

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(set-logic QF_AUFBV)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; input base, that we treat as a bitvector
(declare-fun h_base () (_ BitVec 71))

; input offset, that can be 0-7
(declare-fun h_offset () (_ BitVec 64))
(assert (bvule h_offset (_ bv7 64)))

; input width, that can be up to 64
(declare-fun h_width () (_ BitVec 64))
(assert (bvule h_width (_ bv64 64)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; the spec, which we can state quite simply
(declare-fun spec () (_ BitVec 64))
(assert (= spec (bvand
  ((_ extract 63 0) (bvlshr h_base ((_ zero_extend 7) h_offset)))
  (bvsub (bvshl (_ bv1 64) h_width) (_ bv1 64)))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; transliterate the implementation

; size_t width = h_width + h_offset;
(declare-fun width () (_ BitVec 64))
(assert (= width (bvadd h_width h_offset)))

; size_t aligned_width = width % 8 != 0 ? width + 8 - width % 8 : width;
(declare-fun aligned_width () (_ BitVec 64))
(assert (= aligned_width (ite
  (not (= (bvsmod width (_ bv8 64)) (_ bv0 64)))
    (bvadd width (bvsub (_ bv8 64) (bvsmod width (_ bv8 64))))
    width)))

; low_size = aligned.width / 8
(declare-fun low_size () (_ BitVec 64))
(assert (= low_size (bvudiv aligned_width (_ bv8 64))))

; size_t low_size2 = low_size > sizeof(uint64_t) ? sizeof(uint64_t) : low_size;
(declare-fun low_size2 () (_ BitVec 64))
(assert (= low_size2 (ite (bvugt low_size (_ bv8 64)) (_ bv8 64) low_size)))

; memcpy(&low, h_base, low_size2);
(declare-fun low () (_ BitVec 64))
(assert (= low (bvor
  (ite (bvugt low_size2 (_ bv0 64)) ((_ zero_extend 56) ((_ extract 7 0) h_base)) (_ bv0 64))
  (ite (bvugt low_size2 (_ bv1 64)) (bvshl ((_ zero_extend 56) ((_ extract 15 8) h_base)) (_ bv8 64)) (_ bv0 64))
  (ite (bvugt low_size2 (_ bv2 64)) (bvshl ((_ zero_extend 56) ((_ extract 23 16) h_base)) (_ bv16 64)) (_ bv0 64))
  (ite (bvugt low_size2 (_ bv3 64)) (bvshl ((_ zero_extend 56) ((_ extract 31 24) h_base)) (_ bv24 64)) (_ bv0 64))
  (ite (bvugt low_size2 (_ bv4 64)) (bvshl ((_ zero_extend 56) ((_ extract 39 32) h_base)) (_ bv32 64)) (_ bv0 64))
  (ite (bvugt low_size2 (_ bv5 64)) (bvshl ((_ zero_extend 56) ((_ extract 47 40) h_base)) (_ bv40 64)) (_ bv0 64))
  (ite (bvugt low_size2 (_ bv6 64)) (bvshl ((_ zero_extend 56) ((_ extract 55 48) h_base)) (_ bv48 64)) (_ bv0 64))
  (ite (bvugt low_size2 (_ bv7 64)) (bvshl ((_ zero_extend 56) ((_ extract 63 56) h_base)) (_ bv56 64)) (_ bv0 64)))))

; uint64_t low2 = low >> h_offset;
(declare-fun low2 () (_ BitVec 64))
(assert (= low2 (bvlshr low h_offset)))

; size_t high_size = aligned_width / 8 - low_size2;
(declare-fun high_size () (_ BitVec 64))
(assert (= high_size (bvsub (bvudiv aligned_width (_ bv8 64)) low_size2)))

;   high_size != 0 ? memcpy(&high, h_base + sizeof(low2), high_size) : (void);
(declare-fun high () (_ BitVec 64))
(assert (= high
  (ite (not (= high_size (_ bv0 64))) (bvor
    (ite (bvugt high_size (_ bv0 64)) ((_ zero_extend 56) ((_ extract 7 0) (bvlshr h_base (_ bv64 71)))) (_ bv0 64))
    (ite (bvugt high_size (_ bv1 64)) (bvshl ((_ zero_extend 56) ((_ extract 15 8) (bvlshr h_base (_ bv64 71)))) (_ bv8 64)) (_ bv0 64))
    (ite (bvugt high_size (_ bv2 64)) (bvshl ((_ zero_extend 56) ((_ extract 23 16) (bvlshr h_base (_ bv64 71)))) (_ bv16 64)) (_ bv0 64))
    (ite (bvugt high_size (_ bv3 64)) (bvshl ((_ zero_extend 56) ((_ extract 31 24) (bvlshr h_base (_ bv64 71)))) (_ bv24 64)) (_ bv0 64))
    (ite (bvugt high_size (_ bv4 64)) (bvshl ((_ zero_extend 56) ((_ extract 39 32) (bvlshr h_base (_ bv64 71)))) (_ bv32 64)) (_ bv0 64))
    (ite (bvugt high_size (_ bv5 64)) (bvshl ((_ zero_extend 56) ((_ extract 47 40) (bvlshr h_base (_ bv64 71)))) (_ bv40 64)) (_ bv0 64))
    (ite (bvugt high_size (_ bv6 64)) (bvshl ((_ zero_extend 56) ((_ extract 55 48) (bvlshr h_base (_ bv64 71)))) (_ bv48 64)) (_ bv0 64))
    (ite (bvugt high_size (_ bv7 64)) (bvshl ((_ zero_extend 56) ((_ extract 63 56) (bvlshr h_base (_ bv64 71)))) (_ bv56 64)) (_ bv0 64)))
  (_ bv0 64))))

; uint64_t high2 = high_size != 0 ? high << (sizeof(low2) * 8 - h_offset : 0;
(declare-fun high2 () (_ BitVec 64))
(assert (= high2
  (ite (not (= high_size (_ bv0 64)))
    (bvshl high (bvsub (bvmul (_ bv8 64) (_ bv8 64)) h_offset))
    (_ bv0 64))))

; uint64_t low3 = high_size != 0 ? low2 | high2 : low2;
(declare-fun low3 () (_ BitVec 64))
(assert (= low3
  (ite (not (= high_size (_ bv0 64)))
    (bvor low2 high2)
    low2)))

; uint64_t mask = h_width < sizeof(low3) * 8 ? (UINT64_C(1) << h_width) - 1 : 0;
(declare-fun mask () (_ BitVec 64))
(assert (= mask (ite
  (bvult h_width (bvmul (_ bv8 64) (_ bv8 64)))
    (bvsub (bvshl (_ bv1 64) h_width) (_ bv1 64))
    (bvnot (_ bv0 64)))))

; uint64_t low4 = h_width < sizeof(low3) * 8 ? low3 & mask : low3;
(declare-fun low4 () (_ BitVec 64))
(assert (= low4
  (ite (bvult h_width (bvmul (_ bv8 64) (_ bv8 64)))
    (bvand low3 mask)
    low3)))

; uint64_t ret2 = h_width == 0 ? 0 : low4;
(declare-fun ret2 () (_ BitVec 64))
(assert (= ret2
  (ite (= h_width (_ bv0 64))
    (_ bv0 64)
    low4)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; attempt to find a case where the specification and the value returned by the
; implementation differ
(assert (not (= spec ret2)))
(check-sat)
