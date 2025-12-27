#ifndef _UI_THEMES_H
#define _UI_THEMES_H

#ifdef __cplusplus
extern "C" {
#endif


#define UI_THEME_COLOR_COLOR_NAME 0

#define UI_THEME_DEFAULT 0

// Declarations for gradient style 'Color name'
extern const ui_theme_variable_t _ui_theme_bg_color_color_name[1];
extern const ui_style_variable_t _ui_theme_bg_opa_color_name[1];
extern const ui_style_variable_t _ui_theme_grad_color_color_name[1];
extern const ui_style_variable_t _ui_theme_grad_opa_color_name[1];
extern const ui_style_variable_t _ui_theme_grad_dir_color_name[1];
extern const ui_style_variable_t _ui_theme_main_stop_color_name[1];
extern const ui_style_variable_t _ui_theme_grad_stop_color_name[1];

extern uint8_t ui_theme_idx;

void ui_theme_set(uint8_t theme_idx);


#ifdef __cplusplus
} //extern "C"
#endif

#endif //_UI_THEMES_H

