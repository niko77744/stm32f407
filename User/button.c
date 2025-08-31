#define LOG_TAG "button"
#include "button.h"
#include "elog.h"
#include "multi_button.h"

static Button btn_0, btn_1, btn_2, btn_3;
MultiTimer button_timer;

// Hardware abstraction layer function
uint8_t read_button_gpio(uint8_t button_id)
{
    switch (button_id)
    {
    case btn_id_0:
        return HAL_GPIO_ReadPin(KEY_0_GPIO_Port, KEY_0_Pin) == GPIO_PIN_SET ? 1 : 0;
    case btn_id_1:
        return HAL_GPIO_ReadPin(KEY_1_GPIO_Port, KEY_1_Pin) == GPIO_PIN_SET ? 1 : 0;
    case btn_id_2:
        return HAL_GPIO_ReadPin(KEY_2_GPIO_Port, KEY_2_Pin) == GPIO_PIN_SET ? 1 : 0;
    case btn_id_3:
        return HAL_GPIO_ReadPin(KEY_3_GPIO_Port, KEY_3_Pin) == GPIO_PIN_SET ? 1 : 0;
    default:
        return 0;
    }
}

// Generic event handler that shows button info
void generic_event_handler(Button *btn, const char *event_name)
{
    log_i("Button %d: %s (repeat: %d, pressed: %s)",
          btn->button_id,
          event_name,
          button_get_repeat_count(btn),
          button_is_pressed(btn) ? "Yes" : "No");
}

// Event handlers
void on_press_down(Button *btn) { generic_event_handler(btn, "Press Down"); }
void on_press_up(Button *btn) { generic_event_handler(btn, "Press Up"); }
void on_single_click(Button *btn) { generic_event_handler(btn, "Single Click"); }
void on_double_click(Button *btn) { generic_event_handler(btn, "Double Click"); }
void on_long_press_start(Button *btn) { generic_event_handler(btn, "Long Press Start"); }
void on_long_press_hold(Button *btn) { generic_event_handler(btn, "Long Press Hold"); }
void on_press_repeat(Button *btn) { generic_event_handler(btn, "Press Repeat"); }

void buttons_callback_handler(Button *btn)
{
    ButtonEvent event = button_get_event(btn);
    switch (event)
    {
    case BTN_SINGLE_CLICK:
        on_single_click(btn);
        break;
    case BTN_DOUBLE_CLICK:
        on_double_click(btn);
        break;
    case BTN_LONG_PRESS_START:
        on_long_press_start(btn);
        break;
    case BTN_LONG_PRESS_HOLD:
        on_long_press_hold(btn);
        break;
    case BTN_PRESS_REPEAT:
        on_press_repeat(btn);
        break;
    case BTN_PRESS_DOWN:
        on_press_down(btn);
        break;
    case BTN_PRESS_UP:
        on_press_up(btn);
        break;
    default:
        log_i("Unknown event: %d\n", event);
        break;
    }
}

// Initialize a single button with all event handlers
void init_button(Button *handle, uint8_t (*pin_level)(uint8_t), uint8_t button_id, uint8_t active_level, uint8_t enable_all_events)
{
    button_init(handle, pin_level, active_level, button_id);

    if (enable_all_events)
    {
        button_attach(handle, BTN_PRESS_DOWN, buttons_callback_handler);
        button_attach(handle, BTN_PRESS_UP, buttons_callback_handler);
        button_attach(handle, BTN_SINGLE_CLICK, buttons_callback_handler);
        button_attach(handle, BTN_DOUBLE_CLICK, buttons_callback_handler);
        button_attach(handle, BTN_LONG_PRESS_START, buttons_callback_handler);
        button_attach(handle, BTN_LONG_PRESS_HOLD, buttons_callback_handler);
        button_attach(handle, BTN_PRESS_REPEAT, buttons_callback_handler);
    }
    else
    {
        // Only essential events
        button_attach(handle, BTN_SINGLE_CLICK, buttons_callback_handler);
        button_attach(handle, BTN_DOUBLE_CLICK, buttons_callback_handler);
        button_attach(handle, BTN_LONG_PRESS_START, buttons_callback_handler);
    }

    button_start(handle);
}

static void button_ticks_callback(MultiTimer *timer, void *userData)
{
    button_ticks();
    multiTimerStart(&button_timer, 5, button_ticks_callback, NULL);
}

// Initialize buttons
void buttons_init(void)
{
    // Initialize hardware buttons
    init_button(&btn_0, read_button_gpio, btn_id_0, 0, 0);
    init_button(&btn_1, read_button_gpio, btn_id_1, 0, 0);
    init_button(&btn_2, read_button_gpio, btn_id_2, 0, 0);
    init_button(&btn_3, read_button_gpio, btn_id_3, 0, 0);

    multiTimerStart(&button_timer, 5, button_ticks_callback, NULL);
}
