# KTX-creator

KTX-creator can help you to compressing and packing images into a [KTX](https://www.khronos.org/opengles/sdk/tools/KTX/file_format_spec/) texture.
It supports automatic mipmaps generation and [Adaptive Scalable Texture Compression](https://en.wikipedia.org/wiki/Adaptive_Scalable_Texture_Compression) (ASTC).

### Submodules

You just need to initialize them.

```bash
git submodule init
git submodule update
```

#### ASTC encoder
ASTC encoding library [[github](https://github.com/ARM-software/astc-encoder)]

#### LibKTX
Khronos KTX library [[github](https://github.com/KhronosGroup/KTX-Software)]

#### ImageMagick
ImageMagick library with CMake support [[github](https://github.com/MarcoMartins86/ImageMagick)]

#### Catch2
Test framework for unit-tests [[github](https://github.com/catchorg/Catch2)]

## Build

```bash
cmake -H. -Bbuild
cmake --build build
```

## Usage

You can pack an image into a KTX (`background.ktx`) file by running:

```bash
ktx-creator background.png
```

### Compression

You can specify the compression you want to apply on the image before packing it into a KTX file specifying `-c <compression>`. The following command will compress the png image using ASTC for you.

```bash
ktx-creator -c astc background.png
```

### Mipmaps

You can tell ktx-creator to generate mipmaps by passing `-mipmaps` on the command line interface. This command, for example, will generate the mipmaps, will compress them, and will pack them into a KTX file.

```bash
ktx-creator -mipmaps -c astc background.png
```

## License

See [LICENSE](LICENSE).

This project has some third-party dependencies, each of which may have independent licensing:

- [astc-encoder](https://github.com/ARM-software/astc-encoder): ASTC Evaluation Codec
- [KTX-Software](https://github.com/KhronosGroup/KTX-Software): Khronos Texture Library and Tools
- [catch2](https://github.com/catchorg/Catch2): Testing framework
- [stb](https://github.com/nothings/stb): Single-file public domain (or MIT licensed) libraries
- [ImageMagick](https://github.com/MarcoMartins86/ImageMagick) ImageMagick library
