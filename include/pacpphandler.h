#pragma once

#include "audiohandler.h"

#include "portaudiocpp/PortAudioCpp.hxx"

class PACppHandler : public AudioHandler {

  const unsigned SAMPLE_RATE = 44100;
  const unsigned FRAMES_PER_BUFFER = 256;
  const unsigned CHANNELS = 1;

  portaudio::AutoSystem autoSys;
  portaudio::System &system;

public:

  PACppHandler(): system(portaudio::System::instance()) {
    portaudio::DirectionSpecificStreamParameters inParams(
      system.defaultInputDevice(), 
      CHANNELS, 
      portaudio::FLOAT32,
      false,
      system.defaultInputDevice().defaultLowInputLatency(),
      nullptr
    );
    portaudio::DirectionSpecificStreamParameters outParams(
      system.defaultOutputDevice(),
      CHANNELS,
      portaudio::FLOAT32,
      false,
      system.defaultOutputDevice().defaultLowOutputLatency(),
      nullptr
    );
`
    portaudio::StreamParameters params(
      inParams,
      portaudio::DirectionSpecificStreamParameters::null(),
      SAMPLE_RATE,
      FRAMES_PER_BUFFER,
      paClipOff
    );
    
    portaudio::MemFunCallbackStream<PACppHandler> stream(
      params,
      *this,
      &PACppHandler::callback;
    );
  }

  void callback(const void* inputBuffer, void* outputBuffer,
  ) {

  }
};