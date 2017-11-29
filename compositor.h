#ifndef _COMPOSITOR_H
#define _COMPOSITOR_H

#include "./globals.h"
#include "./simd.h"

// ============================================================================
// [CompositeUtils]
// ============================================================================

namespace CompositeUtils {
  template<bool NonZero>
  static ALWAYS_INLINE uint32_t calcMask(int m) noexcept {
    if (NonZero) {
      if (m < 0) m = -m;
      if (m > 255) m = 255;
    }
    else {
      m = m & 0x1FF;
      if (m > 255) m = 511 - m;
    }
    return uint32_t(m);
  }
}

// ============================================================================
// [CompositorScalar]
// ============================================================================

class CompositorScalar {
public:
  ALWAYS_INLINE CompositorScalar(uint32_t argb32) noexcept {
    _p32 = PixelUtils::premultiply(argb32);
  }

  ALWAYS_INLINE void overwrite(uint32_t* dst) noexcept {
    *dst = _p32;
  }

  ALWAYS_INLINE void composite(uint32_t* dst, uint32_t mask) noexcept {
    dst[0] = PixelUtils::src(dst[0], _p32, mask);
  }

  ALWAYS_INLINE uint32_t cmask(uint32_t* dst, size_t x0, size_t x1, uint32_t mask) noexcept {
    if (mask == 255) {
      while (x0 < x1) {
        overwrite(&dst[x0]);
        x0++;
      }
    }
    else {
      while (x0 < x1) {
        composite(&dst[x0], mask);
        x0++;
      }
    }
    return x0;
  }

  template<bool NonZero>
  ALWAYS_INLINE uint32_t vmask(uint32_t* dst, size_t x0, size_t x1, Cell* cell, int& cover) {
    while (x0 < x1) {
      cover += cell[x0].cover;
      uint32_t mask = CompositeUtils::calcMask<NonZero>(cover - (cell[x0].area >> 9));
      cell[x0].reset();

      if (mask == 255)
        overwrite(&dst[x0]);
      else
        composite(&dst[x0], mask);
      x0++;
    }
    return x0;
  }

  uint32_t _p32;
};

// ============================================================================
// [CompositorSIMD]
// ============================================================================

class CompositorSIMD {
public:
  ALWAYS_INLINE CompositorSIMD(uint32_t argb32) noexcept {
    _p32 = SIMD::vswizi32<0, 0, 0, 0>(SIMD::vcvti32i128(PixelUtils::premultiply(argb32)));
    _u32 = SIMD::vmovli64u8u16(_p32);
  }

  ALWAYS_INLINE void overwrite(uint32_t* dst) noexcept {
    SIMD::vstorei32(dst, _p32);
  }

  ALWAYS_INLINE void composite(uint32_t* dst, uint32_t mask) noexcept {
    SIMD::I128 x0 = SIMD::vswizli16<0, 0, 0, 0>(SIMD::vcvti32i128(mask));
    SIMD::I128 s0 = SIMD::vmovli64u8u16(SIMD::vloadi128_32(dst));
    SIMD::I128 t0;

    t0 = SIMD::vmulu16(x0, _u32);
    x0 = SIMD::vxor(x0, SIMD::u16_00FF_128.i128);
    s0 = SIMD::vmulu16(s0, x0);
    s0 = SIMD::vaddi16(s0, t0);
    s0 = SIMD::vdiv255u16(s0);
    s0 = SIMD::vpacki16u8(s0);

    SIMD::vstorei32(dst, s0);
  }

  ALWAYS_INLINE uint32_t cmask(uint32_t* dst, size_t x0, size_t x1, uint32_t mask) noexcept {
    size_t i = (x1 - x0) / 4;
    if (mask == 255) {
      while (i >= 8) {
        SIMD::vstorei128u(dst + x0 +  0, _p32);
        SIMD::vstorei128u(dst + x0 +  4, _p32);
        SIMD::vstorei128u(dst + x0 +  8, _p32);
        SIMD::vstorei128u(dst + x0 + 12, _p32);
        SIMD::vstorei128u(dst + x0 + 16, _p32);
        SIMD::vstorei128u(dst + x0 + 20, _p32);
        SIMD::vstorei128u(dst + x0 + 24, _p32);
        SIMD::vstorei128u(dst + x0 + 28, _p32);
        x0 += 32;
        i -= 8;
      }

      while (i) {
        SIMD::vstorei128u(dst + x0, _p32);
        x0 += 4;
        i--;
      }

      while (x0 < x1) {
        SIMD::vstorei32(dst + x0, _p32);
        x0++;
      }
    }
    else {
      SIMD::I128 mVal = SIMD::vswizi32<0, 0, 0, 0>(SIMD::vswizli16<0, 0, 0, 0>(SIMD::vcvti32i128(mask)));
      SIMD::I128 mPix = SIMD::vmulu16(_u32, mVal);
      SIMD::I128 mInv = SIMD::vxor(mVal, SIMD::u16_00FF_128.i128);

      while (i >= 2) {
        SIMD::I128 s0, s1;
        SIMD::I128 s2, s3;

        s0 = SIMD::vloadi128u(dst + x0 + 0);
        s2 = SIMD::vloadi128u(dst + x0 + 4);

        s1 = SIMD::vmovhi64u8u16(s0);
        s3 = SIMD::vmovhi64u8u16(s2);

        s0 = SIMD::vmovli64u8u16(s0);
        s2 = SIMD::vmovli64u8u16(s2);

        s1 = SIMD::vmulu16(s1, mInv);
        s3 = SIMD::vmulu16(s3, mInv);

        s0 = SIMD::vmulu16(s0, mInv);
        s2 = SIMD::vmulu16(s2, mInv);

        s1 = SIMD::vaddi16(s1, mPix);
        s3 = SIMD::vaddi16(s3, mPix);

        s0 = SIMD::vaddi16(s0, mPix);
        s2 = SIMD::vaddi16(s2, mPix);

        s1 = SIMD::vdiv255u16(s1);
        s3 = SIMD::vdiv255u16(s3);

        s0 = SIMD::vdiv255u16(s0);
        s2 = SIMD::vdiv255u16(s2);

        s0 = SIMD::vpacki16u8(s0, s1);
        s2 = SIMD::vpacki16u8(s2, s3);

        SIMD::vstorei128u(dst + x0 + 0, s0);
        SIMD::vstorei128u(dst + x0 + 4, s2);
        x0 += 8;
        i -= 2;
      }

      if (i) {
        SIMD::I128 s0, s1;

        s0 = SIMD::vloadi128u(dst + x0 + 0);
        s1 = SIMD::vloadi128u(dst + x0 + 4);
        s1 = SIMD::vmovhi64u8u16(s0);
        s0 = SIMD::vmovli64u8u16(s0);
        s1 = SIMD::vmulu16(s1, mInv);
        s0 = SIMD::vmulu16(s0, mInv);
        s1 = SIMD::vaddi16(s1, mPix);
        s0 = SIMD::vaddi16(s0, mPix);
        s1 = SIMD::vdiv255u16(s1);
        s0 = SIMD::vdiv255u16(s0);
        s0 = SIMD::vpacki16u8(s0, s1);

        SIMD::vstorei128u(dst + x0, s0);
        x0 += 4;
      }

      while (x0 < x1) {
        SIMD::I128 s0;

        s0 = SIMD::vloadi128_32(dst + x0);
        s0 = SIMD::vmovli64u8u16(s0);
        s0 = SIMD::vmulu16(s0, mInv);
        s0 = SIMD::vaddi16(s0, mPix);
        s0 = SIMD::vdiv255u16(s0);
        s0 = SIMD::vpacki16u8(s0, s0);

        SIMD::vstorei32(dst + x0, s0);
        x0++;
      }
    }
    return x0;
  }

  template<bool NonZero>
  ALWAYS_INLINE uint32_t vmask(uint32_t* dst, size_t x0, size_t x1, Cell* cell, int& cover) noexcept {
    SIMD_DEF_I128_1xI32(u32_01FF_128, 0x000001FF);
    SIMD_DEF_I128_1xI32(u16_01FF_128, 0x01FF01FF);

    SIMD::I128 coverXmm = SIMD::vcvti32i128(cover);

    size_t i = (x1 - x0) / 4;
    if (i) {
      coverXmm = SIMD::vswizi32<0, 0, 0, 0>(coverXmm);

      /*
      while (i >= 2) {
        SIMD::I128 m0, m1, m2, m3;
        SIMD::I128 s0, s1, s2, s3;
        SIMD::I128 t0, t1, t2, t3;

        m0 = SIMD::vloadi128u(&cell[x0 + 0]);                  // [  a1 |  c1 |  a0 |  c0 ]
        t0 = SIMD::vloadi128u(&cell[x0 + 2]);                  // [  a3 |  c3 |  a2 |  c2 ]

        m2 = SIMD::vloadi128u(&cell[x0 + 4]);                  // [  a5 |  c5 |  a4 |  c4 ]
        t2 = SIMD::vloadi128u(&cell[x0 + 6]);                  // [  a7 |  c7 |  a6 |  c6 ]

        m0 = SIMD::vswizi32<3, 1, 2, 0>(m0);                   // [  a1 |  a0 |  c1 |  c0 ]
        m2 = SIMD::vswizi32<3, 1, 2, 0>(m2);                   // [  a5 |  a4 |  c5 |  c4 ]

        t0 = SIMD::vswizi32<3, 1, 2, 0>(t0);                   // [  a3 |  a2 |  c3 |  c2 ]
        t2 = SIMD::vswizi32<3, 1, 2, 0>(t2);                   // [  a7 |  a6 |  c7 |  c6 ]

        m1 = SIMD::vunpackhi64(m0, t0);                        // [  a3 |  a2 |  a1 |  a0 ]
        m3 = SIMD::vunpackhi64(m2, t2);                        // [  a7 |  a6 |  a5 |  a4 ]

        m0 = SIMD::vunpackli64(m0, t0);                        // [  c3 |  c2 |  c1 |  c0 ]
        m2 = SIMD::vunpackli64(m2, t2);                        // [  c7 |  c6 |  c5 |  c4 ]

        t0 = SIMD::vslli128b<4>(m0);                           // [  c2 |  c1 |  c0 |  0  ]
        t2 = SIMD::vslli128b<4>(m2);                           // [  c6 |  c5 |  c4 |  0  ]

        m0 = SIMD::vaddi32(m0, t0);                            // [c3:c2|c2:c1|c1:c0|  c0 ]
        t0 = SIMD::vzeroi128();                                // [  0  |  0  |  0  |  0  ]

        m2 = SIMD::vaddi32(m2, t2);                            // [c7:c6|c6:c5|c5:c4|  c4 ]
        t2 = SIMD::vzeroi128();                                // [  0  |  0  |  0  |  0  ]

        SIMD::vstorei128u(&cell[x0 + 0], t0);
        SIMD::vstorei128u(&cell[x0 + 2], t0);
        t0 = SIMD::vunpackli64(t0, m0);                        // [c1:c0|  c0 |  0  |  0  ]

        SIMD::vstorei128u(&cell[x0 + 4], t2);
        SIMD::vstorei128u(&cell[x0 + 6], t2);
        t2 = SIMD::vunpackli64(t2, m2);                        // [c5:c4|  c4 |  0  |  0  ]

        m0 = SIMD::vaddi32(m0, t0);                            // [c3:c0|c2:c0|c1:c0|  c0 ]
        m2 = SIMD::vaddi32(m2, t2);                            // [c7:c4|c6:c4|c5:c4|  c4 ]

        t2 = SIMD::vswizi32<3, 3, 3, 3>(m0);                   // [c3:c0|c3:c0|c3:c0|c3:c0]
        m0 = SIMD::vaddi32(m0, coverXmm);

        m2 = SIMD::vaddi32(m2, t2);                            // [c7:c0|c6:c0|c5:c0|c4:c0]
        m1 = SIMD::vsrai32<9>(m1);
        m3 = SIMD::vsrai32<9>(m3);

        coverXmm = SIMD::vaddi32(coverXmm, m2);
        m0 = SIMD::vsubi32(m0, m1);
        m1 = SIMD::vsubi32(coverXmm, m3);

        if (NonZero) {
          m0 = SIMD::vabsi32(m0);
          m1 = SIMD::vabsi32(m1);
          m0 = SIMD::vpacki32i16(m0, m1);
          m0 = SIMD::vmini16(m0, SIMD::u16_00FF_128.i128);
        }
        else {
          m0 = SIMD::vand(m0, u32_01FF_128.i128);
          m1 = SIMD::vand(m1, u32_01FF_128.i128);
          m0 = SIMD::vpacki32i16(m0, m1);
          m1 = SIMD::vsubi32(u16_01FF_128.i128, m0);
          m0 = SIMD::vmini16(m0, m1);
        }

        m2 = SIMD::vunpackhi16(m0, m0);
        m0 = SIMD::vunpackli16(m0, m0);
        coverXmm = SIMD::vswizi32<3, 3, 3, 3>(coverXmm);

        s0 = SIMD::vloadi128u(dst + x0 + 0);
        s2 = SIMD::vloadi128u(dst + x0 + 4);

        s1 = SIMD::vmovhi64u8u16(s0);
        s0 = SIMD::vmovli64u8u16(s0);

        s3 = SIMD::vmovhi64u8u16(s2);
        s2 = SIMD::vmovli64u8u16(s2);

        m1 = SIMD::vswizi32<3, 3, 2, 2>(m0);
        m0 = SIMD::vswizi32<1, 1, 0, 0>(m0);
        t0 = SIMD::vmulu16(_u32, m0);
        t1 = SIMD::vmulu16(_u32, m1);
        m0 = SIMD::vxor(m0, SIMD::u16_00FF_128.i128);
        m1 = SIMD::vxor(m1, SIMD::u16_00FF_128.i128);
        s0 = SIMD::vmulu16(s0, m0);
        s1 = SIMD::vmulu16(s1, m1);

        m3 = SIMD::vswizi32<3, 3, 2, 2>(m2);
        m2 = SIMD::vswizi32<1, 1, 0, 0>(m2);
        t2 = SIMD::vmulu16(_u32, m2);
        t3 = SIMD::vmulu16(_u32, m3);
        m2 = SIMD::vxor(m2, SIMD::u16_00FF_128.i128);
        m3 = SIMD::vxor(m3, SIMD::u16_00FF_128.i128);
        s2 = SIMD::vmulu16(s2, m2);
        s3 = SIMD::vmulu16(s3, m3);

        s0 = SIMD::vaddi16(s0, t0);
        s1 = SIMD::vaddi16(s1, t1);
        s0 = SIMD::vdiv255u16(s0);
        s1 = SIMD::vdiv255u16(s1);

        s2 = SIMD::vaddi16(s2, t2);
        s3 = SIMD::vaddi16(s3, t3);
        s2 = SIMD::vdiv255u16(s2);
        s3 = SIMD::vdiv255u16(s3);

        s0 = SIMD::vpacki16u8(s0, s1);
        s2 = SIMD::vpacki16u8(s2, s3);

        SIMD::vstorei128u(dst + x0 + 0, s0);
        SIMD::vstorei128u(dst + x0 + 4, s2);

        x0 += 8;
        i -= 2;
      }
      */

      while (i) {
        SIMD::I128 m0, m1;
        SIMD::I128 s0, s1;
        SIMD::I128 t0, t1;

        m0 = SIMD::vloadi128u(&cell[x0 + 0]);                  // [  a1 |  c1 |  a0 |  c0 ]
        t0 = SIMD::vloadi128u(&cell[x0 + 2]);                  // [  a3 |  c3 |  a2 |  c2 ]

        m0 = SIMD::vswizi32<3, 1, 2, 0>(m0);                   // [  a1 |  a0 |  c1 |  c0 ]
        t0 = SIMD::vswizi32<3, 1, 2, 0>(t0);                   // [  a3 |  a2 |  c3 |  c2 ]

        m1 = SIMD::vunpackhi64(m0, t0);                        // [  a3 |  a2 |  a1 |  a0 ]
        m0 = SIMD::vunpackli64(m0, t0);                        // [  c3 |  c2 |  c1 |  c0 ]

        t0 = SIMD::vslli128b<4>(m0);                           // [  c2 |  c1 |  c0 |  0  ]
        m0 = SIMD::vaddi32(m0, t0);                            // [c3:c2|c2:c1|c1:c0|  c0 ]

        t0 = SIMD::vzeroi128();                                // [  0  |  0  |  0  |  0  ]
        SIMD::vstorei128u(&cell[x0 + 0], t0);
        SIMD::vstorei128u(&cell[x0 + 2], t0);
        t0 = SIMD::vunpackli64(t0, m0);                        // [c1:c0|  c0 |  0  |  0  ]

        m1 = SIMD::vsrai32<9>(m1);
        m0 = SIMD::vaddi32(m0, t0);                            // [c3:c0|c2:c0|c1:c0|  c0 ]
        coverXmm = SIMD::vaddi32(coverXmm, m0);
        m1 = SIMD::vsubi32(coverXmm, m1);

        if (NonZero) {
          m0 = SIMD::vabsi32(m1);
          m0 = SIMD::vpacki32i16(m0, m0);
          m0 = SIMD::vmini16(m0, SIMD::u16_00FF_128.i128);
        }
        else {
          m1 = SIMD::vand(m1, u32_01FF_128.i128);
          m1 = SIMD::vpacki32i16(m1, m1);
          m0 = SIMD::vsubi32(u16_01FF_128.i128, m1);
          m0 = SIMD::vmini16(m0, m1);
        }

        m0 = SIMD::vunpackli16(m0, m0);
        coverXmm = SIMD::vswizi32<3, 3, 3, 3>(coverXmm);

        m1 = SIMD::vswizi32<3, 3, 2, 2>(m0);
        m0 = SIMD::vswizi32<1, 1, 0, 0>(m0);

        s0 = SIMD::vloadi128u(dst + x0);
        s1 = SIMD::vmovhi64u8u16(s0);
        s0 = SIMD::vmovli64u8u16(s0);

        t0 = SIMD::vmulu16(_u32, m0);
        t1 = SIMD::vmulu16(_u32, m1);
        m0 = SIMD::vxor(m0, SIMD::u16_00FF_128.i128);
        m1 = SIMD::vxor(m1, SIMD::u16_00FF_128.i128);
        s0 = SIMD::vmulu16(s0, m0);
        s1 = SIMD::vmulu16(s1, m1);
        s0 = SIMD::vaddi16(s0, t0);
        s1 = SIMD::vaddi16(s1, t1);
        s0 = SIMD::vdiv255u16(s0);
        s1 = SIMD::vdiv255u16(s1);
        s0 = SIMD::vpacki16u8(s0, s1);

        SIMD::vstorei128u(dst + x0, s0);
        x0 += 4;
        i--;
      }
    }

    while (x0 < x1) {
      SIMD::I128 m0;
      SIMD::I128 s0;
      SIMD::I128 t0;

      t0 = SIMD::vloadi128_32(&cell[x0].cover);
      m0 = SIMD::vloadi128_32(&cell[x0].area);

      coverXmm = SIMD::vaddi32(coverXmm, t0);
      cell[x0].reset();
      m0 = SIMD::vsrai32<9>(m0);
      m0 = SIMD::vsubi32(coverXmm, m0);

      if (NonZero) {
        m0 = SIMD::vabsi32(m0);
        m0 = SIMD::vpacki32i16(m0, m0);
        m0 = SIMD::vmini16(m0, SIMD::u16_00FF_128.i128);
      }
      else {
        m0 = SIMD::vand(m0, u32_01FF_128.i128);
        m0 = SIMD::vpacki32i16(m0, m0);
        t0 = SIMD::vsubi32(u32_01FF_128.i128, m0);
        m0 = SIMD::vmini16(m0, t0);
      }

      s0 = SIMD::vloadi128_32(dst + x0);
      m0 = SIMD::vswizli16<0, 0, 0, 0>(m0);

      s0 = SIMD::vmovli64u8u16(s0);
      t0 = SIMD::vmulu16(m0, _u32);
      m0 = SIMD::vxor(m0, SIMD::u16_00FF_128.i128);
      s0 = SIMD::vmulu16(s0, m0);
      s0 = SIMD::vaddi16(s0, t0);
      s0 = SIMD::vdiv255u16(s0);
      s0 = SIMD::vpacki16u8(s0, s0);

      SIMD::vstorei32(dst + x0, s0);
      x0++;
    }

    cover = SIMD::vcvti128i32(coverXmm);
    return x0;
  }

  __m128i _p32;
  __m128i _u32;
};

// ============================================================================
// [CompositorSIMD]
// ============================================================================

typedef CompositorSIMD Compositor;

#endif // _COMPOSITOR_H
