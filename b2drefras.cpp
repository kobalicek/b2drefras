// Blend2D Reference Rasterizer - Public Domain
#include <stdlib.h>
#include <string.h>

#include "./b2drefras.h"

template<typename T> inline T tAbs(T a) { return a >= 0 ? a : -a; }
template<typename T> inline T tMin(T a, T b) { return a < b ? a : b; }
template<typename T> inline T tMax(T a, T b) { return a > b ? a : b; }

B2DRefRas::B2DRefRas()
  : _width(0),
    _height(0),
    _stride(0),
    _cells(NULL) {}

B2DRefRas::~B2DRefRas() {
  if (_cells)
    ::free(_cells);
}

bool B2DRefRas::init(int w, int h) {
  if (_width != w || _height != h) {
    if (_cells)
      ::free(_cells);

    _width = w;
    _height = h;

    if (w == 0 || h == 0) {
      _stride = 0;
      _cells = NULL;
      return true;
    }

    _stride = w + 1;

    size_t size = h * _stride * sizeof(Cell);
    _cells = static_cast<Cell*>(::malloc(size));

    if (!_cells) {
      _width = 0;
      _height = 0;
      _stride = 0;
      return false;
    }
  }

  if (_cells) {
    size_t size = h * _stride * sizeof(Cell);
    ::memset(_cells, 0, size);
  }

  return true;
}

void B2DRefRas::addLine(int x0, int y0, int x1, int y1) {
  assert(isInitialized());
  if (x0 != x1 || y0 != y1)
    _renderLine<int64_t>(x0, y0, x1, y1);
}

void B2DRefRas::addQuad(int x0, int y0, int x1, int y1, int x2, int y2) {
  assert(isInitialized());

  Point curveStack[32 * 3 + 1];
  Point* curve = curveStack;

  curve[0].x = x2;
  curve[0].y = y2;
  curve[1].x = x1;
  curve[1].y = y1;
  curve[2].x = x0;
  curve[2].y = y0;

  int levelStack[32];
  int level = 0;
  int top = 0;

  int d = tMax(tAbs(curve[2].x + curve[0].x - 2 * curve[1].x),
               tAbs(curve[2].y + curve[0].y - 2 * curve[1].y));

  while (d > (kA8Scale / 6)) {
    d >>= 2;
    level++;
  }

  levelStack[0] = level;
  top = 0;

  do {
    level = levelStack[top];
    if (level > 1) {
      int a, b;

      curve[4].x = curve[2].x;
      b = curve[1].x;
      a = curve[3].x = (curve[2].x + b) / 2;
      b = curve[1].x = (curve[0].x + b) / 2;
      curve[2].x = (a + b) / 2;

      curve[4].y = curve[2].y;
      b = curve[1].y;
      a = curve[3].y = (curve[2].y + b) / 2;
      b = curve[1].y = (curve[0].y + b) / 2;
      curve[2].y = (a + b) / 2;

      curve += 2;
      top++;
      levelStack[top] = levelStack[top - 1] = level - 1;
      continue;
    }

    x1 = curve[0].x;
    y1 = curve[0].y;

    addLine(x0, y0, x1, y1);

    x0 = x1;
    y0 = y1;

    top--;
    curve -= 2;
  } while (top >= 0);
}

void B2DRefRas::addCubic(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3) {
  assert(isInitialized());

  Point curveStack[32 * 3 + 1];
  Point* curve = curveStack;
  Point* curveEnd = curve + 31 * 3;

  curve[0].x = x3;
  curve[0].y = y3;
  curve[1].x = x2;
  curve[1].y = y2;
  curve[2].x = x1;
  curve[2].y = y1;
  curve[3].x = x0;
  curve[3].y = y0;

  for (;;) {
    // See "Rapid Termination. Evaluation for Recursive Subdivision of Bezier Curves".
    if (curve != curveEnd) {
      int dx , dy ;
      int dx_, dy_;
      int dx1, dy1;
      int dx2, dy2;
      int L, s, limit;

      // dx and dy are x and y components of the P0-P3 chord vector.
      dx = curve[3].x - curve[0].x;
      dy = curve[3].y - curve[0].y;

      // L is an (under)estimate of the Euclidean distance P0-P3.
      //
      // If dx >= dy, then r = sqrt(dx^2 + dy^2) can be overestimated
      // with least maximum error by
      //
      //   r_upperbound = dx + (sqrt(2) - 1) * dy,
      //
      // where sqrt(2) - 1 can be (over)estimated by 107/256, giving an
      // error of no more than 8.4%.
      //
      // Similarly, some elementary calculus shows that r can be
      // underestimated with least maximum error by
      //
      //   r_lowerbound = sqrt(2 + sqrt(2)) / 2 * dx
      //                + sqrt(2 - sqrt(2)) / 2 * dy .
      //
      // 236/256 and 97/256 are (under)estimates of the two algebraic
      // numbers, giving an error of no more than 8.1%.
      dx_ = tAbs(dx);
      dy_ = tAbs(dy);

      // This is the same as:
      //
      //   L = (236 * max( dx_, dy_ ) + 97 * min( dx_, dy_ )) >> 8;
      L = ((dx_ > dy_) ? (236 * dx_ + 97 * dy_) : ( 97 * dx_ + 236 * dy_)) >> 8;

      // Avoid possible arithmetic overflow below by splitting.
      if (L > 32767)
        goto _Split;

      // Max deviation may be as much as (s/L) * 3/4 (if Hain's v = 1).
      limit = L * (int)(kA8Scale / 6);

      // s is L * the perpendicular distance from P1 to the line P0-P3.
      dx1 = curve[1].x - curve[0].x;
      dy1 = curve[1].y - curve[0].y;
      s = tAbs(dy * dx1 - dx * dy1);

      if (s > limit)
        goto _Split;

      // s is L * the perpendicular distance from P2 to the line P0-P3.
      dx2 = curve[2].x - curve[0].x;
      dy2 = curve[2].y - curve[0].y;
      s = tAbs(dy * dx2 - dx * dy2);

      if (s > limit)
        goto _Split;

      // If P1 or P2 is outside P0-P3, split the curve.
      if ((dy * dy1 + dx * dx1 < 0) ||
          (dy * dy2 + dx * dx2 < 0) ||
          (dy * (curve[3].y - curve[1].y) + dx * (curve[3].x - curve[1].x) < 0) ||
          (dy * (curve[3].y - curve[2].y) + dx * (curve[3].x - curve[2].x) < 0))
        goto _Split;
    }

    {
      x1 = curve[0].x;
      y1 = curve[0].y;

      addLine(x0, y0, x1, y1);

      x0 = x1;
      y0 = y1;

      if (curve == curveStack)
        break;

      curve -= 3;
      continue;
    }

_Split:
    {
      int a, b, c, d;

      curve[6].x = curve[3].x;
      c = curve[1].x;
      d = curve[2].x;
      curve[1].x = a = ( curve[0].x + c ) / 2;
      curve[5].x = b = ( curve[3].x + d ) / 2;
      c = ( c + d ) / 2;
      curve[2].x = a = ( a + c ) / 2;
      curve[4].x = b = ( b + c ) / 2;
      curve[3].x = ( a + b ) / 2;

      curve[6].y = curve[3].y;
      c = curve[1].y;
      d = curve[2].y;
      curve[1].y = a = ( curve[0].y + c ) / 2;
      curve[5].y = b = ( curve[3].y + d ) / 2;
      c = ( c + d ) / 2;
      curve[2].y = a = ( a + c ) / 2;
      curve[4].y = b = ( b + c ) / 2;
      curve[3].y = ( a + b ) / 2;
    }

    curve += 3;
    continue;
  }
}

template<typename Fixed>
void B2DRefRas::_renderLine(Fixed x0, Fixed y0, Fixed x1, Fixed y1) {
  Fixed dx = x1 - x0;
  Fixed dy = y1 - y0;

  if (dy == Fixed(0))
    return;

  int cover = int(dy);
  int area;

  if (dx < 0)
    dx = -dx;

  if (dy < 0)
    dy = -dy;

  int yInc = 1;
  int coverSign = 1;

  // Fix RIGHT-TO-LEFT direction:
  //   - swap coordinates,
  //   - invert cover-sign.
  if (x0 > x1) {
    int tx, ty;

    tx = x0; x0 = x1; x1 = tx;
    ty = y0; y0 = y1; y1 = ty;

    coverSign = -coverSign;
  }

  // Fix BOTTOM-TO-TOP direction:
  //   - invert fractional parts of y0 and y1,
  //   - invert cover-sign.
  if (y0 > y1) {
    static const int norm[2] = { 1, 1 - kA8Scale * 2};

    y0 ^= kA8Mask;
    y0 += norm[int(y0 & kA8Mask) == kA8Mask];
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
    _mergeCell(ex0, ey0, cover, (fx0 * 2 + int(dx)) * cover);
    return;
  }

  uint32_t ey1 = ey0 + (j + (fy1 != 0)) * yInc;

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

    goto _Vert_Skip;
    for (;;) {
      do {
        xDlt = xLift;
        xErr += xRem;
        if (xErr >= 0) { xErr -= dy; xDlt++; }

_Vert_Skip:
        area = fx0;
        fx0 += int(xDlt);

        if (fx0 <= 256) {
          cover = (fy1 - fy0) * coverSign;
          area  = (area + fx0) * cover;
          _mergeCell(ex0, ey0, cover, area);

          if (fx0 == 256) {
            ex0++;
            fx0 = 0;
            goto _Vert_Advance;
          }
        }
        else {
          yAcc &= 0xFF;
          fx0  &= kA8Mask;

          cover = (yAcc - fy0) * coverSign;
          area  = (area + kA8Scale) * cover;

          _mergeCell(ex0, ey0, cover, area);
          ex0++;

          cover = (fy1 - yAcc) * coverSign;
          area  = fx0 * cover;
          _mergeCell(ex0, ey0, cover, area);

_Vert_Advance:
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
        goto _Vert_Skip;
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
      goto _Horz_Inside;

    x0 += xDlt;

    cover = (fy1 - fy0) * coverSign;
    area = (fx0 * 2 + int(xDlt)) * cover;

_Horz_Single:
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
      goto _Horz_After;

    for (;;) {
      do {
        xDlt = xLift;
        xErr += xRem;
        if (xErr >= 0) { xErr -= dy; xDlt++; }

        ex0 = int(x0 >> kA8Shift);
        fx0 = int(x0 & kA8Mask);

_Horz_Skip:
        coverAcc -= 256;
        cover = coverAcc;
        assert(cover >= 0 && cover <= 256);

_Horz_Inside:
        x0 += xDlt;

        ex1 = int(x0 >> kA8Shift);
        fx1 = int(x0 & kA8Mask);
        assert(ex0 != ex1);

        if (fx1 == 0)
          fx1 = kA8Scale;
        else
          ex1++;

        area = (fx0 + kA8Scale) * cover;
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

_Horz_After:
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
          goto _Horz_Single;
        }
        else {
          goto _Horz_Skip;
        }
      }
    }

    return;
  }
}

void B2DRefRas::sweepScanline(int y, bool nonZero, uint8_t* buffer) {
  int cover = 0;
  const Cell* cell = &_cells[y * _stride];

  for (int x = 0; x < _width; x++) {
    cover += cell[x].cover;

    int alpha = cover - (cell[x].area >> B2DRefRas::kA8Shift_2);
    if (nonZero) {
      if (alpha < 0) alpha = -alpha;
      if (alpha > 255) alpha = 255;
    }
    else {
      alpha = alpha & 0x1FF;
      if (alpha >= 256) alpha = 511 - alpha;
    }
    buffer[x] = static_cast<uint8_t>(alpha & 0xFF);
  }
}
