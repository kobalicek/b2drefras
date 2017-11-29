#ifndef _SIMD_H
#define _SIMD_H

#include "./globals.h"

#ifdef _MSC_VER
  #include <intrin.h>
#else
  #include <emmintrin.h>
  #include <smmintrin.h>
#endif

#if defined(_M_X64) || defined(__amd64) || defined(__x86_64) || defined(__x86_64__)
  #define SIMD_ARCH_BITS 64
#else
  #define SIMD_ARCH_BITS 32
#endif

#define SIMD_ARCH_SSE3 1
#define SIMD_ARCH_SSSE3 1
#define SIMD_ARCH_SSE4_1 1
#define SIMD_ARCH_AVX 0

#define SIMD_INLINE ALWAYS_INLINE

namespace SIMD {

typedef __m128i I128;
typedef __m128  F128;
typedef __m128d D128;

template<typename T>
union alignas(16) Const128 {
  T d[16 / sizeof(T)];

  I128 i128;
  F128 f128;
  D128 d128;
};

#define SIMD_DEF_I128_1xI8(NAME, X0) \
  static constexpr ::SIMD::Const128<int8_t> NAME = {{ \
    int8_t(X0), int8_t(X0), int8_t(X0), int8_t(X0), \
    int8_t(X0), int8_t(X0), int8_t(X0), int8_t(X0), \
    int8_t(X0), int8_t(X0), int8_t(X0), int8_t(X0), \
    int8_t(X0), int8_t(X0), int8_t(X0), int8_t(X0)  \
  }}

#define SIMD_DEF_I128_16xI8(NAME, X0, X1, X2, X3, X4, X5, X6, X7, X8, X9, X10, X11, X12, X13, X14, X15) \
  static constexpr ::SIMD::Const128<int8_t> NAME = {{ \
    int8_t(X0) , int8_t(X1) , int8_t(X2) , int8_t(X3) , \
    int8_t(X4) , int8_t(X5) , int8_t(X6) , int8_t(X7) , \
    int8_t(X8) , int8_t(X9) , int8_t(X10), int8_t(X11), \
    int8_t(X12), int8_t(X13), int8_t(X14), int8_t(X15)  \
  }}

#define SIMD_DEF_I128_1xI16(NAME, X0) static constexpr ::SIMD::Const128<int16_t> NAME = {{ int16_t(X0), int16_t(X0), int16_t(X0), int16_t(X0), int16_t(X0), int16_t(X0), int16_t(X0), int16_t(X0) }}

#define SIMD_DEF_I128_1xI32(NAME, X0) static constexpr ::SIMD::Const128<int32_t> NAME = {{ int32_t(X0), int32_t(X0), int32_t(X0), int32_t(X0) }}
#define SIMD_DEF_F128_1xF32(NAME, X0) static constexpr ::SIMD::Const128<float  > NAME = {{ float(X0), float(X0), float(X0), float(X0) }}

#define SIMD_DEF_I128_1xI64(NAME, X0) static constexpr ::SIMD::Const128<int64_t> NAME = {{ int64_t(X0), int64_t(X0) }}
#define SIMD_DEF_D128_1xD64(NAME, X0) static constexpr ::SIMD::Const128<double > NAME = {{ double(X0), double(X0) }}

#define SIMD_DEF_I128_4xI32(NAME, X0, X1, X2, X3) static constexpr ::SIMD::Const128<int32_t> NAME = {{ int32_t(X0), int32_t(X1), int32_t(X2), int32_t(X3) }}
#define SIMD_DEF_F128_4xF32(NAME, X0, X1, X2, X3) static constexpr ::SIMD::Const128<float> NAME = {{ float(X0), float(X1), float(X2), float(X3) }}

#define SIMD_DEF_I128_2xI64(NAME, X0, X1) static constexpr ::SIMD::Const128<int64_t> NAME = {{ int64_t(X0), int64_t(X1) }}
#define SIMD_DEF_D128_2xD64(NAME, X0, X1) static constexpr ::SIMD::Const128<double> NAME = {{ double(X0), double(X1) }}

// Must be in anonymous namespace.
namespace {

// ============================================================================
// [SIMD - Cast]
// ============================================================================

template<typename DST, typename SRC_IN> SIMD_INLINE DST vcast(const SRC_IN& x) noexcept { return x; }

template<> SIMD_INLINE F128 vcast(const I128& x) noexcept { return _mm_castsi128_ps(x); }
template<> SIMD_INLINE D128 vcast(const I128& x) noexcept { return _mm_castsi128_pd(x); }
template<> SIMD_INLINE I128 vcast(const F128& x) noexcept { return _mm_castps_si128(x); }
template<> SIMD_INLINE D128 vcast(const F128& x) noexcept { return _mm_castps_pd(x); }
template<> SIMD_INLINE I128 vcast(const D128& x) noexcept { return _mm_castpd_si128(x); }
template<> SIMD_INLINE F128 vcast(const D128& x) noexcept { return _mm_castpd_ps(x); }

// ============================================================================
// [SIMD - I128]
// ============================================================================

SIMD_DEF_I128_1xI32(u16_0080_128, 0x00800080);
SIMD_DEF_I128_1xI32(u16_00FF_128, 0x00FF00FF);
SIMD_DEF_I128_1xI32(u16_0101_128, 0x01010101);

SIMD_DEF_I128_16xI8(pshufb_u32_to_u8_lo , 0, 4, 8,12, 0, 4, 8,12, 0, 4, 8,12, 0, 4, 8, 12);
SIMD_DEF_I128_16xI8(pshufb_u32_to_u16_lo, 0, 1, 4, 5, 8, 9,12,13, 0, 1, 4, 5, 8, 9,12, 13);

SIMD_INLINE I128 vzeroi128() noexcept { return _mm_setzero_si128(); }

SIMD_INLINE I128 vseti128i8(int8_t x) noexcept { return _mm_set1_epi8(x); }
SIMD_INLINE I128 vseti128i16(int16_t x) noexcept { return _mm_set1_epi16(x); }
SIMD_INLINE I128 vseti128i32(int32_t x) noexcept { return _mm_set1_epi32(x); }

SIMD_INLINE I128 vseti128i32(int32_t x1, int32_t x0) noexcept { return _mm_set_epi32(x1, x0, x1, x0); }
SIMD_INLINE I128 vseti128i32(int32_t x3, int32_t x2, int32_t x1, int32_t x0) noexcept { return _mm_set_epi32(x3, x2, x1, x0); }

SIMD_INLINE I128 vseti128i64(int64_t x) noexcept {
  #if SIMD_ARCH_BITS == 64
  return _mm_set1_epi64x(x);
  #else
  return vseti128i32(int32_t(uint64_t(x) >> 32), int32_t(x & 0xFFFFFFFFU));
  #endif
}

SIMD_INLINE I128 vseti128i64(int64_t x1, int64_t x0) noexcept {
return vseti128i32(int32_t(uint64_t(x1) >> 32), int32_t(x1 & 0xFFFFFFFFU),
                   int32_t(uint64_t(x0) >> 32), int32_t(x0 & 0xFFFFFFFFU));
}

SIMD_INLINE I128 vcvti32i128(int32_t x) noexcept { return _mm_cvtsi32_si128(int(x)); }
SIMD_INLINE I128 vcvtu32i128(uint32_t x) noexcept { return _mm_cvtsi32_si128(int(x)); }

SIMD_INLINE int32_t vcvti128i32(const I128& x) noexcept { return int32_t(_mm_cvtsi128_si32(x)); }
SIMD_INLINE uint32_t vcvti128u32(const I128& x) noexcept { return uint32_t(_mm_cvtsi128_si32(x)); }

#if SIMD_ARCH_BITS == 64
// These map to instructions only available to X64.
SIMD_INLINE I128 vcvti64i128(int64_t x) noexcept { return _mm_cvtsi64_si128(x); }
SIMD_INLINE I128 vcvtu64i128(uint64_t x) noexcept { return _mm_cvtsi64_si128(int64_t(x)); }

SIMD_INLINE int64_t vcvti128i64(const I128& x) noexcept { return int64_t(_mm_cvtsi128_si64(x)); }
SIMD_INLINE uint64_t vcvti128u64(const I128& x) noexcept { return uint64_t(_mm_cvtsi128_si64(x)); }
#else
// These are workarounds to still support casting between 64-bit integers and XMMs.
SIMD_INLINE I128 vcvti64i128(int64_t x) noexcept {
  uint32_t lo = uint32_t(uint64_t(x) & 0xFFFFFFFFU);
  uint32_t hi = uint32_t(uint64_t(x) >> 32);
  return _mm_unpacklo_epi32(_mm_cvtsi32_si128(lo), _mm_cvtsi32_si128(hi));
}

SIMD_INLINE I128 vcvtu64i128(uint64_t x) noexcept {
  return vcvti64i128(int64_t(x));
}

SIMD_INLINE int64_t vcvti128i64(const I128& x) noexcept {
  I128 y = _mm_shuffle_epi32(x, _MM_SHUFFLE(2, 3, 0, 1));
  return (int64_t(_mm_cvtsi128_si32(x))      ) +
         (int64_t(_mm_cvtsi128_si32(y)) << 32) ;
}
SIMD_INLINE uint64_t vcvti128u64(const I128& x) noexcept { return uint64_t(vcvti128i64(x)); }
#endif

template<uint8_t A, uint8_t B, uint8_t C, uint8_t D>
SIMD_INLINE I128 vswizli16(const I128& x) noexcept { return _mm_shufflelo_epi16(x, _MM_SHUFFLE(A, B, C, D)); }
template<uint8_t A, uint8_t B, uint8_t C, uint8_t D>
SIMD_INLINE I128 vswizhi16(const I128& x) noexcept { return _mm_shufflehi_epi16(x, _MM_SHUFFLE(A, B, C, D)); }

template<uint8_t A, uint8_t B, uint8_t C, uint8_t D>
SIMD_INLINE I128 vswizi16(const I128& x) noexcept { return vswizhi16<A, B, C, D>(vswizli16<A, B, C, D>(x)); }
template<uint8_t A, uint8_t B, uint8_t C, uint8_t D>
SIMD_INLINE I128 vswizi32(const I128& x) noexcept { return _mm_shuffle_epi32(x, _MM_SHUFFLE(A, B, C, D)); }
template<int A, int B>
SIMD_INLINE I128 vswizi64(const I128& x) noexcept { return vswizi32<A*2 + 1, A*2, B*2 + 1, B*2>(x); }

SIMD_INLINE I128 vswapi64(const I128& x) noexcept { return vswizi64<0, 1>(x); }
SIMD_INLINE I128 vdupli64(const I128& x) noexcept { return vswizi64<0, 0>(x); }
SIMD_INLINE I128 vduphi64(const I128& x) noexcept { return vswizi64<1, 1>(x); }

#if SIMD_ARCH_SSE4_1
SIMD_INLINE I128 vmovli64u8u16(const I128& x) noexcept { return _mm_cvtepu8_epi16(x); }
SIMD_INLINE I128 vmovli64u16u32(const I128& x) noexcept { return _mm_cvtepu16_epi32(x); }
SIMD_INLINE I128 vmovli64u32u64(const I128& x) noexcept { return _mm_cvtepu32_epi64(x); }
#else
SIMD_INLINE I128 vmovli64u8u16(const I128& x) noexcept { return _mm_unpacklo_epi8(x, _mm_setzero_si128()); }
SIMD_INLINE I128 vmovli64u16u32(const I128& x) noexcept { return _mm_unpacklo_epi16(x, _mm_setzero_si128()); }
SIMD_INLINE I128 vmovli64u32u64(const I128& x) noexcept { return _mm_unpacklo_epi32(x, _mm_setzero_si128()); }
#endif

SIMD_INLINE I128 vmovhi64u8u16(const I128& x) noexcept { return _mm_unpackhi_epi8(x, _mm_setzero_si128()); }
SIMD_INLINE I128 vmovhi64u16u32(const I128& x) noexcept { return _mm_unpackhi_epi16(x, _mm_setzero_si128()); }
SIMD_INLINE I128 vmovhi64u32u64(const I128& x) noexcept { return _mm_unpackhi_epi32(x, _mm_setzero_si128()); }

SIMD_INLINE I128 vpacki16i8(const I128& x, const I128& y) noexcept { return _mm_packs_epi16(x, y); }
SIMD_INLINE I128 vpacki16u8(const I128& x, const I128& y) noexcept { return _mm_packus_epi16(x, y); }
SIMD_INLINE I128 vpacki32i16(const I128& x, const I128& y) noexcept { return _mm_packs_epi32(x, y); }

SIMD_INLINE I128 vpacki16i8(const I128& x) noexcept { return vpacki16i8(x, x); }
SIMD_INLINE I128 vpacki16u8(const I128& x) noexcept { return vpacki16u8(x, x); }
SIMD_INLINE I128 vpacki32i16(const I128& x) noexcept { return vpacki32i16(x, x); }

SIMD_INLINE I128 vpacki32u16(const I128& x, const I128& y) noexcept {
  #if SIMD_ARCH_SSE4_1
  return _mm_packus_epi32(x, y);
  #else
  I128 xShifted = _mm_srai_epi32(_mm_slli_epi32(x, 16), 16);
  I128 yShifted = _mm_srai_epi32(_mm_slli_epi32(y, 16), 16);
  return _mm_packs_epi32(xShifted, yShifted);
  #endif
}

SIMD_INLINE I128 vpacki32u16(const I128& x) noexcept {
  #if SIMD_ARCH_SSE4_1
  return vpacki32u16(x, x);
  #else
  I128 xShifted = _mm_srai_epi32(_mm_slli_epi32(x, 16), 16);
  return _mm_packs_epi32(xShifted, xShifted);
  #endif
}

SIMD_INLINE I128 vpacki32i8(const I128& x) noexcept { return vpacki16i8(vpacki32i16(x)); }
SIMD_INLINE I128 vpacki32i8(const I128& x, const I128& y) noexcept { return vpacki16i8(vpacki32i16(x, y)); }
SIMD_INLINE I128 vpacki32i8(const I128& x, const I128& y, const I128& z, const I128& w) noexcept { return vpacki16i8(vpacki32i16(x, y), vpacki32i16(z, w)); }

SIMD_INLINE I128 vpacki32u8(const I128& x) noexcept { return vpacki16u8(vpacki32i16(x)); }
SIMD_INLINE I128 vpacki32u8(const I128& x, const I128& y) noexcept { return vpacki16u8(vpacki32i16(x, y)); }
SIMD_INLINE I128 vpacki32u8(const I128& x, const I128& y, const I128& z, const I128& w) noexcept { return vpacki16u8(vpacki32i16(x, y), vpacki32i16(z, w)); }

// These assume that HI bytes of all inputs are always zero, so the implementation
// can decide between packing with signed/unsigned saturation or vector swizzling.
SIMD_INLINE I128 vpackzzwb(const I128& x) noexcept { return vpacki16u8(x); }
SIMD_INLINE I128 vpackzzwb(const I128& x, const I128& y) noexcept { return vpacki16u8(x, y); }

#if SIMD_ARCH_SSE4_1 || !SIMD_ARCH_SSSE3
SIMD_INLINE I128 vpackzzdw(const I128& x) noexcept { return vpacki32u16(x); }
SIMD_INLINE I128 vpackzzdw(const I128& x, const I128& y) noexcept { return vpacki32u16(x, y); }
#else
SIMD_INLINE I128 vpackzzdw(const I128& x) noexcept {
  return _mm_shuffle_epi8(x, pshufb_u32_to_u16_lo.i128);
}
SIMD_INLINE I128 vpackzzdw(const I128& x, const I128& y) noexcept {
  I128 mask = pshufb_u32_to_u16_lo.i128;
  I128 xLo = _mm_shuffle_epi8(x, mask);
  I128 yLo = _mm_shuffle_epi8(y, mask);
  return _mm_unpacklo_epi64(xLo, yLo);
}
#endif

SIMD_INLINE I128 vpackzzdb(const I128& x) noexcept {
  #if SIMD_ARCH_SSSE3
  I128 mask = pshufb_u32_to_u8_lo.i128;
  return _mm_shuffle_epi8(x, mask);
  #else
  return vpacki16u8(vpacki32i16(x));
  #endif
}

SIMD_INLINE I128 vpackzzdb(const I128& x, const I128& y) noexcept { return vpacki16u8(vpacki32i16(x, y)); }
SIMD_INLINE I128 vpackzzdb(const I128& x, const I128& y, const I128& z, const I128& w) noexcept { return vpacki16u8(vpacki32i16(x, y), vpacki32i16(z, w)); }

SIMD_INLINE I128 vunpackli8(const I128& x, const I128& y) noexcept { return _mm_unpacklo_epi8(x, y); }
SIMD_INLINE I128 vunpackhi8(const I128& x, const I128& y) noexcept { return _mm_unpackhi_epi8(x, y); }

SIMD_INLINE I128 vunpackli16(const I128& x, const I128& y) noexcept { return _mm_unpacklo_epi16(x, y); }
SIMD_INLINE I128 vunpackhi16(const I128& x, const I128& y) noexcept { return _mm_unpackhi_epi16(x, y); }

SIMD_INLINE I128 vunpackli32(const I128& x, const I128& y) noexcept { return _mm_unpacklo_epi32(x, y); }
SIMD_INLINE I128 vunpackhi32(const I128& x, const I128& y) noexcept { return _mm_unpackhi_epi32(x, y); }

SIMD_INLINE I128 vunpackli64(const I128& x, const I128& y) noexcept { return _mm_unpacklo_epi64(x, y); }
SIMD_INLINE I128 vunpackhi64(const I128& x, const I128& y) noexcept { return _mm_unpackhi_epi64(x, y); }

SIMD_INLINE I128 vor(const I128& x, const I128& y) noexcept { return _mm_or_si128(x, y); }
SIMD_INLINE I128 vxor(const I128& x, const I128& y) noexcept { return _mm_xor_si128(x, y); }
SIMD_INLINE I128 vand(const I128& x, const I128& y) noexcept { return _mm_and_si128(x, y); }
SIMD_INLINE I128 vnand(const I128& x, const I128& y) noexcept { return _mm_andnot_si128(x, y); }
SIMD_INLINE I128 vblendmask(const I128& x, const I128& y, const I128& mask) noexcept { return vor(vnand(mask, x), vand(y, mask)); }

//! Blend BITs or BYTEs, taking advantage of `pblendvb` (SSE4.1), if possible.
SIMD_INLINE I128 vblendx(const I128& x, const I128& y, const I128& mask) noexcept {
  #if SIMD_ARCH_SSE4_1
  return _mm_blendv_epi8(x, y, mask);
  #else
  return vblendmask(x, y, mask);
  #endif
}

SIMD_INLINE I128 vaddi8(const I128& x, const I128& y) noexcept { return _mm_add_epi8(x, y); }
SIMD_INLINE I128 vaddi16(const I128& x, const I128& y) noexcept { return _mm_add_epi16(x, y); }
SIMD_INLINE I128 vaddi32(const I128& x, const I128& y) noexcept { return _mm_add_epi32(x, y); }
SIMD_INLINE I128 vaddi64(const I128& x, const I128& y) noexcept { return _mm_add_epi64(x, y); }

SIMD_INLINE I128 vaddsi8(const I128& x, const I128& y) noexcept { return _mm_adds_epi8(x, y); }
SIMD_INLINE I128 vaddsu8(const I128& x, const I128& y) noexcept { return _mm_adds_epu8(x, y); }
SIMD_INLINE I128 vaddsi16(const I128& x, const I128& y) noexcept { return _mm_adds_epi16(x, y); }
SIMD_INLINE I128 vaddsu16(const I128& x, const I128& y) noexcept { return _mm_adds_epu16(x, y); }

SIMD_INLINE I128 vsubi8(const I128& x, const I128& y) noexcept { return _mm_sub_epi8(x, y); }
SIMD_INLINE I128 vsubi16(const I128& x, const I128& y) noexcept { return _mm_sub_epi16(x, y); }
SIMD_INLINE I128 vsubi32(const I128& x, const I128& y) noexcept { return _mm_sub_epi32(x, y); }
SIMD_INLINE I128 vsubi64(const I128& x, const I128& y) noexcept { return _mm_sub_epi64(x, y); }

SIMD_INLINE I128 vsubsi8(const I128& x, const I128& y) noexcept { return _mm_subs_epi8(x, y); }
SIMD_INLINE I128 vsubsu8(const I128& x, const I128& y) noexcept { return _mm_subs_epu8(x, y); }
SIMD_INLINE I128 vsubsi16(const I128& x, const I128& y) noexcept { return _mm_subs_epi16(x, y); }
SIMD_INLINE I128 vsubsu16(const I128& x, const I128& y) noexcept { return _mm_subs_epu16(x, y); }

SIMD_INLINE I128 vmuli16(const I128& x, const I128& y) noexcept { return _mm_mullo_epi16(x, y); }
SIMD_INLINE I128 vmulu16(const I128& x, const I128& y) noexcept { return _mm_mullo_epi16(x, y); }
SIMD_INLINE I128 vmulhi16(const I128& x, const I128& y) noexcept { return _mm_mulhi_epi16(x, y); }
SIMD_INLINE I128 vmulhu16(const I128& x, const I128& y) noexcept { return _mm_mulhi_epu16(x, y); }

template<uint8_t Bits> SIMD_INLINE I128 vslli16(const I128& x) noexcept { return _mm_slli_epi16(x, Bits); }
template<uint8_t Bits> SIMD_INLINE I128 vslli32(const I128& x) noexcept { return _mm_slli_epi32(x, Bits); }
template<uint8_t Bits> SIMD_INLINE I128 vslli64(const I128& x) noexcept { return _mm_slli_epi64(x, Bits); }

template<uint8_t Bits> SIMD_INLINE I128 vsrli16(const I128& x) noexcept { return _mm_srli_epi16(x, Bits); }
template<uint8_t Bits> SIMD_INLINE I128 vsrli32(const I128& x) noexcept { return _mm_srli_epi32(x, Bits); }
template<uint8_t Bits> SIMD_INLINE I128 vsrli64(const I128& x) noexcept { return _mm_srli_epi64(x, Bits); }

template<uint8_t Bits> SIMD_INLINE I128 vsrai16(const I128& x) noexcept { return _mm_srai_epi16(x, Bits); }
template<uint8_t Bits> SIMD_INLINE I128 vsrai32(const I128& x) noexcept { return _mm_srai_epi32(x, Bits); }

template<uint8_t Bytes> SIMD_INLINE I128 vslli128b(const I128& x) noexcept { return _mm_slli_si128(x, Bytes); }
template<uint8_t Bytes> SIMD_INLINE I128 vsrli128b(const I128& x) noexcept { return _mm_srli_si128(x, Bytes); }

#if SIMD_ARCH_SSE4_1
SIMD_INLINE I128 vmini8(const I128& x, const I128& y) noexcept { return _mm_min_epi8(x, y); }
SIMD_INLINE I128 vmaxi8(const I128& x, const I128& y) noexcept { return _mm_max_epi8(x, y); }
#else
SIMD_INLINE I128 vmini8(const I128& x, const I128& y) noexcept { return vblendmask(y, x, _mm_cmpgt_epi8(x, y)); }
SIMD_INLINE I128 vmaxi8(const I128& x, const I128& y) noexcept { return vblendmask(x, y, _mm_cmpgt_epi8(x, y)); }
#endif

SIMD_INLINE I128 vminu8(const I128& x, const I128& y) noexcept { return _mm_min_epu8(x, y); }
SIMD_INLINE I128 vmaxu8(const I128& x, const I128& y) noexcept { return _mm_max_epu8(x, y); }

SIMD_INLINE I128 vmini16(const I128& x, const I128& y) noexcept { return _mm_min_epi16(x, y); }
SIMD_INLINE I128 vmaxi16(const I128& x, const I128& y) noexcept { return _mm_max_epi16(x, y); }

#if SIMD_ARCH_SSE4_1
SIMD_INLINE I128 vminu16(const I128& x, const I128& y) noexcept { return _mm_min_epu16(x, y); }
SIMD_INLINE I128 vmaxu16(const I128& x, const I128& y) noexcept { return _mm_max_epu16(x, y); }
#else
SIMD_INLINE I128 vminu16(const I128& x, const I128& y) noexcept { return _mm_sub_epi16(x, _mm_subs_epu16(x, y)); }
SIMD_INLINE I128 vmaxu16(const I128& x, const I128& y) noexcept { return _mm_add_epi16(x, _mm_subs_epu16(x, y)); }
#endif

#if SIMD_ARCH_SSE4_1
SIMD_INLINE I128 vmini32(const I128& x, const I128& y) noexcept { return _mm_min_epi32(x, y); }
SIMD_INLINE I128 vmaxi32(const I128& x, const I128& y) noexcept { return _mm_max_epi32(x, y); }
#else
SIMD_INLINE I128 vmini32(const I128& x, const I128& y) noexcept { return vblendmask(y, x, _mm_cmpgt_epi32(x, y)); }
SIMD_INLINE I128 vmaxi32(const I128& x, const I128& y) noexcept { return vblendmask(x, y, _mm_cmpgt_epi32(x, y)); }
#endif

SIMD_INLINE I128 vcmpeqi8(const I128& x, const I128& y) noexcept { return _mm_cmpeq_epi8(x, y); }
SIMD_INLINE I128 vcmpgti8(const I128& x, const I128& y) noexcept { return _mm_cmpgt_epi8(x, y); }

SIMD_INLINE I128 vcmpeqi16(const I128& x, const I128& y) noexcept { return _mm_cmpeq_epi16(x, y); }
SIMD_INLINE I128 vcmpgti16(const I128& x, const I128& y) noexcept { return _mm_cmpgt_epi16(x, y); }

SIMD_INLINE I128 vcmpeqi32(const I128& x, const I128& y) noexcept { return _mm_cmpeq_epi32(x, y); }
SIMD_INLINE I128 vcmpgti32(const I128& x, const I128& y) noexcept { return _mm_cmpgt_epi32(x, y); }

#if SIMD_ARCH_SSSE3
SIMD_INLINE I128 vabsi8(const I128& x) noexcept { return _mm_abs_epi8(x); }
#else
SIMD_INLINE I128 vabsi8(const I128& x) noexcept { return vminu8(vsubi8(vzeroi128(), x), x); }
#endif

#if SIMD_ARCH_SSSE3
SIMD_INLINE I128 vabsi16(const I128& x) noexcept { return _mm_abs_epi16(x); }
#else
SIMD_INLINE I128 vabsi16(const I128& x) noexcept { return vmaxi16(vsubi16(vzeroi128(), x), x); }
#endif

#if SIMD_ARCH_SSSE3
SIMD_INLINE I128 vabsi32(const I128& x) noexcept { return _mm_abs_epi32(x); }
#else
SIMD_INLINE I128 vabsi32(const I128& x) noexcept { I128 y = vsrai32<31>(x); return vsubi32(vxor(x, y), y); }
#endif

SIMD_INLINE I128 vloadi128_32(const void* p) noexcept { return _mm_cvtsi32_si128(static_cast<const int*>(p)[0]); }
SIMD_INLINE I128 vloadi128_64(const void* p) noexcept { return _mm_loadl_epi64(static_cast<const I128*>(p)); }
SIMD_INLINE I128 vloadi128a(const void* p) noexcept { return _mm_load_si128(static_cast<const I128*>(p)); }
SIMD_INLINE I128 vloadi128u(const void* p) noexcept { return _mm_loadu_si128(static_cast<const I128*>(p)); }

SIMD_INLINE I128 vloadi128_l64(const I128& x, const void* p) noexcept { return vcast<I128>(_mm_loadl_pd(vcast<D128>(x), static_cast<const double*>(p))); }
SIMD_INLINE I128 vloadi128_h64(const I128& x, const void* p) noexcept { return vcast<I128>(_mm_loadh_pd(vcast<D128>(x), static_cast<const double*>(p))); }

SIMD_INLINE void vstorei32(void* p, const I128& x) noexcept { static_cast<int*>(p)[0] = _mm_cvtsi128_si32(x); }
SIMD_INLINE void vstorei64(void* p, const I128& x) noexcept { _mm_storel_epi64(static_cast<I128*>(p), x); }
SIMD_INLINE void vstorei128a(void* p, const I128& x) noexcept { _mm_store_si128(static_cast<I128*>(p), x); }
SIMD_INLINE void vstorei128u(void* p, const I128& x) noexcept { _mm_storeu_si128(static_cast<I128*>(p), x); }

SIMD_INLINE void vstoreli64(void* p, const I128& x) noexcept { _mm_storel_epi64(static_cast<I128*>(p), x); }
SIMD_INLINE void vstorehi64(void* p, const I128& x) noexcept { _mm_storeh_pd(static_cast<double*>(p), vcast<D128>(x)); }

SIMD_INLINE bool vhasmaski8(const I128& x, int bits0_15) noexcept { return _mm_movemask_epi8(vcast<I128>(x)) == bits0_15; }
SIMD_INLINE bool vhasmaski8(const F128& x, int bits0_15) noexcept { return _mm_movemask_epi8(vcast<I128>(x)) == bits0_15; }
SIMD_INLINE bool vhasmaski8(const D128& x, int bits0_15) noexcept { return _mm_movemask_epi8(vcast<I128>(x)) == bits0_15; }

SIMD_INLINE bool vhasmaski32(const I128& x, int bits0_3) noexcept { return _mm_movemask_ps(vcast<F128>(x)) == bits0_3; }
SIMD_INLINE bool vhasmaski64(const I128& x, int bits0_1) noexcept { return _mm_movemask_pd(vcast<D128>(x)) == bits0_1; }

SIMD_INLINE I128 vdiv255u16(const I128& x) noexcept { return vmulhu16(vaddi16(x, u16_0080_128.i128), u16_0101_128.i128); }

// ============================================================================
// [SIMD - F128]
// ============================================================================

SIMD_INLINE F128 vzerof128() noexcept { return _mm_setzero_ps(); }

SIMD_INLINE F128 vsetf128(float x) noexcept { return _mm_set1_ps(x); }
SIMD_INLINE F128 vsetf128(float x3, float x2, float x1, float x0) noexcept { return _mm_set_ps(x3, x2, x1, x0); }

//! Cast a scalar `float` to `F128` vector type.
SIMD_INLINE F128 vcvtf32f128(float x) noexcept {
  #if defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER)
  // See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=70708
  F128 reg;
  __asm__("" : "=x" (reg) : "0" (x));
  return reg;
  #elif defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  // MSC generates so much code when it sees `_mm_set_ss()`, this is a better alternative.
  return _mm_setr_ps(x, 0, 0, 0);
  #else
  return _mm_set_ss(x);
  #endif
}
SIMD_INLINE float vcvtf128f32(const F128& x) noexcept { return _mm_cvtss_f32(x); }

SIMD_INLINE F128 vcvti32f128(int32_t x) noexcept { return _mm_cvtsi32_ss(vzerof128(), x); }
SIMD_INLINE int32_t vcvtf128i32(const F128& x) noexcept { return _mm_cvtss_si32(x); }
SIMD_INLINE int32_t vcvttf128i32(const F128& x) noexcept { return _mm_cvttss_si32(x); }

#if SIMD_ARCH_BITS == 64
SIMD_INLINE F128 vcvti64f128(int64_t x) noexcept { return _mm_cvtsi64_ss(vzerof128(), x); }
SIMD_INLINE int64_t vcvtf128i64(const F128& x) noexcept { return _mm_cvtss_si64(x); }
SIMD_INLINE int64_t vcvttf128i64(const F128& x) noexcept { return _mm_cvttss_si64(x); }
#endif

template<int A, int B, int C, int D>
SIMD_INLINE F128 vshuff32(const F128& x, const F128& y) noexcept { return _mm_shuffle_ps(x, y, _MM_SHUFFLE(A, B, C, D)); }

template<int A, int B, int C, int D>
SIMD_INLINE F128 vswizf32(const F128& x) noexcept {
  #if !SIMD_ARCH_AVX
  return vcast<F128>(vswizi32<A, B, C, D>(vcast<I128>(x)));
  #else
  return vshuff32<A, B, C, D>(x, x);
  #endif
}

template<int A, int B>
SIMD_INLINE F128 vswizf64(const F128& x) noexcept {
  #if !SIMD_ARCH_AVX
  return vcast<F128>(vswizi64<A, B>(vcast<I128>(x)));
  #else
  return vswizf32<A*2 + 1, A*2, B*2 + 1, B*2>(x);
  #endif
}

SIMD_INLINE F128 vduplf32(const F128& x) noexcept { return vswizf32<2, 2, 0, 0>(x); }
SIMD_INLINE F128 vduphf32(const F128& x) noexcept { return vswizf32<3, 3, 1, 1>(x); }

SIMD_INLINE F128 vswapf64(const F128& x) noexcept { return vswizf64<0, 1>(x); }
SIMD_INLINE F128 vduplf64(const F128& x) noexcept { return vswizf64<0, 0>(x); }
SIMD_INLINE F128 vduphf64(const F128& x) noexcept { return vswizf64<1, 1>(x); }

SIMD_INLINE F128 vunpacklf32(const F128& x, const F128& y) noexcept { return _mm_unpacklo_ps(x, y); }
SIMD_INLINE F128 vunpackhf32(const F128& x, const F128& y) noexcept { return _mm_unpackhi_ps(x, y); }

SIMD_INLINE F128 vor(const F128& x, const F128& y) noexcept { return _mm_or_ps(x, y); }
SIMD_INLINE F128 vxor(const F128& x, const F128& y) noexcept { return _mm_xor_ps(x, y); }
SIMD_INLINE F128 vand(const F128& x, const F128& y) noexcept { return _mm_and_ps(x, y); }
SIMD_INLINE F128 vnand(const F128& x, const F128& y) noexcept { return _mm_andnot_ps(x, y); }
SIMD_INLINE F128 vblendmask(const F128& x, const F128& y, const F128& mask) noexcept { return vor(vnand(mask, x), vand(y, mask)); }

SIMD_INLINE F128 vaddss(const F128& x, const F128& y) noexcept { return _mm_add_ss(x, y); }
SIMD_INLINE F128 vaddps(const F128& x, const F128& y) noexcept { return _mm_add_ps(x, y); }

SIMD_INLINE F128 vsubss(const F128& x, const F128& y) noexcept { return _mm_sub_ss(x, y); }
SIMD_INLINE F128 vsubps(const F128& x, const F128& y) noexcept { return _mm_sub_ps(x, y); }

SIMD_INLINE F128 vmulss(const F128& x, const F128& y) noexcept { return _mm_mul_ss(x, y); }
SIMD_INLINE F128 vmulps(const F128& x, const F128& y) noexcept { return _mm_mul_ps(x, y); }

SIMD_INLINE F128 vdivss(const F128& x, const F128& y) noexcept { return _mm_div_ss(x, y); }
SIMD_INLINE F128 vdivps(const F128& x, const F128& y) noexcept { return _mm_div_ps(x, y); }

SIMD_INLINE F128 vminss(const F128& x, const F128& y) noexcept { return _mm_min_ss(x, y); }
SIMD_INLINE F128 vminps(const F128& x, const F128& y) noexcept { return _mm_min_ps(x, y); }

SIMD_INLINE F128 vmaxss(const F128& x, const F128& y) noexcept { return _mm_max_ss(x, y); }
SIMD_INLINE F128 vmaxps(const F128& x, const F128& y) noexcept { return _mm_max_ps(x, y); }

SIMD_INLINE F128 vcmpeqss(const F128& x, const F128& y) noexcept { return _mm_cmpeq_ss(x, y); }
SIMD_INLINE F128 vcmpeqps(const F128& x, const F128& y) noexcept { return _mm_cmpeq_ps(x, y); }

SIMD_INLINE F128 vcmpness(const F128& x, const F128& y) noexcept { return _mm_cmpneq_ss(x, y); }
SIMD_INLINE F128 vcmpneps(const F128& x, const F128& y) noexcept { return _mm_cmpneq_ps(x, y); }

SIMD_INLINE F128 vcmpgess(const F128& x, const F128& y) noexcept { return _mm_cmpge_ss(x, y); }
SIMD_INLINE F128 vcmpgeps(const F128& x, const F128& y) noexcept { return _mm_cmpge_ps(x, y); }

SIMD_INLINE F128 vcmpgtss(const F128& x, const F128& y) noexcept { return _mm_cmpgt_ss(x, y); }
SIMD_INLINE F128 vcmpgtps(const F128& x, const F128& y) noexcept { return _mm_cmpgt_ps(x, y); }

SIMD_INLINE F128 vcmpless(const F128& x, const F128& y) noexcept { return _mm_cmple_ss(x, y); }
SIMD_INLINE F128 vcmpleps(const F128& x, const F128& y) noexcept { return _mm_cmple_ps(x, y); }

SIMD_INLINE F128 vcmpltss(const F128& x, const F128& y) noexcept { return _mm_cmplt_ss(x, y); }
SIMD_INLINE F128 vcmpltps(const F128& x, const F128& y) noexcept { return _mm_cmplt_ps(x, y); }

SIMD_INLINE F128 vsqrtss(const F128& x) noexcept { return _mm_sqrt_ss(x); }
SIMD_INLINE F128 vsqrtps(const F128& x) noexcept { return _mm_sqrt_ps(x); }

SIMD_INLINE F128 vloadf128_32(const void* p) noexcept { return _mm_load_ss(static_cast<const float*>(p)); }
SIMD_INLINE F128 vloadf128_64(const void* p) noexcept { return vcast<F128>(vloadi128_64(p)); }

SIMD_INLINE F128 vloadf128a(const void* p) noexcept { return _mm_load_ps(static_cast<const float*>(p)); }
SIMD_INLINE F128 vloadf128u(const void* p) noexcept { return _mm_loadu_ps(static_cast<const float*>(p)); }

SIMD_INLINE F128 vloadf128_l64(const F128& x, const void* p) noexcept { return _mm_loadl_pi(x, static_cast<const __m64*>(p)); }
SIMD_INLINE F128 vloadf128_h64(const F128& x, const void* p) noexcept { return _mm_loadh_pi(x, static_cast<const __m64*>(p)); }

SIMD_INLINE void vstoref32(void* p, const F128& x) noexcept { _mm_store_ss(static_cast<float*>(p), x); }
SIMD_INLINE void vstoref64(void* p, const F128& x) noexcept { _mm_storel_pi(static_cast<__m64*>(p), x); }
SIMD_INLINE void vstorelf64(void* p, const F128& x) noexcept { _mm_storel_pi(static_cast<__m64*>(p), x); }
SIMD_INLINE void vstorehf64(void* p, const F128& x) noexcept { _mm_storeh_pi(static_cast<__m64*>(p), x); }
SIMD_INLINE void vstoref128a(void* p, const F128& x) noexcept { _mm_store_ps(static_cast<float*>(p), x); }
SIMD_INLINE void vstoref128u(void* p, const F128& x) noexcept { _mm_storeu_ps(static_cast<float*>(p), x); }

SIMD_INLINE F128 vbroadcastf128_64(const void* p) noexcept {
  #if SIMD_ARCH_SSE3
  return vcast<F128>(_mm_loaddup_pd(static_cast<const double*>(p)));
  #else
  return vduplf64(vloadf128_64(p));
  #endif
}

SIMD_INLINE bool vhasmaskf32(const F128& x, int bits0_3) noexcept { return _mm_movemask_ps(vcast<F128>(x)) == bits0_3; }
SIMD_INLINE bool vhasmaskf64(const F128& x, int bits0_1) noexcept { return _mm_movemask_pd(vcast<D128>(x)) == bits0_1; }

// ============================================================================
// [SIMD - D128]
// ============================================================================

SIMD_INLINE D128 vzerod128() noexcept { return _mm_setzero_pd(); }

SIMD_INLINE D128 vsetd128(double x) noexcept { return _mm_set1_pd(x); }
SIMD_INLINE D128 vsetd128(double x1, double x0) noexcept { return _mm_set_pd(x1, x0); }

//! Cast a scalar `double` to `D128` vector type.
SIMD_INLINE D128 vcvtd64d128(double x) noexcept {
  #if defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER)
  // See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=70708
  D128 reg;
  __asm__("" : "=x" (reg) : "0" (x));
  return reg;
  #elif defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  // MSC generates so much code when it sees `_mm_set_sd()`, this is a better alternative.
  return _mm_setr_pd(x, 0);
  #else
  return _mm_set_sd(x);
  #endif
}
SIMD_INLINE double vcvtd128d64(const D128& x) noexcept { return _mm_cvtsd_f64(x); }

SIMD_INLINE D128 vcvti32d128(int32_t x) noexcept { return _mm_cvtsi32_sd(vzerod128(), x); }
SIMD_INLINE int32_t vcvtd128i32(const D128& x) noexcept { return _mm_cvtsd_si32(x); }
SIMD_INLINE int32_t vcvttd128i32(const D128& x) noexcept { return _mm_cvttsd_si32(x); }

#if SIMD_ARCH_BITS == 64
SIMD_INLINE D128 vcvti64d128(int64_t x) noexcept { return _mm_cvtsi64_sd(vzerod128(), x); }
SIMD_INLINE int64_t vcvtd128i64(const D128& x) noexcept { return _mm_cvtsd_si64(x); }
SIMD_INLINE int64_t vcvttd128i64(const D128& x) noexcept { return _mm_cvttsd_si64(x); }
#endif

SIMD_INLINE D128 vcvtf128d128(const F128& x) noexcept { return _mm_cvtps_pd(x); }
SIMD_INLINE F128 vcvtd128f128(const D128& x) noexcept { return _mm_cvtpd_ps(x); }

SIMD_INLINE F128 vcvti128f128(const I128& x) noexcept { return _mm_cvtepi32_ps(x); }
SIMD_INLINE D128 vcvti128d128(const I128& x) noexcept { return _mm_cvtepi32_pd(x); }

SIMD_INLINE I128 vcvtf128i128(const F128& x) noexcept { return _mm_cvtps_epi32(x); }
SIMD_INLINE I128 vcvttf128i128(const F128& x) noexcept { return _mm_cvttps_epi32(x); }

SIMD_INLINE I128 vcvtd128i128(const D128& x) noexcept { return _mm_cvtpd_epi32(x); }
SIMD_INLINE I128 vcvttd128i128(const D128& x) noexcept { return _mm_cvttpd_epi32(x); }

template<int A, int B>
SIMD_INLINE D128 vshufd64(const D128& x, const D128& y) noexcept { return _mm_shuffle_pd(x, y, (A << 1) | B); }

template<int A, int B>
SIMD_INLINE D128 vswizd64(const D128& x) noexcept {
  #if !SIMD_ARCH_AVX
  return vcast<D128>(vswizi64<A, B>(vcast<I128>(x)));
  #else
  return vshufd64<A, B>(x, x);
  #endif
}

SIMD_INLINE D128 vswapd64(const D128& x) noexcept { return vswizd64<0, 1>(x); }
SIMD_INLINE D128 vdupld64(const D128& x) noexcept { return vswizd64<0, 0>(x); }
SIMD_INLINE D128 vduphd64(const D128& x) noexcept { return vswizd64<1, 1>(x); }

SIMD_INLINE D128 vunpackld64(const D128& x, const D128& y) noexcept { return _mm_unpacklo_pd(x, y); }
SIMD_INLINE D128 vunpackhd64(const D128& x, const D128& y) noexcept { return _mm_unpackhi_pd(x, y); }

SIMD_INLINE D128 vor(const D128& x, const D128& y) noexcept { return _mm_or_pd(x, y); }
SIMD_INLINE D128 vxor(const D128& x, const D128& y) noexcept { return _mm_xor_pd(x, y); }
SIMD_INLINE D128 vand(const D128& x, const D128& y) noexcept { return _mm_and_pd(x, y); }
SIMD_INLINE D128 vnand(const D128& x, const D128& y) noexcept { return _mm_andnot_pd(x, y); }
SIMD_INLINE D128 vblendmask(const D128& x, const D128& y, const D128& mask) noexcept { return vor(vnand(mask, x), vand(y, mask)); }

SIMD_INLINE D128 vaddsd(const D128& x, const D128& y) noexcept { return _mm_add_sd(x, y); }
SIMD_INLINE D128 vaddpd(const D128& x, const D128& y) noexcept { return _mm_add_pd(x, y); }

SIMD_INLINE D128 vsubsd(const D128& x, const D128& y) noexcept { return _mm_sub_sd(x, y); }
SIMD_INLINE D128 vsubpd(const D128& x, const D128& y) noexcept { return _mm_sub_pd(x, y); }

SIMD_INLINE D128 vmulsd(const D128& x, const D128& y) noexcept { return _mm_mul_sd(x, y); }
SIMD_INLINE D128 vmulpd(const D128& x, const D128& y) noexcept { return _mm_mul_pd(x, y); }

SIMD_INLINE D128 vdivsd(const D128& x, const D128& y) noexcept { return _mm_div_sd(x, y); }
SIMD_INLINE D128 vdivpd(const D128& x, const D128& y) noexcept { return _mm_div_pd(x, y); }

SIMD_INLINE D128 vminsd(const D128& x, const D128& y) noexcept { return _mm_min_sd(x, y); }
SIMD_INLINE D128 vminpd(const D128& x, const D128& y) noexcept { return _mm_min_pd(x, y); }

SIMD_INLINE D128 vmaxsd(const D128& x, const D128& y) noexcept { return _mm_max_sd(x, y); }
SIMD_INLINE D128 vmaxpd(const D128& x, const D128& y) noexcept { return _mm_max_pd(x, y); }

SIMD_INLINE D128 vcmpeqsd(const D128& x, const D128& y) noexcept { return _mm_cmpeq_sd(x, y); }
SIMD_INLINE D128 vcmpeqpd(const D128& x, const D128& y) noexcept { return _mm_cmpeq_pd(x, y); }

SIMD_INLINE D128 vcmpnesd(const D128& x, const D128& y) noexcept { return _mm_cmpneq_sd(x, y); }
SIMD_INLINE D128 vcmpnepd(const D128& x, const D128& y) noexcept { return _mm_cmpneq_pd(x, y); }

SIMD_INLINE D128 vcmpgesd(const D128& x, const D128& y) noexcept { return _mm_cmpge_sd(x, y); }
SIMD_INLINE D128 vcmpgepd(const D128& x, const D128& y) noexcept { return _mm_cmpge_pd(x, y); }

SIMD_INLINE D128 vcmpgtsd(const D128& x, const D128& y) noexcept { return _mm_cmpgt_sd(x, y); }
SIMD_INLINE D128 vcmpgtpd(const D128& x, const D128& y) noexcept { return _mm_cmpgt_pd(x, y); }

SIMD_INLINE D128 vcmplesd(const D128& x, const D128& y) noexcept { return _mm_cmple_sd(x, y); }
SIMD_INLINE D128 vcmplepd(const D128& x, const D128& y) noexcept { return _mm_cmple_pd(x, y); }

SIMD_INLINE D128 vcmpltsd(const D128& x, const D128& y) noexcept { return _mm_cmplt_sd(x, y); }
SIMD_INLINE D128 vcmpltpd(const D128& x, const D128& y) noexcept { return _mm_cmplt_pd(x, y); }

SIMD_INLINE D128 vsqrtsd(const D128& x) noexcept { return _mm_sqrt_sd(x, x); }
SIMD_INLINE D128 vsqrtpd(const D128& x) noexcept { return _mm_sqrt_pd(x); }

SIMD_INLINE D128 vloadd128_64(const void* p) noexcept { return _mm_load_sd(static_cast<const double*>(p)); }
SIMD_INLINE D128 vloadd128a(const void* p) noexcept { return _mm_load_pd(static_cast<const double*>(p)); }
SIMD_INLINE D128 vloadd128u(const void* p) noexcept { return _mm_loadu_pd(static_cast<const double*>(p)); }

SIMD_INLINE D128 vloadd128_l64(const D128& x, const void* p) noexcept { return _mm_loadl_pd(x, static_cast<const double*>(p)); }
SIMD_INLINE D128 vloadd128_h64(const D128& x, const void* p) noexcept { return _mm_loadh_pd(x, static_cast<const double*>(p)); }

SIMD_INLINE D128 vbroadcastd128_64(const void* p) noexcept {
  #if SIMD_ARCH_SSE3
  return _mm_loaddup_pd(static_cast<const double*>(p));
  #else
  return vdupld64(vloadd128_64(p));
  #endif
}

SIMD_INLINE void vstored64(void* p, const D128& x) noexcept { _mm_store_sd(static_cast<double*>(p), x); }
SIMD_INLINE void vstoreld64(void* p, const D128& x) noexcept { _mm_storel_pd(static_cast<double*>(p), x); }
SIMD_INLINE void vstorehd64(void* p, const D128& x) noexcept { _mm_storeh_pd(static_cast<double*>(p), x); }
SIMD_INLINE void vstored128a(void* p, const D128& x) noexcept { _mm_store_pd(static_cast<double*>(p), x); }
SIMD_INLINE void vstored128u(void* p, const D128& x) noexcept { _mm_storeu_pd(static_cast<double*>(p), x); }

SIMD_INLINE bool vhasmaskd64(const D128& x, int bits0_1) noexcept { return _mm_movemask_pd(vcast<D128>(x)) == bits0_1; }

} // anonymouse namespace
} // SIMD namespace

#endif // _SIMD_H
