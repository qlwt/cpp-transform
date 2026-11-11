# Matrix Image Transformations

## Overview

An exploration of affine matrix transformations, allowing you to visualize chained transformations over time.

## Installation

```bash
git clone <repository>
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release
cmake --build build/release
```

## Modes

- **Batch Mode** - applies the same transformations to every pixel. Standard image transformations.
- **Precise Mode** - calculates transformations per pixel, enabling pixel-dependent effects at a substantial performance cost.

## Usage

```bash
main {out-dir} {framecount}/{framerate} {width}x{height} [--precise] [flags...]
```

The source image is a procedurally generated repeating pattern. Transformations are centered by default.

### Batch Mode

```bash
build/release/main .frames 240/24 720x480 \
  --rotate "pi / 2 * time" \
  --translate "30 * time"
```

Only global variables are available in expressions: `width`, `height`, and mathematical constants.

### Precise Mode

`--precise` flag must come before transformations

```bash
build/release/main .frames 240/24 720x480 --precise \
  --scale "decay" \
  --rotate "pi / 2 * time * decay^2"
```

Calculates transformations individually for each pixel, allowing pixel-dependent expressions. much slower, use smaller resolutions.

Available variables:

- `px_x`, `px_y` - pixel coordinates
- `origin_x`, `origin_y` - transformation origin
- `decay` - exponential distance-based falloff (`1.0` at the origin, `0.2` at the edges)

### Transformation Flags

| Flag | Argument | Description |
|---|---|---|
| `--rotate` | `radian` | Rotation |
| `--translate[-x\|y]` | `px` | Translation |
| `--mirror` | `radian` | Directional mirror |
| `--mirror[-x\|y]` | - | Axis-aligned mirror |
| `--skew[-x\|y]` | `ratio` | Skew |
| `--scale[-x\|y]` | `ratio` | Scaling |

### Exporting Video

Generated frames can be combined using [FFmpeg](https://ffmpeg.org/):

```bash
ffmpeg -framerate 24 -i .frames/frame_%04d.bmp \
  -c:v libx264 -pix_fmt yuv420p .out/video.mp4
```

## Project Structure

| Directory | Description |
|---|---|
| `vendor/sxm/` | Vendored [cpp-sxm](https://github.com/qlwt/cpp-sxm) math expression library |
| `tr/` | Transformation logic |
| `tr/pattern` | Transformation happens here. Source pattern is also defined here |
