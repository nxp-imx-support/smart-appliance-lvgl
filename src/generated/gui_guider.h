/*
* Copyright 2024 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef struct
{
  
	lv_obj_t *screen;
	lv_obj_t *screen_img_fridge;
	lv_obj_t *screen_label_Date;
	lv_obj_t *screen_label_Time;
	lv_obj_t *screen_btn_temp;
	lv_obj_t *screen_btn_temp_label;
	lv_obj_t *screen_btn_light_on;
	lv_obj_t *screen_btn_light_on_label;
	lv_obj_t *screen_img_nxp;
	lv_obj_t *screen_label_Welcome;
	lv_obj_t *screen_btn_food;
	lv_obj_t *screen_btn_food_label;
	lv_obj_t *screen_btn_standby;
	lv_obj_t *screen_btn_standby_label;
	lv_obj_t *screen_img_home;
	lv_obj_t *screen_label_weather;
	lv_obj_t *screen_img_light_off;
	lv_obj_t *camera;
	lv_obj_t *camera_btn_back;
	lv_obj_t *camera_btn_back_label;
	lv_obj_t *camera_img_display;
	lv_obj_t *camera_canvas_boxes;
	lv_obj_t *camera_img_logo;
}lv_ui;

void ui_init_style(lv_style_t * style);
void init_scr_del_flag(lv_ui *ui);
void setup_ui(lv_ui *ui);
extern lv_ui guider_ui;

void setup_scr_screen(lv_ui *ui);
void setup_scr_camera(lv_ui *ui);
LV_IMG_DECLARE(_bg_alpha_206x195);

LV_IMG_DECLARE(_btn_temp_230x160);

LV_IMG_DECLARE(_img_light_on_230x160);
LV_IMG_DECLARE(_nxp_alpha_100x50);

LV_IMG_DECLARE(_food_camera_230x160);

LV_IMG_DECLARE(_standby_230x160);
LV_IMG_DECLARE(_img_home_alpha_50x50);
LV_IMG_DECLARE(_img_light_off_alpha_230x160);

LV_IMG_DECLARE(_back_button_110x88);
LV_IMG_DECLARE(__alpha_640x480);
LV_IMG_DECLARE(_nxp_alpha_120x60);

LV_FONT_DECLARE(lv_font_montserratMedium_20)
LV_FONT_DECLARE(lv_font_montserratMedium_16)
LV_FONT_DECLARE(lv_font_montserratMedium_60)
LV_FONT_DECLARE(lv_font_montserratMedium_45)
LV_FONT_DECLARE(lv_font_montserratMedium_12)
LV_FONT_DECLARE(lv_font_Alatsi_Regular_26)
LV_FONT_DECLARE(lv_font_montserratMedium_26)
LV_FONT_DECLARE(lv_font_Alatsi_Regular_25)
LV_FONT_DECLARE(lv_font_montserratMedium_23)


#ifdef __cplusplus
}
#endif
#endif
