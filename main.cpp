// Blend2D Reference Rasterizer - Public Domain
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "./b2drefras.h"

#pragma pack(push, 1)
struct BmpFile {
  void init(int w, int h) {
    ::memset(this, 0, sizeof(BmpFile));

    signature[0]    = 'B';
    signature[1]    = 'M';

    headerSize      = 40; // Version 1.
    width           = w;
    height          = h;
    planes          = 1;
    bitsPerPixel    = 8;
    compression     = 0;
    imageSize       = w * h;
    colorsUsed      = 256;
    colorsImportant = 256;

    imageOffset     = 14 + headerSize + 4 * 256;
    fileSize        = imageOffset + imageSize;

    for (int i = 0; i < 256; i++) {
      pal[i].r = static_cast<uint8_t>(i);
      pal[i].g = static_cast<uint8_t>(i);
      pal[i].b = static_cast<uint8_t>(i);
    }
  }

  struct PaletteEntry {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t x;
  };

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
  PaletteEntry pal[256];
};
#pragma pack(pop)

class CmdLine {
public:
  CmdLine(int argc, const char* const* argv)
    : argc(argc),
      argv(argv) {}

  bool hasKey(const char* key) const {
    size_t size = ::strlen(key);
    for (int i = 0; i < argc; i++)
      if (::strlen(argv[i]) >= size && ::memcmp(argv[i], key, size) == 0)
        return true;
    return false;
  }

  const char* getKey(const char* key) const {
    size_t keyLen = ::strlen(key);
    size_t argLen = 0;

    const char* arg = NULL;
    for (int i = 0; i <= argc; i++) {
      if (i == argc)
        return NULL;

      arg = argv[i];
      argLen = ::strlen(arg);
      if (argLen >= keyLen && ::memcmp(arg, key, keyLen) == 0)
        break;
    }

    if (argLen > keyLen && arg[keyLen] == '=')
      return arg + keyLen + 1;
    else
      return arg + keyLen;
  }

  int getInt(const char* key) const {
    const char* value = getKey(key);
    if (!value) return 0;
    return atoi(value);
  }

  int argc;
  const char* const* argv;
};

int main(int argc, char* argv[]) {
  CmdLine cmd(argc, argv);
  int i;

  if (cmd.hasKey("--help") ||
     !cmd.hasKey("--width") ||
     !cmd.hasKey("--height") ||
     !cmd.hasKey("--output")) {
    printf("Usage: aggrefras_test --width=W --height=H --output=file.bmp X Y X Y X Y [...]\n");
    return 1;
  }

  int w = cmd.getInt("--width");
  int h = cmd.getInt("--height");
  bool nonZero = !cmd.hasKey("--even-odd");
  const char* fileName = cmd.getKey("--output");

  // Rasterize the polygon.
  B2DRefRas ras;
  ras.init(w, h);

  double line[4];
  double start[2];
  int index = 0;

  for (i = 1; i < argc; i++) {
    const char* value = argv[i];
    size_t len = ::strlen(value);

    if (value[0] == '-' && value[1] == '-')
      continue;
    line[index++] = atof(value);

    // Initial point, required to close the polygon.
    if (index == 2) {
      double x0 = line[0];
      double y0 = line[1];

      start[0] = x0;
      start[1] = y0;

      if (x0 < 0 || y0 < 0 || x0 > w || y0 > h) {
        printf("Coordinates out of range\n");
        return 1;
      }
    }

    if (index == 4) {
      double x0 = line[0];
      double y0 = line[1];
      double x1 = line[2];
      double y1 = line[3];

      if (x1 < 0 || y1 < 0 || x1 > w || y1 > h) {
        printf("Coordinates out of range\n");
        return 1;
      }

      // Add line (converted to fixed point).
      ras.addLine(static_cast<int>(x0 * 256),
                  static_cast<int>(y0 * 256),
                  static_cast<int>(x1 * 256),
                  static_cast<int>(y1 * 256));

      line[0] = x1;
      line[1] = y1;
      index = 2;
    }
  }

  // Close the polygon
  if (index >= 2) {
      double x0 = line[0];
      double y0 = line[1];
      double x1 = start[0];
      double y1 = start[1];

      // Add line (converted to fixed point).
      ras.addLine(static_cast<int>(x0 * 256),
                  static_cast<int>(y0 * 256),
                  static_cast<int>(x1 * 256),
                  static_cast<int>(y1 * 256));
  }

  // Store the bitmap.
  BmpFile bmp;
  bmp.init(w, h);

  FILE* f = fopen(fileName, "wb");
  if (!f) {
    printf("Cannot open file '%s' for write\n", fileName);
    return 0;
  }

  fwrite(&bmp.signature, sizeof(BmpFile) - 2, 1, f);

  uint8_t* scanline = static_cast<uint8_t*>(::malloc(w));
  for (i = 0; i < h; i++) {
    ras.sweepScanline((h - i - 1), nonZero, scanline);
    fwrite(scanline, w, 1, f);
  }
  ::free(scanline);

  fclose(f);
  return 0;
}
