#include <pebble.h>

static void raleway_init() {

}

static void raleway_deinit() {

}

int main(void) {
  raleway_init();
  app_event_loop();
  raleway_deinit();
}