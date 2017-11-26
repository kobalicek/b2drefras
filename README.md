Rasterizer Experiments
----------------------

This is a playground repository that I use to compare various implementations of 'cell' based rasterizers. All rasterizers implemented here use the same algorithm for rasterization, but use different ways of storing rasterized cells. The rasterizer itself contains an optimized **renderLine()** implementation that differs compared to FreeType/AGG rasterizer in the following:

  * It's optimized to use less division than an original AGG/FreeType implementation.
  * It's optimized to not use call to **renderHLine()** compared to AGG/FreeType.
  * It doesn't produce different cells for ascending and descending lines, if you rasterize the same shape twice, but each having a different direction, nothing will be drawn (all cells will be zero).

The following rasterizers are provided

  * `RasterizerA1` - The simplest possible rasterizer. Uses W*H cell matrix and iterates all cells during `render()`. The idea behind this rasterizer is to provide the most basic implementation that should be used as a reference.
  * `RasterizerA2` - Similar to `RasterizerA1`, but uses a global `[yMin..yMax]` boundary per rasterizer and `[xMin..xMax]` boundary per scanline. This allows to only focus on cells where actually some rendering happened and to quickly skip cells that are outside of the rendered shape.

The rasterizer can be used as a library and contains also a test application that can accept vertices as command line arguments and stores 8-bit BMPs.

Simple usage of the test application:

```bash
$ ./render_cmd --output=render_cmd.bmp --width=256 --height=256 10 10 240 80 120 240
```

The resulting render_cmd.bmp image:

![Output](/render_cmd.bmp)
