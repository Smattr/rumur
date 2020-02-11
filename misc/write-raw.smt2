; proof of correctness of write_raw()

; TODO: introductory explanation of how this is similar to the read_raw() proof

; TODO: explanation of the following SSA
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
;   size_t high_bits = u128_branch ? low_size2 < sizeof(low) ? aligned_width - h_offset - h_width : 0 : 0;
;   uint128_t and_mask2 = u128_branch ? low_size2 < sizeof(low) ? and_mask | (((((uint128_t)1) << high_bits) - 1) << (low_size2 * 8 - high_bits)) : and_mask : 0;
;
;   uint128_t low2 = u128_branch ? (low & and_mask2) | or_mask2 : 0;
;
;   // inline copy_in128()
;   u128_branch ? memcpy(h_base, &low2, low_size2) : (void);
;
;   size_t high_size = u128_branch ? aligned_width / 8 - low_size2 : 0;
;
;   // inline copy_out128()
;   uint128_t high = 0;
;   u128_branch ? high_size != 0 ? memcpy(&high, h_base + sizeof(low2), high_size) : (void) : (void);
;
;   uint128_t or_mask3 = u128_branch ? high_size != 0 ? ((uint128_t)v) >> (sizeof(low2) * 8 - h_offset) : 0 : 0;
;   uint128_t and_mask3 = u128_branch ? high_size != 0 ? (~((uint128_t)0)) & ~((((uint128_t)1) << (aligned_width - h_width)) - 1) : 0 : 0;
;
;   uint128_t high2 = u128_branch ? high_size != 0 ? (high & and_mask3) | or_mask3 : 0 : 0;
;
;   // inline copy_in128()
;   u128_branch ? high_size != 0 ? memcpy(h_base + sizeof(low2), &high2, high_size) : (void) : (void);
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
;   size_t high_bits2 = u128_branch ? 0 : low_size4 < sizeof(low3) ? aligned_width - h_offset - h_width : 0;
;   uint64_t and_mask5 = u128_branch ? 0 : low_size4 < sizeof(low3) ? and_mask4 |(((UINT64_C(1) << high_bits2) - 1) << (low_size4 * 8 - high_bits2)) : and_mask4;
;
;   uint64_t low4 = u128_branch ? 0 : (low3 & and_mask5) | or_mask5;
;
;   // inline copy_in64()
;   u128_branch ? (void) : memcpy(h_base, &low4, low_size4);
;
;   size_t high_size2 = u128_branch ? 0 : aligned_width - low_size4;
;
;   // inline copy_out64()
;   uint64_t high3 = 0;
;   u128_branch ? (void) : high_size2 != 0 ? memcpy(&high3, h_base + sizeof(low4), high_size2) : (void);
;
;   uint64_t or_mask6 = u128_branch ? 0 : high_size2 != 0 ? ((uint64_t)v) >> (sizeof(low4) * 8 - h_offset) : 0;
;   uint64_t and_mask6 = u128_branch ? 0 : high_size2 != 0 ? (~UINT64_C(0)) & ~((UINT64_C(1) << (aligned_width - h_width)) - 1) : 0;
;
;   uint64_t high4 = u128_branch ? 0 : high_size2 != 0 ? (high3 & and_mask6) | or_mask6 : 0;
;
;   // inline copy_in64()
;   u128_branch ? (void) : high_size2 != 0 ? memcpy(h_base + sizeof(low4), &high4, high_size2) : (void);

; TODO: the actual proof
