#include "./globals.h"
#include "./performance.h"
#include "./rasterizer.h"

// ============================================================================
// [Main]
// ============================================================================

struct BenchParams {
  int w;
  int h;
  double factor;
};

int main(int argc, char* argv[]) {
  static const BenchParams benchParams[] = {
    { 16  , 16   , 512.0 },
    { 32  , 32   , 256.0 },
    { 64  , 64   , 128.0 },
    { 128 , 128  , 64.0  },
    { 256 , 256  , 32.0  },
    { 512 , 512  , 16.0  },
    { 1024, 768  , 8.0   },
    { 1920, 1080 , 4.0   },
    { 3840, 2160 , 1.0   }
  };

  uint32_t baseQuantity = 100;
  uint32_t numRepeats = 3;
  uint32_t numPoints = 5;

  for (uint32_t benchId = 0; benchId < uint32_t(ARRAY_SIZE(benchParams)); benchId++) {
    for (uint32_t rasterizerId = 0; rasterizerId < Rasterizer::kIdCount; rasterizerId++) {
      const BenchParams& params = benchParams[benchId];

      Image image;
      Random rnd;
      Point poly[128];

      image.create(params.w, params.h);
      Rasterizer* ras = Rasterizer::newById(rasterizerId);
      ras->init(params.w, params.h);

      double dw = double(params.w - 1);
      double dh = double(params.h - 1);
      uint32_t quantity = uint32_t(double(baseQuantity) * params.factor);

      Performance perf;

      for (uint32_t repeatIndex = 0; repeatIndex < numRepeats; repeatIndex++) {
        rnd.rewind();
        image.fillAll(0x00000000);

        perf.start();
        for (uint32_t i = 0; i < quantity; i++) {
          uint32_t argb32 = rnd.nextUInt32();

          for (uint32_t j = 0; j < numPoints; j++) {
            poly[j].x = rnd.nextDouble() * dw;
            poly[j].y = rnd.nextDouble() * dh;
          }

          poly[numPoints] = poly[0];
          ras->addPoly(poly, numPoints + 1);
          ras->render(image, argb32);
          ras->clear();
        }
        perf.end();
      }

      char fileName[128];
      std::snprintf(fileName, ARRAY_SIZE(fileName), "Bench_%04dx%04d-%s.bmp", image.width(), image.height(), ras->name());
      delete ras;

      if (!image.writeBmp(fileName)) {
        printf("Cannot open file '%s' for writing\n", fileName);
        return 1;
      }

      printf("%-26s [q=%-6u] [%-4u ms]\n", fileName, quantity, perf.best);
    }
    printf("\n");
  }

  return 0;
}
