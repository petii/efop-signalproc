#pragma once

#include "audiohandler.h"

#include <pulse/pulseaudio.h>

class PulseAudioHandler : public AudioHandler {
  pa_mainloop* mainloop;
  pa_mainloop_api* api;
  pa_context* context;
  pa_stream* recordStream;
  pa_sample_spec sampleSpec; 
public:
  PulseAudioHandler()
    : mainloop(pa_mainloop_new()),
      api(pa_mainloop_get_api(mainloop)),
      context(pa_context_new(api,"signalproc"))
    {
    sampleSpec.format = PA_SAMPLE_FLOAT32;
    sampleSpec.rate = SAMPLE_RATE;
    sampleSpec.channels = CHANNELS;

    
  }

  bool startRecording() override {
    return true;
  }
  bool stopRecording() override {
    return true;
  }
  void setChunkSize(size_t newSize) override {

  }
  void setCallback(CallbackType newCallback) override {

  }
  const std::vector<clock::duration>& getMeasurements() const override {

  } 
};
