#ifndef _RASTERIZER_A2_H
#define _RASTERIZER_A2_H

#include "./base.h"

class RasterizerA2 : public Rasterizer {
public:
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
    int32_t cover;
    int32_t area;
  };

  struct Bounds {
    inline void reset() noexcept {
      start = std::numeric_limits<int>::max();
      end = 0;
    }

    inline bool empty() const noexcept {
      return end == 0;
    }

    inline void mergeStart(int x) noexcept { start = std::min(start, x); }
    inline void mergeEnd(int x) noexcept { end = std::max(end, x); }

    inline void union_(int a, int b) noexcept {
      mergeStart(a);
      mergeEnd(b);
    }

    int start;
    int end;
  };

  RasterizerA2() noexcept;
  virtual ~RasterizerA2() noexcept;

  virtual const char* name() const noexcept override;
  virtual bool init(int w, int h) noexcept override;
  virtual void reset() noexcept override;
  virtual void clear() noexcept override;
  virtual bool addPoly(const Point* poly, size_t count) noexcept override;

  template<typename Fixed>
  void _addLine(Fixed x0, Fixed y0, Fixed x1, Fixed y1) noexcept;

  inline void _mergeCell(int x, int y, int cover, int area) noexcept{
    assert(x >= 0 && x < _width);
    assert(y >= 0 && y < _height);

    Cell& cell = _cells[y * _cellStride + x];
    cell.cover += cover;
    cell.area  += area;
  }

  template<bool NonZero>
  inline void _renderImpl(Image& dst, uint32_t argb32) noexcept;

  virtual bool render(Image& dst, uint32_t argb32) noexcept override;

  intptr_t _cellStride;
  Bounds _yBounds;
  Bounds* _xBounds;
  Cell* _cells;
};

#endif // _RASTERIZER_A2_H
