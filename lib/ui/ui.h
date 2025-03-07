// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.1
// LVGL version: 8.3.11
// Project name: MASHARE

#ifndef _MASHARE_UI_H
#define _MASHARE_UI_H

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
#include "ui_events.h"

// SCREEN: ui_nowifi
void ui_nowifi_screen_init(void);
extern lv_obj_t *ui_nowifi;
extern lv_obj_t *ui_WIFIKEYBOARD;
extern lv_obj_t *ui_SSID;
extern lv_obj_t *ui_PASSWORD;
extern lv_obj_t *ui_SSIDLABEL;
extern lv_obj_t *ui_PASSWORDLABEL;
extern lv_obj_t *ui_CONNECTBUTTON;  

// SCREEN: ui_home
void ui_home_screen_init(void);
extern lv_obj_t *ui_home;
extern lv_obj_t *ui_WeatherPanel;
extern lv_obj_t *ui_DATELABEL;
extern lv_obj_t *ui_CONDITIONLABEL;
extern lv_obj_t *ui_TIMELABEL;
extern lv_obj_t *ui_WEATHERICON;
extern lv_obj_t *ui_PlantWaterPanel;
extern lv_obj_t *ui_PLANT1LABEL;
extern lv_obj_t *ui_PLANT2LABEL;
extern lv_obj_t *ui_PLANT3LABEL;
extern lv_obj_t *ui_PLANT4LABEL;
extern lv_obj_t *ui_PLANT1BAR;
extern lv_obj_t *ui_PLANT2BAR;
extern lv_obj_t *ui_PLANT3BAR;
extern lv_obj_t *ui_PLANT4BAR;
extern lv_obj_t *ui_PLANTICON;
extern lv_obj_t *ui_PLANTICON1;
extern lv_obj_t *ui_PLANTICON2;
extern lv_obj_t *ui_PLANTICON3;
extern lv_obj_t *ui_TempPanel;
extern lv_obj_t *ui_BEDROOMLABEL;
extern lv_obj_t *ui_LIVINGROOMLABEL;
extern lv_obj_t *ui_DOGFOODLABEL;
extern lv_obj_t *ui_LRTEMPBAR;
extern lv_obj_t *ui_LRHUMBAR;
extern lv_obj_t *ui_DFLEVELBAR;
extern lv_obj_t *ui_BRHUMBAR;
extern lv_obj_t *ui_BRTEMPBAR;
extern lv_obj_t *ui_DOGICON;
extern lv_obj_t *ui_TEMPICON;
extern lv_obj_t *ui_HUMIDICON;
extern lv_obj_t *ui_TEMPICON1;
extern lv_obj_t *ui_HUMIDICON1;
extern lv_obj_t *ui____initial_actions0;

LV_IMG_DECLARE( ui_img_1336096536);   // assets/icons8-rain-cloud-100.png
LV_IMG_DECLARE( ui_img_1503224665);   // assets/icons8-plant-50.png
LV_IMG_DECLARE( ui_img_1288332288);   // assets/icons8-dog-50.png
LV_IMG_DECLARE( ui_img_1313047014);   // assets/icons8-thermometer-25.png
LV_IMG_DECLARE( ui_img_1991736760);   // assets/icons8-hygrometer-25.png
LV_IMG_DECLARE( ui_img_1276940811);   // assets/icons8-dog-bone-50.png


LV_FONT_DECLARE( ui_font_BebasNeue24);
LV_FONT_DECLARE( ui_font_Heavitas26);
LV_FONT_DECLARE( ui_font_LEMONMILK26);


void ui_init(void);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
