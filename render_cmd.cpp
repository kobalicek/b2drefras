#include "./globals.h"
#include "./rasterizer.h"

// ============================================================================
// [CmdLine]
// ============================================================================

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

  const char* valueOf(const char* key) const {
    size_t keySize = ::strlen(key);
    size_t argSize = 0;

    const char* arg = nullptr;
    for (int i = 0; i <= argc; i++) {
      if (i == argc)
        return nullptr;

      arg = argv[i];
      argSize = ::strlen(arg);
      if (argSize >= keySize && ::memcmp(arg, key, keySize) == 0)
        break;
    }

    if (argSize > keySize && arg[keySize] == '=')
      return arg + keySize + 1;
    else
      return arg + keySize;
  }

  int intValueOf(const char* key) const {
    const char* value = valueOf(key);
    if (!value) return 0;
    return atoi(value);
  }

  int argc;
  const char* const* argv;
};

// ============================================================================
// [Main]
// ============================================================================

int main(int argc, char* argv[]) {
  CmdLine cmd(argc, argv);
  int i;

  if (cmd.hasKey("--help") ||
     !cmd.hasKey("--width") ||
     !cmd.hasKey("--height") ||
     !cmd.hasKey("--output")) {
    printf("Usage: b2drefras --width=W --height=H --output=file.bmp X Y X Y X Y [...]\n");
    return 1;
  }

  int w = cmd.intValueOf("--width");
  int h = cmd.intValueOf("--height");
  bool nonZero = !cmd.hasKey("--even-odd");
  const char* fileName = cmd.valueOf("--output");

  uint32_t color = 0xFFFFFFFF;

  Image image;
  image.create(w, h);
  image.fillAll(0);

  // Rasterize the polygon.
  Rasterizer* ras = Rasterizer::newById(image, Rasterizer::kIdA1, 0);
  ras->setFillMode(nonZero ? Rasterizer::kFillNonZero : Rasterizer::kFillEvenOdd);

  double line[4];
  double start[2];
  int index = 0;

  for (i = 1; i < argc; i++) {
    const char* value = argv[i];
    if (value[0] == '-' && value[1] == '-')
      continue;

    // Initial point, required to close the polygon.
    line[index++] = atof(value);
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

      Point poly[] = { { x0, y0 }, { x1, y1 } };
      ras->addPoly(poly, 2);

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

    Point poly[] = { { x0, y0 }, { x1, y1 } };
    ras->addPoly(poly, 2);
  }

  ras->render(color);
  delete ras;

  if (!image.writeBmp(fileName)) {
    printf("Cannot open file '%s' for writing\n", fileName);
    return 1;
  }

  return 0;
}
