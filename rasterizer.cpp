#include "./rasterizer.h"

// ============================================================================
// [Rasterizer]
// ============================================================================

Rasterizer::Rasterizer() noexcept
  : _width(0),
    _height(0),
    _nonZero(false) {}
Rasterizer::~Rasterizer() noexcept {}

// ============================================================================
// [Rasterizer - NewById]
// ============================================================================

Rasterizer* newRasterizerA1() noexcept;
Rasterizer* newRasterizerA2() noexcept;
Rasterizer* newRasterizerA3(uint32_t pixelsPerOneBit) noexcept;

Rasterizer* Rasterizer::newById(uint32_t id) {
  switch (id) {
    case kIdA1   : return newRasterizerA1();
    case kIdA2   : return newRasterizerA2();
    case kIdA3x4 : return newRasterizerA3(4);
    case kIdA3x8 : return newRasterizerA3(8);
    case kIdA3x16: return newRasterizerA3(16);
    case kIdA3x32: return newRasterizerA3(32);

    default:
      return nullptr;
  }
}

// ============================================================================
// [CellRasterizer]
// ============================================================================

CellRasterizer::CellRasterizer() noexcept : Rasterizer() {}
CellRasterizer::~CellRasterizer() noexcept {}
