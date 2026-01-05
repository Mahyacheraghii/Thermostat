#include "../../GUI.h"



void GUI_initScreen__screen () {
    GUI_Screen__screen = lv_obj_create( NULL );
    lv_obj_remove_flag( GUI_Screen__screen, LV_OBJ_FLAG_SCROLLABLE );
     GUI_Image__screen__dashboardImg = lv_image_create( GUI_Screen__screen );
     lv_obj_add_flag( GUI_Image__screen__dashboardImg, LV_OBJ_FLAG_ADV_HITTEST );
     lv_obj_remove_flag( GUI_Image__screen__dashboardImg, LV_OBJ_FLAG_SCROLLABLE );
     lv_obj_set_align( GUI_Image__screen__dashboardImg, LV_ALIGN_CENTER );
     lv_obj_set_pos( GUI_Image__screen__dashboardImg, 141, -100 );
     lv_obj_set_size( GUI_Image__screen__dashboardImg, 24, 24 );
     lv_img_set_zoom( GUI_Image__screen__dashboardImg, 204 );
     lv_obj_add_event_cb( GUI_Image__screen__dashboardImg, GUI_event__Image__screen__dashboardImg__Clicked, LV_EVENT_CLICKED, NULL );

    GUI_Image__screen__wifiImage = lv_image_create( GUI_Screen__screen );
    lv_obj_add_flag( GUI_Image__screen__wifiImage, LV_OBJ_FLAG_ADV_HITTEST );
    lv_obj_remove_flag( GUI_Image__screen__wifiImage, LV_OBJ_FLAG_SCROLLABLE );
    lv_obj_set_align( GUI_Image__screen__wifiImage, LV_ALIGN_CENTER );
    lv_obj_set_pos( GUI_Image__screen__wifiImage, -140, -100 );
    lv_obj_set_size( GUI_Image__screen__wifiImage, 24, 24 );
    lv_img_set_zoom( GUI_Image__screen__wifiImage, 204 );
    lv_obj_add_event_cb( GUI_Image__screen__wifiImage, GUI_event__Image__screen__wifiImage__Clicked, LV_EVENT_CLICKED, NULL );

     GUI_Arc__screen__arc = lv_arc_create( GUI_Screen__screen );
     lv_arc_set_value( GUI_Arc__screen__arc, 50 );
     lv_arc_set_bg_angles( GUI_Arc__screen__arc, 120, 60 );
     lv_obj_set_align( GUI_Arc__screen__arc, LV_ALIGN_CENTER );
     lv_obj_set_pos( GUI_Arc__screen__arc, 1, -13 );
     lv_obj_set_size( GUI_Arc__screen__arc, 144, 142 );
     lv_obj_add_event_cb( GUI_Arc__screen__arc, GUI_event__Arc__screen__arc__Clicked, LV_EVENT_CLICKED, NULL );

     GUI_Container__screen__DataContainer = lv_obj_create( GUI_Screen__screen );
     lv_obj_remove_style_all( GUI_Container__screen__DataContainer );
     lv_obj_remove_flag( GUI_Container__screen__DataContainer, LV_OBJ_FLAG_CLICKABLE );
     lv_obj_remove_flag( GUI_Container__screen__DataContainer, LV_OBJ_FLAG_SCROLLABLE );
     lv_obj_set_align( GUI_Container__screen__DataContainer, LV_ALIGN_CENTER );
     lv_obj_set_pos( GUI_Container__screen__DataContainer, 0, -5 );
     lv_obj_set_size( GUI_Container__screen__DataContainer, 200, 140 );

      GUI_Container__screen__moistureContainer = lv_obj_create( GUI_Container__screen__DataContainer );
      lv_obj_remove_style_all( GUI_Container__screen__moistureContainer );
      lv_obj_remove_flag( GUI_Container__screen__moistureContainer, LV_OBJ_FLAG_CLICKABLE );
      lv_obj_remove_flag( GUI_Container__screen__moistureContainer, LV_OBJ_FLAG_SCROLLABLE );
      lv_obj_set_align( GUI_Container__screen__moistureContainer, LV_ALIGN_CENTER );
      lv_obj_set_pos( GUI_Container__screen__moistureContainer, 0, 55 );
      lv_obj_set_size( GUI_Container__screen__moistureContainer, 70, 28 );

       GUI_Image__screen__moistureImg = lv_image_create( GUI_Container__screen__moistureContainer );
       lv_obj_add_flag( GUI_Image__screen__moistureImg, LV_OBJ_FLAG_ADV_HITTEST );
       lv_obj_remove_flag( GUI_Image__screen__moistureImg, LV_OBJ_FLAG_SCROLLABLE );
       lv_obj_set_align( GUI_Image__screen__moistureImg, LV_ALIGN_CENTER );
       lv_obj_set_size( GUI_Image__screen__moistureImg, 24, 24 );
       lv_img_set_zoom( GUI_Image__screen__moistureImg, 204 );

       GUI_Label__screen__moisture = lv_label_create( GUI_Container__screen__moistureContainer );
       lv_obj_set_align( GUI_Label__screen__moisture, LV_ALIGN_CENTER );
       lv_obj_set_size( GUI_Label__screen__moisture, LV_SIZE_CONTENT, LV_SIZE_CONTENT );

      GUI_Label__screen__currentTemperature = lv_label_create( GUI_Container__screen__DataContainer );
      lv_obj_set_align( GUI_Label__screen__currentTemperature, LV_ALIGN_CENTER );
      lv_obj_set_pos( GUI_Label__screen__currentTemperature, 0, 22 );
      lv_obj_set_size( GUI_Label__screen__currentTemperature, LV_SIZE_CONTENT, LV_SIZE_CONTENT );

      GUI_Label__screen__setTemperature = lv_label_create( GUI_Container__screen__DataContainer );
      lv_obj_set_align( GUI_Label__screen__setTemperature, LV_ALIGN_CENTER );
      lv_obj_set_pos( GUI_Label__screen__setTemperature, 0, -6 );
      lv_obj_set_size( GUI_Label__screen__setTemperature, LV_SIZE_CONTENT, LV_SIZE_CONTENT );

      GUI_Label__screen__set_to = lv_label_create( GUI_Container__screen__DataContainer );
      lv_obj_set_align( GUI_Label__screen__set_to, LV_ALIGN_CENTER );
      lv_obj_set_pos( GUI_Label__screen__set_to, 0, -32 );
      lv_obj_set_size( GUI_Label__screen__set_to, LV_SIZE_CONTENT, LV_SIZE_CONTENT );

     GUI_Container__screen__controlleersContainer = lv_obj_create( GUI_Screen__screen );
     lv_obj_remove_style_all( GUI_Container__screen__controlleersContainer );
     lv_obj_remove_flag( GUI_Container__screen__controlleersContainer, LV_OBJ_FLAG_CLICKABLE );
     lv_obj_remove_flag( GUI_Container__screen__controlleersContainer, LV_OBJ_FLAG_SCROLLABLE );
     lv_obj_set_align( GUI_Container__screen__controlleersContainer, LV_ALIGN_CENTER );
     lv_obj_set_pos( GUI_Container__screen__controlleersContainer, 3, 93 );
     lv_obj_set_size( GUI_Container__screen__controlleersContainer, 160, 30 );

      GUI_Image__screen__moodImg = lv_image_create( GUI_Container__screen__controlleersContainer );
      lv_obj_add_flag( GUI_Image__screen__moodImg, LV_OBJ_FLAG_ADV_HITTEST );
      lv_obj_remove_flag( GUI_Image__screen__moodImg, LV_OBJ_FLAG_SCROLLABLE );
      lv_obj_set_align( GUI_Image__screen__moodImg, LV_ALIGN_CENTER );
      lv_obj_set_size( GUI_Image__screen__moodImg, 24, 24 );
      lv_img_set_zoom( GUI_Image__screen__moodImg, 204 );
      lv_obj_add_event_cb( GUI_Image__screen__moodImg, GUI_event__Image__screen__moodImg__Clicked, LV_EVENT_CLICKED, NULL );

      GUI_Image__screen__fanImg = lv_image_create( GUI_Container__screen__controlleersContainer );
      lv_obj_add_flag( GUI_Image__screen__fanImg, LV_OBJ_FLAG_ADV_HITTEST );
      lv_obj_remove_flag( GUI_Image__screen__fanImg, LV_OBJ_FLAG_SCROLLABLE );
      lv_obj_set_align( GUI_Image__screen__fanImg, LV_ALIGN_CENTER );
      lv_obj_set_size( GUI_Image__screen__fanImg, 24, 24 );
      lv_img_set_zoom( GUI_Image__screen__fanImg, 204 );
      lv_obj_add_event_cb( GUI_Image__screen__fanImg, GUI_event__Image__screen__fanImg__Clicked, LV_EVENT_CLICKED, NULL );

      GUI_Image__screen__pumpImg = lv_image_create( GUI_Container__screen__controlleersContainer );
      lv_obj_add_flag( GUI_Image__screen__pumpImg, LV_OBJ_FLAG_ADV_HITTEST );
      lv_obj_remove_flag( GUI_Image__screen__pumpImg, LV_OBJ_FLAG_SCROLLABLE );
      lv_obj_set_align( GUI_Image__screen__pumpImg, LV_ALIGN_CENTER );
      lv_obj_set_size( GUI_Image__screen__pumpImg, 24, 24 );
      lv_img_set_zoom( GUI_Image__screen__pumpImg, 204 );
      lv_obj_add_event_cb( GUI_Image__screen__pumpImg, GUI_event__Image__screen__pumpImg__Clicked, LV_EVENT_CLICKED, NULL );

      GUI_Image__screen__powerImg = lv_image_create( GUI_Container__screen__controlleersContainer );
      lv_obj_add_flag( GUI_Image__screen__powerImg, LV_OBJ_FLAG_ADV_HITTEST );
      lv_obj_remove_flag( GUI_Image__screen__powerImg, LV_OBJ_FLAG_SCROLLABLE );
      lv_obj_set_align( GUI_Image__screen__powerImg, LV_ALIGN_CENTER );
      lv_obj_set_size( GUI_Image__screen__powerImg, 24, 24 );
      lv_img_set_zoom( GUI_Image__screen__powerImg, 204 );
      lv_obj_add_event_cb( GUI_Image__screen__powerImg, GUI_event__Image__screen__powerImg__Clicked, LV_EVENT_CLICKED, NULL );


    GUI_initScreenStyles__screen();
    GUI_initScreenTexts__screen();
}


void GUI_initScreenTexts__screen () {
       lv_label_set_text( GUI_Label__screen__moisture, "41%" );
      lv_label_set_text( GUI_Label__screen__currentTemperature, "20°" );
      lv_label_set_text( GUI_Label__screen__setTemperature, "23°" );
      lv_label_set_text( GUI_Label__screen__set_to, "set to" );
}


void GUI_initScreenStyles__screen () {
    lv_obj_add_style( GUI_Screen__screen, &GUI_Style__class_720QKVxWEmbvhM__, LV_PART_MAIN | LV_STATE_DEFAULT );

     lv_image_set_src( GUI_Image__screen__dashboardImg, &dashboard );

     lv_image_set_src( GUI_Image__screen__wifiImage, &wifi );
     lv_obj_set_style_bg_opa( GUI_Image__screen__wifiImage, 0, LV_PART_MAIN | LV_STATE_DEFAULT );
     lv_obj_set_style_border_width( GUI_Image__screen__wifiImage, 0, LV_PART_MAIN | LV_STATE_DEFAULT );
     lv_obj_set_style_outline_width( GUI_Image__screen__wifiImage, 0, LV_PART_MAIN | LV_STATE_DEFAULT );

     lv_obj_add_style( GUI_Arc__screen__arc, &GUI_Style__class_wKRNl64hJph3yK__, LV_PART_MAIN | LV_STATE_DEFAULT );
     lv_obj_add_style( GUI_Arc__screen__arc, &GUI_Style__class_BRPSMN5kLlcn7C__, LV_PART_INDICATOR | LV_STATE_DEFAULT );
     lv_obj_add_style( GUI_Arc__screen__arc, &GUI_Style__class_THgObigAvqHpFq__, LV_PART_KNOB | LV_STATE_DEFAULT );

     lv_obj_add_style( GUI_Container__screen__DataContainer, &GUI_Style__class_kdhj6JKmHjoRTB__, LV_PART_MAIN | LV_STATE_DEFAULT );
     lv_obj_set_layout( GUI_Container__screen__DataContainer, LV_LAYOUT_NONE );

      lv_obj_add_style( GUI_Container__screen__moistureContainer, &GUI_Style__class_JqUcMs1ZNPRfQq__, LV_PART_MAIN | LV_STATE_DEFAULT );

       lv_image_set_src( GUI_Image__screen__moistureImg, &pump );

       lv_obj_add_style( GUI_Label__screen__moisture, &GUI_Style__class_MsKIDbZdEXuzIO__, LV_PART_MAIN | LV_STATE_DEFAULT );

      lv_obj_add_style( GUI_Label__screen__currentTemperature, &GUI_Style__class_0TWSkus0ZPqgtj__, LV_PART_MAIN | LV_STATE_DEFAULT );

      lv_obj_add_style( GUI_Label__screen__setTemperature, &GUI_Style__class_VRI9xFdITfSk2x__, LV_PART_MAIN | LV_STATE_DEFAULT );

      lv_obj_add_style( GUI_Label__screen__set_to, &GUI_Style__class_1nSloISkFO20jp__, LV_PART_MAIN | LV_STATE_DEFAULT );

     lv_obj_add_style( GUI_Container__screen__controlleersContainer, &GUI_Style__class_Mjy6X8hngx7TUa__, LV_PART_MAIN | LV_STATE_DEFAULT );

      lv_image_set_src( GUI_Image__screen__moodImg, &sun );

      lv_image_set_src( GUI_Image__screen__fanImg, &fan );

      lv_image_set_src( GUI_Image__screen__pumpImg, &pump );

     lv_image_set_src( GUI_Image__screen__powerImg, &power );

}

void GUI_initScreen__wifi () {
    GUI_Screen__wifi = lv_obj_create( NULL );
    lv_obj_remove_flag( GUI_Screen__wifi, LV_OBJ_FLAG_SCROLLABLE );

    GUI_Button__wifi__backBtn = lv_btn_create( GUI_Screen__wifi );
    lv_obj_set_align( GUI_Button__wifi__backBtn, LV_ALIGN_TOP_LEFT );
    lv_obj_set_pos( GUI_Button__wifi__backBtn, 10, 8 );
    lv_obj_set_size( GUI_Button__wifi__backBtn, 36, 28 );
    lv_obj_add_event_cb( GUI_Button__wifi__backBtn, GUI_event__Button__wifi__backBtn__Clicked, LV_EVENT_CLICKED, NULL );

    GUI_Label__wifi__backLabel = lv_label_create( GUI_Button__wifi__backBtn );
    lv_label_set_text( GUI_Label__wifi__backLabel, "<" );
    lv_obj_center( GUI_Label__wifi__backLabel );

    GUI_Label__wifi__title = lv_label_create( GUI_Screen__wifi );
    lv_label_set_text( GUI_Label__wifi__title, "Wi-Fi Setup" );
    lv_obj_set_align( GUI_Label__wifi__title, LV_ALIGN_TOP_MID );
    lv_obj_set_pos( GUI_Label__wifi__title, 0, 12 );

    GUI_TextArea__wifi__ssid = lv_textarea_create( GUI_Screen__wifi );
    lv_obj_set_align( GUI_TextArea__wifi__ssid, LV_ALIGN_TOP_MID );
    lv_obj_set_pos( GUI_TextArea__wifi__ssid, 0, 50 );
    lv_obj_set_size( GUI_TextArea__wifi__ssid, 220, 32 );
    lv_textarea_set_placeholder_text( GUI_TextArea__wifi__ssid, "SSID" );
    lv_textarea_set_one_line( GUI_TextArea__wifi__ssid, true );
    lv_obj_add_event_cb( GUI_TextArea__wifi__ssid, GUI_event__TextArea__wifi__ssid__Focused, LV_EVENT_FOCUSED, NULL );

    GUI_TextArea__wifi__pass = lv_textarea_create( GUI_Screen__wifi );
    lv_obj_set_align( GUI_TextArea__wifi__pass, LV_ALIGN_TOP_MID );
    lv_obj_set_pos( GUI_TextArea__wifi__pass, 0, 90 );
    lv_obj_set_size( GUI_TextArea__wifi__pass, 220, 32 );
    lv_textarea_set_placeholder_text( GUI_TextArea__wifi__pass, "Password" );
    lv_textarea_set_one_line( GUI_TextArea__wifi__pass, true );
    lv_textarea_set_password_mode( GUI_TextArea__wifi__pass, true );
    lv_obj_add_event_cb( GUI_TextArea__wifi__pass, GUI_event__TextArea__wifi__pass__Focused, LV_EVENT_FOCUSED, NULL );

    GUI_Button__wifi__connectBtn = lv_btn_create( GUI_Screen__wifi );
    lv_obj_set_align( GUI_Button__wifi__connectBtn, LV_ALIGN_TOP_MID );
    lv_obj_set_pos( GUI_Button__wifi__connectBtn, 0, 130 );
    lv_obj_set_size( GUI_Button__wifi__connectBtn, 140, 32 );
    lv_obj_add_event_cb( GUI_Button__wifi__connectBtn, GUI_event__Button__wifi__connectBtn__Clicked, LV_EVENT_CLICKED, NULL );

    GUI_Label__wifi__connectLabel = lv_label_create( GUI_Button__wifi__connectBtn );
    lv_label_set_text( GUI_Label__wifi__connectLabel, "Connect" );
    lv_obj_center( GUI_Label__wifi__connectLabel );

    GUI_Button__wifi__clearBtn = lv_btn_create( GUI_Screen__wifi );
    lv_obj_set_align( GUI_Button__wifi__clearBtn, LV_ALIGN_TOP_MID );
    lv_obj_set_pos( GUI_Button__wifi__clearBtn, 0, 168 );
    lv_obj_set_size( GUI_Button__wifi__clearBtn, 140, 28 );
    lv_obj_add_event_cb( GUI_Button__wifi__clearBtn, GUI_event__Button__wifi__clearBtn__Clicked, LV_EVENT_CLICKED, NULL );

    GUI_Label__wifi__clearLabel = lv_label_create( GUI_Button__wifi__clearBtn );
    lv_label_set_text( GUI_Label__wifi__clearLabel, "Clear Saved" );
    lv_obj_center( GUI_Label__wifi__clearLabel );

    GUI_Label__wifi__status = lv_label_create( GUI_Screen__wifi );
    lv_label_set_text( GUI_Label__wifi__status, "Status: idle" );
    lv_obj_set_align( GUI_Label__wifi__status, LV_ALIGN_TOP_MID );
    lv_obj_set_pos( GUI_Label__wifi__status, 0, 202 );

    GUI_Keyboard__wifi__keyboard = lv_keyboard_create( GUI_Screen__wifi );
    lv_obj_set_size( GUI_Keyboard__wifi__keyboard, 320, 120 );
    lv_obj_set_align( GUI_Keyboard__wifi__keyboard, LV_ALIGN_BOTTOM_MID );
    lv_obj_add_flag( GUI_Keyboard__wifi__keyboard, LV_OBJ_FLAG_HIDDEN );
}

void GUI_initScreenTexts__wifi () {
}

void GUI_initScreenStyles__wifi () {
}
