#ifndef _RASTERIZER_A1_H
#define _RASTERIZER_A1_H

#include "./base.h"

class RasterizerA1 : public Rasterizer {
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

  RasterizerA1() noexcept;
  virtual ~RasterizerA1() noexcept;

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

    Cell& cell = _cells[y * _stride + x];
    cell.cover += cover;
    cell.area  += area;
  }

  template<bool NonZero>
  inline void _renderImpl(Image& dst, uint32_t argb32) noexcept;

  virtual bool render(Image& dst, uint32_t argb32) noexcept override;

  intptr_t _stride;
  Cell* _cells;
};

#endif // _RASTERIZER_A1_H
