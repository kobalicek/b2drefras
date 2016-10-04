Reference Rasterizer
--------------------

This is a simplified implementation of Blend2D rasterizer that doesn't use spans or anything else to store **Cell** data. The rasterizer uses a 2D array to store all cells instead, which simplifies the implementation and makes it easy to compare with optimized rasterizers.

The rasterizer itself contains an optimized **renderLine()** implementation that differs compared to FreeType/AGG rasterizer. It contains these improvements:

  * It's optimized to use less division than an original AGG/FreeType implementation.
  * It's optimized to not use call to **renderHLine()** compared to AGG/FreeType.
  * It doesn't produce different cells for ascending and descending lines, if you rasterize the same shape twice, but each having a different direction, nothing will be drawn (all cells will be zero).

The rasterizer can be used as a library and contains also a test application that can accept vertices as command line arguments and stores 8-bit BMPs.

Simple usage of the test application:

```bash
$ ./b2drefras --output=b2drefras.bmp --width=256 --height=256 10 10 240 80 120 240
```

The resulting b2drefras.bmp image:

![Output](/b2drefras.bmp)
