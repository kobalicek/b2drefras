#include "./rasterizer.h"

#include <cstring>

// ============================================================================
// [Rasterizer]
// ============================================================================

Rasterizer::Rasterizer(Image& dst, uint32_t options) noexcept
  : _dst(&dst),
    _width(0),
    _height(0),
    _options(options),
    _fillMode(kFillEvenOdd) {}
Rasterizer::~Rasterizer() noexcept {}

void Rasterizer::addOptionsToName() noexcept {
  if (hasOption(kOptionSIMD))
    std::strcat(_name, "_SIMD");
}

// ============================================================================
// [Rasterizer - NewById]
// ============================================================================

Rasterizer* newRasterizerA1(Image& dst, uint32_t options) noexcept;
Rasterizer* newRasterizerA2(Image& dst, uint32_t options) noexcept;
Rasterizer* newRasterizerA3(Image& dst, uint32_t options, uint32_t n) noexcept;
Rasterizer* newRasterizerAGG(Image& dst, uint32_t options) noexcept;

Rasterizer* Rasterizer::newById(Image& dst, uint32_t id, uint32_t options) {
  switch (id) {
    case kIdAGG  : return newRasterizerAGG(dst, options);
    case kIdA1   : return newRasterizerA1(dst, options);
    case kIdA2   : return newRasterizerA2(dst, options);
    case kIdA3x4 : return newRasterizerA3(dst, options, 4);
    case kIdA3x8 : return newRasterizerA3(dst, options, 8);
    case kIdA3x16: return newRasterizerA3(dst, options, 16);
    case kIdA3x32: return newRasterizerA3(dst, options, 32);

    default:
      return nullptr;
  }
}

// ============================================================================
// [CellRasterizer]
// ============================================================================

CellRasterizer::CellRasterizer(Image& dst, uint32_t options) noexcept
  : Rasterizer(dst, options) {}
CellRasterizer::~CellRasterizer() noexcept {}
