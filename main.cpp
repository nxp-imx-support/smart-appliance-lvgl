/*
 * Copyright 2024 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <linux/dma-buf.h>
#include <linux/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <memory>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <bits/types/struct_timespec.h>
#include <bits/types/struct_timeval.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <cassert>
#include <g2d.h>
#include "lvgl/lvgl.h"
#include "lvgl/src/widgets/lv_canvas.h"
#include "lv_drivers/indev/evdev.h"
#include "lv_drivers/display/drm.h"
#include "lv_drivers/indev/evdev.h"
#include "gui_guider.h"
#include "events_init.h"
#include "src/custom/custom.h"
#include "ml/yolov4_tflite.h"
#include "matter/log_parse.h"

#include <sys/ipc.h>
#include <sys/msg.h>

// for VIT
#define MSG_SIZE 512

typedef struct {
    long mtype;
    char mtext[MSG_SIZE];
} message_buf;

//#define DEBUG

// for LVGL of DRM boot
#define H_RES (800)
#define V_RES (480)
#define DISP_BUF_SIZE (800 * 480)

//for getting buffer
#define CAPTURE_BUF_SIZE 4
#define CLEAR(x) memset(&(x), 0, sizeof(x))

// v4l2 output
#define WIDTH 640
#define HEIGHT 480

#define LVGL_REFRESH_DELAY_US 5000

// ML
#define MAXOBJ 20

// mutex, condition and read/write lock
static pthread_mutex_t mutex_ml = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t ml_cond = PTHREAD_COND_INITIALIZER;
static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

static int video_fd = 0;
static const char *video_devname = "/dev/video0";
static struct v4l2_requestbuffers bufrequest;
volatile bool g_ui_camera = false;
volatile bool light_ctl_flag = true;
FILE *matter_handle = NULL;
static void *handle = NULL; /* g2d handler */
static struct g2d_buf *g_sbuf, *g_dbuf; /* g2d src/dst buffer */
static uint8_t *g_src_buf; /* buffer for rendering and ml */
#ifdef DEBUG
struct timeval lastTimestamp;
struct timeval tv1, tv2;
#endif

struct box
{
    uint32_t x, y, w, h;
    uint32_t num;
    const char * tag;
};

static struct box result[MAXOBJ];

static const char* labelNames[9] = {
    "fresh_apple", "normal_apple", "rotten_apple", "fresh_banana", "normal_banana",
    "rotten_banana", "fresh_orange", "normal_orange", "rotten_orange"
};

// lvgl
lv_img_dsc_t img_preview_desc = {
    .header = {
        .cf = LV_IMG_CF_TRUE_COLOR,
        .always_zero = 0,
        .reserved = 0,
        .w = WIDTH,
        .h = HEIGHT,
    },
    .data_size = WIDTH * HEIGHT * 4,
    .data = NULL,
};
lv_ui guider_ui;

typedef struct _V4L2_BufferRecord
{
    int mValid;
    void *mStart;
    int mOffset;
    int mLength;
    int mIndex;
    int fd;
} V4L2_BufferRecord;
V4L2_BufferRecord v4l2_buffer_record[CAPTURE_BUF_SIZE];
struct v4l2_buffer vbuffer;

// CSC
static void yuyv2bgr(g2d_buf *s_buf, g2d_buf *d_buf, int w, int h, void *handle)
{
    struct g2d_surface src, dst;
    src.format = G2D_YUYV;
    // Output RGBA
    dst.format = G2D_ARGB8888;
    src.planes[0] = s_buf->buf_paddr;
    src.planes[1] = s_buf->buf_paddr + WIDTH * HEIGHT;
    src.planes[2] = s_buf->buf_paddr + WIDTH * HEIGHT * 2;
    src.left = 0;
    src.top = 0;
    src.right = WIDTH;
    src.bottom = HEIGHT;
    src.stride = WIDTH;
    src.width = WIDTH;
    src.height = HEIGHT;
    src.rot = G2D_ROTATION_0;
    dst.planes[0] = d_buf->buf_paddr;
    dst.planes[1] = d_buf->buf_paddr + WIDTH * HEIGHT;
    dst.planes[2] = d_buf->buf_paddr + WIDTH * HEIGHT * 2;
    dst.left = 0;
    dst.top = 0;
    dst.right = WIDTH;
    dst.bottom = HEIGHT;
    dst.stride = WIDTH;
    dst.width = WIDTH;
    dst.height = HEIGHT;
    dst.rot = G2D_ROTATION_0;
    g2d_blit(handle, &src, &dst);
    g2d_finish(handle);
}

static int camera_init(void)
{
    int ret, i;
    int timeout = 0;

    // 1. open video node, polling for 5s
    do {
        video_fd = open(video_devname, O_RDWR, 0);
        sleep(1);
        timeout ++;
    } while (video_fd <= 0 && timeout < 5);

    if (video_fd <= 0)
    {
        printf("[native_camera][%s](%d) Open %s Failed:%s\n", __FUNCTION__, __LINE__, video_devname, strerror(errno));
        return -1;
    }
    printf("[native_camera][%s](%d) Open %s Success ^_^\n", __FUNCTION__, __LINE__, video_devname);

    // 2. init Camera
    struct v4l2_capability caps;
    ret = ioctl(video_fd, VIDIOC_QUERYCAP, &caps);
    if (ret < 0)
    {
        printf("[native_camera][%s](%d) failed to get device caps for %s (%d = %s)\n", __FUNCTION__, __LINE__,
            video_devname, errno, strerror(errno));
        close(video_fd);
        printf("exit camera thread finish******************\n");
        return -2;
    }

    // 3. print camera parameters
    printf("[native_camera] Open Device: %s (fd=%d)\n", video_devname, video_fd);
    printf("[native_camera]    Driver: %s\n", caps.driver);
    printf("[native_camera]    Card: %s\n", caps.card);
    printf("[native_camera]    Version: %u.%u.%u\n",
            (caps.version >> 16) & 0xFF,
            (caps.version >> 8) & 0xFF,
            (caps.version) & 0xFF);
    printf("[native_camera]    All Caps: %08X\n", caps.capabilities);
    printf("[native_camera]    Dev Caps: %08X\n", caps.device_caps);

    // 4. show the format supported
    printf("[native_camera]Supported capture formats:\n");
    struct v4l2_fmtdesc formatDescriptions;
    formatDescriptions.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    for (i = 0;; i++)
    {
        formatDescriptions.index = i;
        if (ioctl(video_fd, VIDIOC_ENUM_FMT, &formatDescriptions) == 0)
        {
            printf("[native_camera]  %2d: %s 0x%08X 0x%X\n",
                    i,
                    formatDescriptions.description,
                    formatDescriptions.pixelformat,
                    formatDescriptions.flags);
        }
        else
        {
            // No more formats available
            break;
        }
    }

    // 5. S_FMT
    struct v4l2_format format;
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix_mp.width = WIDTH;
    format.fmt.pix_mp.height = HEIGHT;
    format.fmt.pix_mp.pixelformat = V4L2_PIX_FMT_YUYV;
    format.fmt.pix_mp.field = V4L2_FIELD_NONE;
    printf("[native_camera]Requesting format %c%c%c%c (0x%08X)\n",
            ((char *)&format.fmt.pix.pixelformat)[0],
            ((char *)&format.fmt.pix.pixelformat)[1],
            ((char *)&format.fmt.pix.pixelformat)[2],
            ((char *)&format.fmt.pix.pixelformat)[3],
            format.fmt.pix.pixelformat);

    if (ioctl(video_fd, VIDIOC_S_FMT, &format) < 0)
    {
        printf("[native_camera][%s]  VIDIOC_S_FMT failed! %s.\n", __FUNCTION__, strerror(errno));
        close(video_fd);
        printf("exit camera thread finish******************\n");
        return -5;
    }

    // 6. G_FMT
    memset(&format, 0, sizeof(format));
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(video_fd, VIDIOC_G_FMT, &format) == 0)
    {
        printf("[native_camera][%s] Current output format:  fmt=0x%X, %dx%d\n", __FUNCTION__,
                format.fmt.pix_mp.pixelformat,
                format.fmt.pix_mp.width,
                format.fmt.pix_mp.height);
    }
    else
    {
        printf("[native_camera] VIDIOC_G_FMT: %s\n", strerror(errno));
        close(video_fd);
        printf("exit camera thread finish******************\n");
        return -6;
    }

    /* set frame rate */
    struct v4l2_streamparm fps;
    CLEAR(fps);
    fps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(video_fd, VIDIOC_G_PARM, &fps) == -1)
        printf("VIDIOC_G_PARM");
    fps.parm.capture.timeperframe.numerator = 1;
    fps.parm.capture.timeperframe.denominator = 30; /* 30fps */
    if (ioctl(video_fd, VIDIOC_S_PARM, &fps) == -1)
        printf("VIDIOC_S_PARM error");

    // 7. request buffer
    CLEAR(bufrequest);
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = CAPTURE_BUF_SIZE;

    if (ioctl(video_fd, VIDIOC_REQBUFS, &bufrequest) < 0)
    {
        printf("[native_camera] VIDIOC_REQBUFS: %s\n", strerror(errno));
        close(video_fd);
        printf("exit camera thread finish******************\n");
        return -7;
    }
    printf("[native_camera][%s,%d]\n", __func__, __LINE__);

    // 8. mmap
    struct v4l2_buffer buffer;
    struct v4l2_plane planes;
    memset(v4l2_buffer_record, 0, sizeof(v4l2_buffer_record));

    for (i = 0; i < CAPTURE_BUF_SIZE; i++)
    {
        // Get the information on the buffer that was created for us
        CLEAR(buffer);
        CLEAR(planes);
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;

        if (ioctl(video_fd, VIDIOC_QUERYBUF, &buffer) < 0)
        {
            printf("[native_camera] VIDIOC_QUERYBUF: %s\n", strerror(errno));
            close(video_fd);
            printf("exit camera thread finish******************\n");
            return -8;
        }
        v4l2_buffer_record[i].mLength = buffer.length;
        printf("[native_camera] mLength: %d\n", v4l2_buffer_record[i].mLength);

        v4l2_buffer_record[i].mStart = mmap(NULL, v4l2_buffer_record[i].mLength,
                                            PROT_READ | PROT_WRITE, MAP_SHARED, video_fd,
                                            buffer.m.offset);
        if (v4l2_buffer_record[i].mStart == MAP_FAILED)
        {
            printf("[native_camera] mmap failed!!!!!\n");
            close(video_fd);
            printf("exit camera thread finish******************\n");
            return -8;
        }
    }
    printf("[native_camera][%s,%d]\n", __func__, __LINE__);
    for (i = 0; i < CAPTURE_BUF_SIZE; i++)
    {
        memset(&buffer, 0, sizeof(buffer));
        CLEAR(planes);
        buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buffer.memory = V4L2_MEMORY_MMAP;
        buffer.index = i;
        if (ioctl(video_fd, VIDIOC_QBUF, &buffer) < 0)
        {
            printf("[native_camera] %s  VIDIOC_QBUF failed! %s.\n", __FUNCTION__, strerror(errno));
            close(video_fd);
            printf("exit camera thread finish******************\n");
            return -8;
        }
    }
    printf("[native_camera][%s,%d]\n", __func__, __LINE__);
    printf("[native_camera] VIDIOC_STREAMON Start\n");

    // 9. Stream ON
    {
        enum v4l2_buf_type type;
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (ioctl(video_fd, VIDIOC_STREAMON, &type) < 0)
        {
            printf("[native_camera] %s  VIDIOC_STREAMON failed! %s.\n", __FUNCTION__, strerror(errno));
            close(video_fd);
            printf("exit camera thread finish******************\n");
            return -9;
        }
        printf("[native_camera] %s  stream on END!!!\n", __FUNCTION__);
    }
    return 0;
}

void stream_off(void)
{
    int i;

    // 10. Stream OFF
    enum v4l2_buf_type type;
    printf("[native_camera] %s  stream OFF START!!!", __FUNCTION__);
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(video_fd, VIDIOC_STREAMOFF, &type) < 0) {
        printf("[native_camera] %s  VIDIOC_STREAMOFF failed! %s.",
            __FUNCTION__, strerror(errno));
        close(video_fd);
        printf("exit camera thread finish******************\n");
    }
    printf("[native_camera] %s  stream OFF END!!!", __FUNCTION__);

    // 11. clean_buffer
    CLEAR(bufrequest);
    bufrequest.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufrequest.memory = V4L2_MEMORY_MMAP;
    bufrequest.count = 0;
    if (ioctl(video_fd, VIDIOC_REQBUFS, &bufrequest) < 0) {
        printf("[native_camera] VIDIOC_REQBUFS: %s", strerror(errno));
        close(video_fd);
        printf("exit camera thread finish******************\n");
    }

    for (i = 0; i < CAPTURE_BUF_SIZE; i++) {
        /* to make it page size aligned */
        uint32_t size = (v4l2_buffer_record[i].mLength + 4095) & (~4095);
        printf("[native_camera] munmapped size = %u, virt_addr = 0x%p\n",
            size, v4l2_buffer_record[i].mStart);
        if (munmap(v4l2_buffer_record[i].mStart, size) < 0)
        {
        printf("[native_camera] munmapped size = %u, virt_addr = 0x%p  Failed \n",
                size, v4l2_buffer_record[i].mStart);
        }
    }
}

// Add box
static void canvas_draw_boxes(struct box *boxes, uint32_t count)
{
    int i;

    for (i = 0; i < count; i++)
    {
        lv_draw_rect_dsc_t rect_dsc;
        lv_draw_rect_dsc_init(&rect_dsc);
        rect_dsc.radius = 10;
        rect_dsc.bg_opa = LV_OPA_TRANSP;
        rect_dsc.border_width = 10;
        rect_dsc.border_opa = LV_OPA_100;

        lv_draw_label_dsc_t label_dsc;
        lv_draw_label_dsc_init(&label_dsc);

        //Green_fresh; Orange_normal; Blue_rotten
        if (boxes[i].num == 0 || boxes[i].num == 3 || boxes[i].num == 6)
        {
            rect_dsc.border_color = lv_palette_main(LV_PALETTE_GREEN);
            label_dsc.color = lv_palette_main(LV_PALETTE_GREEN);
        }
        else if (boxes[i].num == 1 || boxes[i].num == 4 || boxes[i].num == 7)
        {
            rect_dsc.border_color = lv_palette_main(LV_PALETTE_ORANGE);
            label_dsc.color = lv_palette_main(LV_PALETTE_ORANGE);
        }
        else if (boxes[i].num == 2 || boxes[i].num == 5 || boxes[i].num == 8)
        {
            rect_dsc.border_color = lv_palette_main(LV_PALETTE_BLUE);
            label_dsc.color = lv_palette_main(LV_PALETTE_BLUE);
        }
        else
        {
            rect_dsc.border_color = lv_color_white();
            label_dsc.color = lv_color_white();
        }

        lv_canvas_draw_rect(guider_ui.camera_canvas_boxes,
            boxes[i].x,
            boxes[i].y,
            boxes[i].w,
            boxes[i].h,
            &rect_dsc);
        lv_canvas_draw_text(guider_ui.camera_canvas_boxes,
            boxes[i].x + 2*rect_dsc.border_width,
            boxes[i].y + 2*rect_dsc.border_width,
            boxes[i].w * 2,
            &label_dsc,
            boxes[i].tag);
    }
}

void *cam_thread_func(void *)
{
    int frame_cnt = 0;

    if (g2d_open(&handle))
    {
        printf("g2d_open fail.\n");
        return NULL;
    }

    for (;;)
    {
        int res;
        struct v4l2_plane planes;
        CLEAR(vbuffer);
        CLEAR(planes);
        vbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        vbuffer.memory = V4L2_MEMORY_MMAP;
        vbuffer.length = 2;

        // Wait for a buffer to be ready
        if (ioctl(video_fd, VIDIOC_DQBUF, &vbuffer) < 0)
        {
            printf("[native_camera] %s  VIDIOC_DQBUF failed! %s.\n", __FUNCTION__, strerror(errno));
            close(video_fd);
            printf("exit camera thread finish******************\n");
            return NULL;
        }

#ifdef DEBUG
        if (frame_cnt % 30 == 0)
        {
            long diff = (vbuffer.timestamp.tv_sec - lastTimestamp.tv_sec) * 1000 +
                        (vbuffer.timestamp.tv_usec - lastTimestamp.tv_usec) / 1000;
            printf("%d[video:%d] %s frame size=%d/%d fps =%2f\n", vbuffer.index, frame_cnt, __FUNCTION__,
                    v4l2_buffer_record[vbuffer.index].mLength, v4l2_buffer_record[vbuffer.index].mLength, 30000.0 / diff);
            lastTimestamp = vbuffer.timestamp;
        }
#endif

        // CSC
        if (g_ui_camera) {
            memcpy(g_sbuf->buf_vaddr, (uint8_t *)v4l2_buffer_record[vbuffer.index].mStart, WIDTH * HEIGHT * 2);
            yuyv2bgr(g_sbuf, g_dbuf, WIDTH, HEIGHT, handle);
            // aquire write lock to fill g_src_buf
            pthread_rwlock_wrlock(&rwlock);
            memcpy(g_src_buf, (uint8_t *)g_dbuf->buf_vaddr, WIDTH * HEIGHT * 4);
            img_preview_desc.data = (uint8_t *)g_src_buf;
            lv_img_set_src(guider_ui.camera_img_display, &img_preview_desc);
            pthread_rwlock_unlock(&rwlock);

            if (frame_cnt % 3 == 0)
                pthread_cond_signal(&ml_cond);
        }

        frame_cnt++;
        // Queue the capture vbuffer for reuse
        if (ioctl(video_fd, VIDIOC_QBUF, &vbuffer) < 0)
        {
            printf("[native_camera] %s  VIDIOC_QBUF failed! %s.\n", __FUNCTION__, strerror(errno));
            close(video_fd);
            printf("exit camera thread finish******************\n");
            return NULL;
        }
    }
    return NULL;
}

void *ml_thread_func(void *)
{
    YOLOV4 model("/usr/share/ml_model/yolov4-tiny-freshness-vela.tflite", 2, 2);
    Prediction out_pred;
    cv::Mat rgb_frame;
    int obj_size, status;

    while (1) {
        pthread_mutex_lock(&mutex_ml);
        status = pthread_cond_wait(&ml_cond, &mutex_ml);
        pthread_mutex_unlock(&mutex_ml);
        if (status == 0) {
            cv::Mat bgra_frame(HEIGHT, WIDTH, CV_8UC4, g_src_buf);
            cv::cvtColor(bgra_frame, rgb_frame, cv::COLOR_BGRA2RGB);
            // aquire read lock for g_src_buf
            pthread_rwlock_rdlock(&rwlock);
            model.run(rgb_frame, out_pred);
            pthread_rwlock_unlock(&rwlock);

            // draw result
            auto boxes = out_pred.boxes;
            auto labels = out_pred.labels;

            if (boxes.size() >= MAXOBJ)
                obj_size = MAXOBJ;
            else
                obj_size = boxes.size();

            for (int i = 0; i < obj_size; i++) {
                auto box = boxes[i];
                // auto score = scores[i];
                auto label = labels[i];

                result[i].x = box.x;
                result[i].y = box.y;
                result[i].w = box.width;
                result[i].h = box.height;
                result[i].num = label;
                result[i].tag = labelNames[label];
            }

            // aquire write lock to avoid rendering
            pthread_rwlock_wrlock(&rwlock);
            lv_canvas_fill_bg(guider_ui.camera_canvas_boxes, lv_color_hex(0xffffff), 0);
            canvas_draw_boxes(result, obj_size);
            pthread_rwlock_unlock(&rwlock);
            out_pred = {};
        }
    }

    return (void*)0;
}

// lvgl
void sig_handler(int signum)
{
    printf("\nInside Signal handler function\n");
    printf("------SIGINT signal catched------\n");
    printf("Program exit...\n");
    lv_deinit();
    drm_exit();
    exit(0);
}

static void update_datetime(void)
{
    time_t t = time(NULL);
    struct tm *tmp;
    char cur_date[30];
    char cur_time[10];

    tmp = localtime(&t);
    strftime(cur_date, sizeof(cur_date), "\n%A, %b %d", tmp);
    strftime(cur_time, sizeof(cur_time), "%H:%M", tmp);
    pthread_rwlock_wrlock(&rwlock);
    lv_label_set_text(guider_ui.screen_label_Date, cur_date);
    lv_label_set_text(guider_ui.screen_label_Time, cur_time);
    pthread_rwlock_unlock(&rwlock);
}

static void get_weather_report()
{
    int pipefd[2], i = 0;
    const char* argv[] = {"curl", "-s",
        "wttr.in/Suzhou?format=\"%l\\n+%C\\n+%t\\n\"", NULL};
    int status;
    char buf[256];

    /* clean up buf */
    memset(buf, 0, sizeof(buf));

    /* setup pipe for two process */
    status = pipe(pipefd);

    /* fork to execute curl */
    int fid = fork();
    if (fid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], 1);
        close(pipefd[1]);
        execvp("curl", (char* const*) argv);
    }
    close(pipefd[1]);

    while (read(pipefd[0], buf, sizeof(buf)) > 0)
    do {
        ssize_t r = read(pipefd[0], &buf[i], sizeof(buf) - i);
        if (r > 0) {
            i += r;
        }
    } while (errno == EAGAIN || errno == EINTR);

    wait(&status);

    if (strlen(buf) < 5) return;

    /* remove " */
    for (i = 0; i < strlen(buf); i++) {
        if ('\"' == buf[i])
            buf[i] = ' ';
    }

    /* update label text */
    pthread_rwlock_wrlock(&rwlock);
    lv_label_set_text(guider_ui.screen_label_weather, buf);
    pthread_rwlock_unlock(&rwlock);
}

void *weather_thread_func(void *)
{

    while (1) {
        /* update date time */
        update_datetime();
        sleep(30);
        /* get weather report */
        get_weather_report();
        /* assume 10s worst case on offline */
        sleep(20);
    }

    return NULL;
}

void *matter_thread_func(void *)
{
    int ret = 0;
    char buf[10];
    bool Bulb_status, Network_status;
    FILE *fd;
    memset(buf, 0, sizeof(buf));

    while(1) {
        read_bool("ifconfig", 2, &Network_status);
        printf("network status is %d \n", Network_status);

        if(Network_status == true)
            break;

        sleep(3);
    }

    /* init Bule status */
    interactive_start("chip-tool interactive start     \
                --paa-trust-store-path /etc/dcl_paas   \
                --storage-directory /usr/share/matter", &fd);
    while(!ret) {
        ret = read_bool("chip-tool onoff read on-off 1234 1  \
            --storage-directory /usr/share/matter", 1, &Bulb_status);
    }

    input_cmd("colorcontrol move-to-hue 75 0 0 0 0 1234 1", fd);
    input_cmd("levelcontrol move-to-level 20 0 0 0 1234 1", fd);

    light_ctl_flag = Bulb_status;
    matter_handle = fd;

    events_ui_sync(light_ctl_flag);

    /* read temperature sensor data */
    while(1) {
        read_string("chip-tool temperaturemeasurement read measured-value \
                        8888 3 --storage-directory /usr/share/matter", buf);

        /* update label text */
        pthread_rwlock_wrlock(&rwlock);
        lv_label_set_text(guider_ui.screen_btn_temp_label, buf);
        pthread_rwlock_unlock(&rwlock);
        sleep(15);
    }

    interactive_end(fd);
    pthread_exit(0);
    return NULL;
}

void *receiveMessage(void *)
{
    int msqid;
    key_t key;
    message_buf recvbuf;

    while(1) {
        if(matter_handle != NULL)
        break;
        sleep(1);
    }

    // Generate the same key as sending end
    if ((key = ftok(".", 'v')) == -1) {
        perror("ftok");
        exit(1);
    }
    // Get message queues
    if ((msqid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }

    while (1) {
        if (msgrcv(msqid, &recvbuf, sizeof(recvbuf.mtext), 1, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
        if (strcmp(recvbuf.mtext,"TURN ON LIGHT") == 0) {
            input_cmd("onoff on 1234 1", matter_handle);
            light_ctl_flag = true;
            events_ui_sync(light_ctl_flag);
            printf("Received TURN ON LIGHT\n");
        }
        else if (strcmp(recvbuf.mtext,"TURN OFF LIGHT") == 0) {
            input_cmd("onoff off 1234 1", matter_handle);
            light_ctl_flag = false;
            events_ui_sync(light_ctl_flag);
            printf("Received TURN OFF LIGHT\n");
        }
        else if (strcmp(recvbuf.mtext,"STANDBY") == 0) {
            /* call to suspend */
            int fd;
            fd = open("/sys/power/state", O_RDWR);
            if (fd > 0) {
                    write(fd, "mem", 3);
                    close(fd);
            }
            printf("Received STANDBY\n");
        }
    }
}

int main(void)
{
    static lv_color_t buf[DISP_BUF_SIZE];
    static lv_disp_draw_buf_t disp_buf;
    static lv_disp_t *disp;
    static lv_disp_drv_t disp_drv;
    static lv_indev_drv_t indev_drv;
    pthread_t video_thread, inference_thread, weather_thread, matter_thread, vit_thread;

    /* Register signal handler */
    signal(SIGINT, sig_handler);

    lv_init();

    /*DRM init*/
    drm_init();

    /*Initialize a descriptor for the buffer*/
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;
    disp_drv.flush_cb = drm_flush;
    /* Screen Size */
    disp_drv.hor_res = H_RES;
    disp_drv.ver_res = V_RES;
    disp = lv_disp_drv_register(&disp_drv);

    /* Initialize and register a display input driver */
    lv_indev_drv_init(&indev_drv);

    indev_drv.type = LV_INDEV_TYPE_POINTER;
    /* lv_gesture_dir_t lv_indev_get_gesture_dir(const lv_indev_t * indev) */
    indev_drv.read_cb = evdev_read;
    lv_indev_t *my_indev = lv_indev_drv_register(&indev_drv);

    /* Set Image Cache size */
    lv_img_cache_set_size(10);

    /* Demo init */
    setup_ui(&guider_ui);
    events_init(&guider_ui);
    custom_init(&guider_ui);

    /* show GUI first */
    lv_task_handler();

    /* init camera */
    if (0 == camera_init()) {
        /* allocate buffer for G2D and rendering */
        g_sbuf = g2d_alloc(WIDTH * HEIGHT * 4, 0);
        g_dbuf = g2d_alloc(WIDTH * HEIGHT * 4, 0);
        g_src_buf = (uint8_t *)malloc(WIDTH * HEIGHT * 4);
        /* create threads for camera q/dq and ML inference */
        pthread_create(&video_thread, NULL, cam_thread_func, NULL);
        pthread_create(&inference_thread, NULL, ml_thread_func, NULL);
    } else {
        printf("No UVC camera connected!\n");
    }

    /* Linux input device init */
    evdev_init();

    /**
     * Create thread to get weather report.
     * Since the curl may timeout when network is unavailable
     * and block the whole UI startup. Need a async thread.
     */
    pthread_create(&weather_thread, NULL, weather_thread_func, NULL);

    /**
     * Create thread to start Matter controller service.
     * start the service when get ip address
     */
    pthread_create(&matter_thread, NULL, matter_thread_func, NULL);

    pthread_create(&vit_thread, NULL, receiveMessage, NULL);

    /*Handle LitlevGL tasks (tickless mode)*/
    while (1)
    {
#ifdef DEBUG
        gettimeofday(&tv1, NULL);
#endif
        /* aquire read lock, since rendering only need read consume */
        pthread_rwlock_rdlock(&rwlock);
        lv_task_handler();
        pthread_rwlock_unlock(&rwlock);
#ifdef DEBUG
        gettimeofday(&tv2, NULL);
        int time_handler = (tv2.tv_sec * 1000 + tv2.tv_usec / (1000)) - (tv1.tv_sec * 1000 + tv1.tv_usec / (1000));
        printf("lv_task_handler tasks:%d ms\n", time_handler);
#endif
        usleep(LVGL_REFRESH_DELAY_US);
    }

    return 0;
}
