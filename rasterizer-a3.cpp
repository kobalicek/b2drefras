#include "./compositor.h"
#include "./intutils.h"
#include "./rasterizer.h"
#include "./simd.h"

// ============================================================================
// [RasterizerA3]
// ============================================================================

template<uint32_t N>
class RasterizerA3 : public CellRasterizer {
public:
  typedef IntUtils::BitWord BitWord;
  static constexpr uint32_t kBitWordBits = IntUtils::kBitWordBits;
  static constexpr uint32_t kPixelsPerOneBit = N;
  static constexpr uint32_t kPixelsPerBitWord = kPixelsPerOneBit * kBitWordBits;

  RasterizerA3(Image& dst, uint32_t options) noexcept;
  virtual ~RasterizerA3() noexcept;

  bool init(int w, int h) noexcept;

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

  template<class Compositor, bool NonZero>
  inline void _renderImpl(uint32_t argb32) noexcept;

  virtual void render(uint32_t argb32) noexcept override;

  Bounds _yBounds;

  size_t _bitStride;
  BitWord* _bits;

  size_t _cellStride;
  Cell* _cells;
};

// ============================================================================
// [RasterizerA3 - Construction / Destruction]
// ============================================================================

template<uint32_t N>
RasterizerA3<N>::RasterizerA3(Image& dst, uint32_t options) noexcept
  : CellRasterizer(dst, options),
    _yBounds { 0, 0 },
    _bitStride(0),
    _bits(nullptr),
    _cellStride(0),
    _cells(nullptr) {
  std::snprintf(_name, ARRAY_SIZE(_name), "A3x%u", kPixelsPerOneBit);
  addOptionsToName();
  init(dst.width(), dst.height());
}

template<uint32_t N>
RasterizerA3<N>::~RasterizerA3() noexcept {
  reset();
}

// ============================================================================
// [RasterizerA3 - Basics]
// ============================================================================

template<uint32_t N>
bool RasterizerA3<N>::init(int w, int h) noexcept {
  if (_width != w || _height != h) {
    if (_bits) std::free(_bits);
    if (_cells) std::free(_cells);

    _width = w;
    _height = h;

    if (w == 0 || h == 0) {
      _yBounds.reset();
      _bitStride = 0;
      _bits = nullptr;
      _cellStride = 0;
      _cells = nullptr;
      return true;
    }

    _bitStride = IntUtils::nBitWordsForNBits((size_t(w) + 1 + kPixelsPerOneBit - 1) / kPixelsPerOneBit);
    _bits = static_cast<BitWord*>(std::malloc(h * _bitStride * sizeof(BitWord)));

    _cellStride = w + 1;
    _cells = static_cast<Cell*>(std::malloc(h * _cellStride * sizeof(Cell)));

    if (!_bits || !_cells) {
      if (_bits) std::free(_bits);
      if (_cells) std::free(_cells);

      _width = 0;
      _height = 0;
      _yBounds.reset();

      _bitStride = 0;
      _bits = nullptr;
      _cellStride = 0;
      _cells = nullptr;

      return false;
    }

    _yBounds.reset();

    std::memset(_cells, 0, _height * _cellStride * sizeof(Cell));
    std::memset(_bits, 0, _height * _bitStride * sizeof(BitWord));
  }
  else {
    // This is much faster, will only clear the affected area.
    clear();
  }

  return true;
}

template<uint32_t N>
void RasterizerA3<N>::reset() noexcept {
  if (isInitialized()) {
    std::free(_bits);
    std::free(_cells);

    _width = 0;
    _height = 0;
    _yBounds.reset();

    _bitStride = 0;
    _bits = nullptr;

    _cellStride = 0;
    _cells = nullptr;
  }
}

template<uint32_t N>
void RasterizerA3<N>::clear() noexcept {
  if (isInitialized()) {
    size_t y0 = size_t(_yBounds.start);
    size_t y1 = size_t(_yBounds.end);

    BitWord* bitPtr = _bits + y0 * _bitStride;
    Cell* cellPtr = _cells + y0 * _cellStride;

    while (y0 <= y1) {
      size_t i = size_t(_bitStride);
      size_t x = 0;

      do {
        IntUtils::BitWordFlipIterator<BitWord> it(*bitPtr);
        *bitPtr = 0;

        if (it.hasNext()) {
          size_t xEnd = std::min<size_t>(_width, x + kPixelsPerBitWord);
          do {
            size_t x0 = x + it.nextAndFlip() * kPixelsPerOneBit;
            size_t x1;

            if (it.hasNext())
              x1 = std::min<size_t>(xEnd, x + it.nextAndFlip() * kPixelsPerOneBit);
            else
              x1 = std::min<size_t>(xEnd, x0 + kPixelsPerOneBit);

            std::memset(cellPtr + x0, 0, (x1 - x0) * sizeof(Cell));
          } while (it.hasNext());
        }

        x += kPixelsPerBitWord;
        bitPtr++;
      } while (--i);

      cellPtr += _cellStride;
      y0++;
    }

    _yBounds.reset();
  }
}

// ============================================================================
// [RasterizerA3 - AddPoly / AddLine]
// ============================================================================

template<uint32_t N>
bool RasterizerA3<N>::addPoly(const Point* poly, size_t count) noexcept {
  assert(isInitialized());

  if (count < 2)
    return true;

  int x0 = static_cast<int>(poly[0].x * 256);
  int y0 = static_cast<int>(poly[0].y * 256);

  for (size_t i = 1; i < count; i++) {
    int x1 = static_cast<int>(poly[i].x * 256);
    int y1 = static_cast<int>(poly[i].y * 256);

    if (x0 != x1 || y0 != y1)
      _addLine<int64_t>(x0, y0, x1, y1);

    x0 = x1;
    y0 = y1;
  }

  return true;
}

template<uint32_t N>
template<typename Fixed>
void RasterizerA3<N>::_addLine(Fixed x0, Fixed y0, Fixed x1, Fixed y1) noexcept {
  Fixed dx = x1 - x0;
  Fixed dy = y1 - y0;

  if (dy == Fixed(0))
    return;

  int cover = int(dy);
  int area;

  if (dx < 0) dx = -dx;
  if (dy < 0) dy = -dy;

  int yInc = 1;
  int coverSign = 1;

  // Fix RIGHT-TO-LEFT direction:
  //   - swap coordinates,
  //   - invert cover-sign.
  if (x0 > x1) {
    std::swap(x0, x1);
    std::swap(y0, y1);
    coverSign = -coverSign;
  }

  // Fix BOTTOM-TO-TOP direction:
  //   - invert fractional parts of y0 and y1,
  //   - invert cover-sign.
  if (y0 > y1) {
    y0 ^= kA8Mask;
    y0 += int(y0 & kA8Mask) == kA8Mask ? 1 - kA8Scale * 2 : 1;
    y1  = y0 + dy;

    yInc = -1;
    coverSign = -coverSign;
  }

  // Extract the raster and fractional coordinates.
  int ex0 = int(x0 >> kA8Shift);
  int fx0 = int(x0 & kA8Mask);

  int ey0 = int(y0 >> kA8Shift);
  int fy0 = int(y0 & kA8Mask);

  int ex1 = int(x1 >> kA8Shift);
  int fy1 = int(y1 & kA8Mask);

  // NOTE: Variable `i` is just a loop counter. We need to make sure to handle
  // the start and end points of the line, which use the same loop body, but
  // require special handling.
  //
  //   - `i` - How many Y iterations to do now.
  //   - `j` - How many Y iterations to do next.
  int i = 1;
  int j = int(y1 >> kA8Shift) - ey0;

  // Single-Cell.
  if ((j | ((fx0 + int(dx)) > 256)) == 0) {
    _yBounds.union_(ey0, ey0);
    IntUtils::bitVectorSetBit(&_bits[ey0 * _bitStride], ex0 / kPixelsPerOneBit, true);

    _mergeCell(ex0, ey0, cover, (fx0 * 2 + int(dx)) * cover);
    return;
  }

  uint32_t ey1 = ey0 + (j + (fy1 != 0)) * yInc;
  if (ey0 <= ey1)
    _yBounds.union_(ey0, ey1);
  else
    _yBounds.union_(ey1, ey0);

  // Strictly horizontal line (always one cell per scanline).
  if (dx == 0) {
    if (j > 0)
      cover = (kA8Scale - fy0) * coverSign;

    fy0  = coverSign << kA8Shift;
    fy1 *= coverSign;
    fx0 *= 2;

    size_t bitIndex = ex0 / kPixelsPerOneBit;
    BitWord* bitBase = _bits + (bitIndex / kBitWordBits);
    BitWord bitMask = BitWord(1) << (bitIndex % kBitWordBits);

    for (;;) {
      area = fx0 * cover;
      do {
        bitBase[ey0 * _bitStride] |= bitMask;
        _mergeCell(ex0, ey0, cover, area);
        ey0 += yInc;
      } while (--i);

      if (ey0 == ey1)
        break;

      cover = fy1;
      i = j;
      j = 1;

      if (i <= 1)
        continue;

      cover = fy0;
      i--;
    }

    return;
  }

  Fixed xErr = -dy / 2, xBase, xLift, xRem, xDlt = dx;
  Fixed yErr = -dx / 2, yBase, yLift, yRem, yDlt = dy;

  xBase = dx * kA8Scale;
  xLift = xBase / dy;
  xRem  = xBase % dy;

  yBase = dy * kA8Scale;
  yLift = yBase / dx;
  yRem  = yBase % dx;

  if (j != 0) {
    Fixed p = Fixed(kA8Scale - fy0) * dx;
    xDlt  = p / dy;
    xErr += p % dy;
    fy1 = kA8Scale;
  }

  if (ex0 != ex1) {
    Fixed p = Fixed(kA8Scale - fx0) * dy;
    yDlt = p / dx;
    yErr += p % dx;
  }

  // Vertical direction -> One/Two cells per scanline.
  if (dy >= dx) {
    int yAcc = int(y0) + int(yDlt);

    goto VertSkip;
    for (;;) {
      do {
        xDlt = xLift;
        xErr += xRem;
        if (xErr >= 0) { xErr -= dy; xDlt++; }

VertSkip:
        area = fx0;
        fx0 += int(xDlt);

        if (fx0 <= 256) {
          cover = (fy1 - fy0) * coverSign;
          area  = (area + fx0) * cover;

          IntUtils::bitVectorSetBit(&_bits[ey0 * _bitStride], ex0 / kPixelsPerOneBit, true);
          _mergeCell(ex0, ey0, cover, area);

          if (fx0 == 256) {
            ex0++;
            fx0 = 0;
            goto VertAdvance;
          }
        }
        else {
          yAcc &= 0xFF;
          fx0  &= kA8Mask;

          cover = (yAcc - fy0) * coverSign;
          area  = (area + kA8Scale) * cover;

          IntUtils::bitVectorSetBit(&_bits[ey0 * _bitStride], ex0 / kPixelsPerOneBit, true);
          _mergeCell(ex0, ey0, cover, area);
          ex0++;

          cover = (fy1 - yAcc) * coverSign;
          area  = fx0 * cover;
          IntUtils::bitVectorSetBit(&_bits[ey0 * _bitStride], ex0 / kPixelsPerOneBit, true);
          _mergeCell(ex0, ey0, cover, area);

VertAdvance:
          yAcc += int(yLift);
          yErr += yRem;
          if (yErr >= 0) { yErr -= dx; yAcc++; }
        }

        ey0 += yInc;
      } while (--i);

      if (ey0 == ey1)
        break;

      i = j;
      j = 1;

      if (i > 1) {
        fy0 = 0;
        fy1 = kA8Scale;
        i--;
      }
      else {
        fy0 = 0;
        fy1 = int(y1 & kA8Mask);

        xDlt = x1 - (ex0 << 8) - fx0;
        goto VertSkip;
      }
    }

    return;
  }
  // Horizontal direction -> Two or more cells per scanline.
  else {
    int fx1;
    int coverAcc = fy0;

    cover = int(yDlt);
    coverAcc += cover;

    if (j != 0)
      fy1 = kA8Scale;

    if (fx0 + int(xDlt) > 256)
      goto HorzInside;

    x0 += xDlt;

    cover = (fy1 - fy0) * coverSign;
    area = (fx0 * 2 + int(xDlt)) * cover;

HorzSingle:
    IntUtils::bitVectorSetBit(&_bits[ey0 * _bitStride], ex0 / kPixelsPerOneBit, true);
    _mergeCell(ex0, ey0, cover, area);

    ey0 += yInc;
    if (ey0 == ey1)
      return;

    if (fx0 + int(xDlt) == 256) {
      coverAcc += int(yLift);
      yErr += yRem;
      if (yErr >= 0) { yErr -= dx; coverAcc++; }
    }

    if (--i == 0)
      goto HorzAfter;

    for (;;) {
      do {
        xDlt = xLift;
        xErr += xRem;
        if (xErr >= 0) { xErr -= dy; xDlt++; }

        ex0 = int(x0 >> kA8Shift);
        fx0 = int(x0 & kA8Mask);

HorzSkip:
        coverAcc -= 256;
        cover = coverAcc;
        assert(cover >= 0 && cover <= 256);

HorzInside:
        x0 += xDlt;

        ex1 = int(x0 >> kA8Shift);
        fx1 = int(x0 & kA8Mask);
        assert(ex0 != ex1);

        if (fx1 == 0)
          fx1 = kA8Scale;
        else
          ex1++;

        area = (fx0 + kA8Scale) * cover;
        IntUtils::bitVectorFill(&_bits[ey0 * _bitStride], ex0 / kPixelsPerOneBit, (ex1 / kPixelsPerOneBit) - (ex0 / kPixelsPerOneBit) + 1);

        while (ex0 != ex1 - 1) {
          _mergeCell(ex0, ey0, cover * coverSign, area * coverSign);

          cover = int(yLift);
          yErr += yRem;
          if (yErr >= 0) { yErr -= dx; cover++; }

          coverAcc += cover;
          area  = kA8Scale * cover;

          ex0++;
        }

        cover += fy1 - coverAcc;
        area   = fx1 * cover;
        _mergeCell(ex0, ey0, cover * coverSign, area * coverSign);

        if (fx1 == kA8Scale) {
          coverAcc += int(yLift);
          yErr += yRem;
          if (yErr >= 0) { yErr -= dx; coverAcc++; }
        }

        ey0 += yInc;
      } while (--i);

      if (ey0 == ey1)
        break;

HorzAfter:
      i = j;
      j = 1;

      if (i > 1) {
        fy1 = kA8Scale;
        i--;
      }
      else {
        fy1 = int(y1 & kA8Mask);
        xDlt = x1 - x0;

        ex0 = int(x0 >> kA8Shift);
        fx0 = int(x0 & kA8Mask);

        if (fx0 + int(xDlt) <= 256) {
          cover = fy1 * coverSign;
          area = (fx0 * 2 + int(xDlt)) * cover;
          goto HorzSingle;
        }
        else {
          goto HorzSkip;
        }
      }
    }

    return;
  }
}

// ============================================================================
// [RasterizerA3 - Render]
// ============================================================================

template<uint32_t N>
template<class Compositor, bool NonZero>
inline void RasterizerA3<N>::_renderImpl(uint32_t argb32) noexcept {
  uint8_t* dstLine = _dst->data();
  intptr_t dstStride = _dst->stride();

  size_t y0 = size_t(_yBounds.start);
  size_t y1 = size_t(_yBounds.end);

  BitWord* bitPtr = _bits + y0 * _bitStride;
  Cell* cellLine = _cells + y0 * _cellStride;

  Compositor compositor(argb32);
  dstLine += y0 * dstStride;

  while (y0 <= y1) {
    size_t nBits = size_t(_bitStride);

    Cell* cell = cellLine;
    uint32_t* dstPix = reinterpret_cast<uint32_t*>(dstLine);

    int cover = 0;
    size_t x0 = 0;
    size_t xOffset = 0;

    do {
      IntUtils::BitWordFlipIterator<BitWord> it(*bitPtr);
      *bitPtr = 0;

      while (it.hasNext()) {
        size_t x1 = xOffset + it.nextAndFlip() * kPixelsPerOneBit;
        if (x0 < x1) {
          uint32_t mask = CompositeUtils::calcMask<NonZero>(cover);
          if (mask)
            compositor.cmask(dstPix, x0, x1, mask);
          x0 = x1;
        }

        if (it.hasNext())
          x1 = xOffset + it.nextAndFlip() * kPixelsPerOneBit;
        else
          x1 = std::min<size_t>(_width, xOffset + kPixelsPerBitWord);

        compositor.template vmask<NonZero>(dstPix, x0, x1, cell, cover);
        x0 = x1;
      }

      xOffset += kPixelsPerBitWord;
      bitPtr++;
    } while (--nBits);

    if (x0 < _width) {
      uint32_t mask = CompositeUtils::calcMask<NonZero>(cover);
      if (mask)
        compositor.cmask(dstPix, x0, _width, mask);
    }

    dstLine += dstStride;
    cellLine += _cellStride;
    y0++;
  }

  _yBounds.reset();
}

template<uint32_t N>
void RasterizerA3<N>::render(uint32_t argb32) noexcept {
  doRender(*this, argb32);
}

// ============================================================================
// [RasterizerA3 - New]
// ============================================================================

Rasterizer* newRasterizerA3(Image& dst, uint32_t options, uint32_t n) noexcept {
  switch (n) {
    case 4 : return new(std::nothrow) RasterizerA3<4>(dst, options);
    case 8 : return new(std::nothrow) RasterizerA3<8>(dst, options);
    case 16: return new(std::nothrow) RasterizerA3<16>(dst, options);
    case 32: return new(std::nothrow) RasterizerA3<32>(dst, options);
    default:
      return nullptr;
  }
}
