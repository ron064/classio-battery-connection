#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_battery_layer;
static TextLayer *s_connection_layer;

static Window *s_sec_window;
bool SecLoaded = false;
static TextLayer *s_sec_layer;

static void update_only_sec(){
  if (!SecLoaded){
    window_stack_push(s_sec_window, false);
    SecLoaded=true;  
    //optionally move frequently updated layer to frequently updated window.
  }
}
static void update_all(){
  if (SecLoaded){
    window_stack_pop(false);
    SecLoaded=false;
    //optionally move frequently updated layer to main window
  }
}
void sec_window_update(struct Layer *layer, GContext *ctx) {
  graphics_fill_rect(ctx, GRect(88,40,40,34), 0, 0);
}
static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
  update_all();
}

static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
  // Needs to be static because it's used by the system later.
  static char s_time_text[] = "00:00:00";
  static char s_sec_text[] = "00";

  strftime(s_time_text, sizeof(s_time_text), "%T", tick_time);
  text_layer_set_text(s_time_layer, s_time_text);
  strftime(s_sec_text, sizeof(s_sec_text), "%S", tick_time);
  text_layer_set_text(s_sec_layer, s_sec_text);

  if ((units_changed & MINUTE_UNIT) >0){
    handle_battery(battery_state_service_peek()); //don't know why it's needed at all
    update_all();
  }
  else
    update_only_sec();
}

static void handle_bluetooth(bool connected) {
  text_layer_set_text(s_connection_layer, connected ? "connected" : "disconnected");
  update_all();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_time_layer = text_layer_create(GRect(0, 40, bounds.size.w, 34));
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  s_connection_layer = text_layer_create(GRect(0, 90, bounds.size.w, 34));
  text_layer_set_text_color(s_connection_layer, GColorWhite);
  text_layer_set_background_color(s_connection_layer, GColorClear);
  text_layer_set_font(s_connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_connection_layer, GTextAlignmentCenter);
  handle_bluetooth(bluetooth_connection_service_peek());

  s_battery_layer = text_layer_create(GRect(0, 120, bounds.size.w, 34));
  text_layer_set_text_color(s_battery_layer, GColorWhite);
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);
  text_layer_set_text(s_battery_layer, "100% charged");

  // Ensures time is displayed immediately (will break if NULL tick event accessed).
  // (This is why it's a good idea to have a separate routine to do the update itself.)
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);
  handle_second_tick(current_time, SECOND_UNIT+MINUTE_UNIT);

  tick_timer_service_subscribe(SECOND_UNIT+MINUTE_UNIT, handle_second_tick);
  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(handle_bluetooth);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_connection_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
}

static void main_window_unload(Window *window) {
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_connection_layer);
  text_layer_destroy(s_battery_layer);
}

static void init() {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);

  // creating the smaller window for the frequent updates.
  s_sec_window = window_create();
  window_set_background_color(s_sec_window, GColorBlack);
  Layer *window_sec_layer = window_get_root_layer(s_sec_window);
  layer_set_update_proc(window_sec_layer, sec_window_update);

  s_sec_layer = text_layer_create(GRect(88, 40, 40, 34));
  text_layer_set_text_color(s_sec_layer, GColorWhite);
  text_layer_set_background_color(s_sec_layer, GColorClear);
  text_layer_set_font(s_sec_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_sec_layer, GTextAlignmentLeft);
  layer_add_child(window_sec_layer, text_layer_get_layer(s_sec_layer));

}

static void deinit() {
  text_layer_destroy(s_sec_layer);
  window_destroy(s_sec_window);

  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
