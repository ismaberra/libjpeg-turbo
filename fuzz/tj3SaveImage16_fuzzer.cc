#include <turbojpeg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Fuzz entry point for libFuzzer
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Need at least 5 bytes for width, height, and pixel format
  if (size < 5) return 0;

  // Read width and height (little-endian, 2 bytes each)
  uint16_t width = data[0] | (data[1] << 8);
  uint16_t height = data[2] | (data[3] << 8);

  // Clamp width and height to small values to avoid excessive memory usage
  width = width % 32 + 1;
  height = height % 32 + 1;

  // Read pixel format byte and map to a valid TJPF_* constant
  uint8_t raw_format = data[4];
  const int format_map[] = { TJPF_RGB, TJPF_GRAY, TJPF_BGR, TJPF_RGBX };
  int pixelFormat = format_map[raw_format % (sizeof(format_map) / sizeof(int))];
  int channels = tjPixelSize[pixelFormat];  // Number of channels per pixel

  // Calculate pitch (number of bytes per row)
  size_t pitch = width * channels * sizeof(short);
  size_t required_size = 5 + pitch * height;

  // If not enough input data to fill the image buffer, exit
  if (size < required_size) return 0;

  // Interpret the remaining data as a 16-bit pixel buffer
  const unsigned short *buffer = (const unsigned short *)(data + 5);

  // Initialize the TurboJPEG compression handle
  tjhandle handle = tj3Init(TJINIT_COMPRESS);
  if (!handle) return 0;

  // Attempt to save the image to a dummy path
  tj3SaveImage16(handle, "/dev/null", buffer, width, pitch, height, pixelFormat);

  // Clean up
  tj3Destroy(handle);
  return 0;
}
