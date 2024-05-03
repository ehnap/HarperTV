#pragma once

struct AudStreamParameters {
    int sample_rate;
    enum AVSampleFormat sample_format;
    int frame_size ;
    int channels;
};