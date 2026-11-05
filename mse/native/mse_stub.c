#include "mse_native.h"
#include <stddef.h>

void *mse_host_create(void)
{
    return NULL;
}

void mse_host_destroy(void *h)
{
    (void)h;
}

const void *mse_host_vtable(void)
{
    return NULL;
}

int mse_host_frames(void *h)
{
    (void)h;
    return 0;
}

int mse_host_dropped_frames(void *h)
{
    (void)h;
    return 0;
}

void mse_host_video_decoder(void *h, char *out, int cap)
{
    (void)h;
    if (cap > 0)
        out[0] = 0;
}

int64_t mse_host_samples_played(void *h)
{
    (void)h;
    return 0;
}

int mse_host_copy_frame(void *h, uint8_t *dst, int cap, int *w, int *ht, int *gen)
{
    (void)h;
    (void)dst;
    (void)cap;
    if (w)
        *w = 0;
    if (ht)
        *ht = 0;
    if (gen)
        *gen = 0;
    return 0;
}

void mse_host_set_video_surface(void *h, void *window)
{
    (void)h;
    (void)window;
}

void mse_host_set_js_event(void (*fn)(const char *name))
{
    (void)fn;
}

void mse_set_log_callback(void (*fn)(int level, const char *msg))
{
    (void)fn;
}

void mse_ios_log(const char *msg)
{
    (void)msg;
}

void mse_ios_recover_playback(void)
{
}

void mse_ios_park_video(void)
{
}

void mse_host_set_volume(void *h, float vol)
{
    (void)h;
    (void)vol;
}

void mse_host_set_speed(void *h, float speed, float tempo)
{
    (void)h;
    (void)speed;
    (void)tempo;
}

float mse_host_playback_rate(void *h)
{
    (void)h;
    return 1.0f;
}

void mse_host_set_pitch(void *h, float pitch)
{
    (void)h;
    (void)pitch;
}

void mse_host_set_eq(void *h, const float *bands_db)
{
    (void)h;
    (void)bands_db;
}

void mse_host_set_bass(void *h, float db)
{
    (void)h;
    (void)db;
}

void mse_host_set_virtualizer(void *h, float width)
{
    (void)h;
    (void)width;
}

void mse_host_set_preamp(void *h, float db)
{
    (void)h;
    (void)db;
}

int mse_host_video_size(void *h, int *w, int *ht)
{
    (void)h;
    if (w)
        *w = 0;
    if (ht)
        *ht = 0;
    return 0;
}

int mse_host_pcm_levels(void *h, float *out, int n)
{
    (void)h;
    (void)out;
    (void)n;
    return 0;
}

int mse_ios_mux_m4a(const char *audio_path, const char *cover_path, const char *output_path,
                    const char *title, const char *artist, const char *video_id)
{
    (void)audio_path;
    (void)cover_path;
    (void)output_path;
    (void)title;
    (void)artist;
    (void)video_id;
    return -1;
}

int mse_ios_mux_mp4(const char *video_path, const char *audio_path, const char *output_path,
                    const char *title, const char *artist, const char *video_id)
{
    (void)video_path;
    (void)audio_path;
    (void)output_path;
    (void)title;
    (void)artist;
    (void)video_id;
    return -1;
}

const char *mse_ios_mux_error(void)
{
    return "iOS MP4 mux unavailable";
}
