#include <pebble.h>

// Necessary AppSync variables.
AppSync sync;
uint8_t sync_buffer[96];

// App-specific data
Window        *window;        // All apps must have at least one window
TextLayer     *time_layer;    // The clock
TextLayer     *month_layer;   // The month
TextLayer     *weather_layer; // The weather, eventually. Right now the day of the week.
Layer         *hand_layer;    // The hand layer we use to update the hands.
GPoint        center;         // Point of the center of the screen

const char    DAYS_OF_WEEK[7][3] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
const char    MONTH_NAMES[12][3] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

//enum for key data with AppSync.
enum {
    BLUETOOTH_VIBE_KEY  = 0x0,
    HOURLY_VIBE_KEY     = 0x1,
    BATTERY_HAND_KEY    = 0x2,
    CHARGE_BLINK_KEY    = 0x3,
    INVERTED_COLORS_KEY = 0x4,
    MINUTE_HANDS_KEY    = 0x5,
    STORAGE_VERSION_KEY = 0x10
};

//Struct for using and storing options for the user.
struct Persist {
    int bluetooth_vibe;
    int hourly_vibe;
    int battery_hand;
    int charge_blink;
    int inverted_colors;
    int minute_hands;
    int storage_version;
};

struct Persist options;

// Control booleans
static bool wasConnected;
static bool blackCharging; // Used for flashing the hand while charging.  Only true if isCharging is true.
static bool isCharging;

// Variable for the hour hand.
static const GPathInfo HOUR_HAND_POINTS =
{
    4,
    (GPoint []) {
        { -3, 30 },
        { 2,  30 },
        { 2,  70 },
        { -3, 70 }
    }
};

// Array for displaying the correct battery state.
static const GPathInfo BATTERY_POINTS[] = {
    { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 30 }, { -4, 30 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 32 }, { -4, 32 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 34 }, { -4, 34 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 36 }, { -4,  36 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 38 }, { -4, 38 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 40 }, { -4, 40 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 42 }, { -4, 42 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 44 }, { -4, 44 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 46 }, { -4, 46 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 48 }, { -4, 48 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 50 }, { -4, 50 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 52 }, { -4, 52 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 54 }, { -4, 54 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 56 }, { -4, 56 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 58 }, { -4, 58 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 60 }, { -4, 60 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 62 }, { -4, 62 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 64 }, { -4, 64 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 66 }, { -4, 66 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 68 }, { -4, 68 } } }, { 4, (GPoint []) { { -4, 30 }, { 3, 30 }, { 3, 70 }, { -4, 70 } } }
};

//Second array for pesky pixel problem.
static const GPathInfo BATTERY_POINTS2[] = {
    { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 30 }, { -3, 30 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 32 }, { -3, 32 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 34 }, { -3, 34 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 36 }, { -3,  36 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 38 }, { -3, 38 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 40 }, { -3, 40 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 42 }, { -3, 42 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 44 }, { -3, 44 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 46 }, { -3, 46 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 48 }, { -3, 48 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 50 }, { -3, 50 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 52 }, { -3, 52 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 54 }, { -3, 54 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 56 }, { -3, 56 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 58 }, { -3, 58 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 60 }, { -3, 60 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 62 }, { -3, 62 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 64 }, { -3, 64 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 66 }, { -3, 66 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 68 }, { -3, 68 } } }, { 4, (GPoint []) { { -3, 30 }, { 2, 30 }, { 2, 70 }, { -3, 70 } } }
};

//Constant for the number of battery increments.
static const int BATTERY_INCS = 20;

// GPaths for hands.
static GPath *hour_hand;
static GPath *batt_hand;
static GPath *batt_hand2;

// Used to create the hour hand.
static void CreateHourHand() {
    hour_hand = gpath_create(&HOUR_HAND_POINTS);
    gpath_move_to(hour_hand, center);
}
// Used to create the battery hand.
static void CreateBatteryHands(int charge_percent) {
    if((BATTERY_INCS * charge_percent) / 90 > BATTERY_INCS) {
        batt_hand  = gpath_create(&BATTERY_POINTS[BATTERY_INCS]);
        batt_hand2 = gpath_create(&BATTERY_POINTS2[BATTERY_INCS]);
    } else {
        batt_hand  = gpath_create(&BATTERY_POINTS[(BATTERY_INCS * charge_percent) / 100]);
        batt_hand2 = gpath_create(&BATTERY_POINTS2[(BATTERY_INCS * charge_percent) / 100]);
    }
    gpath_move_to(batt_hand, center);
    gpath_move_to(batt_hand2, center);
}

//Used to update the center numbers.
static void update_number(int number) {
    static char time_text[] = "00"; // Needs to be static because it's used by the system later.

    // Change the number display.
    time_text[0] = '0' + (number / 10);
    if(time_text[0] == '0') {
        time_text[0] = '0' + number;
        time_text[1] = '\0';
    } else {
        time_text[1] = '0' + (number % 10);
    }
    text_layer_set_text(time_layer, time_text);
}

// Update the hands and redraw them.
static void hand_update(Layer *layer, GContext *ctx) {
    // Get the current time.
    time_t    now = time(NULL);
    struct tm *t  = localtime(&now);
    int       rotationAngle;
    //Update the centered number.
    update_number(options.minute_hands == 0 ? t->tm_min : (clock_is_24h_style() ? t->tm_hour : ((t->tm_hour + 11) % 12) + 1));

    // Get the rotation angle.
    if(options.minute_hands == 1) {
        rotationAngle = ((TRIG_MAX_ANGLE) / 60 * (t->tm_min + 30));
    } else {
        rotationAngle = ((TRIG_MAX_ANGLE) / 12 * ((t->tm_hour % 12) + 6));
    }
    //Rotate the "hour" hand always.
    gpath_rotate_to(hour_hand, rotationAngle);

    // If we are charging, then we pick the color based on blackCharging.  Then we draw.
    if(blackCharging) {
        blackCharging = false; //Set to false for next time.
    } else {
        if(options.inverted_colors == 1) {
            graphics_context_set_stroke_color(ctx, GColorBlack);
            graphics_context_set_fill_color(ctx, GColorBlack);
        } else {
            graphics_context_set_stroke_color(ctx, GColorWhite);
            graphics_context_set_fill_color(ctx, GColorWhite);
        }

        //If the user wants the battery hand, draw it.  Otherwise just fill the hour hand.
        if(options.battery_hand == 1) {
            //Depending on the rotation angle, we load one of the battery hands.
            if(rotationAngle - TRIG_MAX_ANGLE / 2 < TRIG_MAX_ANGLE / 16 || rotationAngle + TRIG_MAX_ANGLE / 8 > (3 * TRIG_MAX_ANGLE / 2) || (rotationAngle + TRIG_MAX_ANGLE / 8 > TRIG_MAX_ANGLE && rotationAngle < TRIG_MAX_ANGLE + TRIG_MAX_ANGLE / 16)) {
                gpath_rotate_to(batt_hand, rotationAngle);
                gpath_draw_filled(ctx, batt_hand);
            } else {
                gpath_rotate_to(batt_hand2, rotationAngle);
                gpath_draw_filled(ctx, batt_hand2);
            }
        } else {
            gpath_draw_filled(ctx, hour_hand);
        }
        gpath_draw_outline(ctx, hour_hand);
    }
}

// Called once per minute while the watch is not charging.
static void handle_minute_tick(struct tm* tick_time, TimeUnits units_changed) {
    static char month_text[]    = "000 00";
    static char day_week_text[] = "000";

    //If the day changed, we update day, month, and day of the week.
    if(units_changed & DAY_UNIT) {
        month_text[4] = '0' + (tick_time->tm_mday / 10);
        if(month_text[4] == '0') {
            month_text[4] = '0' + tick_time->tm_mday;
            month_text[5] = '\0';
        } else {
            month_text[5] = '0' + (tick_time->tm_mday % 10);
        }

        month_text[0] = MONTH_NAMES[tick_time->tm_mon][0];
        month_text[1] = MONTH_NAMES[tick_time->tm_mon][1];
        month_text[2] = MONTH_NAMES[tick_time->tm_mon][2];

        day_week_text[0] = DAYS_OF_WEEK[tick_time->tm_wday][0];
        day_week_text[1] = DAYS_OF_WEEK[tick_time->tm_wday][1];
        day_week_text[2] = DAYS_OF_WEEK[tick_time->tm_wday][2];

        text_layer_set_text(month_layer, month_text);
        text_layer_set_text(weather_layer, day_week_text);
    }

    //If the user wants hourly vibes, give it to them.
    if(units_changed & HOUR_UNIT && options.hourly_vibe == 1) {
        vibes_short_pulse();
    }

    // If the user desires the hour hand, do this.
    if(options.minute_hands == 0) {
        if(units_changed & HOUR_UNIT) {
            // Update the layer.
            layer_mark_dirty(hand_layer);
        } else {
            //Update the number here because it wouldn't have been done otherwise.
            update_number(tick_time->tm_min);
        }
    } else {
        // If the user wants the minute hand.
        // Update the layer.
        layer_mark_dirty(hand_layer);
    }
}


// Called once per second during charging.
static void handle_second_tick(struct tm* tick_time, TimeUnits units_changed) {
    // If this is an even second, then the hands are displayed.  Hands are not displayed for odds seconds.
    if((tick_time->tm_sec % 2) == 0) {
        blackCharging = false;
    } else {
        blackCharging = true;
    }
    // Mark the layer dirty so the hands can be redrawn.
    layer_mark_dirty(hand_layer);

    // If the day or hour changed, we call the minute handler to update these things.
    if(units_changed & DAY_UNIT) {
        handle_minute_tick(tick_time, DAY_UNIT);
    } else if(units_changed & HOUR_UNIT) {
        handle_minute_tick(tick_time, HOUR_UNIT);
    }
}

// Toggle the tick timer subscription.
static void ToggleTickTimer() {
    tick_timer_service_unsubscribe();
    if(options.charge_blink == 1 && isCharging) {
        tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
    } else {
        tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
    }
}

// Handler for a battery status change.
static void battery_change(BatteryChargeState charge_state) {
    // Re-create batt hand.
    gpath_destroy(batt_hand);
    gpath_destroy(batt_hand2);
    CreateBatteryHands(charge_state.charge_percent);

    // If the watch was charging, but is no longer, change things.
    if(!charge_state.is_charging && isCharging) {
        // Change the control variables.
        isCharging    = false;
        blackCharging = false;

        //Unsubscribe from the second handler because we don't need it and subscribe to the minute handler.
        ToggleTickTimer();
    }
    // If the watching wasn't charging, but now is, we update things.
    // Note: the flashing should stop when the battery is fully charged, but the watch is still plugged in.
    if(!isCharging && charge_state.is_charging) {
        // Change the control variables.
        isCharging = true;

        //If the user wants the charging blink feature, turn it on.
        if(options.charge_blink == 1) {
            ToggleTickTimer();
            blackCharging = true;
        }
    }
    // Mark the hand layer dirty to redraw it.
    layer_mark_dirty(hand_layer);
}

// If the bluetooth connection status has changed, we vibrate appropriately and change the control variables.
static void bluetooth_change(bool connected) {
    if(!connected && wasConnected) {
        wasConnected = false;
        vibes_long_pulse();
    } else if(connected && !wasConnected) {
        wasConnected = true;
        vibes_double_pulse();
        // Update the time in case it has changed (e.g. flight across time zones).
        psleep(10000);
        time_t    now           = time(NULL);
        struct tm *current_time = localtime(&now);
        handle_minute_tick(current_time, DAY_UNIT);
    }
}

// This is called when the phone wishes to update the options via AppSync.
static void sync_tuple_changed_callback(const uint32_t key, const Tuple *new_tuple, const Tuple *old_tuple, void *context) {
    switch(key) {
    case BLUETOOTH_VIBE_KEY:
        options.bluetooth_vibe = new_tuple->value->uint8;
        bluetooth_connection_service_unsubscribe();
        wasConnected = bluetooth_connection_service_peek();
        if(options.bluetooth_vibe == 1) {
            bluetooth_connection_service_subscribe(&bluetooth_change);
        }
        break;
    case HOURLY_VIBE_KEY:
        options.hourly_vibe = new_tuple->value->uint8;
        break;
    case BATTERY_HAND_KEY:
        options.battery_hand = new_tuple->value->uint8;
        if(options.battery_hand == 0 && options.charge_blink == 0) {
            battery_state_service_unsubscribe();
        } else {
            gpath_destroy(batt_hand);
            gpath_destroy(batt_hand2);
            CreateBatteryHands(battery_state_service_peek().charge_percent);
            if(options.charge_blink == 0) {
                battery_state_service_subscribe(&battery_change);
            }
        }
        layer_mark_dirty(hand_layer);
        break;
    case CHARGE_BLINK_KEY:
        options.charge_blink = new_tuple->value->uint8;
        isCharging           = battery_state_service_peek().is_charging;
        ToggleTickTimer();
        if(options.charge_blink == 0) {
            if(options.battery_hand == 0) {
                battery_state_service_subscribe(&battery_change);
            }
        }
        break;
    case INVERTED_COLORS_KEY:
        options.inverted_colors = new_tuple->value->uint8;
        if(options.inverted_colors == 0) {
            window_set_background_color(window, GColorBlack);
            text_layer_set_text_color(time_layer, GColorWhite);
            text_layer_set_text_color(month_layer, GColorWhite);
            text_layer_set_text_color(weather_layer, GColorWhite);
            layer_mark_dirty(hand_layer);
        } else {
            window_set_background_color(window, GColorWhite);
            text_layer_set_text_color(time_layer, GColorBlack);
            text_layer_set_text_color(month_layer, GColorBlack);
            text_layer_set_text_color(weather_layer, GColorBlack);
            layer_mark_dirty(hand_layer);
        }
        break;
    case MINUTE_HANDS_KEY:
        options.minute_hands = new_tuple->value->uint8;
        layer_mark_dirty(hand_layer);
        break;
    }
}

// Handle the start-up of the app
static void do_init(void) {
    //Initialize the options struct by looking for persistent data.
    options.bluetooth_vibe  = persist_exists(BLUETOOTH_VIBE_KEY) ? persist_read_int(BLUETOOTH_VIBE_KEY) : 1;
    options.hourly_vibe     = persist_exists(HOURLY_VIBE_KEY) ? persist_read_int(HOURLY_VIBE_KEY) : 0;
    options.battery_hand    = persist_exists(BATTERY_HAND_KEY) ? persist_read_int(BATTERY_HAND_KEY) : 1;
    options.charge_blink    = persist_exists(CHARGE_BLINK_KEY) ? persist_read_int(CHARGE_BLINK_KEY) : 1;
    options.inverted_colors = persist_exists(INVERTED_COLORS_KEY) ? persist_read_int(INVERTED_COLORS_KEY) : 0;
    options.minute_hands    = persist_exists(MINUTE_HANDS_KEY) ? persist_read_int(MINUTE_HANDS_KEY) : 0;
    options.storage_version = persist_exists(STORAGE_VERSION_KEY) ? persist_read_int(STORAGE_VERSION_KEY) : 2;

    // Create our app's base window
    window = window_create();
    window_set_background_color(window, options.inverted_colors == 1 ? GColorWhite : GColorBlack);
    window_stack_push(window, true);

    Layer *root_layer = window_get_root_layer(window);
    GRect bounds      = layer_get_bounds(root_layer);
    center = grect_center_point(&bounds);

    // Declare input and output buffer sizes for AppSync.
    const int inbound_size  = 96;
    const int outbound_size = 96;
    app_message_open(inbound_size, outbound_size);

    // Tuplet of initial values for the options.
    Tuplet initial_values[] = {
        TupletInteger(BLUETOOTH_VIBE_KEY,  options.bluetooth_vibe),
        TupletInteger(HOURLY_VIBE_KEY,     options.hourly_vibe),
        TupletInteger(BATTERY_HAND_KEY,    options.battery_hand),
        TupletInteger(CHARGE_BLINK_KEY,    options.charge_blink),
        TupletInteger(INVERTED_COLORS_KEY, options.inverted_colors),
        TupletInteger(MINUTE_HANDS_KEY,    options.minute_hands)
    };

    // Initialize the AppSync.
    app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values), sync_tuple_changed_callback, NULL, NULL);

    // Init the text layer used to show the minutes
    time_layer = text_layer_create(GRect((bounds.size.w - 50) / 2, (bounds.size.h - 50) / 2, 50 /* width */, 50 /* height */));
    text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
    text_layer_set_text_color(time_layer, options.inverted_colors == 0 ? GColorWhite : GColorBlack);
    text_layer_set_background_color(time_layer, GColorClear);
    text_layer_set_font(time_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_MAIN_40)));

    //Init month layer to show month text.
    month_layer = text_layer_create(GRect(2, bounds.size.h - 18, 60, 18));
    text_layer_set_text_alignment(month_layer, GTextAlignmentLeft);
    text_layer_set_text_color(month_layer, options.inverted_colors == 0 ? GColorWhite : GColorBlack);
    text_layer_set_background_color(month_layer, GColorClear);
    text_layer_set_font(month_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LOWER_15)));

    // Init "weather" layer that show the day of the week.
    weather_layer = text_layer_create(GRect(bounds.size.w - 36, bounds.size.h - 18, 34, 18));
    text_layer_set_text_alignment(weather_layer, GTextAlignmentRight);
    text_layer_set_text_color(weather_layer, options.inverted_colors == 0 ? GColorWhite : GColorBlack);
    text_layer_set_background_color(weather_layer, GColorClear);
    text_layer_set_font(weather_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_LOWER_15)));

    //Init the hand layer used to update the hands.
    hand_layer = layer_create(bounds);
    layer_set_update_proc(hand_layer, hand_update);
    layer_add_child(root_layer, hand_layer);

    //Get the current battery state.
    isCharging    = battery_state_service_peek().is_charging;
    blackCharging = isCharging;

    //Init the hands.
    CreateHourHand();
    CreateBatteryHands(battery_state_service_peek().charge_percent);

    // Ensures time is displayed immediately (will break if NULL tick event accessed).
    // (This is why it's a good idea to have a separate routine to do the update itself.)
    time_t    now           = time(NULL);
    struct tm *current_time = localtime(&now);
    handle_minute_tick(current_time, DAY_UNIT);

    //Subscribe to the tick service.  If the watch is charging we subscribe the second unit, and minute unit, otherwise.
    ToggleTickTimer();

    //Add the layers to the window.
    layer_add_child(root_layer, text_layer_get_layer(time_layer));
    layer_add_child(root_layer, text_layer_get_layer(month_layer));
    layer_add_child(root_layer, text_layer_get_layer(weather_layer));

    //Mark the hand layer dirty so the hands will be drawn correctly.
    layer_mark_dirty(hand_layer);

    //Get the current bluetooth state.
    wasConnected = bluetooth_connection_service_peek();

    //Subscribe to the bluetooth service.
    if(options.bluetooth_vibe == 1) {
        bluetooth_connection_service_subscribe(&bluetooth_change);
    }

    //Subscribe to the battery state service.
    if(options.battery_hand == 1 || options.charge_blink == 1) {
        battery_state_service_subscribe(&battery_change);
    }
}

static void do_deinit(void) {
    //Write to persistent data.
    persist_write_int(BLUETOOTH_VIBE_KEY, options.bluetooth_vibe);
    persist_write_int(HOURLY_VIBE_KEY, options.hourly_vibe);
    persist_write_int(BATTERY_HAND_KEY, options.battery_hand);
    persist_write_int(CHARGE_BLINK_KEY, options.charge_blink);
    persist_write_int(INVERTED_COLORS_KEY, options.inverted_colors);
    persist_write_int(MINUTE_HANDS_KEY, options.minute_hands);
    persist_write_int(STORAGE_VERSION_KEY, options.storage_version);

    //Destroy everything.
    layer_remove_child_layers(window_get_root_layer(window));
    text_layer_destroy(time_layer);
    text_layer_destroy(month_layer);
    text_layer_destroy(weather_layer);
    gpath_destroy(hour_hand);
    gpath_destroy(batt_hand);
    gpath_destroy(batt_hand2);
    layer_destroy(hand_layer);
    tick_timer_service_unsubscribe();
    bluetooth_connection_service_unsubscribe();
    battery_state_service_unsubscribe();
    app_sync_deinit(&sync);
    window_destroy(window);
}

// The main event/run loop for our app
int main(void) {
    do_init();
    app_event_loop();
    do_deinit();
}
