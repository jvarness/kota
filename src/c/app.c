#include "fonts.h"
#include "shapes.h"

static Window* window;
static GFont valera_small;
static GFont valera_large;
static TextLayer* time_layer;
static TextLayer* date_layer;
static TextLayer* day_of_week_layer;
static Layer* battery_layer;
static GPath* battery_path;
static int battery_percent;

static void update_battery_ind(Layer *layer, GContext *ctx) {
  GRect battery_rect = layer_get_bounds(battery_layer);
  int fill_width = (battery_rect.size.w - 17);
  int percentage_width = (battery_percent / 100.0) * fill_width;
  graphics_fill_rect(ctx, GRect(battery_rect.origin.x + 6, battery_rect.origin.y + 6, percentage_width, battery_rect.size.h - 11), 0, GCornerNone);
  graphics_fill_rect(ctx, GRect(battery_rect.origin.x + 44, battery_rect.origin.y + 6, 4, 13), 0, GCornerNone);
  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 3);
  gpath_draw_outline(ctx, battery_path);
}

static void update_time() {
  static char time_buffer[10];
  static char date_buffer[10];
  static char day_buffer[10];
  time_t now = time(NULL);
  struct tm* local = localtime(&now);
  
  strftime(time_buffer, sizeof(time_buffer), clock_is_24h_style() ? "%k:%M" : "%l:%M%P", local);
  strftime(date_buffer, sizeof(date_buffer), "%b %e", local);
  strftime(day_buffer, sizeof(day_buffer), "%A", local);
  
  text_layer_set_text(time_layer, time_buffer);
  text_layer_set_text(date_layer, date_buffer);
  text_layer_set_text(day_of_week_layer, day_buffer);
}

static void update_battery(BatteryChargeState state) {  
  battery_percent = state.charge_percent;
  layer_mark_dirty(battery_layer);
}

static void tick_handler(struct tm* time_changed, TimeUnits units_changed) {
  update_time();
}

static void battery_handler(BatteryChargeState state) {
  update_battery(state);
}

static void kota_window_load() {
  Layer* win_layer = window_get_root_layer(window);
  GRect dimensions = layer_get_bounds(win_layer);
  
  time_layer = text_layer_create(GRect(0, dimensions.size.h / 2 - 20, dimensions.size.w, 44));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorBlack);
  text_layer_set_font(time_layer, valera_large);
  
  date_layer = text_layer_create(GRect(0, dimensions.size.h / 2 - 64, dimensions.size.w, 30));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorBlack);
  text_layer_set_font(date_layer, valera_small);
  
  day_of_week_layer = text_layer_create(GRect(0, dimensions.size.h / 2 - 44, dimensions.size.w, 30));
  text_layer_set_text_alignment(day_of_week_layer, GTextAlignmentCenter);
  text_layer_set_background_color(day_of_week_layer, GColorClear);
  text_layer_set_text_color(day_of_week_layer, GColorBlack);
  text_layer_set_font(day_of_week_layer, valera_small);
  
  battery_layer = layer_create(GRect(dimensions.size.w / 2 - 26, dimensions.size.h / 2 + 29, 50, 24));
  layer_set_update_proc(battery_layer, update_battery_ind);
  
  layer_add_child(win_layer, text_layer_get_layer(time_layer));
  layer_add_child(win_layer, text_layer_get_layer(date_layer));
  layer_add_child(win_layer, text_layer_get_layer(day_of_week_layer));
  layer_add_child(win_layer, battery_layer);
  
  update_time();
  update_battery(battery_state_service_peek());
}

static void setup_battery_path() {
  battery_path = gpath_create(&BATTERY_SQUARE);
}

static void kota_window_unload() {
  gpath_destroy(battery_path);
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(day_of_week_layer);
  fonts_unload_custom_font(valera_large);
  fonts_unload_custom_font(valera_small);
  layer_destroy(battery_layer);
}

static void kota_init() {
  valera_large = get_font_large();
  valera_small = get_font_small();

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = kota_window_load,
    .unload = kota_window_unload
  });
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(battery_handler);
  
  setup_battery_path();

  window_set_background_color(window, GColorWhite);
  window_stack_push(window, false);
}

static void kota_deinit() {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  kota_init();
  app_event_loop();
  kota_deinit();
}