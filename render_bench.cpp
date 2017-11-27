#include "./globals.h"
#include "./performance.h"
#include "./rasterizer.h"

// ============================================================================
// [Main]
// ============================================================================

struct BenchParams {
  int w;
  int h;
};

int main(int argc, char* argv[]) {
  static const BenchParams benchParams[] = {
    { 16  , 16   },
    { 32  , 32   },
    { 64  , 64   },
    { 128 , 128  },
    { 256 , 256  },
    { 512 , 512  },
    { 1024, 768  },
    { 1920, 1080 },
    { 3840, 2160 }
  };

  uint32_t quantity = 400;
  uint32_t numRepeats = 3;
  uint32_t numPoints = 10;

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

      printf("%-26s [%u ms]\n", fileName, perf.best);
    }
  }

  return 0;
}
