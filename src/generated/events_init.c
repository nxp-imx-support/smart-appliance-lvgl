/*
* Copyright 2024 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "lvgl.h"
#include "matter/log_parse.h"

extern bool g_ui_camera;
extern bool light_ctl_flag;
extern FILE *matter_handle;

static void screen_btn_light_on_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);

	switch (code) {
	case LV_EVENT_CLICKED:
	{
		if(matter_handle == NULL) 
		return;

		if (light_ctl_flag == true) {
			light_ctl_flag = false;
			input_cmd("onoff off 1234 1", matter_handle);
		} else {
			light_ctl_flag = true;
			input_cmd("onoff on 1234 1", matter_handle);
		}
		events_ui_sync(light_ctl_flag);
		break;
	}
	default:
		break;
	}
}

static void screen_btn_food_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);

	switch (code) {
	case LV_EVENT_CLICKED:
	{
		//Write the load screen code.
		lv_obj_t * act_scr = lv_scr_act();
		lv_disp_t * d = lv_obj_get_disp(act_scr);
		if (d->prev_scr == NULL &&
				(d->scr_to_load == NULL || d->scr_to_load == act_scr)) {
			lv_scr_load_anim(guider_ui.camera,
				LV_SCR_LOAD_ANIM_NONE, 200, 200, false);
			g_ui_camera = true;
		}
		break;
	}
	default:
		break;
	}
}

static void screen_btn_standby_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	switch (code) {
	case LV_EVENT_CLICKED:
	{
		/* call to suspend */
		int fd;
		fd = open("/sys/power/state", O_RDWR);
		if (fd > 0) {
			write(fd, "mem", 3);
			close(fd);
		}
		break;
	}
	default:
		break;
	}
}

void events_init_screen(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->screen_btn_light_on, screen_btn_light_on_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_btn_food, screen_btn_food_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_btn_standby, screen_btn_standby_event_handler, LV_EVENT_ALL, NULL);
}

static void camera_btn_back_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);

	switch (code) {
	case LV_EVENT_CLICKED:
	{
		//Write the load screen code.
		lv_obj_t * act_scr = lv_scr_act();
		lv_disp_t * d = lv_obj_get_disp(act_scr);
		if (d->prev_scr == NULL &&
				(d->scr_to_load == NULL || d->scr_to_load == act_scr)) {
			g_ui_camera = false;
			lv_scr_load_anim(guider_ui.screen,
				LV_SCR_LOAD_ANIM_NONE, 200, 200, false);
		}
		break;
	}
	default:
		break;
	}
}
void events_init_camera(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->camera_btn_back, camera_btn_back_event_handler, LV_EVENT_ALL, NULL);
}

void events_init(lv_ui *ui)
{

}

void events_ui_sync(bool status)
{
	if (status == false) {
		lv_obj_set_style_bg_img_src(
			guider_ui.screen_btn_light_on,
				&_img_light_off_alpha_230x160,
				LV_PART_MAIN|LV_STATE_DEFAULT);
	} else {
		lv_obj_set_style_bg_img_src(
			guider_ui.screen_btn_light_on,
			&_img_light_on_230x160,
			LV_PART_MAIN|LV_STATE_DEFAULT);
	}
}
