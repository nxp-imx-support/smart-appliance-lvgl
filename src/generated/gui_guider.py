# Copyright 2024 NXP
# NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
# accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
# activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
# comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
# terms, then you may not retain, install, activate or otherwise use the software.

import SDL
import utime as time
import usys as sys
import lvgl as lv
import lodepng as png
import ustruct
import fs_driver

lv.init()
SDL.init(w=800,h=480)

# Register SDL display driver.
disp_buf1 = lv.disp_draw_buf_t()
buf1_1 = bytearray(800*10)
disp_buf1.init(buf1_1, None, len(buf1_1)//4)
disp_drv = lv.disp_drv_t()
disp_drv.init()
disp_drv.draw_buf = disp_buf1
disp_drv.flush_cb = SDL.monitor_flush
disp_drv.hor_res = 800
disp_drv.ver_res = 480
disp_drv.register()

# Regsiter SDL mouse driver
indev_drv = lv.indev_drv_t()
indev_drv.init()
indev_drv.type = lv.INDEV_TYPE.POINTER
indev_drv.read_cb = SDL.mouse_read
indev_drv.register()

fs_drv = lv.fs_drv_t()
fs_driver.fs_register(fs_drv, 'Z')

# Below: Taken from https://github.com/lvgl/lv_binding_micropython/blob/master/driver/js/imagetools.py#L22-L94

COLOR_SIZE = lv.color_t.__SIZE__
COLOR_IS_SWAPPED = hasattr(lv.color_t().ch,'green_h')

class lodepng_error(RuntimeError):
    def __init__(self, err):
        if type(err) is int:
            super().__init__(png.error_text(err))
        else:
            super().__init__(err)

# Parse PNG file header
# Taken from https://github.com/shibukawa/imagesize_py/blob/ffef30c1a4715c5acf90e8945ceb77f4a2ed2d45/imagesize.py#L63-L85

def get_png_info(decoder, src, header):
    # Only handle variable image types

    if lv.img.src_get_type(src) != lv.img.SRC.VARIABLE:
        return lv.RES.INV

    data = lv.img_dsc_t.__cast__(src).data
    if data == None:
        return lv.RES.INV

    png_header = bytes(data.__dereference__(24))

    if png_header.startswith(b'\211PNG\r\n\032\n'):
        if png_header[12:16] == b'IHDR':
            start = 16
        # Maybe this is for an older PNG version.
        else:
            start = 8
        try:
            width, height = ustruct.unpack(">LL", png_header[start:start+8])
        except ustruct.error:
            return lv.RES.INV
    else:
        return lv.RES.INV

    header.always_zero = 0
    header.w = width
    header.h = height
    header.cf = lv.img.CF.TRUE_COLOR_ALPHA

    return lv.RES.OK

def convert_rgba8888_to_bgra8888(img_view):
    for i in range(0, len(img_view), lv.color_t.__SIZE__):
        ch = lv.color_t.__cast__(img_view[i:i]).ch
        ch.red, ch.blue = ch.blue, ch.red

# Read and parse PNG file

def open_png(decoder, dsc):
    img_dsc = lv.img_dsc_t.__cast__(dsc.src)
    png_data = img_dsc.data
    png_size = img_dsc.data_size
    png_decoded = png.C_Pointer()
    png_width = png.C_Pointer()
    png_height = png.C_Pointer()
    error = png.decode32(png_decoded, png_width, png_height, png_data, png_size)
    if error:
        raise lodepng_error(error)
    img_size = png_width.int_val * png_height.int_val * 4
    img_data = png_decoded.ptr_val
    img_view = img_data.__dereference__(img_size)

    if COLOR_SIZE == 4:
        convert_rgba8888_to_bgra8888(img_view)
    else:
        raise lodepng_error("Error: Color mode not supported yet!")

    dsc.img_data = img_data
    return lv.RES.OK

# Above: Taken from https://github.com/lvgl/lv_binding_micropython/blob/master/driver/js/imagetools.py#L22-L94

decoder = lv.img.decoder_create()
decoder.info_cb = get_png_info
decoder.open_cb = open_png

def anim_x_cb(obj, v):
    obj.set_x(v)

def anim_y_cb(obj, v):
    obj.set_y(v)

def anim_width_cb(obj, v):
    obj.set_width(v)

def anim_height_cb(obj, v):
    obj.set_height(v)

def anim_img_zoom_cb(obj, v):
    obj.set_zoom(v)

def anim_img_rotate_cb(obj, v):
    obj.set_angle(v)

global_font_cache = {}
def test_font(font_family, font_size):
    global global_font_cache
    if font_family + str(font_size) in global_font_cache:
        return global_font_cache[font_family + str(font_size)]
    if font_size % 2:
        candidates = [
            (font_family, font_size),
            (font_family, font_size-font_size%2),
            (font_family, font_size+font_size%2),
            ("montserrat", font_size-font_size%2),
            ("montserrat", font_size+font_size%2),
            ("montserrat", 16)
        ]
    else:
        candidates = [
            (font_family, font_size),
            ("montserrat", font_size),
            ("montserrat", 16)
        ]
    for (family, size) in candidates:
        try:
            if eval(f'lv.font_{family}_{size}'):
                global_font_cache[font_family + str(font_size)] = eval(f'lv.font_{family}_{size}')
                if family != font_family or size != font_size:
                    print(f'WARNING: lv.font_{family}_{size} is used!')
                return eval(f'lv.font_{family}_{size}')
        except AttributeError:
            try:
                load_font = lv.font_load(f"Z:MicroPython/lv_font_{family}_{size}.fnt")
                global_font_cache[font_family + str(font_size)] = load_font
                return load_font
            except:
                if family == font_family and size == font_size:
                    print(f'WARNING: lv.font_{family}_{size} is NOT supported!')

global_image_cache = {}
def load_image(file):
    global global_image_cache
    if file in global_image_cache:
        return global_image_cache[file]
    try:
        with open(file,'rb') as f:
            data = f.read()
    except:
        print(f'Could not open {file}')
        sys.exit()

    img = lv.img_dsc_t({
        'data_size': len(data),
        'data': data
    })
    global_image_cache[file] = img
    return img

def calendar_event_handler(e,obj):
    code = e.get_code()

    if code == lv.EVENT.VALUE_CHANGED:
        source = e.get_current_target()
        date = lv.calendar_date_t()
        if source.get_pressed_date(date) == lv.RES.OK:
            source.set_highlighted_dates([date], 1)

def spinbox_increment_event_cb(e, obj):
    code = e.get_code()
    if code == lv.EVENT.SHORT_CLICKED or code == lv.EVENT.LONG_PRESSED_REPEAT:
        obj.increment()
def spinbox_decrement_event_cb(e, obj):
    code = e.get_code()
    if code == lv.EVENT.SHORT_CLICKED or code == lv.EVENT.LONG_PRESSED_REPEAT:
        obj.decrement()

def digital_clock_cb(timer, obj, current_time, show_second, use_ampm):
    hour = int(current_time[0])
    minute = int(current_time[1])
    second = int(current_time[2])
    ampm = current_time[3]
    second = second + 1
    if second == 60:
        second = 0
        minute = minute + 1
        if minute == 60:
            minute = 0
            hour = hour + 1
            if use_ampm:
                if hour == 12:
                    if ampm == 'AM':
                        ampm = 'PM'
                    elif ampm == 'PM':
                        ampm = 'AM'
                if hour > 12:
                    hour = hour % 12
    hour = hour % 24
    if use_ampm:
        if show_second:
            obj.set_text("%02d:%02d:%02d %s" %(hour, minute, second, ampm))
        else:
            obj.set_text("%02d:%02d %s" %(hour, minute, ampm))
    else:
        if show_second:
            obj.set_text("%02d:%02d:%02d" %(hour, minute, second))
        else:
            obj.set_text("%02d:%02d" %(hour, minute))
    current_time[0] = hour
    current_time[1] = minute
    current_time[2] = second
    current_time[3] = ampm

def analog_clock_cb(timer, obj):
    datetime = time.localtime()
    hour = datetime[3]
    if hour >= 12: hour = hour - 12
    obj.set_time(hour, datetime[4], datetime[5])

def datetext_event_handler(e, obj):
    code = e.get_code()
    target = e.get_target()
    if code == lv.EVENT.FOCUSED:
        if obj is None:
            bg = lv.layer_top()
            bg.add_flag(lv.obj.FLAG.CLICKABLE)
            obj = lv.calendar(bg)
            scr = target.get_screen()
            scr_height = scr.get_height()
            scr_width = scr.get_width()
            obj.set_size(int(scr_width * 0.8), int(scr_height * 0.8))
            datestring = target.get_text()
            year = int(datestring.split('/')[0])
            month = int(datestring.split('/')[1])
            day = int(datestring.split('/')[2])
            obj.set_showed_date(year, month)
            highlighted_days=[lv.calendar_date_t({'year':year, 'month':month, 'day':day})]
            obj.set_highlighted_dates(highlighted_days, 1)
            obj.align(lv.ALIGN.CENTER, 0, 0)
            lv.calendar_header_arrow(obj)
            obj.add_event_cb(lambda e: datetext_calendar_event_handler(e, target), lv.EVENT.ALL, None)
            scr.update_layout()

def datetext_calendar_event_handler(e, obj):
    code = e.get_code()
    target = e.get_current_target()
    if code == lv.EVENT.VALUE_CHANGED:
        date = lv.calendar_date_t()
        if target.get_pressed_date(date) == lv.RES.OK:
            obj.set_text(f"{date.year}/{date.month}/{date.day}")
            bg = lv.layer_top()
            bg.clear_flag(lv.obj.FLAG.CLICKABLE)
            bg.set_style_bg_opa(lv.OPA.TRANSP, 0)
            target.delete()

def screen_ta_event_cb(e,kb):
    code = e.get_code()
    ta = e.get_target()
    if code == lv.EVENT.FOCUSED:
        kb.set_textarea(ta)
        kb.move_foreground()
        kb.clear_flag(lv.obj.FLAG.HIDDEN)

    if code == lv.EVENT.DEFOCUSED:
        kb.set_textarea(None)
        kb.move_background()
        kb.add_flag(lv.obj.FLAG.HIDDEN)

def camera_ta_event_cb(e,kb):
    code = e.get_code()
    ta = e.get_target()
    if code == lv.EVENT.FOCUSED:
        kb.set_textarea(ta)
        kb.move_foreground()
        kb.clear_flag(lv.obj.FLAG.HIDDEN)

    if code == lv.EVENT.DEFOCUSED:
        kb.set_textarea(None)
        kb.move_background()
        kb.add_flag(lv.obj.FLAG.HIDDEN)

# Create screen
screen = lv.obj()
screen.set_size(800, 480)
screen.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen.set_style_bg_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen.set_style_bg_color(lv.color_hex(0x000000), lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_img_fridge
screen_img_fridge = lv.img(screen)
screen_img_fridge.set_src(load_image(r"C:\NXP\GUI-Guider-Projects\SmartAPPLinux\generated\MicroPython\bg_206_195.png"))
screen_img_fridge.add_flag(lv.obj.FLAG.CLICKABLE)
screen_img_fridge.set_pivot(50,50)
screen_img_fridge.set_angle(0)
screen_img_fridge.set_pos(566, 253)
screen_img_fridge.set_size(206, 195)
screen_img_fridge.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_img_fridge, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_img_fridge.set_style_img_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_Date
screen_label_Date = lv.label(screen)
screen_label_Date.set_text("\nMonday, June 30")
screen_label_Date.set_long_mode(lv.label.LONG.WRAP)
screen_label_Date.set_pos(566, 15)
screen_label_Date.set_size(206, 221)
screen_label_Date.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_label_Date, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_Date.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_text_font(test_font("montserratMedium", 18), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_text_letter_space(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_bg_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_bg_color(lv.color_hex(0x262626), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Date.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_Time
screen_label_Time = lv.label(screen)
screen_label_Time.set_text("16:00")
screen_label_Time.set_long_mode(lv.label.LONG.WRAP)
screen_label_Time.set_pos(573, 62)
screen_label_Time.set_size(192, 76)
screen_label_Time.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_label_Time, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_Time.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Time.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Time.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Time.set_style_text_font(test_font("montserratMedium", 60), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Time.set_style_text_letter_space(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Time.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Time.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Time.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Time.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Time.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Time.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Time.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Time.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_btn_temperature
screen_btn_temperature = lv.btn(screen)
screen_btn_temperature_label = lv.label(screen_btn_temperature)
screen_btn_temperature_label.set_text("0.0ºC")
screen_btn_temperature_label.set_long_mode(lv.label.LONG.WRAP)
screen_btn_temperature_label.align(lv.ALIGN.CENTER, 0, 0)
screen_btn_temperature.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_btn_temperature.set_pos(38, 108)
screen_btn_temperature.set_size(230, 160)
screen_btn_temperature.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_btn_temperature, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_btn_temperature.set_style_bg_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_temperature.set_style_bg_color(lv.color_hex(0x262626), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_temperature.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_temperature.set_style_radius(5, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_temperature.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_temperature.set_style_bg_img_src(load_image(r"C:\NXP\GUI-Guider-Projects\SmartAPPLinux\generated\MicroPython\btn_bg_230_160.png"), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_temperature.set_style_bg_img_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_temperature.set_style_text_color(lv.color_hex(0xff9700), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_temperature.set_style_text_font(test_font("montserratMedium", 60), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_temperature.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_img_snow
screen_img_snow = lv.img(screen)
screen_img_snow.set_src(load_image(r"C:\NXP\GUI-Guider-Projects\SmartAPPLinux\generated\MicroPython\icn_cool_26_26.png"))
screen_img_snow.add_flag(lv.obj.FLAG.CLICKABLE)
screen_img_snow.set_pivot(50,50)
screen_img_snow.set_angle(0)
screen_img_snow.set_pos(72, 138)
screen_img_snow.set_size(26, 26)
screen_img_snow.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_img_snow, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_img_snow.set_style_img_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_btn_lamp
screen_btn_lamp = lv.btn(screen)
screen_btn_lamp_label = lv.label(screen_btn_lamp)
screen_btn_lamp_label.set_text("\n\n      Lamp")
screen_btn_lamp_label.set_long_mode(lv.label.LONG.WRAP)
screen_btn_lamp_label.align(lv.ALIGN.LEFT_MID, 0, 0)
screen_btn_lamp.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_btn_lamp.set_pos(38, 285)
screen_btn_lamp.set_size(230, 160)
screen_btn_lamp.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_btn_lamp, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_btn_lamp.set_style_bg_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_lamp.set_style_bg_color(lv.color_hex(0x262626), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_lamp.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_lamp.set_style_radius(5, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_lamp.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_lamp.set_style_bg_img_src(load_image(r"C:\NXP\GUI-Guider-Projects\SmartAPPLinux\generated\MicroPython\img_lamp_230_160.png"), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_lamp.set_style_bg_img_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_lamp.set_style_text_color(lv.color_hex(0x262626), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_lamp.set_style_text_font(test_font("Alatsi_Regular", 26), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_lamp.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_img_nxp1
screen_img_nxp1 = lv.img(screen)
screen_img_nxp1.set_src(load_image(r"C:\NXP\GUI-Guider-Projects\SmartAPPLinux\generated\MicroPython\nxp_100_50.png"))
screen_img_nxp1.add_flag(lv.obj.FLAG.CLICKABLE)
screen_img_nxp1.set_pivot(50,50)
screen_img_nxp1.set_angle(0)
screen_img_nxp1.set_pos(28, 30)
screen_img_nxp1.set_size(100, 50)
screen_img_nxp1.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_img_nxp1, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_img_nxp1.set_style_img_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_Welcome
screen_label_Welcome = lv.label(screen)
screen_label_Welcome.set_text("Welcome back home")
screen_label_Welcome.set_long_mode(lv.label.LONG.WRAP)
screen_label_Welcome.set_pos(128, 45)
screen_label_Welcome.set_size(326, 31)
screen_label_Welcome.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_label_Welcome, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_Welcome.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_text_font(test_font("montserratMedium", 26), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_text_letter_space(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_bg_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_bg_color(lv.color_hex(0x262626), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_Welcome.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_img_weather
screen_img_weather = lv.img(screen)
screen_img_weather.set_src(load_image(r"C:\NXP\GUI-Guider-Projects\SmartAPPLinux\generated\MicroPython\weather_80_60.png"))
screen_img_weather.add_flag(lv.obj.FLAG.CLICKABLE)
screen_img_weather.set_pivot(50,50)
screen_img_weather.set_angle(0)
screen_img_weather.set_pos(586, 156)
screen_img_weather.set_size(80, 60)
screen_img_weather.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_img_weather, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_img_weather.set_style_img_recolor_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_img_weather.set_style_img_recolor(lv.color_hex(0xffd800), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_img_weather.set_style_img_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_btn_camera
screen_btn_camera = lv.btn(screen)
screen_btn_camera_label = lv.label(screen_btn_camera)
screen_btn_camera_label.set_text("")
screen_btn_camera_label.set_long_mode(lv.label.LONG.WRAP)
screen_btn_camera_label.align(lv.ALIGN.CENTER, 0, 0)
screen_btn_camera.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_btn_camera.set_pos(285, 108)
screen_btn_camera.set_size(230, 160)
screen_btn_camera.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_btn_camera, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_btn_camera.set_style_bg_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_camera.set_style_bg_color(lv.color_hex(0x262626), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_camera.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_camera.set_style_radius(5, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_camera.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_camera.set_style_bg_img_src(load_image(r"C:\NXP\GUI-Guider-Projects\SmartAPPLinux\generated\MicroPython\camera_230_160.png"), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_camera.set_style_bg_img_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_camera.set_style_text_color(lv.color_hex(0x69CA00), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_camera.set_style_text_font(test_font("montserratMedium", 60), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_camera.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_btn_doorbell
screen_btn_doorbell = lv.btn(screen)
screen_btn_doorbell_label = lv.label(screen_btn_doorbell)
screen_btn_doorbell_label.set_text("\n\n\n                      Doorbell\n                                      ")
screen_btn_doorbell_label.set_long_mode(lv.label.LONG.WRAP)
screen_btn_doorbell_label.align(lv.ALIGN.LEFT_MID, 0, 0)
screen_btn_doorbell.set_style_pad_all(0, lv.STATE.DEFAULT)
screen_btn_doorbell.set_pos(285, 285)
screen_btn_doorbell.set_size(230, 160)
screen_btn_doorbell.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_btn_doorbell, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_btn_doorbell.set_style_bg_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_doorbell.set_style_bg_color(lv.color_hex(0x262626), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_doorbell.set_style_bg_grad_dir(lv.GRAD_DIR.HOR, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_doorbell.set_style_bg_grad_color(lv.color_hex(0x888585), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_doorbell.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_doorbell.set_style_radius(5, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_doorbell.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_doorbell.set_style_bg_img_src(load_image(r"C:\NXP\GUI-Guider-Projects\SmartAPPLinux\generated\MicroPython\doorbell_230_160.png"), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_doorbell.set_style_bg_img_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_doorbell.set_style_text_color(lv.color_hex(0x000000), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_doorbell.set_style_text_font(test_font("Alatsi_Regular", 26), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_btn_doorbell.set_style_text_align(lv.TEXT_ALIGN.LEFT, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_img_home
screen_img_home = lv.img(screen)
screen_img_home.set_src(load_image(r"C:\NXP\GUI-Guider-Projects\SmartAPPLinux\generated\MicroPython\img_home_50_50.png"))
screen_img_home.add_flag(lv.obj.FLAG.CLICKABLE)
screen_img_home.set_pivot(50,50)
screen_img_home.set_angle(0)
screen_img_home.set_pos(465, 32)
screen_img_home.set_size(50, 50)
screen_img_home.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_img_home, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_img_home.set_style_img_recolor_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_img_home.set_style_img_recolor(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_img_home.set_style_img_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create screen_label_weather
screen_label_weather = lv.label(screen)
screen_label_weather.set_text("Cloudy\n\n26ºC")
screen_label_weather.set_long_mode(lv.label.LONG.WRAP)
screen_label_weather.set_pos(673, 160)
screen_label_weather.set_size(76, 57)
screen_label_weather.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for screen_label_weather, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
screen_label_weather.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_weather.set_style_radius(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_weather.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_weather.set_style_text_font(test_font("montserratMedium", 18), lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_weather.set_style_text_letter_space(2, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_weather.set_style_text_line_space(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_weather.set_style_text_align(lv.TEXT_ALIGN.RIGHT, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_weather.set_style_bg_opa(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_weather.set_style_pad_top(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_weather.set_style_pad_right(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_weather.set_style_pad_bottom(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_weather.set_style_pad_left(0, lv.PART.MAIN|lv.STATE.DEFAULT)
screen_label_weather.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

screen.update_layout()
# Create camera
camera = lv.obj()
camera.set_size(800, 480)
camera.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for camera, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
camera.set_style_bg_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
camera.set_style_bg_color(lv.color_hex(0x262626), lv.PART.MAIN|lv.STATE.DEFAULT)

# Create camera_img_display
camera_img_display = lv.img(camera)
camera_img_display.set_src(load_image(r"C:\NXP\GUI-Guider-Projects\SmartAPPLinux\generated\MicroPython\_800_480.png"))
camera_img_display.add_flag(lv.obj.FLAG.CLICKABLE)
camera_img_display.set_pivot(50,50)
camera_img_display.set_angle(0)
camera_img_display.set_pos(0, 0)
camera_img_display.set_size(800, 480)
camera_img_display.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for camera_img_display, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
camera_img_display.set_style_img_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create camera_img_logo
camera_img_logo = lv.img(camera)
camera_img_logo.set_src(load_image(r"C:\NXP\GUI-Guider-Projects\SmartAPPLinux\generated\MicroPython\nxp_100_50.png"))
camera_img_logo.add_flag(lv.obj.FLAG.CLICKABLE)
camera_img_logo.set_pivot(50,50)
camera_img_logo.set_angle(0)
camera_img_logo.set_pos(690, 25)
camera_img_logo.set_size(100, 50)
camera_img_logo.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for camera_img_logo, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
camera_img_logo.set_style_img_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create camera_btn_back
camera_btn_back = lv.btn(camera)
camera_btn_back_label = lv.label(camera_btn_back)
camera_btn_back_label.set_text("")
camera_btn_back_label.set_long_mode(lv.label.LONG.WRAP)
camera_btn_back_label.align(lv.ALIGN.CENTER, 0, 0)
camera_btn_back.set_style_pad_all(0, lv.STATE.DEFAULT)
camera_btn_back.set_pos(0, 5)
camera_btn_back.set_size(123, 90)
camera_btn_back.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for camera_btn_back, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
camera_btn_back.set_style_bg_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
camera_btn_back.set_style_bg_color(lv.color_hex(0x262626), lv.PART.MAIN|lv.STATE.DEFAULT)
camera_btn_back.set_style_border_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
camera_btn_back.set_style_radius(5, lv.PART.MAIN|lv.STATE.DEFAULT)
camera_btn_back.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)
camera_btn_back.set_style_bg_img_src(load_image(r"C:\NXP\GUI-Guider-Projects\SmartAPPLinux\generated\MicroPython\back_button_123_90.png"), lv.PART.MAIN|lv.STATE.DEFAULT)
camera_btn_back.set_style_bg_img_opa(255, lv.PART.MAIN|lv.STATE.DEFAULT)
camera_btn_back.set_style_text_color(lv.color_hex(0xffffff), lv.PART.MAIN|lv.STATE.DEFAULT)
camera_btn_back.set_style_text_font(test_font("montserratMedium", 16), lv.PART.MAIN|lv.STATE.DEFAULT)
camera_btn_back.set_style_text_align(lv.TEXT_ALIGN.CENTER, lv.PART.MAIN|lv.STATE.DEFAULT)

# Create camera_canvas_boxes
camera_canvas_boxes = lv.canvas(camera)
buf_camera_canvas_boxes = bytearray(800*480*4)
camera_canvas_boxes.set_buffer(buf_camera_canvas_boxes, 800, 480, lv.img.CF.TRUE_COLOR_ALPHA)
camera_canvas_boxes.fill_bg(lv.color_hex(0x000000), 0)
camera_canvas_boxes.set_pos(0, 0)
camera_canvas_boxes.set_size(800, 480)
camera_canvas_boxes.set_scrollbar_mode(lv.SCROLLBAR_MODE.OFF)
# Set style for camera_canvas_boxes, Part: lv.PART.MAIN, State: lv.STATE.DEFAULT.
camera_canvas_boxes.set_style_shadow_width(0, lv.PART.MAIN|lv.STATE.DEFAULT)

camera.update_layout()

def screen_btn_camera_event_handler(e):
    code = e.get_code()
    if (code == lv.EVENT.CLICKED):
        lv.scr_load_anim(camera, lv.SCR_LOAD_ANIM.NONE, 200, 200, False)

screen_btn_camera.add_event_cb(lambda e: screen_btn_camera_event_handler(e), lv.EVENT.ALL, None)

def screen_btn_doorbell_event_handler(e):
    code = e.get_code()
    if (code == lv.EVENT.CLICKED):
        lv.scr_load_anim(camera, lv.SCR_LOAD_ANIM.NONE, 200, 200, False)

screen_btn_doorbell.add_event_cb(lambda e: screen_btn_doorbell_event_handler(e), lv.EVENT.ALL, None)

def camera_btn_back_event_handler(e):
    code = e.get_code()
    if (code == lv.EVENT.CLICKED):
        lv.scr_load_anim(screen, lv.SCR_LOAD_ANIM.NONE, 200, 200, False)

camera_btn_back.add_event_cb(lambda e: camera_btn_back_event_handler(e), lv.EVENT.ALL, None)

# content from custom.py

# Load the default screen
lv.scr_load(screen)

while SDL.check():
    time.sleep_ms(5)

