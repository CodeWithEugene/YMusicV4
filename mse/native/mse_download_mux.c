#include "mse_native.h"

#include <libavcodec/codec_par.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
#include <stdio.h>
#include <string.h>

static char g_mux_error[512];

static int mux_fail(const char *step, int err)
{
    char detail[256];
    av_strerror(err, detail, sizeof(detail));
    snprintf(g_mux_error, sizeof(g_mux_error), "%s: %s", step, detail);
    return -1;
}

static int open_input(const char *path, AVFormatContext **input)
{
    int rc = avformat_open_input(input, path, NULL, NULL);
    if (rc < 0)
        return mux_fail("open input", rc);
    rc = avformat_find_stream_info(*input, NULL);
    if (rc < 0)
        return mux_fail("read input streams", rc);
    return 0;
}

static AVStream *first_stream(AVFormatContext *input, enum AVMediaType type)
{
    unsigned int i;
    for (i = 0; i < input->nb_streams; i++) {
        if (input->streams[i]->codecpar->codec_type == type)
            return input->streams[i];
    }
    return NULL;
}

static int copy_stream(AVFormatContext *output, AVStream *input, AVStream **created)
{
    AVStream *stream = avformat_new_stream(output, NULL);
    int rc;
    if (!stream)
        return mux_fail("create output stream", AVERROR(ENOMEM));
    rc = avcodec_parameters_copy(stream->codecpar, input->codecpar);
    if (rc < 0)
        return mux_fail("copy stream parameters", rc);
    stream->codecpar->codec_tag = 0;
    stream->time_base = input->time_base;
    *created = stream;
    return 0;
}

static int read_stream_packet(AVFormatContext *input, AVStream *stream, AVPacket *packet)
{
    int rc;
    while ((rc = av_read_frame(input, packet)) >= 0) {
        if (packet->stream_index == stream->index)
            return 1;
        av_packet_unref(packet);
    }
    if (rc == AVERROR_EOF)
        return 0;
    return mux_fail("read packet", rc);
}

static int write_packet(AVFormatContext *output, AVPacket *packet, AVStream *source, AVStream *target)
{
    int rc;
    av_packet_rescale_ts(packet, source->time_base, target->time_base);
    packet->stream_index = target->index;
    packet->pos = -1;
    rc = av_interleaved_write_frame(output, packet);
    av_packet_unref(packet);
    if (rc < 0)
        return mux_fail("write packet", rc);
    return 0;
}

static int write_interleaved(AVFormatContext *video, AVStream *video_in, AVStream *video_out,
                             AVFormatContext *audio, AVStream *audio_in, AVStream *audio_out,
                             AVFormatContext *output)
{
    AVPacket *video_packet = av_packet_alloc();
    AVPacket *audio_packet = av_packet_alloc();
    int have_video;
    int have_audio;
    int rc = -1;
    if (!video_packet || !audio_packet) {
        mux_fail("allocate packet", AVERROR(ENOMEM));
        goto done;
    }
    have_video = read_stream_packet(video, video_in, video_packet);
    have_audio = read_stream_packet(audio, audio_in, audio_packet);
    while (have_video > 0 || have_audio > 0) {
        if (have_video < 0 || have_audio < 0)
            goto done;
        if (have_video > 0 &&
            (have_audio == 0 ||
             video_packet->dts == AV_NOPTS_VALUE ||
             (audio_packet->dts != AV_NOPTS_VALUE &&
              av_compare_ts(video_packet->dts, video_in->time_base,
                            audio_packet->dts, audio_in->time_base) <= 0))) {
            if (write_packet(output, video_packet, video_in, video_out) < 0)
                goto done;
            have_video = read_stream_packet(video, video_in, video_packet);
        } else {
            if (write_packet(output, audio_packet, audio_in, audio_out) < 0)
                goto done;
            have_audio = read_stream_packet(audio, audio_in, audio_packet);
        }
    }
    rc = 0;

done:
    av_packet_free(&video_packet);
    av_packet_free(&audio_packet);
    return rc;
}

int mse_ios_mux_mp4(const char *video_path, const char *audio_path, const char *output_path,
                    const char *title, const char *artist, const char *video_id)
{
    AVFormatContext *video = NULL;
    AVFormatContext *audio = NULL;
    AVFormatContext *output = NULL;
    AVStream *video_in;
    AVStream *audio_in;
    AVStream *video_out = NULL;
    AVStream *audio_out = NULL;
    int rc = -1;

    g_mux_error[0] = 0;
    if (!video_path || !audio_path || !output_path) {
        snprintf(g_mux_error, sizeof(g_mux_error), "missing mux path");
        return -1;
    }
    if (open_input(video_path, &video) < 0 || open_input(audio_path, &audio) < 0)
        goto done;
    video_in = first_stream(video, AVMEDIA_TYPE_VIDEO);
    audio_in = first_stream(audio, AVMEDIA_TYPE_AUDIO);
    if (!video_in || !audio_in) {
        snprintf(g_mux_error, sizeof(g_mux_error), "missing video or audio stream");
        goto done;
    }
    rc = avformat_alloc_output_context2(&output, NULL, "mp4", output_path);
    if (rc < 0 || !output) {
        mux_fail("create MP4 output", rc < 0 ? rc : AVERROR_UNKNOWN);
        rc = -1;
        goto done;
    }
    if (copy_stream(output, video_in, &video_out) < 0 || copy_stream(output, audio_in, &audio_out) < 0) {
        rc = -1;
        goto done;
    }
    if (title && title[0])
        av_dict_set(&output->metadata, "title", title, 0);
    if (artist && artist[0])
        av_dict_set(&output->metadata, "artist", artist, 0);
    if (video_id && video_id[0])
        av_dict_set(&output->metadata, "composer", video_id, 0);
    if (!(output->oformat->flags & AVFMT_NOFILE)) {
        rc = avio_open2(&output->pb, output_path, AVIO_FLAG_WRITE, NULL, NULL);
        if (rc < 0) {
            mux_fail("open MP4 output", rc);
            rc = -1;
            goto done;
        }
    }
    rc = avformat_write_header(output, NULL);
    if (rc < 0) {
        mux_fail("write MP4 header", rc);
        rc = -1;
        goto done;
    }
    if (write_interleaved(video, video_in, video_out, audio, audio_in, audio_out, output) < 0) {
        rc = -1;
        goto done;
    }
    rc = av_write_trailer(output);
    if (rc < 0) {
        mux_fail("write MP4 trailer", rc);
        rc = -1;
        goto done;
    }
    rc = 0;

done:
    avformat_close_input(&video);
    avformat_close_input(&audio);
    if (output) {
        if (output->pb)
            avio_closep(&output->pb);
        avformat_free_context(output);
    }
    if (rc != 0)
        remove(output_path);
    return rc;
}

const char *mse_ios_mux_error(void)
{
    return g_mux_error;
}
