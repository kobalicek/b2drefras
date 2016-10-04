#include <assert.h>
#include <stddef.h>
#include <stdint.h>

class B2DRefRas {
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

  struct Point {
    int x, y;
  };

  B2DRefRas();
  ~B2DRefRas();

  bool init(int w, int h);
  inline bool isInitialized() const { return _cells != NULL; }

  void addLine(int x0, int y0, int x1, int y1);
  void addQuad(int x0, int y0, int x1, int y1, int x2, int y2);
  void addCubic(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3);

  template<typename Fixed>
  void _renderLine(Fixed x0, Fixed y0, Fixed x1, Fixed y1);

  inline void _mergeCell(int x, int y, int cover, int area) {
    assert(x >= 0 && x < _width);
    assert(y >= 0 && y < _height);

    Cell& cell = _cells[y * _stride + x];
    cell.cover += cover;
    cell.area  += area;
  }

  int _width;
  int _height;
  intptr_t _stride;
  Cell* _cells;
};
