#include "pebble.h"

Window *window;
TextLayer *text_date_layer;
TextLayer *text_delta_layer;
TextLayer *text_time_layer;
Layer *line_layer;

void line_layer_update_callback(Layer *layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

typedef struct{
	char name[40];
	int time;
} Event;

Event events[] = {{"old history", 20054}, {"Practice Round\nTable3 Side2", 1423404900}};
int numEvents = sizeof(events)/sizeof(events[0]);

Event nextEvent(){
	int i = 0;
	while (i < numEvents){
		if (events[i].time >= time(NULL)){
			return events[i];
		}
		i++;
	}
	Event never = {"Nothing", 1423363015};
	return never;
}

void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  //static char date_text[] = "Xxxxxxxxx 00";

  char *time_format;

  if (!tick_time) {
    time_t now = time(NULL);
    tick_time = localtime(&now);
  }

  // TODO: Only update the date when it's changed.
  //strftime(date_text, sizeof(date_text), "%B %e", tick_time);
  //text_layer_set_text(text_date_layer, date_text);
	Event next = nextEvent();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "{%s, %d}", next.name, next.time);
	text_layer_set_text(text_date_layer, next.name);
	
	char deltaText[] = "in XXXX minutes  ";
	
	int dt = next.time - time(NULL);
	snprintf(deltaText, sizeof(deltaText)-1, "in %d minutes", (dt+30)/60);
	
	text_layer_set_text(text_delta_layer, deltaText);


  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(text_time_layer, time_text);
}

void handle_deinit(void) {
  tick_timer_service_unsubscribe();
}

void handle_init(void) {
  window = window_create();
  window_stack_push(window, true /* Animated */);
  window_set_background_color(window, GColorBlack);

  Layer *window_layer = window_get_root_layer(window);

  text_date_layer = text_layer_create(GRect(8, 65, 144-8, 168-65));
  text_layer_set_text_color(text_date_layer, GColorWhite);
  text_layer_set_background_color(text_date_layer, GColorClear);
  text_layer_set_font(text_date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(text_date_layer));

  text_delta_layer = text_layer_create(GRect(8, 130, 144-8, 168-130));
  text_layer_set_text_color(text_delta_layer, GColorWhite);
  text_layer_set_background_color(text_delta_layer, GColorClear);
  text_layer_set_font(text_delta_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  layer_add_child(window_layer, text_layer_get_layer(text_delta_layer));

  text_time_layer = text_layer_create(GRect(7, 2, 144-7, 168-2));
  text_layer_set_text_color(text_time_layer, GColorWhite);
  text_layer_set_background_color(text_time_layer, GColorClear);
  text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

  GRect line_frame = GRect(8, 60, 139, 2);
  line_layer = layer_create(line_frame);
  layer_set_update_proc(line_layer, line_layer_update_callback);
  layer_add_child(window_layer, line_layer);

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  handle_minute_tick(NULL, MINUTE_UNIT);
}


int main(void) {
  handle_init();
  app_event_loop();
  
  handle_deinit();
}
