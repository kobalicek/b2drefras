#include "./base.h"
#include "./rasterizer-a1.h"
#include "./rasterizer-a2.h"

Rasterizer::Rasterizer() noexcept
  : _width(0),
    _height(0),
    _nonZero(false) {}
Rasterizer::~Rasterizer() noexcept {}

Rasterizer* Rasterizer::newById(uint32_t id) {
  switch (id) {
    case kIdA1: return new (std::nothrow) RasterizerA1();
    case kIdA2: return new (std::nothrow) RasterizerA2();

    default:
      return nullptr;
  }
}
