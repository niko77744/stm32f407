#define LOG_TAG "ui"
#include "ui.h"

// 按钮点击事件回调函数
static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED)
    {
        static uint8_t cnt = 0;
        cnt++;

        // 获取按钮的标签对象并更新文本
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Clicked: %d", cnt);

        // 可以在这里添加其他点击后的操作
        // 例如改变按钮颜色、发送消息等
    }
}

void create_clickable_button(void)
{
    // 创建一个按钮
    lv_obj_t *btn = lv_btn_create(lv_scr_act());

    // 设置按钮位置和大小
    lv_obj_set_size(btn, 120, 50);              // 宽度 120, 高度 50
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 0, 0); // 居中显示

    // 为按钮添加标签
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Click Me!");
    lv_obj_center(label);

    // 设置按钮样式（可选）
    static lv_style_t style_btn;
    lv_style_init(&style_btn);

    // 正常状态样式
    lv_style_set_bg_color(&style_btn, lv_color_hex(0x007ac3));
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_radius(&style_btn, 10);
    lv_style_set_border_width(&style_btn, 0);

    // 按下状态样式
    lv_style_set_bg_color(&style_btn, lv_color_hex(0x005a93));

    // 应用样式
    lv_obj_add_style(btn, &style_btn, 0);

    // 设置标签样式（可选）
    static lv_style_t style_label;
    lv_style_init(&style_label);
    lv_style_set_text_color(&style_label, lv_color_white());
    lv_style_set_text_font(&style_label, &lv_font_montserrat_14);
    lv_obj_add_style(label, &style_label, 0);

    // 添加点击事件回调
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
}
