/*
* Copyright 2024 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"


void setup_scr_screen(lv_ui *ui)
{
	//Write codes screen
	ui->screen = lv_obj_create(NULL);
	lv_obj_set_size(ui->screen, 800, 480);

	//Write style for screen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_img_fridge
	ui->screen_img_fridge = lv_img_create(ui->screen);
	lv_obj_add_flag(ui->screen_img_fridge, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_img_fridge, &_bg_alpha_206x195);
	lv_img_set_pivot(ui->screen_img_fridge, 50,50);
	lv_img_set_angle(ui->screen_img_fridge, 0);
	lv_obj_set_pos(ui->screen_img_fridge, 566, 253);
	lv_obj_set_size(ui->screen_img_fridge, 206, 195);

	//Write style for screen_img_fridge, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->screen_img_fridge, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_label_Date
	ui->screen_label_Date = lv_label_create(ui->screen);
	lv_label_set_text(ui->screen_label_Date, "\nMonday, June 30");
	lv_label_set_long_mode(ui->screen_label_Date, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->screen_label_Date, 566, 15);
	lv_obj_set_size(ui->screen_label_Date, 206, 221);

	//Write style for screen_label_Date, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_label_Date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_label_Date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_label_Date, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_label_Date, &lv_font_montserratMedium_20, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_label_Date, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_label_Date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_label_Date, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_label_Date, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_label_Date, lv_color_hex(0x262626), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_label_Date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_label_Date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_label_Date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_label_Date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_label_Date, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_label_Time
	ui->screen_label_Time = lv_label_create(ui->screen);
	lv_label_set_text(ui->screen_label_Time, "16:00");
	lv_label_set_long_mode(ui->screen_label_Time, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->screen_label_Time, 573, 62);
	lv_obj_set_size(ui->screen_label_Time, 192, 76);

	//Write style for screen_label_Time, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_label_Time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_label_Time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_label_Time, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_label_Time, &lv_font_montserratMedium_60, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_label_Time, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_label_Time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_label_Time, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_label_Time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_label_Time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_label_Time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_label_Time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_label_Time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_label_Time, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_btn_temp
	ui->screen_btn_temp = lv_btn_create(ui->screen);
	ui->screen_btn_temp_label = lv_label_create(ui->screen_btn_temp);
	lv_label_set_text(ui->screen_btn_temp_label, "0.0°C");
	lv_label_set_long_mode(ui->screen_btn_temp_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(ui->screen_btn_temp_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_btn_temp, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_btn_temp, 38, 107);
	lv_obj_set_size(ui->screen_btn_temp, 230, 160);

	//Write style for screen_btn_temp, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_btn_temp, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_btn_temp, lv_color_hex(0x262626), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_temp, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_btn_temp, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_btn_temp, &_btn_temp_230x160, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_temp, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_btn_temp, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_btn_temp, &lv_font_montserratMedium_45, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_temp, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_btn_light_on
	ui->screen_btn_light_on = lv_btn_create(ui->screen);
	ui->screen_btn_light_on_label = lv_label_create(ui->screen_btn_light_on);
	lv_label_set_text(ui->screen_btn_light_on_label, "          \n         \n        Light");
	lv_label_set_long_mode(ui->screen_btn_light_on_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(ui->screen_btn_light_on_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_btn_light_on, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_btn_light_on, 38, 285);
	lv_obj_set_size(ui->screen_btn_light_on, 230, 160);

	//Write style for screen_btn_light_on, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_btn_light_on, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_btn_light_on, lv_color_hex(0x262626), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_light_on, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_light_on, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_btn_light_on, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_btn_light_on, &_img_light_on_230x160, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_light_on, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_btn_light_on, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_btn_light_on, &lv_font_Alatsi_Regular_26, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_light_on, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_img_nxp
	ui->screen_img_nxp = lv_img_create(ui->screen);
	lv_obj_add_flag(ui->screen_img_nxp, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_img_nxp, &_nxp_alpha_100x50);
	lv_img_set_pivot(ui->screen_img_nxp, 50,50);
	lv_img_set_angle(ui->screen_img_nxp, 0);
	lv_obj_set_pos(ui->screen_img_nxp, 28, 30);
	lv_obj_set_size(ui->screen_img_nxp, 100, 50);

	//Write style for screen_img_nxp, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->screen_img_nxp, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_label_Welcome
	ui->screen_label_Welcome = lv_label_create(ui->screen);
	lv_label_set_text(ui->screen_label_Welcome, "Welcome back home");
	lv_label_set_long_mode(ui->screen_label_Welcome, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->screen_label_Welcome, 128, 45);
	lv_obj_set_size(ui->screen_label_Welcome, 326, 31);

	//Write style for screen_label_Welcome, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_label_Welcome, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_label_Welcome, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_label_Welcome, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_label_Welcome, &lv_font_montserratMedium_26, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_label_Welcome, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_label_Welcome, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_label_Welcome, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_label_Welcome, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_label_Welcome, lv_color_hex(0x262626), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_label_Welcome, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_label_Welcome, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_label_Welcome, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_label_Welcome, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_label_Welcome, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_btn_food
	ui->screen_btn_food = lv_btn_create(ui->screen);
	ui->screen_btn_food_label = lv_label_create(ui->screen_btn_food);
	lv_label_set_text(ui->screen_btn_food_label, "      \n\n      Food");
	lv_label_set_long_mode(ui->screen_btn_food_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(ui->screen_btn_food_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_btn_food, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_btn_food, 285, 107);
	lv_obj_set_size(ui->screen_btn_food, 230, 160);

	//Write style for screen_btn_food, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_btn_food, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_btn_food, lv_color_hex(0x262626), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_food, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_food, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_btn_food, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_btn_food, &_food_camera_230x160, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_food, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_btn_food, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_btn_food, &lv_font_Alatsi_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_food, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_btn_standby
	ui->screen_btn_standby = lv_btn_create(ui->screen);
	ui->screen_btn_standby_label = lv_label_create(ui->screen_btn_standby);
	lv_label_set_text(ui->screen_btn_standby_label, "       \n\n      Standby");
	lv_label_set_long_mode(ui->screen_btn_standby_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(ui->screen_btn_standby_label, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_set_style_pad_all(ui->screen_btn_standby, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->screen_btn_standby, 285, 285);
	lv_obj_set_size(ui->screen_btn_standby, 230, 160);

	//Write style for screen_btn_standby, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->screen_btn_standby, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->screen_btn_standby, lv_color_hex(0x262626), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_dir(ui->screen_btn_standby, LV_GRAD_DIR_HOR, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_color(ui->screen_btn_standby, lv_color_hex(0x888585), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->screen_btn_standby, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_btn_standby, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_btn_standby, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->screen_btn_standby, &_standby_230x160, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->screen_btn_standby, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_btn_standby, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_btn_standby, &lv_font_Alatsi_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_btn_standby, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_img_home
	ui->screen_img_home = lv_img_create(ui->screen);
	lv_obj_add_flag(ui->screen_img_home, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_img_home, &_img_home_alpha_50x50);
	lv_img_set_pivot(ui->screen_img_home, 50,50);
	lv_img_set_angle(ui->screen_img_home, 0);
	lv_obj_set_pos(ui->screen_img_home, 465, 32);
	lv_obj_set_size(ui->screen_img_home, 50, 50);

	//Write style for screen_img_home, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_recolor_opa(ui->screen_img_home, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_recolor(ui->screen_img_home, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_img_opa(ui->screen_img_home, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_label_weather
	ui->screen_label_weather = lv_label_create(ui->screen);
	lv_label_set_text(ui->screen_label_weather, "Cloudy\n\n26°C");
	lv_label_set_long_mode(ui->screen_label_weather, LV_LABEL_LONG_WRAP);
	lv_obj_set_pos(ui->screen_label_weather, 566, 135);
	lv_obj_set_size(ui->screen_label_weather, 206, 100);

	//Write style for screen_label_weather, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_border_width(ui->screen_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->screen_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->screen_label_weather, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->screen_label_weather, &lv_font_montserratMedium_23, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_letter_space(ui->screen_label_weather, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_line_space(ui->screen_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->screen_label_weather, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(ui->screen_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(ui->screen_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(ui->screen_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(ui->screen_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(ui->screen_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->screen_label_weather, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes screen_img_light_off
	ui->screen_img_light_off = lv_img_create(ui->screen);
	lv_obj_add_flag(ui->screen_img_light_off, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_img_light_off, &_img_light_off_alpha_230x160);
	lv_img_set_pivot(ui->screen_img_light_off, 50,50);
	lv_img_set_angle(ui->screen_img_light_off, 0);
	lv_obj_set_pos(ui->screen_img_light_off, 38, 285);
	lv_obj_set_size(ui->screen_img_light_off, 230, 160);
	lv_obj_add_flag(ui->screen_img_light_off, LV_OBJ_FLAG_HIDDEN);

	//Write style for screen_img_light_off, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->screen_img_light_off, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Update current screen layout.
	lv_obj_update_layout(ui->screen);

	//Init events for screen.
	events_init_screen(ui);
}
