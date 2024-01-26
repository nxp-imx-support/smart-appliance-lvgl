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


void setup_scr_camera(lv_ui *ui)
{
	//Write codes camera
	ui->camera = lv_obj_create(NULL);
	lv_obj_set_size(ui->camera, 800, 480);

	//Write style for camera, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->camera, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->camera, lv_color_hex(0x262626), LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes camera_btn_back
	ui->camera_btn_back = lv_btn_create(ui->camera);
	ui->camera_btn_back_label = lv_label_create(ui->camera_btn_back);
	lv_label_set_text(ui->camera_btn_back_label, "");
	lv_label_set_long_mode(ui->camera_btn_back_label, LV_LABEL_LONG_WRAP);
	lv_obj_align(ui->camera_btn_back_label, LV_ALIGN_CENTER, 0, 0);
	lv_obj_set_style_pad_all(ui->camera_btn_back, 0, LV_STATE_DEFAULT);
	lv_obj_set_pos(ui->camera_btn_back, 2, 12);
	lv_obj_set_size(ui->camera_btn_back, 110, 88);

	//Write style for camera_btn_back, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_bg_opa(ui->camera_btn_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(ui->camera_btn_back, lv_color_hex(0x262626), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(ui->camera_btn_back, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_radius(ui->camera_btn_back, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_shadow_width(ui->camera_btn_back, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(ui->camera_btn_back, &_back_button_110x88, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_opa(ui->camera_btn_back, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_color(ui->camera_btn_back, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_font(ui->camera_btn_back, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_obj_set_style_text_align(ui->camera_btn_back, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes camera_img_display
	ui->camera_img_display = lv_img_create(ui->camera);
	lv_obj_add_flag(ui->camera_img_display, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_pivot(ui->camera_img_display, 50,50);
	lv_img_set_angle(ui->camera_img_display, 0);
	lv_obj_set_pos(ui->camera_img_display, 116, 0);
	lv_obj_set_size(ui->camera_img_display, 640, 480);

	//Write style for camera_img_display, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->camera_img_display, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write codes camera_canvas_boxes
	ui->camera_canvas_boxes = lv_canvas_create(ui->camera);
	static lv_color_t buf_camera_canvas_boxes[640*480*4];
	lv_canvas_set_buffer(ui->camera_canvas_boxes, buf_camera_canvas_boxes, 640, 480, LV_IMG_CF_TRUE_COLOR_ALPHA);
	lv_canvas_fill_bg(ui->camera_canvas_boxes, lv_color_hex(0xffffff), 0);
	lv_obj_set_pos(ui->camera_canvas_boxes, 116, 0);
	lv_obj_set_size(ui->camera_canvas_boxes, 640, 480);
	lv_obj_set_scrollbar_mode(ui->camera_canvas_boxes, LV_SCROLLBAR_MODE_OFF);

	//Write codes camera_img_logo
	ui->camera_img_logo = lv_img_create(ui->camera);
	lv_obj_add_flag(ui->camera_img_logo, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->camera_img_logo, &_nxp_alpha_120x60);
	lv_img_set_pivot(ui->camera_img_logo, 50,50);
	lv_img_set_angle(ui->camera_img_logo, 0);
	lv_obj_set_pos(ui->camera_img_logo, 636, 25);
	lv_obj_set_size(ui->camera_img_logo, 120, 60);

	//Write style for camera_img_logo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
	lv_obj_set_style_img_opa(ui->camera_img_logo, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Update current screen layout.
	lv_obj_update_layout(ui->camera);

	
	//Init events for screen.
	events_init_camera(ui);
}
