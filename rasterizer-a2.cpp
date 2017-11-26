#include "./rasterizer-a2.h"

#include <string.h>
#include <stdlib.h>

RasterizerA2::RasterizerA2() noexcept :
  Rasterizer(),
  _cellStride(0),
  _yBounds { 0, 0 },
  _xBounds(nullptr),
  _cells(nullptr) {}

RasterizerA2::~RasterizerA2() noexcept {
  reset();
}

const char* RasterizerA2::name() const noexcept {
  return "A2";
}

bool RasterizerA2::init(int w, int h) noexcept {
  if (_width != w || _height != h) {
    if (_cells) std::free(_cells);
    if (_xBounds) std::free(_xBounds);

    _width = w;
    _height = h;

    if (w == 0 || h == 0) {
      _cellStride = 0;
      _yBounds.reset();
      _xBounds = nullptr;
      _cells = nullptr;
      return true;
    }

    _cellStride = w + 1;
    _xBounds = static_cast<Bounds*>(std::malloc(h * sizeof(Bounds)));
    _cells = static_cast<Cell*>(std::malloc(h * _cellStride * sizeof(Cell)));

    if (!_cells || !_xBounds) {
      if (_cells) std::free(_cells);
      if (_xBounds) std::free(_xBounds);

      _width = 0;
      _height = 0;
      _cellStride = 0;
      _yBounds.reset();
      _xBounds = nullptr;
      _cells = nullptr;
      return false;
    }

    size_t size = _height * _cellStride * sizeof(Cell);
    std::memset(_cells, 0, size);
    _yBounds.reset();
    for (int y = 0; y < _height; y++)
      _xBounds[y].reset();
  }
  else {
    // This is much faster, will only clear the affected area.
    clear();
  }

  return true;
}

void RasterizerA2::reset() noexcept {
  if (isInitialized()) {
    std::free(_cells);
    std::free(_xBounds);

    _width = 0;
    _height = 0;
    _cellStride = 0;
    _yBounds.reset();
    _xBounds = nullptr;
    _cells = nullptr;
  }
}

void RasterizerA2::clear() noexcept {
  if (isInitialized()) {
    size_t y0 = size_t(_yBounds.start);
    size_t y1 = size_t(_yBounds.end);

    Cell* cellPtr = _cells + y0 * _cellStride;
    while (y0 <= y1) {
      int x0 = _xBounds[y0].start;
      int x1 = _xBounds[y0].end;

      if (x0 <= x1) {
        size_t width = size_t(x1 - x0 + 1);
        std::memset(cellPtr + x0, 0, width * sizeof(Cell));
        _xBounds[y0].reset();
      }

      cellPtr += _cellStride;
      y0++;
    }

    _yBounds.reset();
  }
}

bool RasterizerA2::addPoly(const Point* poly, size_t count) noexcept {
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

template<typename Fixed>
void RasterizerA2::_addLine(Fixed x0, Fixed y0, Fixed x1, Fixed y1) noexcept {
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
    _xBounds[ey0].union_(ex0, ex0);
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

    for (;;) {
      area = fx0 * cover;
      do {
        _xBounds[ey0].union_(ex0, ex0);
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
          _xBounds[ey0].union_(ex0, ex0);
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

          _xBounds[ey0].mergeStart(ex0);
          _mergeCell(ex0, ey0, cover, area);
          ex0++;

          cover = (fy1 - yAcc) * coverSign;
          area  = fx0 * cover;
          _xBounds[ey0].mergeEnd(ex0);
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
    _xBounds[ey0].mergeStart(ex0);
    _xBounds[ey0].mergeEnd(ex0);
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
        _xBounds[ey0].mergeStart(ex0);

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
        _xBounds[ey0].mergeEnd(ex0);
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

template<bool NonZero>
inline void RasterizerA2::_renderImpl(Image& dst, uint32_t argb32) noexcept {
  uint8_t* dstLine = dst.data();
  intptr_t stride = dst.stride();
  uint32_t prgb32 = PixelUtils::premultiply(argb32);

  size_t y0 = size_t(_yBounds.start);
  size_t y1 = size_t(_yBounds.end);

  dstLine += y0 * stride;
  while (y0 <= y1) {
    uint32_t* dstPix = reinterpret_cast<uint32_t*>(dstLine);
    const Cell* cell = &_cells[y0 * _cellStride];

    int cover = 0;
    int x = _xBounds[y0].start;
    int xEnd = std::min(_xBounds[y0].end + 1, _width);

    while (x < xEnd) {
      cover += cell[x].cover;
      int mask = cover - (cell[x].area >> kA8Shift_2);

      if (NonZero) {
        if (mask < 0) mask = -mask;
        if (mask > 255) mask = 255;
      }
      else {
        mask = mask & 0x1FF;
        if (mask > 255) mask = 511 - mask;
      }

      x++;
      if (!mask)
        continue;

      if (mask == 255)
        dstPix[x - 1] = prgb32;
      else
        dstPix[x - 1] = PixelUtils::src(dstPix[x - 1], prgb32, uint32_t(mask));
    }

    y0++;
    dstLine += stride;
  }
}

bool RasterizerA2::render(Image& dst, uint32_t argb32) noexcept {
  if (_nonZero)
    _renderImpl<true>(dst, argb32);
  else
    _renderImpl<false>(dst, argb32);
  return true;
}
