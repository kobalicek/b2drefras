#ifndef _BASE_H
#define _BASE_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <limits>

// ============================================================================
// [Globals]
// ============================================================================

#define ARRAY_SIZE(X) (sizeof(X) / sizeof(X[0]))

struct Point { double x, y; };
struct PointFx { int x, y; };

// ============================================================================
// [Random]
// ============================================================================

//! Simple PRNG (Pseudo Random Number Generator).
//!
//! The current implementation uses a PRNG called `XORSHIFT+`, which has `64`
//! bit seed, `128` bits of state and full period `2^128-1`.
//!
//! Based on a paper by Sebastiano Vigna:
//!   http://vigna.di.unimi.it/ftp/papers/xorshiftplus.pdf
class Random {
public:
  // The number is arbitrary, it means nothing.
  static constexpr uint64_t kZeroSeed = 0x1F0A2BE71D163FA0U;

  // The constants used are the constants suggested as `23/18/5`.
  enum Steps : uint32_t {
    kStep1_SHL = 23,
    kStep2_SHR = 18,
    kStep3_SHR = 5
  };

  struct State {
    inline uint64_t seed() const noexcept { return _seed; }

    inline bool eq(const State& other) const noexcept {
      return _seed    == other._seed    &&
             _data[0] == other._data[0] &&
             _data[1] == other._data[1] ;
    }

    uint64_t _seed;
    uint64_t _data[2];
  };

  inline Random(uint64_t seed = 0) noexcept { reset(seed); }
  inline Random(const Random& other) noexcept = default;
  inline Random& operator=(const Random& other) noexcept = default;

  //! Get the initial seed of the PRNG.
  inline uint64_t initialSeed() const noexcept { return _state._seed; }

  //! Get the current PRNG state, including its seed.
  inline const State& state() const noexcept { return _state; }
  //! Set the current PRNG state, including its seed.
  inline void setState(const State& state) noexcept { _state = state; }

  //! Reset the PRNG and initialize it to the given `seed`.
  inline void reset(uint64_t seed = 0) noexcept {
    // Generate the state data by using splitmix64.
    _state._seed = seed;
    for (uint32_t i = 0; i < 2; i++) {
      seed += 0x9E3779B97F4A7C15U;
      uint64_t x = seed;
      x = (x ^ (x >> 30)) * 0xBF58476D1CE4E5B9U;
      x = (x ^ (x >> 27)) * 0x94D049BB133111EBU;
      x = (x ^ (x >> 31));
      _state._data[i] = x != 0 ? x : kZeroSeed;
    }
  }

  //! Rewind the PRNG so it starts from the beginning.
  inline void rewind() noexcept { reset(_state._seed); }

  inline uint64_t nextUInt64() noexcept {
    uint64_t x = _state._data[0];
    uint64_t y = _state._data[1];

    x ^= x << kStep1_SHL;
    y ^= y >> kStep3_SHR;
    x ^= x >> kStep2_SHR;
    x ^= y;

    _state._data[0] = y;
    _state._data[1] = x;

    return x + y;
  }

  inline uint32_t nextUInt32() noexcept {
    return uint32_t(nextUInt64() >> 32);
  }

  inline double nextDouble() noexcept {
    // Number of bits needed to shift right to extract mantissa.
    uint32_t kMantissaShift = 64 - 52;
    uint64_t kExponentMask = 0x3FF0000000000000U;

    union Bin64 {
      uint64_t u;
      double d;
    };

    Bin64 bin64 = { (nextUInt64() >> kMantissaShift) | kExponentMask };
    return bin64.d - 1.0;
  }

  State _state;
};

// ============================================================================
// [PixelUtils]
// ============================================================================

namespace PixelUtils {
  inline uint32_t udiv255(uint32_t x) noexcept {
    return ((x + 128) * 257) >> 16;
  }

  inline uint32_t premultiply(uint32_t pix) noexcept {
    uint32_t rb = pix;
    uint32_t ag = pix;
    uint32_t alpha = pix >> 24;

    ag |= 0xFF000000U;
    ag >>= 8;

    rb = (rb & 0x00FF00FFU) * alpha;
    ag = (ag & 0x00FF00FFU) * alpha;

    rb += 0x00800080U;
    ag += 0x00800080U;

    rb = (rb + ((rb >> 8) & 0x00FF00FFU)) & 0xFF00FF00U;
    ag = (ag + ((ag >> 8) & 0x00FF00FFU)) & 0xFF00FF00U;

    rb >>= 8;
    return ag + rb;
  }

  inline uint32_t src(uint32_t aPix, uint32_t bPix, uint32_t mask) noexcept {
    uint32_t a20 = (aPix     ) & 0x00FF00FFU;
    uint32_t a31 = (aPix >> 8) & 0x00FF00FFU;

    uint32_t b20 = (bPix     ) & 0x00FF00FFU;
    uint32_t b31 = (bPix >> 8) & 0x00FF00FFU;

    uint32_t mInv = 255 - mask;

    return (((a20 * mInv + b20 * mask) & 0xFF00FF00) >> 8 ) |
           (((a31 * mInv + b31 * mask) & 0xFF00FF00)      ) ;
  }
}

// ============================================================================
// [BmpHeader]
// ============================================================================

#pragma pack(push, 1)
struct BmpHeader {
  void init(int w, int h) {
    std::memset(this, 0, sizeof(BmpHeader));

    signature[0]    = 'B';
    signature[1]    = 'M';

    headerSize      = 40; // Version 1.
    width           = w;
    height          = -h;
    planes          = 1;
    bitsPerPixel    = 32;
    compression     = 0;
    imageSize       = w * h * 4;
    colorsUsed      = 0;
    colorsImportant = 0;

    imageOffset     = 14 + headerSize;
    fileSize        = imageOffset + imageSize;
  }

  uint8_t padding[2];
  uint8_t signature[2];
  uint32_t fileSize;
  uint32_t reserved;
  uint32_t imageOffset;
  uint32_t headerSize;
  int32_t width;
  int32_t height;
  uint16_t planes;
  uint16_t bitsPerPixel;
  uint32_t compression;
  uint32_t imageSize;
  uint32_t horzResolution;
  uint32_t vertResolution;
  uint32_t colorsUsed;
  uint32_t colorsImportant;
};
#pragma pack(pop)

// ============================================================================
// [Image]
// ============================================================================

// 32-bit ARGB image.
class Image {
public:
  inline Image() noexcept :
    _width(0),
    _height(0),
    _stride(0),
    _data(nullptr) {}

  inline Image(Image&& other) noexcept :
    _width(other._width),
    _height(other._height),
    _stride(other._stride),
    _data(other._data) { other._data = nullptr; }

  inline ~Image() noexcept {
    if (_data)
      std::free(_data);
  }

  Image(const Image& other) noexcept = delete;
  Image& operator=(const Image& other) noexcept = delete;

  void reset() noexcept {
    if (_data) {
      std::free(_data);
      _width = 0;
      _height = 0;
      _stride = 0;
      _data = nullptr;
    }
  }

  bool create(int w, int h) noexcept {
    if (_data)
      std::free(_data);

    bool ok = true;
    if (w > 0 && h > 0) {
      _width = w;
      _height = h;
      _stride = w * 4;
      _data = static_cast<uint8_t*>(std::malloc(size_t(_stride * h)));

      ok = _data != nullptr;
      if (ok) return ok;
    }

    _width = 0;
    _height = 0;
    _stride = 0;
    _data = nullptr;

    return ok;
  }

  inline int width() const noexcept { return _width; }
  inline int height() const noexcept { return _height; }
  inline intptr_t stride() const noexcept { return _stride; }

  template<typename T = uint8_t>
  inline T* data() const noexcept { return reinterpret_cast<T*>(_data); }

  void fillAll(uint32_t argb32) noexcept {
    fillRect(0, 0, int(_width), int(_height), argb32);
  }

  void fillRect(int x, int y, int w, int h, uint32_t argb32) noexcept {
    int x0 = x;
    int y0 = y;
    int x1 = x + w;
    int y1 = y + h;

    if (x0 < 0) x0 = 0;
    if (y0 < 0) y0 = 0;
    if (x1 > _width) x1 = _width;
    if (y1 > _height) y1 = _height;

    if (x0 >= x1)
      return;

    uint8_t* scanline = _data + intptr_t(y0) * _stride + x0 * 4;
    size_t width = size_t(x1 - x0);

    uint32_t prgb32 = PixelUtils::premultiply(argb32);
    while (y0 < y1) {
      uint32_t* p = reinterpret_cast<uint32_t*>(scanline);
      for (size_t i = 0; i < width; i++)
        p[i] = prgb32;
      y0++;
    }
  }

  bool writeBmp(const char* fileName) const noexcept {
    BmpHeader bmp;
    bmp.init(_width, _height);

    std::FILE* f = std::fopen(fileName, "wb");
    if (!f)
      return false;

    std::fwrite(&bmp.signature, sizeof(BmpHeader) - 2, 1, f);
    std::fwrite(_data, _stride * _height, 1, f);
    std::fclose(f);
    return true;
  }

  int _width;
  int _height;
  intptr_t _stride;
  uint8_t* _data;
};

// ============================================================================
// [Rasterizer]
// ============================================================================

class Rasterizer {
public:
  // Rasterizer ID.
  enum Id : uint32_t {
    kIdA1 = 0,
    kIdA2,
    kIdCount
  };
  static Rasterizer* newById(uint32_t id);

  Rasterizer() noexcept;
  virtual ~Rasterizer() noexcept;

  virtual const char* name() const noexcept = 0;

  virtual bool init(int w, int h) noexcept = 0;
  virtual void reset() noexcept = 0;

  virtual void clear() noexcept = 0;
  virtual bool addPoly(const Point* poly, size_t count) noexcept = 0;
  virtual bool render(Image& dst, uint32_t argb32) noexcept = 0;

  inline bool isInitialized() const noexcept { return _width != 0; }
  inline int width() const noexcept { return _width; }
  inline int height() const noexcept { return _height; }

  int _width;
  int _height;
  bool _nonZero;
};

#endif // _BASE_H
