# ArtificialMorph

This is a research codebase for "Artificial morphogenesis of curved surface stuctures inspired by differential growth in biology".
Given a triangular mesh of a target surface, the tool computes and exports the corresponding layout of nonshrinking elements (STL) to be 3D-printed on a heat-shrink film. Upon thermal activation, the sheet transforms from planar to the prescribed curved shape.

Any work which utilizes this code shall include the following reference:

Morikawa, K., Nakamura, T., Matsumoto, Y., Matsuda, K., Akiyama, M., Yamasaki, S., Kondo, S. & Inoue, Y.

Artificial morphogenesis of curved surface structures inspired by differential growth in biology.

Journal of the Royal Society Interface (2026).

https://doi.org/10.1098/rsif.2025.1094 .

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| [Eigen](https://eigen.tuxfamily.org/) | 3.x | Linear algebra |
| [libigl](https://libigl.github.io/) | 2.x | Mesh I/O and geometry processing |
| C++17 compiler (g++) | — | Build |

## Build

### 1. Install dependencies

Install Eigen and libigl header libraries. For example, on Ubuntu/Debian:

```bash
sudo apt install libeigen3-dev
```

For libigl, download the header-only library from https://github.com/libigl/libigl and note the path to the `include` directory.

### 2. Compile the main code

```bash
cd ArtificialMorph
make LIBIGL_INCLUDE=/path/to/libigl/include
```

You can also set `EIGEN_INCLUDE` if Eigen is not installed in `/usr/include/eigen3`:

```bash
make EIGEN_INCLUDE=/path/to/eigen3 LIBIGL_INCLUDE=/path/to/libigl/include
```

### 3. Compile the evaluation code

```bash
cd Data/3dScan/hemisphere_shape_eval
make LIBIGL_INCLUDE=/path/to/libigl/include

cd Data/3dScan/various_shape_eval
make LIBIGL_INCLUDE=/path/to/libigl/include
```

## Usage

### Generating a 3D-printable pattern

```bash
cd ArtificialMorph/hemisphere_100
../a.out
```

The program reads `input/input.txt` and produces:

| Output file | Description |
|-------------|-------------|
| `result_triprism.stl` | 3D model of triangular prisms for printing |
| `target_3d_shape.off` | Scaled target 3D shape |
| `param_and_triangles.off` | 2D parameterization with triangle layout |
| `param.off` | 2D parameterization |

### Input configuration (`input/input.txt`)

```
InputFileName3d       hemisphere_100      # Target 3D shape
InputFileFormat3d     off
Input2dShapeMode      1                   # 0: auto-generate via harmonic parameterization
                                          # 1: use provided 2D shape
InputFileName2d       hemisphere_100_param
InputFileFormat2d     off
FilmShrinkRate        0.35                # Length-based shrinkage ratio of the film
PrintWidth            150                 # Maximum width (mm)
Height                1                   # Print thickness (mm)
AreaOfTargetShape     7000                # Target area of the final shape (mm²)
End
```

## File Formats

- **OFF** (Object File Format): Triangle meshes for target shapes, parameterizations, and scanned data
- **STL**: Output 3D model for 3D printing
- **VTK**: Visualization files with scalar fields (shape evaluation)
- **PLY**: Visualization meshes with per-vertex scalar coloring

## License
- MIT License
