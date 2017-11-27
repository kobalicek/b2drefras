#ifndef _RASTERIZER_A3_H
#define _RASTERIZER_A3_H

#include "./rasterizer.h"
#include "./intutils.h"

class RasterizerA3 : public CellRasterizer {
public:
  typedef IntUtils::BitWord BitWord;
  static constexpr uint32_t kBitWordBits = IntUtils::kBitWordBits;

  static constexpr uint32_t kPixelsPerOneBit = 16;
  static constexpr uint32_t kPixelsPerBitWord = kPixelsPerOneBit * kBitWordBits;

  RasterizerA3() noexcept;
  virtual ~RasterizerA3() noexcept;

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

  Bounds _yBounds;

  size_t _bitStride;
  BitWord* _bits;

  size_t _cellStride;
  Cell* _cells;
};

#endif // _RASTERIZER_A3_H
