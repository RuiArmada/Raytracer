#include <cstdio>
#include <fmt/color.h>
#include <fmt/core.h>
#include <image.hpp>

#include <OpenImageIO/imageio.h>

using namespace OIIO;

Image::Image(uint32_t width, uint32_t height) : width{width}, height{height} {
  imageData.resize(width * height * 3);
}

bool Image::writePNG(const std::string &filename) {
  std::unique_ptr<ImageOutput> out = ImageOutput::create(filename);
  if (!out)
    return false;
  ImageSpec spec(width, height, 3, TypeDesc::HALF);
  out->open(filename, spec);
  out->write_image(TypeDesc::FLOAT, imageData.data());
  out->close();

  return true;
}
