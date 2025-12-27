#include "GUI.h"



const ui_theme_variable_t _ui_theme_bg_color_color_name[1] = {0xA83939};
const ui_style_variable_t _ui_theme_bg_opa_color_name[1] = {255};
const ui_style_variable_t _ui_theme_grad_color_color_name[1] = {0x000000};
const ui_style_variable_t _ui_theme_grad_opa_color_name[1] = {0};
const ui_style_variable_t _ui_theme_grad_dir_color_name[1] = {LV_GRAD_DIR_NONE};
const ui_style_variable_t _ui_theme_main_stop_color_name[1] = {0};
const ui_style_variable_t _ui_theme_grad_stop_color_name[1] = {0};

uint8_t ui_theme_idx = UI_THEME_DEFAULT;


void ui_theme_set(uint8_t theme_idx) {
  ui_theme_idx = theme_idx;
  _ui_theme_set_variable_styles(UI_VARIABLE_STYLES_MODE_FOLLOW);
}
