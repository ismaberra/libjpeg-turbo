#include <turbojpeg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size < 5) return 0;  // Besoin d'au moins 5 octets pour width, height, format

  // Lire les dimensions (petit-endian)
  uint16_t width = data[0] | (data[1] << 8);
  uint16_t height = data[2] | (data[3] << 8);

  // Limiter la taille (éviter les crashs/allocs géantes)
  width = width % 512 + 1;
  height = height % 512 + 1;

  // Lire et filtrer le pixelFormat
  uint8_t raw_format = data[4];
  const int format_map[] = { TJPF_RGB, TJPF_GRAY, TJPF_BGR, TJPF_RGBX };
  int pixelFormat = format_map[raw_format % (sizeof(format_map) / sizeof(int))];
  int channels = tjPixelSize[pixelFormat];

  // Calcul du pitch et vérification du buffer
  size_t pitch = width * channels * sizeof(short);
  size_t required_size = 5 + pitch * height;
  if (size < required_size) return 0;

  const short *buffer = (const short *)(data + 5);

  // Créer le contexte TurboJPEG
  tjhandle handle = tj3Init(TJINIT_COMPRESS);
  if (!handle) return 0;

  // Appel principal
  tj3SaveImage16(handle, "/dev/null", buffer, width, pitch, height, pixelFormat);

  tj3Destroy(handle);
  return 0;
}
