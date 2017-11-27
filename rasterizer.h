#ifndef _RASTERIZER_H
#define _RASTERIZER_H

#include "./globals.h"

// ============================================================================
// [Rasterizer]
// ============================================================================

class Rasterizer {
public:
  // Rasterizer ID.
  enum Id : uint32_t {
    kIdA1 = 0,
    kIdA2,
    kIdA3,
    kIdCount
  };
  static Rasterizer* newById(uint32_t id);

  Rasterizer() noexcept;
  virtual ~Rasterizer() noexcept;

  virtual const char* name() const noexcept = 0;

  virtual bool init(int w, int h) noexcept = 0;
  virtual void reset() noexcept = 0;

  virtual void clear() noexcept = 0;
  virtual bool addPoly(const Point* poly, size_t count) noexcept = 0;
  virtual bool render(Image& dst, uint32_t argb32) noexcept = 0;

  inline bool isInitialized() const noexcept { return _width != 0; }
  inline int width() const noexcept { return _width; }
  inline int height() const noexcept { return _height; }

  int _width;
  int _height;
  bool _nonZero;
};

// ============================================================================
// [CellRasterizer]
// ============================================================================

class CellRasterizer : public Rasterizer {
public:
  CellRasterizer() noexcept;
  virtual ~CellRasterizer() noexcept;

  enum Alpha {
    kA8Shift    = 8,              // 8-bit alpha.
    kA8Shift_2  = kA8Shift + 1,

    kA8Scale    = 1 << kA8Shift,  // 256.
    kA8Mask     = kA8Scale - 1,   // 255.

    kA8Scale_2  = kA8Scale * 2,   // 512.
    kA8Mask_2   = kA8Scale_2 - 1, // 511.

    kA8MaxI32   = static_cast<int>((1U << 31) / static_cast<unsigned int>(kA8Scale_2))
  };

  struct Cell {
    inline void reset() noexcept {
      cover = 0;
      area = 0;
    };

    int32_t cover;
    int32_t area;
  };

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
};

#endif // _RASTERIZER_H
