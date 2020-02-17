; proof of correctness of write_raw()

; Similar to read-raw.smt2, the following proves that write_raw() does what it
; is intended to do. Unlike the read_raw() proof, we cannot write a simple
; equivalence to describe write_raw()’s correctness because we want to prove
; multiple things. Namely, (1) it writes the given value to the target byte
; buffer and (2) it does not overwrite any unrelated bits.
;
; The implementation of write_raw() can be viewed in
; ../rumur/resources/header.c. To begin with, we transform it into a branchless
; Static Single Assignment (SSA) form:
;
;   // inline handle_align()
;   size_t width = h_width + h_offset;
;   size_t aligned_width = width % 8 != 0 ? width + 8 - width % 8 : width;
;
;   bool u128_branch = defined(__SIZEOF_INT128__) && aligned_width > (sizeof(uint64_t) - 1) * 8;
;
;   size_t low_size = u128_branch ? aligned_width / 8 : 0;
;   size_t low_size2 = u128_branch ? low_size > sizeof(uint128_t) ? sizeof(uint128_t) : low_size : 0;
;
;   // inline copy_out128()
;   uint128_t low = 0;
;   u128_branch ? memcpy(&low, h_base, low_size2) : (void);
;
;   uint128_t or_mask = u128_branch ? ((uint128_t)v) << h_offset : 0;
;   uint128_t or_mask2 = u128_branch ? low_size2 < sizeof(low) ? or_mask & ((((uint128_t)1) << (low_size2 * 8)) - 1) : or_mask : 0;
;
;   uint128_t and_mask = u128_branch ? (((uint128_t)1) << h_offset) - 1 : 0;
;   size_t high_bits = u128_branch ? h_width + h_offset < sizeof(low) * 8 ? aligned_width - h_offset - h_width : 0 : 0;
;   uint128_t and_mask2 = u128_branch ? h_width + h_offset < sizeof(low) * 8 ? and_mask | (((((uint128_t)1) << high_bits) - 1) << (low_size2 * 8 - high_bits)) : and_mask : 0;
;
;   uint128_t low2 = u128_branch ? (low & and_mask2) | or_mask2 : 0;
;
;   // inline copy_in128()
;   void *h_base2 = h_base;
;   u128_branch ? memcpy(h_base2, &low2, low_size2) : (void);
;
;   size_t high_size = u128_branch ? aligned_width / 8 - low_size2 : 0;
;
;   // inline copy_out128()
;   uint128_t high = 0;
;   u128_branch ? high_size != 0 ? memcpy(&high, h_base + sizeof(low2), high_size) : (void) : (void);
;
;   uint128_t or_mask3 = u128_branch ? high_size != 0 ? ((uint128_t)v) >> (sizeof(low2) * 8 - h_offset) : 0 : 0;
;   uint128_t and_mask3 = u128_branch ? high_size != 0 ? ~((((uint128_t)1) << (h_width + h_offset - sizeof(low2) * 8)) - 1) : 0 : 0;
;
;   uint128_t high2 = u128_branch ? high_size != 0 ? (high & and_mask3) | or_mask3 : 0 : 0;
;
;   // inline copy_in128()
;   void *h_base3 = h_base2;
;   u128_branch ? high_size != 0 ? memcpy(h_base3 + sizeof(low2), &high2, high_size) : (void) : (void);
;
;   // return from u128_branch
;
;   // uint64_t logic follows
;
;   size_t low_size3 = u128_branch ? 0 : aligned_width / 8;
;   size_t low_size4 = u128_branch ? 0 : low_size3 > sizeof(uint64_t) ? sizeof(uint64_t) : low_size3;
;
;   // inline copy_out64()
;   uint64_t low3 = 0;
;   u128_branch ? (void) : memcpy(&low3, h_base, low_size4);
;
;   uint64_t or_mask4 = u128_branch ? 0 : ((uint64_t)v) << h_offset;
;   uint64_t or_mask5 = u128_branch ? 0 : low_size4 < sizeof(low3) ? or_mask4 & ((UINT64_C(1) << (low_size4 * 8)) - 1) : or_mask4;
;
;   uint64_t and_mask4 = u128_branch ? 0 : (UINT64_C(1) << h_offset) - 1;
;   size_t high_bits2 = u128_branch ? 0 : h_width + h_offset < sizeof(low3) * 8 ? aligned_width - h_offset - h_width : 0;
;   uint64_t and_mask5 = u128_branch ? 0 : h_wdith + h_offset < sizeof(low3) * 8 ? and_mask4 | (((UINT64_C(1) << high_bits2) - 1) << (low_size4 * 8 - high_bits2)) : and_mask4;
;
;   uint64_t low4 = u128_branch ? 0 : (low3 & and_mask5) | or_mask5;
;
;   // inline copy_in64()
;   void *h_base4 = h_base;
;   u128_branch ? (void) : memcpy(h_base4, &low4, low_size4);
;
;   size_t high_size2 = u128_branch ? 0 : aligned_width / 8 - low_size4;
;
;   // inline copy_out64()
;   uint64_t high3 = 0;
;   u128_branch ? (void) : high_size2 != 0 ? memcpy(&high3, h_base4 + sizeof(low4), high_size2) : (void);
;
;   uint64_t or_mask6 = u128_branch ? 0 : high_size2 != 0 ? ((uint64_t)v) >> (sizeof(low4) * 8 - h_offset) : 0;
;   uint64_t and_mask6 = u128_branch ? 0 : high_size2 != 0 ? ~((UINT64_C(1) << (h_width + h_offset - sizeof(low4) * 8)) - 1) : 0;
;
;   uint64_t high4 = u128_branch ? 0 : high_size2 != 0 ? (high3 & and_mask6) | or_mask6 : 0;
;
;   // inline copy_in64()
;   void *h_base5 = h_base4;
;   u128_branch ? (void) : high_size2 != 0 ? memcpy(h_base5 + sizeof(low4), &high4, high_size2) : (void);
;
; Now we can start translating to SMT.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(set-logic QF_AUFBV)
(set-option :produce-models true)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; input value, that we treat as 128-bit to be future proof
(declare-fun v () (_ BitVec 128))

; input base, that we treat as a bitvector
(declare-fun h_base () (_ BitVec 136))

; input offset, that can be 0-7
(declare-fun h_offset () (_ BitVec 64))
(assert (bvule h_offset (_ bv7 64)))

; input width, that can be up to 64, but we future proof this to allow up to 128
(declare-fun h_width () (_ BitVec 64))
(assert (bvule h_width (_ bv128 64)))

; treat whether we have a 128-bit type as an input
(declare-fun defined__SIZEOF_INT128__ () Bool)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; We cannot yet describe the spec, as we need to refer to the final value of
; h_base. Defer this to after we have expressed the logic of write_raw().

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; translate this
;
;  /* sanitise input value */
;  if (h.width < sizeof(v) * 8) {
;    v &= (UINT64_C(1) << h.width) - 1;
;  }
;
; into an assumption on the input
(assert (bvult v (bvshl (_ bv1 128) ((_ zero_extend 64) h_width))))

; now transliterate the rest of the implementation

; size_t width = h_width + h_offset;
(declare-fun width () (_ BitVec 64))
(assert (= width (bvadd h_width h_offset)))

; size_t aligned_width = width % 8 != 0 ? width + 8 - width % 8 : width;
(declare-fun aligned_width () (_ BitVec 64))
(assert (= aligned_width (ite
  (not (= (bvsmod width (_ bv8 64)) (_ bv0 64)))
    (bvadd width (bvsub (_ bv8 64) (bvsmod width (_ bv8 64))))
    width)))

; bool u128_branch = defined(__SIZEOF_INT128__) && aligned_width > (sizeof(uint64_t) - 1) * 8;
(declare-fun u128_branch () Bool)
(assert (= u128_branch (and defined__SIZEOF_INT128__ (bvugt aligned_width (bvmul (bvsub (_ bv8 64) (_ bv1 64)) (_ bv8 64))))))

; size_t low_size = u128_branch ? aligned_width / 8 : 0;
(declare-fun low_size () (_ BitVec 64))
(assert (= low_size (ite
  u128_branch
    (bvudiv aligned_width (_ bv8 64))
    (_ bv0 64))))

; size_t low_size2 = u128_branch ? low_size > sizeof(uint128_t) ? sizeof(uint128_t) : low_size : 0;
(declare-fun low_size2 () (_ BitVec 64))
(assert (= low_size2 (ite
  u128_branch
    (ite
      (bvugt low_size (_ bv16 64))
      (_ bv16 64)
      low_size)
    (_ bv0 64))))

; uint128_t low = 0;
(declare-fun low () (_ BitVec 128))
(assert (= low (ite
  u128_branch
    (bvor (ite (bvugt low_size2 (_  bv0 64)) (bvshl ((_ zero_extend 120) ((_ extract   7   0) h_base)) (_   bv0 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_  bv1 64)) (bvshl ((_ zero_extend 120) ((_ extract  15   8) h_base)) (_   bv8 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_  bv2 64)) (bvshl ((_ zero_extend 120) ((_ extract  23  16) h_base)) (_  bv16 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_  bv3 64)) (bvshl ((_ zero_extend 120) ((_ extract  31  24) h_base)) (_  bv24 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_  bv4 64)) (bvshl ((_ zero_extend 120) ((_ extract  39  32) h_base)) (_  bv32 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_  bv5 64)) (bvshl ((_ zero_extend 120) ((_ extract  47  40) h_base)) (_  bv40 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_  bv6 64)) (bvshl ((_ zero_extend 120) ((_ extract  55  48) h_base)) (_  bv48 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_  bv7 64)) (bvshl ((_ zero_extend 120) ((_ extract  63  56) h_base)) (_  bv56 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_  bv8 64)) (bvshl ((_ zero_extend 120) ((_ extract  71  64) h_base)) (_  bv64 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_  bv9 64)) (bvshl ((_ zero_extend 120) ((_ extract  79  72) h_base)) (_  bv72 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_ bv10 64)) (bvshl ((_ zero_extend 120) ((_ extract  87  80) h_base)) (_  bv80 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_ bv11 64)) (bvshl ((_ zero_extend 120) ((_ extract  95  88) h_base)) (_  bv88 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_ bv12 64)) (bvshl ((_ zero_extend 120) ((_ extract 103  96) h_base)) (_  bv96 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_ bv13 64)) (bvshl ((_ zero_extend 120) ((_ extract 111 104) h_base)) (_ bv104 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_ bv14 64)) (bvshl ((_ zero_extend 120) ((_ extract 119 112) h_base)) (_ bv112 128)) (_ bv0 128))
          (ite (bvugt low_size2 (_ bv15 64)) (bvshl ((_ zero_extend 120) ((_ extract 127 120) h_base)) (_ bv120 128)) (_ bv0 128)))
    (_ bv0 128))))

; uint128_t or_mask = u128_branch ? ((uint128_t)v) << h_offset : 0;
(declare-fun or_mask () (_ BitVec 128))
(assert (= or_mask (ite
  u128_branch
    (bvshl v ((_ zero_extend 64) h_offset))
    (_ bv0 128))))

; uint128_t or_mask2 = u128_branch ? low_size2 < sizeof(low) ? or_mask & ((((uint128_t)1) << (low_size2 * 8)) - 1) : or_mask : 0;
(declare-fun or_mask2 () (_ BitVec 128))
(assert (= or_mask2 (ite
  u128_branch
    (ite
      (bvult low_size2 (_ bv16 64))
      (bvand or_mask (bvsub (bvshl (_ bv1 128) (bvmul ((_ zero_extend 64) low_size2) (_ bv8 128))) (_ bv1 128)))
      or_mask)
    (_ bv0 128))))

; uint128_t and_mask = u128_branch ? (((uint128_t)1) << h_offset) - 1 : 0;
(declare-fun and_mask () (_ BitVec 128))
(assert (= and_mask (ite
  u128_branch
    (bvsub (bvshl (_ bv1 128) ((_ zero_extend 64) h_offset)) (_ bv1 128))
    (_ bv0 128))))

; size_t high_bits = u128_branch ? h_width + h_offset < sizeof(low) * 8 ? aligned_width - h_offset - h_width : 0 : 0;
(declare-fun high_bits () (_ BitVec 64))
(assert (= high_bits (ite
  u128_branch
    (ite
      (bvult (bvadd h_width h_offset) (bvmul (_ bv16 64) (_ bv8 64)))
      (bvsub (bvsub aligned_width h_offset) h_width)
      (_ bv0 64))
    (_ bv0 64))))

; uint128_t and_mask2 = u128_branch ? h_width + h_offset < sizeof(low) * 8 ? and_mask | (((((uint128_t)1) << high_bits) - 1) << (low_size2 * 8 - high_bits)) : and_mask : 0;
(declare-fun and_mask2 () (_ BitVec 128))
(assert (= and_mask2 (ite
  u128_branch
    (ite
      (bvult (bvadd h_width h_offset) (bvmul (_ bv16 64) (_ bv8 64)))
      (bvor and_mask (bvshl (bvsub (bvshl (_ bv1 128) ((_ zero_extend 64) high_bits)) (_ bv1 128)) (bvsub (bvmul ((_ zero_extend 64) low_size2) (_ bv8 128)) ((_ zero_extend 64) high_bits))))
      and_mask)
    (_ bv0 128))))

; uint128_t low2 = u128_branch ? (low & and_mask2) | or_mask2 : 0;
(declare-fun low2 () (_ BitVec 128))
(assert (= low2 (ite
  u128_branch
    (bvor (bvand low and_mask2) or_mask2)
    (_ bv0 128))))

; u128_branch ? memcpy(h_base2, &low2, low_size2) : (void);
(declare-fun h_base2 () (_ BitVec 136))
(assert (= h_base2 (ite
  u128_branch
    (bvor (ite (bvugt low_size2 (_  bv0 64)) (bvshl ((_ zero_extend 128) ((_ extract   7   0) low2)) (_   bv0 136)) (bvshl ((_ zero_extend 128) ((_ extract   7    0) h_base)) (_   bv0 136)))
          (ite (bvugt low_size2 (_  bv1 64)) (bvshl ((_ zero_extend 128) ((_ extract  15   8) low2)) (_   bv8 136)) (bvshl ((_ zero_extend 128) ((_ extract  15    8) h_base)) (_   bv8 136)))
          (ite (bvugt low_size2 (_  bv2 64)) (bvshl ((_ zero_extend 128) ((_ extract  23  16) low2)) (_  bv16 136)) (bvshl ((_ zero_extend 128) ((_ extract  23   16) h_base)) (_  bv16 136)))
          (ite (bvugt low_size2 (_  bv3 64)) (bvshl ((_ zero_extend 128) ((_ extract  31  24) low2)) (_  bv24 136)) (bvshl ((_ zero_extend 128) ((_ extract  31   24) h_base)) (_  bv24 136)))
          (ite (bvugt low_size2 (_  bv4 64)) (bvshl ((_ zero_extend 128) ((_ extract  39  32) low2)) (_  bv32 136)) (bvshl ((_ zero_extend 128) ((_ extract  39   32) h_base)) (_  bv32 136)))
          (ite (bvugt low_size2 (_  bv5 64)) (bvshl ((_ zero_extend 128) ((_ extract  47  40) low2)) (_  bv40 136)) (bvshl ((_ zero_extend 128) ((_ extract  47   40) h_base)) (_  bv40 136)))
          (ite (bvugt low_size2 (_  bv6 64)) (bvshl ((_ zero_extend 128) ((_ extract  55  48) low2)) (_  bv48 136)) (bvshl ((_ zero_extend 128) ((_ extract  55   48) h_base)) (_  bv48 136)))
          (ite (bvugt low_size2 (_  bv7 64)) (bvshl ((_ zero_extend 128) ((_ extract  63  56) low2)) (_  bv56 136)) (bvshl ((_ zero_extend 128) ((_ extract  63   56) h_base)) (_  bv56 136)))
          (ite (bvugt low_size2 (_  bv8 64)) (bvshl ((_ zero_extend 128) ((_ extract  71  64) low2)) (_  bv64 136)) (bvshl ((_ zero_extend 128) ((_ extract  71   64) h_base)) (_  bv64 136)))
          (ite (bvugt low_size2 (_  bv9 64)) (bvshl ((_ zero_extend 128) ((_ extract  79  72) low2)) (_  bv72 136)) (bvshl ((_ zero_extend 128) ((_ extract  79   72) h_base)) (_  bv72 136)))
          (ite (bvugt low_size2 (_ bv10 64)) (bvshl ((_ zero_extend 128) ((_ extract  87  80) low2)) (_  bv80 136)) (bvshl ((_ zero_extend 128) ((_ extract  87   80) h_base)) (_  bv80 136)))
          (ite (bvugt low_size2 (_ bv11 64)) (bvshl ((_ zero_extend 128) ((_ extract  95  88) low2)) (_  bv88 136)) (bvshl ((_ zero_extend 128) ((_ extract  95   88) h_base)) (_  bv88 136)))
          (ite (bvugt low_size2 (_ bv12 64)) (bvshl ((_ zero_extend 128) ((_ extract 103  96) low2)) (_  bv96 136)) (bvshl ((_ zero_extend 128) ((_ extract 103   96) h_base)) (_  bv96 136)))
          (ite (bvugt low_size2 (_ bv13 64)) (bvshl ((_ zero_extend 128) ((_ extract 111 104) low2)) (_ bv104 136)) (bvshl ((_ zero_extend 128) ((_ extract 111  104) h_base)) (_ bv104 136)))
          (ite (bvugt low_size2 (_ bv14 64)) (bvshl ((_ zero_extend 128) ((_ extract 119 112) low2)) (_ bv112 136)) (bvshl ((_ zero_extend 128) ((_ extract 119  112) h_base)) (_ bv112 136)))
          (ite (bvugt low_size2 (_ bv15 64)) (bvshl ((_ zero_extend 128) ((_ extract 127 120) low2)) (_ bv120 136)) (bvshl ((_ zero_extend 128) ((_ extract 127  120) h_base)) (_ bv120 136)))
          (bvshl ((_ zero_extend 128) ((_ extract 135 128) h_base)) (_ bv128 136)))
    (_ bv0 136))))

; size_t high_size = u128_branch ? aligned_width / 8 - low_size2 : 0;
(declare-fun high_size () (_ BitVec 64))
(assert (= high_size (ite
  u128_branch
    (bvsub (bvudiv aligned_width (_ bv8 64)) low_size2)
    (_ bv0 64))))

; u128_branch ? high_size != 0 ? memcpy(&high, h_base + sizeof(low2), high_size) : (void) : (void);
(declare-fun high () (_ BitVec 128))
(assert (= high (ite
  u128_branch
    (ite
      (not (= high_size (_ bv0 64)))
      (bvor (ite (bvugt high_size (_  bv0 64)) (bvshl ((_ zero_extend 120) ((_ extract   7   0) (bvlshr h_base2 (_ bv128 136)))) (_   bv0 128)) (_ bv0 128))
            (ite (bvugt high_size (_  bv1 64)) (bvshl ((_ zero_extend 120) ((_ extract  15   8) (bvlshr h_base2 (_ bv128 136)))) (_   bv8 128)) (_ bv0 128))
            (ite (bvugt high_size (_  bv2 64)) (bvshl ((_ zero_extend 120) ((_ extract  23  16) (bvlshr h_base2 (_ bv128 136)))) (_  bv16 128)) (_ bv0 128))
            (ite (bvugt high_size (_  bv3 64)) (bvshl ((_ zero_extend 120) ((_ extract  31  24) (bvlshr h_base2 (_ bv128 136)))) (_  bv24 128)) (_ bv0 128))
            (ite (bvugt high_size (_  bv4 64)) (bvshl ((_ zero_extend 120) ((_ extract  39  32) (bvlshr h_base2 (_ bv128 136)))) (_  bv32 128)) (_ bv0 128))
            (ite (bvugt high_size (_  bv5 64)) (bvshl ((_ zero_extend 120) ((_ extract  47  40) (bvlshr h_base2 (_ bv128 136)))) (_  bv40 128)) (_ bv0 128))
            (ite (bvugt high_size (_  bv6 64)) (bvshl ((_ zero_extend 120) ((_ extract  55  48) (bvlshr h_base2 (_ bv128 136)))) (_  bv48 128)) (_ bv0 128))
            (ite (bvugt high_size (_  bv7 64)) (bvshl ((_ zero_extend 120) ((_ extract  63  56) (bvlshr h_base2 (_ bv128 136)))) (_  bv56 128)) (_ bv0 128))
            (ite (bvugt high_size (_  bv8 64)) (bvshl ((_ zero_extend 120) ((_ extract  71  64) (bvlshr h_base2 (_ bv128 136)))) (_  bv64 128)) (_ bv0 128))
            (ite (bvugt high_size (_  bv9 64)) (bvshl ((_ zero_extend 120) ((_ extract  79  72) (bvlshr h_base2 (_ bv128 136)))) (_  bv72 128)) (_ bv0 128))
            (ite (bvugt high_size (_ bv10 64)) (bvshl ((_ zero_extend 120) ((_ extract  87  80) (bvlshr h_base2 (_ bv128 136)))) (_  bv80 128)) (_ bv0 128))
            (ite (bvugt high_size (_ bv11 64)) (bvshl ((_ zero_extend 120) ((_ extract  95  88) (bvlshr h_base2 (_ bv128 136)))) (_  bv88 128)) (_ bv0 128))
            (ite (bvugt high_size (_ bv12 64)) (bvshl ((_ zero_extend 120) ((_ extract 103  96) (bvlshr h_base2 (_ bv128 136)))) (_  bv96 128)) (_ bv0 128))
            (ite (bvugt high_size (_ bv13 64)) (bvshl ((_ zero_extend 120) ((_ extract 111 104) (bvlshr h_base2 (_ bv128 136)))) (_ bv104 128)) (_ bv0 128))
            (ite (bvugt high_size (_ bv14 64)) (bvshl ((_ zero_extend 120) ((_ extract 119 112) (bvlshr h_base2 (_ bv128 136)))) (_ bv112 128)) (_ bv0 128))
            (ite (bvugt high_size (_ bv15 64)) (bvshl ((_ zero_extend 120) ((_ extract 127 120) (bvlshr h_base2 (_ bv128 136)))) (_ bv120 128)) (_ bv0 128)))
      (_ bv0 128))
    (_ bv0 128))))

; uint128_t or_mask3 = u128_branch ? high_size != 0 ? ((uint128_t)v) >> (sizeof(low2) * 8 - h_offset) : 0 : 0;
(declare-fun or_mask3 () (_ BitVec 128))
(assert (= or_mask3 (ite
  u128_branch
    (ite
      (not (= high_size (_ bv0 64)))
      (bvlshr v (bvsub (bvmul (_ bv16 128) (_ bv8 128)) ((_ zero_extend 64) h_offset)))
      (_ bv0 128))
    (_ bv0 128))))

; uint128_t and_mask3 = u128_branch ? high_size != 0 ? ~((((uint128_t)1) << (h_width + h_offset - sizeof(low2) * 8)) - 1) : 0 : 0;
(declare-fun and_mask3 () (_ BitVec 128))
(assert (= and_mask3 (ite
  u128_branch
    (ite
      (not (= high_size (_ bv0 64)))
      (bvnot (bvsub (bvshl (_ bv1 128) ((_ zero_extend 64) (bvsub (bvadd h_width h_offset) (_ bv128 64)))) (_ bv1 128)))
      (_ bv0 128))
    (_ bv0 128))))

; uint128_t high2 = u128_branch ? high_size != 0 ? (high & and_mask3) | or_mask3 : 0 : 0;
(declare-fun high2 () (_ BitVec 128))
(assert (= high2 (ite
  u128_branch
    (ite
      (not (= high_size (_ bv0 64)))
      (bvor (bvand high and_mask3) or_mask3)
      (_ bv0 128))
    (_ bv0 128))))

; u128_branch ? high_size != 0 ? memcpy(h_base + sizeof(low2), &high2, high_size) : (void) : (void);
(declare-fun h_base3 () (_ BitVec 136))
(assert (= h_base3 (ite
  u128_branch
    (ite
      (not (= high_size (_ bv0 64)))
      (bvor ((_ zero_extend 8) ((_ extract 127 0) h_base2))
            (ite (bvugt high_size (_ bv0 64)) (bvshl ((_ zero_extend 128) ((_ extract 7 0) high2)) (_ bv128 136)) (bvshl ((_ zero_extend 128) ((_ extract 135 128) h_base2)) (_ bv128 136))))
      h_base2)
    (_ bv0 136))))

; size_t low_size3 = u128_branch ? 0 : aligned_width / 8;
(declare-fun low_size3 () (_ BitVec 64))
(assert (= low_size3 (ite
  u128_branch
    (_ bv0 64)
    (bvudiv aligned_width (_ bv8 64)))))

; size_t low_size4 = u128_branch ? 0 : low_size3 > sizeof(uint64_t) ? sizeof(uint64_t) : low_size3;
(declare-fun low_size4 () (_ BitVec 64))
(assert (= low_size4 (ite
  u128_branch
    (_ bv0 64)
    (ite
      (bvugt low_size3 (_ bv8 64))
      (_ bv8 64)
      low_size3))))

; u128_branch ? (void) : memcpy(&low3, h_base, low_size4);
(declare-fun low3 () (_ BitVec 64))
(assert (= low3 (ite
  u128_branch
    (_ bv0 64)
    (bvor (ite (bvugt low_size4 (_ bv0 64)) (bvshl ((_ zero_extend 56) ((_ extract  7  0) h_base)) (_  bv0 64)) (_  bv0 64))
          (ite (bvugt low_size4 (_ bv1 64)) (bvshl ((_ zero_extend 56) ((_ extract 15  8) h_base)) (_  bv8 64)) (_  bv0 64))
          (ite (bvugt low_size4 (_ bv2 64)) (bvshl ((_ zero_extend 56) ((_ extract 23 16) h_base)) (_ bv16 64)) (_  bv0 64))
          (ite (bvugt low_size4 (_ bv3 64)) (bvshl ((_ zero_extend 56) ((_ extract 31 24) h_base)) (_ bv24 64)) (_  bv0 64))
          (ite (bvugt low_size4 (_ bv4 64)) (bvshl ((_ zero_extend 56) ((_ extract 39 32) h_base)) (_ bv32 64)) (_  bv0 64))
          (ite (bvugt low_size4 (_ bv5 64)) (bvshl ((_ zero_extend 56) ((_ extract 47 40) h_base)) (_ bv40 64)) (_  bv0 64))
          (ite (bvugt low_size4 (_ bv6 64)) (bvshl ((_ zero_extend 56) ((_ extract 55 48) h_base)) (_ bv48 64)) (_  bv0 64))
          (ite (bvugt low_size4 (_ bv7 64)) (bvshl ((_ zero_extend 56) ((_ extract 63 56) h_base)) (_ bv56 64)) (_  bv0 64))))))

; uint64_t or_mask4 = u128_branch ? 0 : ((uint64_t)v) << h_offset;
(declare-fun or_mask4 () (_ BitVec 64))
(assert (= or_mask4 (ite
  u128_branch
    (_ bv0 64)
    (bvshl ((_ extract 63 0) v) h_offset))))

; uint64_t or_mask5 = u128_branch ? 0 : low_size4 < sizeof(low3) ? or_mask4 & ((UINT64_C(1) << (low_size4 * 8)) - 1) : or_mask4;
(declare-fun or_mask5 () (_ BitVec 64))
(assert (= or_mask5 (ite
  u128_branch
    (_ bv0 64)
    (ite
      (bvult low_size4 (_ bv8 64))
      (bvand or_mask4 (bvsub (bvshl (_ bv1 64) (bvmul low_size4 (_ bv8 64))) (_ bv1 64)))
      or_mask4))))

; uint64_t and_mask4 = u128_branch ? 0 : (UINT64_C(1) << h_offset) - 1;
(declare-fun and_mask4 () (_ BitVec 64))
(assert (= and_mask4 (ite
  u128_branch
    (_ bv0 64)
    (bvsub (bvshl (_ bv1 64) h_offset) (_ bv1 64)))))

; size_t high_bits2 = u128_branch ? 0 : h_wdith + h_offset < sizeof(low3) * 8 ? aligned_width - h_offset - h_width : 0;
(declare-fun high_bits2 () (_ BitVec 64))
(assert (= high_bits2 (ite
  u128_branch
    (_ bv0 64)
    (ite
      (bvult (bvadd h_width h_offset) (bvmul (_ bv8 64) (_ bv8 64)))
      (bvsub (bvsub aligned_width h_offset) h_width)
      (_ bv0 64)))))

; uint64_t and_mask5 = u128_branch ? 0 : h_width + h_offset < sizeof(low3) * 8 ? and_mask4 | (((UINT64_C(1) << high_bits2) - 1) << (low_size4 * 8 - high_bits2)) : and_mask4;
(declare-fun and_mask5 () (_ BitVec 64))
(assert (= and_mask5 (ite
  u128_branch
    (_ bv0 64)
    (ite
      (bvult (bvadd h_width h_offset) (bvmul (_ bv8 64) (_ bv8 64)))
      (bvor and_mask4 (bvshl (bvsub (bvshl (_ bv1 64) high_bits2) (_ bv1 64)) (bvsub (bvmul low_size4 (_ bv8 64)) high_bits2)))
      and_mask4))))

; uint64_t low4 = u128_branch ? 0 : (low3 & and_mask5) | or_mask5;
(declare-fun low4 () (_ BitVec 64))
(assert (= low4 (ite
  u128_branch
    (_ bv0 64)
    (bvor (bvand low3 and_mask5) or_mask5))))

; u128_branch ? (void) : memcpy(h_base4, &low4, low_size4);
(declare-fun h_base4 () (_ BitVec 136))
(assert (= h_base4 (ite
  u128_branch
    (_ bv0 136)
    (bvor (ite (bvugt low_size4 (_ bv0 64)) (bvshl ((_ zero_extend 128) ((_ extract  7  0) low4)) (_  bv0 136)) (bvshl ((_ zero_extend 128) ((_ extract  7  0) h_base)) (_  bv0 136)))
          (ite (bvugt low_size4 (_ bv1 64)) (bvshl ((_ zero_extend 128) ((_ extract 15  8) low4)) (_  bv8 136)) (bvshl ((_ zero_extend 128) ((_ extract 15  8) h_base)) (_  bv8 136)))
          (ite (bvugt low_size4 (_ bv2 64)) (bvshl ((_ zero_extend 128) ((_ extract 23 16) low4)) (_ bv16 136)) (bvshl ((_ zero_extend 128) ((_ extract 23 16) h_base)) (_ bv16 136)))
          (ite (bvugt low_size4 (_ bv3 64)) (bvshl ((_ zero_extend 128) ((_ extract 31 24) low4)) (_ bv24 136)) (bvshl ((_ zero_extend 128) ((_ extract 31 24) h_base)) (_ bv24 136)))
          (ite (bvugt low_size4 (_ bv4 64)) (bvshl ((_ zero_extend 128) ((_ extract 39 32) low4)) (_ bv32 136)) (bvshl ((_ zero_extend 128) ((_ extract 39 32) h_base)) (_ bv32 136)))
          (ite (bvugt low_size4 (_ bv5 64)) (bvshl ((_ zero_extend 128) ((_ extract 47 40) low4)) (_ bv40 136)) (bvshl ((_ zero_extend 128) ((_ extract 47 40) h_base)) (_ bv40 136)))
          (ite (bvugt low_size4 (_ bv6 64)) (bvshl ((_ zero_extend 128) ((_ extract 55 48) low4)) (_ bv48 136)) (bvshl ((_ zero_extend 128) ((_ extract 55 48) h_base)) (_ bv48 136)))
          (ite (bvugt low_size4 (_ bv7 64)) (bvshl ((_ zero_extend 128) ((_ extract 63 56) low4)) (_ bv56 136)) (bvshl ((_ zero_extend 128) ((_ extract 63 56) h_base)) (_ bv56 136)))
          (bvshl ((_ zero_extend 64) ((_ extract 135 64) h_base)) (_ bv64 136))))))

; size_t high_size2 = u128_branch ? 0 : aligned_width / 8 - low_size4;
(declare-fun high_size2 () (_ BitVec 64))
(assert (= high_size2 (ite
  u128_branch
    (_ bv0 64)
    (bvsub (bvudiv aligned_width (_ bv8 64)) low_size4))))


; u128_branch ? (void) : high_size2 != 0 ? memcpy(&high3, h_base4 + sizeof(low4), high_size2) : (void);
(declare-fun high3 () (_ BitVec 64))
(assert (= high3 (ite
  u128_branch
    (_ bv0 64)
    (ite
      (not (= high_size2 (_ bv0 64)))
      (bvor (ite (bvugt high_size2 (_ bv0 64)) (bvshl ((_ zero_extend 56) ((_ extract  7  0) (bvlshr h_base4 (_ bv64 136)))) (_  bv0 64)) (_ bv0 64))
            (ite (bvugt high_size2 (_ bv1 64)) (bvshl ((_ zero_extend 56) ((_ extract 15  8) (bvlshr h_base4 (_ bv64 136)))) (_  bv8 64)) (_ bv0 64))
            (ite (bvugt high_size2 (_ bv2 64)) (bvshl ((_ zero_extend 56) ((_ extract 23 16) (bvlshr h_base4 (_ bv64 136)))) (_ bv16 64)) (_ bv0 64))
            (ite (bvugt high_size2 (_ bv3 64)) (bvshl ((_ zero_extend 56) ((_ extract 31 24) (bvlshr h_base4 (_ bv64 136)))) (_ bv24 64)) (_ bv0 64))
            (ite (bvugt high_size2 (_ bv4 64)) (bvshl ((_ zero_extend 56) ((_ extract 39 32) (bvlshr h_base4 (_ bv64 136)))) (_ bv32 64)) (_ bv0 64))
            (ite (bvugt high_size2 (_ bv5 64)) (bvshl ((_ zero_extend 56) ((_ extract 47 40) (bvlshr h_base4 (_ bv64 136)))) (_ bv40 64)) (_ bv0 64))
            (ite (bvugt high_size2 (_ bv6 64)) (bvshl ((_ zero_extend 56) ((_ extract 55 48) (bvlshr h_base4 (_ bv64 136)))) (_ bv48 64)) (_ bv0 64))
            (ite (bvugt high_size2 (_ bv7 64)) (bvshl ((_ zero_extend 56) ((_ extract 63 56) (bvlshr h_base4 (_ bv64 136)))) (_ bv56 64)) (_ bv0 64)))
      (_ bv0 64)))))

; uint64_t or_mask6 = u128_branch ? 0 : high_size2 != 0 ? ((uint64_t)v) >> (sizeof(low4) * 8 - h_offset) : 0;
(declare-fun or_mask6 () (_ BitVec 64))
(assert (= or_mask6 (ite
  u128_branch
    (_ bv0 64)
    (ite
      (not (= high_size2 (_ bv0 64)))
      (bvlshr ((_ extract 63 0) v) (bvsub (bvmul (_ bv8 64) (_ bv8 64)) h_offset))
      (_ bv0 64)))))

; uint64_t and_mask6 = u128_branch ? 0 : high_size2 != 0 ? ~((UINT64_C(1) << (h_width + h_offset - sizeof(low4) * 8)) - 1) : 0;
(declare-fun and_mask6 () (_ BitVec 64))
(assert (= and_mask6 (ite
  u128_branch
    (_ bv0 64)
    (ite
      (not (= high_size2 (_ bv0 64)))
      (bvnot (bvsub (bvshl (_ bv1 64) (bvsub (bvadd h_width h_offset) (_ bv64 64))) (_ bv1 64)))
      (_ bv0 64)))))

; uint64_t high4 = u128_branch ? 0 : high_size2 != 0 ? (high3 & and_mask6) | or_mask6 : 0;
(declare-fun high4 () (_ BitVec 64))
(assert (= high4 (ite
  u128_branch
    (_ bv0 64)
    (ite
      (not (= high_size2 (_ bv0 64)))
      (bvor (bvand high3 and_mask6) or_mask6)
      (_ bv0 64)))))

; u128_branch ? (void) : high_size2 != 0 ? memcpy(h_base5 + sizeof(low4), &high4, high_size2) : (void);
(declare-fun h_base5 () (_ BitVec 136))
(assert (= h_base5 (ite
  u128_branch
    (_ bv0 136)
    (ite
      (not (= high_size2 (_ bv0 64)))
      (bvor ((_ zero_extend 72) ((_ extract 63 0) h_base4))
            (ite (bvugt high_size2 (_ bv0 64)) (bvshl ((_ zero_extend 128) ((_ extract  7  0) high4)) (_  bv64 136)) (bvshl ((_ zero_extend 128) ((_ extract  71  64) h_base4)) (_  bv64 136)))
            (ite (bvugt high_size2 (_ bv1 64)) (bvshl ((_ zero_extend 128) ((_ extract 15  8) high4)) (_  bv72 136)) (bvshl ((_ zero_extend 128) ((_ extract  79  72) h_base4)) (_  bv72 136)))
            (ite (bvugt high_size2 (_ bv2 64)) (bvshl ((_ zero_extend 128) ((_ extract 23 16) high4)) (_  bv80 136)) (bvshl ((_ zero_extend 128) ((_ extract  87  80) h_base4)) (_  bv80 136)))
            (ite (bvugt high_size2 (_ bv3 64)) (bvshl ((_ zero_extend 128) ((_ extract 31 24) high4)) (_  bv88 136)) (bvshl ((_ zero_extend 128) ((_ extract  95  88) h_base4)) (_  bv88 136)))
            (ite (bvugt high_size2 (_ bv4 64)) (bvshl ((_ zero_extend 128) ((_ extract 39 32) high4)) (_  bv96 136)) (bvshl ((_ zero_extend 128) ((_ extract 103  96) h_base4)) (_  bv96 136)))
            (ite (bvugt high_size2 (_ bv5 64)) (bvshl ((_ zero_extend 128) ((_ extract 47 40) high4)) (_ bv104 136)) (bvshl ((_ zero_extend 128) ((_ extract 111 104) h_base4)) (_ bv104 136)))
            (ite (bvugt high_size2 (_ bv6 64)) (bvshl ((_ zero_extend 128) ((_ extract 55 48) high4)) (_ bv112 136)) (bvshl ((_ zero_extend 128) ((_ extract 119 112) h_base4)) (_ bv112 136)))
            (ite (bvugt high_size2 (_ bv7 64)) (bvshl ((_ zero_extend 128) ((_ extract 63 56) high4)) (_ bv120 136)) (bvshl ((_ zero_extend 128) ((_ extract 127 120) h_base4)) (_ bv120 136)))
            (bvshl ((_ zero_extend 128) ((_ extract 135 128) h_base4)) (_ bv128 136)))
      h_base4))))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; We can now prove the specification we described at the top of this file.

(assert (not (and

  ; on the 128-bit branch, the input value is correctly written to the store
	(or (not u128_branch)
	  (= (bvlshr (bvand h_base3 (bvsub (bvshl (_ bv1 136) ((_ zero_extend 72) (bvadd h_width h_offset))) (_ bv1 136))) ((_ zero_extend 72) h_offset))
	     (bvand ((_ zero_extend 8) v) (bvsub (bvshl (_ bv1 136) ((_ zero_extend 72) h_width)) (_ bv1 136)))))

  ; on the 128-bit branch, all bits below the store range are unaffected
	(or (not u128_branch) (= h_offset (_ bv0 64))
	  (= (bvand h_base  (bvsub (bvshl (_ bv1 136) ((_ zero_extend 72) h_offset)) (_ bv1 136)))
	     (bvand h_base3 (bvsub (bvshl (_ bv1 136) ((_ zero_extend 72) h_offset)) (_ bv1 136)))))

  ; on the 128-bit branch, all bits above the store range are unaffected
	(or (not u128_branch)
	  (= (bvlshr h_base  ((_ zero_extend 72) (bvadd h_width h_offset)))
	     (bvlshr h_base3 ((_ zero_extend 72) (bvadd h_width h_offset)))))

  ; on the 64-bit branch with a width ≤ 64, the input value is correctly
  ; written to the store
	(or u128_branch (bvugt h_width (_ bv64 64))
	  (= (bvlshr (bvand h_base5 (bvsub (bvshl (_ bv1 136) ((_ zero_extend 72) (bvadd h_width h_offset))) (_ bv1 136))) ((_ zero_extend 72) h_offset))
	     (bvand ((_ zero_extend 8) v) (bvsub (bvshl (_ bv1 136) ((_ zero_extend 72) h_width)) (_ bv1 136)))))

  ; on the 64-bit branch with a width ≤ 64, all bits below the store range are
  ; unaffected
	(or u128_branch (= h_offset (_ bv0 64)) (bvugt h_width (_ bv64 64))
	  (= (bvand h_base  (bvsub (bvshl (_ bv1 136) ((_ zero_extend 72) h_offset)) (_ bv1 136)))
	     (bvand h_base5 (bvsub (bvshl (_ bv1 136) ((_ zero_extend 72) h_offset)) (_ bv1 136)))))

  ; on the 64-bit branch with a width ≤ 64, all bits above the store range are
  ; unaffected
	(or u128_branch (bvugt h_width (_ bv64 64))
	  (= (bvlshr h_base  ((_ zero_extend 72) (bvadd h_width h_offset)))
	     (bvlshr h_base5 ((_ zero_extend 72) (bvadd h_width h_offset)))))

	)))

(check-sat)
