#ifndef _INTUTILS_H
#define _INTUTILS_H

#include "./globals.h"

namespace IntUtils {

namespace Internal {
  // IntBySize - Make an int-type by size (signed or unsigned) that is the
  //             same as types defined by <stdint.h>.
  // Int32Or64 - Make an int-type that has at least 32 bits: [u]int[32|64]_t.

  template<size_t SIZE, int IS_SIGNED>
  struct IntBySize {}; // Fail if not specialized.

  template<> struct IntBySize<1, 0> { typedef uint8_t  Type; };
  template<> struct IntBySize<1, 1> { typedef int8_t   Type; };
  template<> struct IntBySize<2, 0> { typedef uint16_t Type; };
  template<> struct IntBySize<2, 1> { typedef int16_t  Type; };
  template<> struct IntBySize<4, 0> { typedef uint32_t Type; };
  template<> struct IntBySize<4, 1> { typedef int32_t  Type; };
  template<> struct IntBySize<8, 0> { typedef uint64_t Type; };
  template<> struct IntBySize<8, 1> { typedef int64_t  Type; };

  template<typename T, int IS_SIGNED = std::is_signed<T>::value>
  struct Int32Or64 : public IntBySize<sizeof(T) <= 4 ? size_t(4) : sizeof(T), IS_SIGNED> {};
}

//! Cast an integer `x` to either `int32_t`, uint32_t`, `int64_t`, or `uint64_t` depending on `T`.
template<typename T>
constexpr typename Internal::Int32Or64<T>::Type asNormalized(T x) noexcept { return (typename Internal::Int32Or64<T>::Type)x; }

//! Cast an integer `x` to either `int32_t` or `int64_t` depending on `T`.
template<typename T>
constexpr typename Internal::Int32Or64<T, 1>::Type asInt(T x) noexcept { return (typename Internal::Int32Or64<T, 1>::Type)x; }

//! Cast an integer `x` to either `uint32_t` or `uint64_t` depending on `T`.
template<typename T>
constexpr typename Internal::Int32Or64<T, 0>::Type asUInt(T x) noexcept { return (typename Internal::Int32Or64<T, 0>::Type)x; }

// ============================================================================
// [IntUtils::BitSizeOf]
// ============================================================================

template<typename T>
constexpr uint32_t bitSizeOf() noexcept { return uint32_t(sizeof(T) * 8U); }

// ============================================================================
// [IntUtils::BitUtilities]
// ============================================================================

//! Returns `0 - x` in a safe way (no undefined behavior), works for unsigned numbers as well.
template<typename T>
constexpr T neg(const T& x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return T(U(0) - U(x));
}

template<typename T>
constexpr T allOnes() noexcept { return neg<T>(T(1)); }

//! Returns `x << y` (shift left logical) by explicitly casting `x` to an unsigned type and back.
template<typename X, typename Y>
constexpr X shl(const X& x, const Y& y) noexcept {
  typedef typename std::make_unsigned<X>::type U;
  return X(U(x) << y);
}

//! Returns `x >> y` (shift right logical) by explicitly casting `x` to an unsigned type and back.
template<typename X, typename Y>
constexpr X shr(const X& x, const Y& y) noexcept {
  typedef typename std::make_unsigned<X>::type U;
  return X(U(x) >> y);
}

//! Returns `x >> y` (shift right arithmetic) by explicitly casting `x` to a signed type and back.
template<typename X, typename Y>
constexpr X sar(const X& x, const Y& y) noexcept {
  typedef typename std::make_signed<X>::type S;
  return X(S(x) >> y);
}

//! Returns `x | (x >> y)` - helper used by some bit manipulation helpers.
template<typename X, typename Y>
constexpr X or_shr(const X& x, const Y& y) noexcept { return X(x | shr(x, y)); }

//! Returns `x & -x` - extracts lowest set isolated bit (like BLSI instruction).
template<typename T>
constexpr T blsi(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return T(U(x) & neg(U(x)));
}

//! Returns `x & (x - 1)` - resets lowest set bit (like BLSR instruction).
template<typename T>
constexpr T blsr(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return T(U(x) & (U(x) - U(1)));
}

//! Generate a trailing bit-mask that has `n` least significant (trailing) bits set.
template<typename T, typename COUNT>
constexpr T lsbMask(COUNT n) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return (sizeof(U) < sizeof(uintptr_t))
    ? T(U((uintptr_t(1) << n) - uintptr_t(1)))
    // Shifting more bits than the type provides is UNDEFINED BEHAVIOR.
    // In such case we trash the result by ORing it with a mask that has
    // all bits set and discards the UNDEFINED RESULT of the shift.
    : T(((U(1) << n) - U(1U)) | neg(U(n >= COUNT(bitSizeOf<T>()))));
}

//! Get whether `x` has Nth bit set.
template<typename T, typename INDEX>
constexpr bool bitTest(T x, INDEX n) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return (U(x) & (U(1) << n)) != 0;
}

//! Get whether the `x` is a power of two (only one bit is set).
template<typename T>
constexpr bool isPowerOf2(T x) noexcept {
  typedef typename std::make_unsigned<T>::type U;
  return x && !(U(x) & (U(x) - U(1)));
}

// ============================================================================
// [IntUtils::CTZ]
// ============================================================================

//! \internal
namespace Internal {
  constexpr uint32_t ctzGenericImpl(uint32_t xAndNegX) noexcept {
    return 31 - ((xAndNegX & 0x0000FFFFU) ? 16 : 0)
              - ((xAndNegX & 0x00FF00FFU) ?  8 : 0)
              - ((xAndNegX & 0x0F0F0F0FU) ?  4 : 0)
              - ((xAndNegX & 0x33333333U) ?  2 : 0)
              - ((xAndNegX & 0x55555555U) ?  1 : 0);
  }

  constexpr uint32_t ctzGenericImpl(uint64_t xAndNegX) noexcept {
    return 63 - ((xAndNegX & 0x00000000FFFFFFFFU) ? 32 : 0)
              - ((xAndNegX & 0x0000FFFF0000FFFFU) ? 16 : 0)
              - ((xAndNegX & 0x00FF00FF00FF00FFU) ?  8 : 0)
              - ((xAndNegX & 0x0F0F0F0F0F0F0F0FU) ?  4 : 0)
              - ((xAndNegX & 0x3333333333333333U) ?  2 : 0)
              - ((xAndNegX & 0x5555555555555555U) ?  1 : 0);
  }

  template<typename T>
  constexpr T ctzGeneric(T x) noexcept {
    return ctzGenericImpl(x & neg(x));
  }

  static ALWAYS_INLINE uint32_t ctz(uint32_t x) noexcept {
    #if defined(_MSC_VER)
    unsigned long i;
    _BitScanForward(&i, x);
    return uint32_t(i);
    #elif defined(__GNUC__)
    return uint32_t(__builtin_ctz(x));
    #else
    return ctzGeneric(x);
    #endif
  }

  static ALWAYS_INLINE uint32_t ctz(uint64_t x) noexcept {
    #if defined(_MSC_VER)
    unsigned long i;
    _BitScanForward64(&i, x);
    return uint32_t(i);
    #elif defined(__GNUC__)
    return uint32_t(__builtin_ctzll(x));
    #else
    return ctzGeneric(x);
    #endif
  }
}

//! Count trailing zeros in `x` (returns a position of a first bit set in `x`).
//!
//! NOTE: The input MUST NOT be zero, otherwise the result is undefined.
template<typename T>
static inline uint32_t ctz(T x) noexcept { return Internal::ctz(asUInt(x)); }

// ============================================================================
// [IntUtils::BitOps]
// ============================================================================

struct Set    { template<typename T> static inline T op(T x, T y) noexcept { (void)(x); return  y; } };
struct SetNot { template<typename T> static inline T op(T x, T y) noexcept { (void)(x); return ~y; } };
struct And    { template<typename T> static inline T op(T x, T y) noexcept { return  x &  y; } };
struct AndNot { template<typename T> static inline T op(T x, T y) noexcept { return  x & ~y; } };
struct NotAnd { template<typename T> static inline T op(T x, T y) noexcept { return ~x &  y; } };
struct Or     { template<typename T> static inline T op(T x, T y) noexcept { return  x |  y; } };
struct Xor    { template<typename T> static inline T op(T x, T y) noexcept { return  x ^  y; } };

// ============================================================================
// [IntUtils::BitWord]
// ============================================================================

typedef Internal::IntBySize<sizeof(uintptr_t), 0>::Type BitWord;
static constexpr uint32_t kBitWordBits = sizeof(BitWord) * 8;

constexpr size_t nBitWordsForNBits(size_t nBits) noexcept {
  return (nBits + kBitWordBits - 1) / kBitWordBits;
}

namespace Internal {
  template<typename T, class OPERATOR, class FULL_WORD_OP>
  static inline void bitVectorOp(T* buf, size_t index, size_t count) noexcept {
    if (count == 0)
      return;

    const size_t kTSizeInBits = bitSizeOf<T>();
    size_t vecIndex = index / kTSizeInBits; // T[]
    size_t bitIndex = index % kTSizeInBits; // T[][]

    buf += vecIndex;

    // The first BitWord requires special handling to preserve bits outside the fill region.
    const T kFillMask = allOnes<T>();
    size_t firstNBits = std::min<size_t>(kTSizeInBits - bitIndex, count);

    buf[0] = OPERATOR::op(buf[0], (kFillMask >> (kTSizeInBits - firstNBits)) << bitIndex);
    buf++;
    count -= firstNBits;

    // All bits between the first and last affected BitWords can be just filled.
    while (count >= kTSizeInBits) {
      buf[0] = FULL_WORD_OP::op(buf[0], kFillMask);
      buf++;
      count -= kTSizeInBits;
    }

    // The last BitWord requires special handling as well
    if (count)
      buf[0] = OPERATOR::op(buf[0], kFillMask >> (kTSizeInBits - count));
  }
}

//! Set bit in a bit-vector `buf` at `index`.
template<typename T>
static inline bool bitVectorGetBit(T* buf, size_t index) noexcept {
  const size_t kTSizeInBits = bitSizeOf<T>();

  size_t vecIndex = index / kTSizeInBits;
  size_t bitIndex = index % kTSizeInBits;

  return bool((buf[vecIndex] >> bitIndex) & 0x1U);
}

//! Set bit in a bit-vector `buf` at `index` to `value`.
template<typename T>
static inline void bitVectorSetBit(T* buf, size_t index, bool value) noexcept {
  const size_t kTSizeInBits = bitSizeOf<T>();

  size_t vecIndex = index / kTSizeInBits;
  size_t bitIndex = index % kTSizeInBits;

  T bitMask = T(1U) << bitIndex;
  if (value)
    buf[vecIndex] |= bitMask;
  else
    buf[vecIndex] &= ~bitMask;
}

//! Fill `count` bits in bit-vector `buf` starting at bit-index `index`.
template<typename T>
static inline void bitVectorFill(T* buf, size_t index, size_t count) noexcept { Internal::bitVectorOp<T, Or, Set>(buf, index, count); }

//! Clear `count` bits in bit-vector `buf` starting at bit-index `index`.
template<typename T>
static inline void bitVectorClear(T* buf, size_t index, size_t count) noexcept { Internal::bitVectorOp<T, AndNot, SetNot>(buf, index, count); }

//! Iterates over each bit in a number which is set to 1.
//!
//! Example of use:
//!
//! ```
//! uint32_t bitsToIterate = 0x110F;
//! IntUtils::BitWordIterator<uint32_t> it(bitsToIterate);
//!
//! while (it.hasNext()) {
//!   uint32_t bitIndex = it.next();
//!   std::printf("Bit at %u is set\n", unsigned(bitIndex));
//! }
//! ```
template<typename T>
class BitWordIterator {
public:
  explicit constexpr BitWordIterator(T bitWord) noexcept
    : _bitWord(bitWord) {}

  ALWAYS_INLINE void init(T bitWord) noexcept { _bitWord = bitWord; }
  ALWAYS_INLINE bool hasNext() const noexcept { return _bitWord != 0; }

  ALWAYS_INLINE uint32_t next() noexcept {
    assert(_bitWord != 0);
    uint32_t index = ctz(_bitWord);
    _bitWord ^= T(1U) << index;
    return index;
  }

  T _bitWord;
};

template<typename T>
class BitWordFlipIterator {
public:
  ALWAYS_INLINE BitWordFlipIterator(T bitWord, T xorMask = 0) noexcept {
    init(bitWord, xorMask);
  }

  ALWAYS_INLINE void init(T bitWord, T xorMask = 0) noexcept {
    _bitWord = bitWord ^ xorMask;
    _xorMask = xorMask;
  }

  ALWAYS_INLINE bool hasNext() const noexcept {
    return _bitWord != T(0);
  }

  ALWAYS_INLINE size_t next() noexcept {
    T bitWord = _bitWord;
    assert(bitWord != T(0));

    uint32_t index = ctz(bitWord);
    bitWord ^= T(1U) << index;

    _bitWord = bitWord;
    return index;
  }

  ALWAYS_INLINE size_t nextAndFlip() noexcept {
    T bitWord = _bitWord;
    assert(bitWord != T(0));

    uint32_t index = ctz(bitWord);
    bitWord ^= allOnes<T>() << index;
    _xorMask ^= allOnes<T>();

    _bitWord = bitWord;
    return index;
  }

  ALWAYS_INLINE size_t peekNext() const noexcept {
    return ctz(_bitWord);
  }

  T _bitWord;
  T _xorMask;
};

template<typename T>
class BitVectorIterator {
public:
  ALWAYS_INLINE BitVectorIterator(const T* data, size_t numBitWords, size_t start = 0) noexcept {
    init(data, numBitWords, start);
  }

  ALWAYS_INLINE void init(const T* data, size_t numBitWords, size_t start = 0) noexcept {
    const T* ptr = data + (start / bitSizeOf<T>());
    size_t idx = alignDown(start, bitSizeOf<T>());
    size_t end = numBitWords * bitSizeOf<T>();

    T bitWord = T(0);
    if (idx < end) {
      bitWord = *ptr++ & (allOnes<T>() << (start % bitSizeOf<T>()));
      while (!bitWord && (idx += bitSizeOf<T>()) < end)
        bitWord = *ptr++;
    }

    _ptr = ptr;
    _idx = idx;
    _end = end;
    _current = bitWord;
  }

  ALWAYS_INLINE bool hasNext() const noexcept {
    return _current != T(0);
  }

  ALWAYS_INLINE size_t next() noexcept {
    T bitWord = _current;
    assert(bitWord != T(0));

    uint32_t bit = ctz(bitWord);
    bitWord ^= T(1U) << bit;

    size_t n = _idx + bit;
    while (!bitWord && (_idx += bitSizeOf<T>()) < _end)
      bitWord = *_ptr++;

    _current = bitWord;
    return n;
  }

  ALWAYS_INLINE size_t peekNext() const noexcept {
    assert(_current != T(0));
    return _idx + ctz(_current);
  }

  const T* _ptr;
  size_t _idx;
  size_t _end;
  T _current;
};

template<typename T>
class BitVectorFlipIterator {
public:
  ALWAYS_INLINE BitVectorFlipIterator(const T* data, size_t numBitWords, size_t start = 0, T xorMask = 0) noexcept {
    init(data, numBitWords, start, xorMask);
  }

  ALWAYS_INLINE void init(const T* data, size_t numBitWords, size_t start = 0, T xorMask = 0) noexcept {
    const T* ptr = data + (start / bitSizeOf<T>());
    size_t idx = alignDown(start, bitSizeOf<T>());
    size_t end = numBitWords * bitSizeOf<T>();

    T bitWord = T(0);
    if (idx < end) {
      bitWord = (*ptr++ ^ xorMask) & (allOnes<T>() << (start % bitSizeOf<T>()));
      while (!bitWord && (idx += bitSizeOf<T>()) < end)
        bitWord = *ptr++ ^ xorMask;
    }

    _ptr = ptr;
    _idx = idx;
    _end = end;
    _current = bitWord;
    _xorMask = xorMask;
  }

  ALWAYS_INLINE bool hasNext() const noexcept {
    return _current != T(0);
  }

  ALWAYS_INLINE size_t next() noexcept {
    T bitWord = _current;
    assert(bitWord != T(0));

    uint32_t bit = ctz(bitWord);
    bitWord ^= T(1U) << bit;

    size_t n = _idx + bit;
    while (!bitWord && (_idx += bitSizeOf<T>()) < _end)
      bitWord = *_ptr++ ^ _xorMask;

    _current = bitWord;
    return n;
  }

  ALWAYS_INLINE size_t nextAndFlip() noexcept {
    T bitWord = _current;
    assert(bitWord != T(0));

    uint32_t bit = ctz(bitWord);
    bitWord ^= allOnes<T>() << bit;
    _xorMask ^= allOnes<T>();

    size_t n = _idx + bit;
    while (!bitWord && (_idx += bitSizeOf<T>()) < _end)
      bitWord = *_ptr++ ^ _xorMask;

    _current = bitWord;
    return n;
  }

  ALWAYS_INLINE size_t peekNext() const noexcept {
    assert(_current != T(0));
    return _idx + ctz(_current);
  }

  const T* _ptr;
  size_t _idx;
  size_t _end;
  T _current;
  T _xorMask;
};

} // IntUtils namespace

#endif // _INTUTILS_H
