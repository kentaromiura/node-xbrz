#include <node.h>

#define cimg_use_png
#define cimg_display 0
#include "CImg/CImg.h"
#include "xbrz/xbrz.h"
#include <sstream>

using namespace cimg_library;

namespace demo {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

template <uint32_t N> inline
unsigned char getByte(uint32_t val) { return static_cast<unsigned char>((val >> (8 * N)) & 0xff); }

bool validScale(const char* scale) {
  bool output = false;
  if (0 == std::strcmp(scale, "1.5")) output = true;
  if (0 == std::strcmp(scale, "2")) output = true;
  if (0 == std::strcmp(scale, "3")) output = true;
  if (0 == std::strcmp(scale, "4")) output = true;
  if (0 == std::strcmp(scale, "5")) output = true;
  if (0 == std::strcmp(scale, "6")) output = true;
  return output;
}

int getScale(const char* scale) {
  int output = 3;
  if (0 == std::strcmp(scale, "2")) output = 2;
  if (0 == std::strcmp(scale, "4")) output = 4;
  if (0 == std::strcmp(scale, "5")) output = 5;
  if (0 == std::strcmp(scale, "6")) output = 6;
  return output;
}

bool wantHalf(const char* scale) {
  return (0 == std::strcmp(scale, "1.5"));
}

void resize(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 3) {
    args.GetIsolate()->ThrowException(
      v8::String::NewFromUtf8(
        isolate,
        "xbrz.resize(filename, outputfilename, scale) requires 3 parameter"
      )
    );
    return;
  }

  v8::String::Utf8Value utfFilename(args[0]);
  const char* filename = *utfFilename;
  v8::String::Utf8Value utfOutputFN(args[1]);
  const char* outputfilename = *utfOutputFN;
  v8::String::Utf8Value utfRS(args[2]);
  const char* requestedScale = *utfRS;

  if (!validScale(requestedScale)) {
    args.GetIsolate()->ThrowException(
      v8::String::NewFromUtf8(isolate, "Scale must be 1.5, 2, 3, 4, 5 or 6")
    );
    return;
  }
  CImg<uint32_t> image(filename);

  int scale = getScale(requestedScale);
  int width = image.width();
  int height = image.height();
  int depth = image.depth();
  int spectrum = image.spectrum();

  bool isRGBA = spectrum == 4;
  bool doHalf = wantHalf(requestedScale);

  uint32_t* p_raw = new uint32_t[width * height];
  // RGBA to ARGB
  for(uint32_t y = 0; y < (uint32_t)width; y++) {
    for(uint32_t x = 0; x < (uint32_t)height; x++) {
      uint32_t r = image(y, x, 0, 0);
      uint32_t g = image(y, x, 0, 1);
      uint32_t b = image(y, x, 0, 2);
      uint32_t a = 0;
      if (isRGBA) a = image(y, x, 0, 3);
      p_raw[x*width + y] = (a << 24) | (r << 16) | (g << 8) | b;
    }
  }

  uint32_t* p_output = new uint32_t[scale * scale * height * width];

  xbrz::scale(
    scale,
    p_raw,
    p_output,
    width,
    height,
    isRGBA? xbrz::ColorFormat::ARGB : xbrz::ColorFormat::RGB
  );

  delete[] p_raw;

  CImg<uint32_t> output(width * scale, height * scale, depth, spectrum);

  // ARGB to RGBA
  for(uint32_t y = 0; y < (uint32_t)output.width(); y++) {
    for(uint32_t x = 0; x < (uint32_t)output.height(); x++) {
      uint32_t pixel = p_output[x * width * scale +  y];

      /* R */ output(y, x, 0) = getByte<2>(pixel);
      /* G */ output(y, x, 1) = getByte<1>(pixel);
      /* B */ output(y, x, 2) = getByte<0>(pixel);
      /* A */ if (isRGBA) output(y, x, 3) = getByte<3>(pixel);
    }
  }
  delete[] p_output;

  if (doHalf) output.resize_halfXY();

  output.save(outputfilename);

  args.GetReturnValue().Set(
    v8::Undefined(isolate)
  );
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "resize", resize);
}

NODE_MODULE(addon, init)

}  // namespace demo
