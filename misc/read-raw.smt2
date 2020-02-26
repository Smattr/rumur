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
; precision number. However, instead of using these bounds, we assume that in
; future we may support widths up to 128 bits if we have a uint128_t type
; available. So we need to treat the base as a 128 + 7 = 135-bit vector.
;
; The implementation of read_raw() can be viewed in ../rumur/resources/header.c.
; To put it into a more suitable form for SMT reasoning, we transform it into a
; branchless Static Single Assignment (SSA) form:
;
;   // inline handle_align()
;   size_t width = h_width + h_offset;
;   size_t aligned_width = width % 8 != 0 ? width + 8 - width % 8 : width;
;
;   bool u128_branch = aligned_width > (sizeof(uint64_t) - 1) * 8;
;
;   size_t low_size = u128_branch ? aligned_width / 8 : 0;
;   size_t low_size2 = u128_branch ? low_size > sizeof(uint128_t) ? sizeof(uint128_t) : low_size : 0;
;
;   // inline copy_out128()
;   uint128_t low = 0;
;   u128_branch ? memcpy(&low, h_base, low_size2) : (void);
;
;   uint128_t low2 = u128_branch ? low >> h_offset : 0;
;
;   size_t high_size = u128_branch ? aligned_width / 8 - low_size2 : 0;
;
;   // inline copy_out128()
;   uint128_t high = 0;
;   u128_branch ? high_size != 0 ? memcpy(&high, h_base + sizeof(low2), high_size) : (void) : (void);
;
;   uint128_t high2 = u128_branch ? high_size != 0 ? high << (sizeof(low2) * 8 - h_offset) : 0 : 0;
;
;   uint128_t low3 = u128_branch ? high_size != 0 ? low2 | high2 : low2 : 0;
;
;   uint128_t mask = u128_branch ? h_width < sizeof(low3) * 8 ? (((uint128_t)1) << h_width) - 1 : 0 : 0;
;   uint128_t low4 = u128_branch ? h_width < sizeof(low3) * 8 ? low3 & mask : low3 : 0;
;
;   uint64_t ret = low4;
;
;   // future possibility where we can read out >64-bit values
;   uint128_t ret2 = low4;
;
;   size_t low_size3 = aligned_width / 8;
;   size_t low_size4 = low_size3 > sizeof(uint64_t) ? sizeof(uint64_t) : low_size3;
;
;   // inline copy_out64()
;   uint64_t low5 = 0;
;   memcpy(&low5, h_base, low_size4);
;
;   uint64_t low6 = low5 >> h_offset;
;
;   size_t high_size2 = aligned_width / 8 - low_size4;
;
;   // inline copy_out64()
;   uint64_t high3 = 0;
;   high_size2 != 0 ? memcpy(&high3, h_base + sizeof(low6), high_size2) : (void);
;   uint64_t high4 = high_size2 != 0 ? high3 << (sizeof(low6) * 8 - h_offset : 0;
;
;   uint64_t low7 = high_size2 != 0 ? low6 | high4 : low6;
;
;   uint64_t mask2 = h_width < sizeof(low7) * 8 ? (UINT64_C(1) << h_width) - 1 : 0;
;   uint64_t low8 = h_width < sizeof(low7) * 8 ? low7 & mask2 : low7;
;
;   uint64_t ret3 = h_width == 0 ? 0 : low8;
;
; Now we can construct a correctness proof by transliterating both the
; specification and implementation into SMT, and then proving that they are
; equal for all valid inputs.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(set-logic QF_AUFBV)
(set-option :produce-models true)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; input base, that we treat as a bitvector
(declare-fun h_base () (_ BitVec 135))

; input offset, that can be 0-7
(declare-fun h_offset () (_ BitVec 64))
(assert (bvule h_offset (_ bv7 64)))

; input width, that can be up to 64, but we future proof this to allow up to 128
(declare-fun h_width () (_ BitVec 64))
(assert (bvule h_width (_ bv128 64)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; the spec, which we can state quite simply
(declare-fun spec () (_ BitVec 128))
(assert (= spec (bvand
  ((_ extract 127 0) (bvlshr h_base ((_ zero_extend 71) h_offset)))
  (bvsub (bvshl (_ bv1 128) ((_ zero_extend 64) h_width)) (_ bv1 128)))))

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

; bool u128_branch = aligned_width > (sizeof(uint64_t) - 1) * 8;
(declare-fun u128_branch () Bool)
(assert (= u128_branch (bvugt aligned_width (bvmul (bvsub (_ bv8 64) (_ bv1 64)) (_ bv8 64)))))

; size_t low_size = u128_branch ? aligned_width / 8 : 0;
(declare-fun low_size () (_ BitVec 64))
(assert (= low_size
  (ite u128_branch
    (bvudiv aligned_width (_ bv8 64))
    (_ bv0 64))))

; size_t low_size2 = u128_branch ? low_size > sizeof(uint128_t) ? sizeof(uint128_t) : low_size : 0;
(declare-fun low_size2 () (_ BitVec 64))
(assert (= low_size2
  (ite u128_branch
    (ite (bvugt low_size (_ bv16 64))
      (_ bv16 64)
      low_size)
    (_ bv0 64))))

; u128_branch ? memcpy(&low, h_base, low_size2) : (void);
(declare-fun low () (_ BitVec 128))
(assert (= low
  (ite u128_branch (bvor
    (ite (bvugt low_size2 (_ bv0  64))        ((_ zero_extend 120) ((_ extract   7   0) h_base)) (_ bv0   128))
    (ite (bvugt low_size2 (_ bv1  64)) (bvshl ((_ zero_extend 120) ((_ extract  15   8) h_base)) (_ bv8   128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv2  64)) (bvshl ((_ zero_extend 120) ((_ extract  23  16) h_base)) (_ bv16  128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv3  64)) (bvshl ((_ zero_extend 120) ((_ extract  31  24) h_base)) (_ bv24  128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv4  64)) (bvshl ((_ zero_extend 120) ((_ extract  39  32) h_base)) (_ bv32  128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv5  64)) (bvshl ((_ zero_extend 120) ((_ extract  47  40) h_base)) (_ bv40  128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv6  64)) (bvshl ((_ zero_extend 120) ((_ extract  55  48) h_base)) (_ bv48  128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv7  64)) (bvshl ((_ zero_extend 120) ((_ extract  63  56) h_base)) (_ bv56  128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv8  64)) (bvshl ((_ zero_extend 120) ((_ extract  71  64) h_base)) (_ bv64  128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv9  64)) (bvshl ((_ zero_extend 120) ((_ extract  79  72) h_base)) (_ bv72  128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv10 64)) (bvshl ((_ zero_extend 120) ((_ extract  87  80) h_base)) (_ bv80  128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv11 64)) (bvshl ((_ zero_extend 120) ((_ extract  95  88) h_base)) (_ bv88  128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv12 64)) (bvshl ((_ zero_extend 120) ((_ extract 103  96) h_base)) (_ bv96  128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv13 64)) (bvshl ((_ zero_extend 120) ((_ extract 111 104) h_base)) (_ bv104 128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv14 64)) (bvshl ((_ zero_extend 120) ((_ extract 119 112) h_base)) (_ bv112 128)) (_ bv0 128))
    (ite (bvugt low_size2 (_ bv15 64)) (bvshl ((_ zero_extend 120) ((_ extract 127 120) h_base)) (_ bv120 128)) (_ bv0 128)))
    (_ bv0 128))))

; uint128_t low2 = u128_branch ? low >> h_offset : 0;
(declare-fun low2 () (_ BitVec 128))
(assert (= low2
  (ite u128_branch
    (bvlshr low ((_ zero_extend 64) h_offset))
    (_ bv0 128))))

; size_t high_size = u128_branch ? aligned_width / 8 - low_size2 : 0;
(declare-fun high_size () (_ BitVec 64))
(assert (= high_size
  (ite u128_branch
    (bvsub (bvudiv aligned_width (_ bv8 64)) low_size2)
    (_ bv0 64))))

; u128_branch ? high_size != 0 ? memcpy(&high, h_base + sizeof(low2), high_size) : (void) : (void);
(declare-fun high () (_ BitVec 128))
(assert (= high
  (ite u128_branch
    (ite (not (= high_size (_ bv0 64))) (bvor
      (ite (bvugt high_size (_  bv0 64)) (bvshl ((_ zero_extend 120) ((_ extract   7   0) (bvlshr h_base (_ bv128 135)))) (_  bv0  128)) (_ bv0 128))
      (ite (bvugt high_size (_  bv1 64)) (bvshl ((_ zero_extend 120) ((_ extract  15   8) (bvlshr h_base (_ bv128 135)))) (_  bv8  128)) (_ bv0 128))
      (ite (bvugt high_size (_  bv2 64)) (bvshl ((_ zero_extend 120) ((_ extract  23  16) (bvlshr h_base (_ bv128 135)))) (_  bv16 128)) (_ bv0 128))
      (ite (bvugt high_size (_  bv3 64)) (bvshl ((_ zero_extend 120) ((_ extract  31  24) (bvlshr h_base (_ bv128 135)))) (_  bv24 128)) (_ bv0 128))
      (ite (bvugt high_size (_  bv4 64)) (bvshl ((_ zero_extend 120) ((_ extract  39  32) (bvlshr h_base (_ bv128 135)))) (_  bv32 128)) (_ bv0 128))
      (ite (bvugt high_size (_  bv5 64)) (bvshl ((_ zero_extend 120) ((_ extract  47  40) (bvlshr h_base (_ bv128 135)))) (_  bv40 128)) (_ bv0 128))
      (ite (bvugt high_size (_  bv6 64)) (bvshl ((_ zero_extend 120) ((_ extract  55  48) (bvlshr h_base (_ bv128 135)))) (_  bv48 128)) (_ bv0 128))
      (ite (bvugt high_size (_  bv7 64)) (bvshl ((_ zero_extend 120) ((_ extract  63  56) (bvlshr h_base (_ bv128 135)))) (_  bv56 128)) (_ bv0 128))
      (ite (bvugt high_size (_  bv8 64)) (bvshl ((_ zero_extend 120) ((_ extract  71  64) (bvlshr h_base (_ bv128 135)))) (_  bv64 128)) (_ bv0 128))
      (ite (bvugt high_size (_  bv9 64)) (bvshl ((_ zero_extend 120) ((_ extract  79  72) (bvlshr h_base (_ bv128 135)))) (_  bv72 128)) (_ bv0 128))
      (ite (bvugt high_size (_ bv10 64)) (bvshl ((_ zero_extend 120) ((_ extract  87  80) (bvlshr h_base (_ bv128 135)))) (_  bv80 128)) (_ bv0 128))
      (ite (bvugt high_size (_ bv11 64)) (bvshl ((_ zero_extend 120) ((_ extract  95  88) (bvlshr h_base (_ bv128 135)))) (_  bv88 128)) (_ bv0 128))
      (ite (bvugt high_size (_ bv12 64)) (bvshl ((_ zero_extend 120) ((_ extract 103  96) (bvlshr h_base (_ bv128 135)))) (_  bv96 128)) (_ bv0 128))
      (ite (bvugt high_size (_ bv13 64)) (bvshl ((_ zero_extend 120) ((_ extract 111 104) (bvlshr h_base (_ bv128 135)))) (_ bv104 128)) (_ bv0 128))
      (ite (bvugt high_size (_ bv14 64)) (bvshl ((_ zero_extend 120) ((_ extract 119 112) (bvlshr h_base (_ bv128 135)))) (_ bv112 128)) (_ bv0 128))
      (ite (bvugt high_size (_ bv15 64)) (bvshl ((_ zero_extend 120) ((_ extract 127 120) (bvlshr h_base (_ bv128 135)))) (_ bv120 128)) (_ bv0 128)))
      (_ bv0 128))
    (_ bv0 128))))

; uint128_t high2 = u128_branch ? high_size != 0 ? high << (sizeof(low2) * 8 - h_offset) : 0 : 0;
(declare-fun high2 () (_ BitVec 128))
(assert (= high2
  (ite u128_branch
    (ite (not (= high_size (_ bv0 64)))
      (bvshl high (bvsub (bvmul (_ bv16 128) (_ bv8 128)) ((_ zero_extend 64) h_offset)))
      (_ bv0 128))
    (_ bv0 128))))

; uint128_t low3 = u128_branch ? high_size != 0 ? low2 | high2 : low2 : 0;
(declare-fun low3 () (_ BitVec 128))
(assert (= low3
  (ite u128_branch
    (ite (not (= high_size (_ bv0 64)))
      (bvor low2 high2)
      low2)
    (_ bv0 128))))

; uint128_t mask = u128_branch ? h_width < sizeof(low3) * 8 ? (((uint128_t)1) << h_width) - 1 : 0 : 0;
(declare-fun mask () (_ BitVec 128))
(assert (= mask
  (ite u128_branch
    (ite (bvult h_width (bvmul (_ bv16 64) (_ bv8 64)))
      (bvsub (bvshl (_ bv1 128) ((_ zero_extend 64) h_width)) (_ bv1 128))
      (_ bv0 128))
   (_ bv0 128))))

; uint128_t low4 = u128_branch ? h_width < sizeof(low3) * 8 ? low3 & mask : low3 : 0;
(declare-fun low4 () (_ BitVec 128))
(assert (= low4
  (ite u128_branch
    (ite (bvult h_width (bvmul (_ bv16 64) (_ bv8 64)))
      (bvand low3 mask)
      low3)
    (_ bv0 128))))

; uint64_t ret = low4;
(declare-fun ret () (_ BitVec 64))
(assert (= ret
  ((_ extract 63 0) low4)))

; uint128_t ret2 = low4;
(declare-fun ret2 () (_ BitVec 128))
(assert (= ret2 low4))

; low_size3 = aligned.width / 8
(declare-fun low_size3 () (_ BitVec 64))
(assert (= low_size3 (bvudiv aligned_width (_ bv8 64))))

; size_t low_size4 = low_size3 > sizeof(uint64_t) ? sizeof(uint64_t) : low_size3;
(declare-fun low_size4 () (_ BitVec 64))
(assert (= low_size4 (ite (bvugt low_size3 (_ bv8 64)) (_ bv8 64) low_size3)))

; memcpy(&low5, h_base, low_size4);
(declare-fun low5 () (_ BitVec 64))
(assert (= low5 (bvor
  (ite (bvugt low_size4 (_ bv0 64))        ((_ zero_extend 56) ((_ extract  7  0) h_base)) (_  bv0 64))
  (ite (bvugt low_size4 (_ bv1 64)) (bvshl ((_ zero_extend 56) ((_ extract 15  8) h_base)) (_  bv8 64)) (_ bv0 64))
  (ite (bvugt low_size4 (_ bv2 64)) (bvshl ((_ zero_extend 56) ((_ extract 23 16) h_base)) (_ bv16 64)) (_ bv0 64))
  (ite (bvugt low_size4 (_ bv3 64)) (bvshl ((_ zero_extend 56) ((_ extract 31 24) h_base)) (_ bv24 64)) (_ bv0 64))
  (ite (bvugt low_size4 (_ bv4 64)) (bvshl ((_ zero_extend 56) ((_ extract 39 32) h_base)) (_ bv32 64)) (_ bv0 64))
  (ite (bvugt low_size4 (_ bv5 64)) (bvshl ((_ zero_extend 56) ((_ extract 47 40) h_base)) (_ bv40 64)) (_ bv0 64))
  (ite (bvugt low_size4 (_ bv6 64)) (bvshl ((_ zero_extend 56) ((_ extract 55 48) h_base)) (_ bv48 64)) (_ bv0 64))
  (ite (bvugt low_size4 (_ bv7 64)) (bvshl ((_ zero_extend 56) ((_ extract 63 56) h_base)) (_ bv56 64)) (_ bv0 64)))))

; uint64_t low6 = low5 >> h_offset;
(declare-fun low6 () (_ BitVec 64))
(assert (= low6 (bvlshr low5 h_offset)))

; size_t high_size2 = aligned_width / 8 - low_size4;
(declare-fun high_size2 () (_ BitVec 64))
(assert (= high_size2 (bvsub (bvudiv aligned_width (_ bv8 64)) low_size4)))

; high_size2 != 0 ? memcpy(&high3, h_base + sizeof(low6), high_size2) : (void);
(declare-fun high3 () (_ BitVec 64))
(assert (= high3
  (ite (not (= high_size2 (_ bv0 64))) (bvor
    (ite (bvugt high_size2 (_ bv0 64))        ((_ zero_extend 56) ((_ extract  7  0) (bvlshr h_base (_ bv64 135)))) (_  bv0 64))
    (ite (bvugt high_size2 (_ bv1 64)) (bvshl ((_ zero_extend 56) ((_ extract 15  8) (bvlshr h_base (_ bv64 135)))) (_  bv8 64)) (_ bv0 64))
    (ite (bvugt high_size2 (_ bv2 64)) (bvshl ((_ zero_extend 56) ((_ extract 23 16) (bvlshr h_base (_ bv64 135)))) (_ bv16 64)) (_ bv0 64))
    (ite (bvugt high_size2 (_ bv3 64)) (bvshl ((_ zero_extend 56) ((_ extract 31 24) (bvlshr h_base (_ bv64 135)))) (_ bv24 64)) (_ bv0 64))
    (ite (bvugt high_size2 (_ bv4 64)) (bvshl ((_ zero_extend 56) ((_ extract 39 32) (bvlshr h_base (_ bv64 135)))) (_ bv32 64)) (_ bv0 64))
    (ite (bvugt high_size2 (_ bv5 64)) (bvshl ((_ zero_extend 56) ((_ extract 47 40) (bvlshr h_base (_ bv64 135)))) (_ bv40 64)) (_ bv0 64))
    (ite (bvugt high_size2 (_ bv6 64)) (bvshl ((_ zero_extend 56) ((_ extract 55 48) (bvlshr h_base (_ bv64 135)))) (_ bv48 64)) (_ bv0 64))
    (ite (bvugt high_size2 (_ bv7 64)) (bvshl ((_ zero_extend 56) ((_ extract 63 56) (bvlshr h_base (_ bv64 135)))) (_ bv56 64)) (_ bv0 64)))
  (_ bv0 64))))

; uint64_t high4 = high_size2 != 0 ? high3 << (sizeof(low6) * 8 - h_offset : 0;
(declare-fun high4 () (_ BitVec 64))
(assert (= high4
  (ite (not (= high_size2 (_ bv0 64)))
    (bvshl high3 (bvsub (bvmul (_ bv8 64) (_ bv8 64)) h_offset))
    (_ bv0 64))))

; uint64_t low7 = high_size2 != 0 ? low6 | high4 : low6;
(declare-fun low7 () (_ BitVec 64))
(assert (= low7
  (ite (not (= high_size2 (_ bv0 64)))
    (bvor low6 high4)
    low6)))

; uint64_t mask2 = h_width < sizeof(low7) * 8 ? (UINT64_C(1) << h_width) - 1 : 0;
(declare-fun mask2 () (_ BitVec 64))
(assert (= mask2 (ite
  (bvult h_width (bvmul (_ bv8 64) (_ bv8 64)))
    (bvsub (bvshl (_ bv1 64) h_width) (_ bv1 64))
    (bvnot (_ bv0 64)))))

; uint64_t low8 = h_width < sizeof(low7) * 8 ? low7 & mask2 : low7;
(declare-fun low8 () (_ BitVec 64))
(assert (= low8
  (ite (bvult h_width (bvmul (_ bv8 64) (_ bv8 64)))
    (bvand low7 mask2)
    low7)))

; uint64_t ret3 = h_width == 0 ? 0 : low8;
(declare-fun ret3 () (_ BitVec 64))
(assert (= ret3
  (ite (= h_width (_ bv0 64))
    (_ bv0 64)
    low8)))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; attempt to find a case where the specification and the value returned by the
; implementation differ
(assert (not (and

  ; the uint128_t with uint64_t return path only returns the correct answer for
  ; 57-64-bit unpacks
  (or (not u128_branch) (bvugt h_width (_ bv64 64)) (= ((_ extract 63 0) spec) ret))

  ; the uint128_t with uint128_t return path is correct for all widths when
  ; enabled
  (or (not u128_branch) (= spec ret2))

  ; the uint64_t path returns the correct answer for widths ≤64
  (or (bvugt h_width (_ bv64 64)) (= ((_ extract 63 0) spec) ret3))

  )))
(check-sat)
;(get-model)
