#define LOG_TAG "ui"
#include "ui.h"
#include "fal.h"

// 按钮ID枚举
typedef enum
{
    BTN_ID_JUMP = 1,
    BTN_ID_ERASE = 2
} button_id_t;

// 按钮点击事件回调函数
static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    // 获取按钮的用户数据（按钮ID）
    button_id_t btn_id = (button_id_t)(uintptr_t)lv_obj_get_user_data(btn);
    static const struct fal_partition *part_dev = NULL;

    if (code == LV_EVENT_CLICKED)
    {
        switch (btn_id)
        {
        case BTN_ID_JUMP:
        {
            JumpToApp();
        }
        break;
        case BTN_ID_ERASE:
        {
            part_dev = fal_partition_find("app1");
            fal_partition_erase_all(part_dev);
        }
        break;
        default:
            break;
        }
    }
}

void create_jump_button(void)
{
    // 创建一个按钮
    lv_obj_t *btn = lv_btn_create(lv_scr_act());

    // 设置按钮的用户数据（按钮ID）
    lv_obj_set_user_data(btn, (void *)(uintptr_t)BTN_ID_JUMP);

    // 设置按钮位置和大小
    lv_obj_set_size(btn, 120, 50);              // 宽度 120, 高度 50
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 0, 0); // 居中显示

    // 为按钮添加标签
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Jump");
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

void create_erase_button(void)
{
    // 创建一个按钮
    lv_obj_t *btn = lv_btn_create(lv_scr_act());

    // 设置按钮的用户数据（按钮ID）
    lv_obj_set_user_data(btn, (void *)(uintptr_t)BTN_ID_ERASE);

    // 设置按钮位置和大小
    lv_obj_set_size(btn, 120, 50);               // 宽度 120, 高度 50
    lv_obj_align(btn, LV_ALIGN_TOP_RIGHT, 0, 0); // 居中显示

    // 为按钮添加标签
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Erase");
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
