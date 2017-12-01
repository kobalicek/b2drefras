#ifndef _RASTERIZER_H
#define _RASTERIZER_H

#include "./compositor.h"
#include "./globals.h"

// ============================================================================
// [Rasterizer]
// ============================================================================

class Rasterizer {
public:
  // Rasterizer ID.
  enum Id : uint32_t {
    kIdAGG = 0,
    kIdA1,
    kIdA2,
    kIdA3x4,
    kIdA3x8,
    kIdA3x16,
    kIdA3x32,
    kIdCount
  };

  enum Options : uint32_t {
    kOptionSIMD = 0x01
  };

  enum FillMode : uint32_t {
    kFillEvenOdd = 0,
    kFillNonZero = 1
  };

  static Rasterizer* newById(Image& dst, uint32_t id, uint32_t options);

  Rasterizer(Image& dst, uint32_t options) noexcept;
  virtual ~Rasterizer() noexcept;

  inline bool isInitialized() const noexcept { return _width != 0; }

  const char* name() const noexcept { return _name; }
  void addOptionsToName() noexcept;

  inline int width() const noexcept { return _width; }
  inline int height() const noexcept { return _height; }

  inline uint32_t options() const noexcept { return _options; }
  inline bool hasOption(uint32_t option) const noexcept { return (_options & option) != 0; }

  inline uint32_t fillMode() const noexcept { return _fillMode; }
  inline void setFillMode(uint32_t fillMode) noexcept { _fillMode = fillMode; }

  virtual void reset() noexcept = 0;
  virtual void clear() noexcept = 0;
  virtual bool addPoly(const Point* poly, size_t count) noexcept = 0;
  virtual void render(uint32_t argb32) noexcept = 0;

  template<class SELF>
  static void doRender(SELF& self, uint32_t argb32) noexcept {
    if (self.fillMode() == kFillNonZero) {
      if (self.hasOption(kOptionSIMD))
        self.template _renderImpl<CompositorSIMD, true>(argb32);
      else
        self.template _renderImpl<CompositorScalar, true>(argb32);
    }
    else {
      if (self.hasOption(kOptionSIMD))
        self.template _renderImpl<CompositorSIMD, false>(argb32);
      else
        self.template _renderImpl<CompositorScalar, false>(argb32);
    }
  }

  Image* _dst;
  char _name[32];
  int _width;
  int _height;
  uint32_t _options;
  uint32_t _fillMode;
};

// ============================================================================
// [CellRasterizer]
// ============================================================================

class CellRasterizer : public Rasterizer {
public:
  CellRasterizer(Image& dst, uint32_t options) noexcept;
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
};

#endif // _RASTERIZER_H
