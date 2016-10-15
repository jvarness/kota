#include "fonts.h"

static GFont get_font(ResHandle handle) {
  return fonts_load_custom_font(handle);
}

GFont get_font_large() {
  return get_font(resource_get_handle(RESOURCE_ID_VALERA_ROUND_36));
}

GFont get_font_small() {
  return get_font(resource_get_handle(RESOURCE_ID_VALERA_ROUND_24));
}