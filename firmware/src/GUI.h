#ifndef _GUI_HEADER_INCLUDED
#define _GUI_HEADER_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif


#if defined __has_include
#if __has_include("lvgl.h")
#include "lvgl.h"
#elif __has_include("lvgl/lvgl.h")
#include "lvgl/lvgl.h"
#else
#include "lvgl.h"
#endif
#else
#include "lvgl.h"
#endif
#include "ui_helpers.h"

#include "ui_theme_manager.h"

#include "GUI_Themes.h"

extern lv_obj_t* GUI_Screen__screen;
extern lv_obj_t* GUI_Screen__wifi;
extern lv_obj_t*  GUI_Image__screen__dashboardImg;
extern lv_obj_t*  GUI_Image__screen__wifiImage;
extern lv_obj_t*  GUI_Arc__screen__arc;
extern lv_obj_t*  GUI_Container__screen__DataContainer;
extern lv_obj_t*   GUI_Container__screen__moistureContainer;
extern lv_obj_t*    GUI_Image__screen__moistureImg;
LV_FONT_DECLARE( regular_text_1 );
extern lv_obj_t*    GUI_Label__screen__moisture;
extern lv_obj_t*   GUI_Label__screen__currentTemperature;
LV_FONT_DECLARE( font_3 );
extern lv_obj_t*   GUI_Label__screen__setTemperature;
LV_FONT_DECLARE( font_4_1 );
extern lv_obj_t*   GUI_Label__screen__set_to;
extern lv_obj_t*  GUI_Container__screen__controlleersContainer;
extern lv_obj_t*   GUI_Image__screen__moodImg;
extern lv_obj_t*   GUI_Image__screen__fanImg;
extern lv_obj_t*   GUI_Image__screen__pumpImg;
extern lv_obj_t*   GUI_Image__screen__powerImg;
extern lv_obj_t*  GUI_Button__wifi__backBtn;
extern lv_obj_t*   GUI_Label__wifi__backLabel;
extern lv_obj_t*  GUI_Label__wifi__title;
extern lv_obj_t*  GUI_TextArea__wifi__ssid;
extern lv_obj_t*  GUI_TextArea__wifi__pass;
extern lv_obj_t*  GUI_Button__wifi__connectBtn;
extern lv_obj_t*   GUI_Label__wifi__connectLabel;
extern lv_obj_t*  GUI_Button__wifi__clearBtn;
extern lv_obj_t*   GUI_Label__wifi__clearLabel;
extern lv_obj_t*  GUI_Label__wifi__status;
extern lv_obj_t*  GUI_Keyboard__wifi__keyboard;


// Screen-specific function declarations
void GUI_initScreen__screen ();
void GUI_initScreenTexts__screen ();
void GUI_initScreenStyles__screen ();
void GUI_initScreen__wifi ();
void GUI_initScreenTexts__wifi ();
void GUI_initScreenStyles__wifi ();

extern lv_style_t GUI_Style__class_720QKVxWEmbvhM__;
extern lv_style_t GUI_Style__class_wKRNl64hJph3yK__;
extern lv_style_t GUI_Style__class_BRPSMN5kLlcn7C__;
extern lv_style_t GUI_Style__class_THgObigAvqHpFq__;
extern lv_style_t GUI_Style__class_kdhj6JKmHjoRTB__;
extern lv_style_t GUI_Style__class_JqUcMs1ZNPRfQq__;
extern lv_style_t GUI_Style__class_MsKIDbZdEXuzIO__;
extern lv_style_t GUI_Style__class_0TWSkus0ZPqgtj__;
extern lv_style_t GUI_Style__class_VRI9xFdITfSk2x__;
extern lv_style_t GUI_Style__class_1nSloISkFO20jp__;
extern lv_style_t GUI_Style__class_Mjy6X8hngx7TUa__;


void GUI_load ();

void GUI_init ();

void GUI_refresh ();


void GUI_initHAL ();
void HAL_init ();

void GUI_initFramework ();

void GUI_loadContent ();


void GUI_initContent ();


void GUI_initTheme ();


void GUI_initScreens ();


void GUI_loadFirstScreen ();


void GUI_initScreenContents ();

void GUI_initScreenTexts ();

void GUI_initScreenStyles ();


void GUI_initGlobalStyles ();


void GUI_initAnimations ();




void GUI_event__Arc__screen__arc__Clicked (lv_event_t* event);
void GUI_event__Image__screen__moodImg__Clicked (lv_event_t* event);
void GUI_event__Image__screen__powerImg__Clicked (lv_event_t* event);
void GUI_event__Image__screen__wifiImage__Clicked (lv_event_t* event);
void GUI_event__Image__screen__dashboardImg__Clicked (lv_event_t* event);
void GUI_event__Button__wifi__backBtn__Clicked (lv_event_t* event);
void GUI_event__Button__wifi__connectBtn__Clicked (lv_event_t* event);
void GUI_event__Button__wifi__clearBtn__Clicked (lv_event_t* event);
void GUI_event__TextArea__wifi__ssid__Focused (lv_event_t* event);
void GUI_event__TextArea__wifi__pass__Focused (lv_event_t* event);
void GUI_event__TextArea__wifi__ssid__Defocused (lv_event_t* event);
void GUI_event__TextArea__wifi__pass__Defocused (lv_event_t* event);
void GUI_event__Keyboard__wifi__keyboard__Action (lv_event_t* event);
 void PowerToggle (lv_event_t* event);



#ifdef __cplusplus
} //extern "C"
#endif

#endif //_GUI_HEADER_INCLUDED

LV_IMG_DECLARE( dashboard );
LV_IMG_DECLARE( wifi );
LV_IMG_DECLARE( fan );
LV_IMG_DECLARE( pump );
LV_IMG_DECLARE( power );
LV_IMG_DECLARE( sun );
LV_IMG_DECLARE( winter );
LV_IMG_DECLARE( fan_fast );
LV_IMG_DECLARE( fan_slow );
LV_IMG_DECLARE( fan_off );
LV_IMG_DECLARE( pump_on );
LV_IMG_DECLARE( pump_off );
LV_IMG_DECLARE( power_on );
LV_IMG_DECLARE( power_off );
LV_IMG_DECLARE( wifi_full );
LV_IMG_DECLARE( wifi_midum );
LV_IMG_DECLARE( wifi_low );
