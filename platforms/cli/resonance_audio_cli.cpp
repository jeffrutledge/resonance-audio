#include <iostream>
#include <memory>

#include <platforms/common/room_properties.h>
#include <platforms/common/room_effects_utils.h>
#include <api/resonance_audio_api.h>
#include <utils/wav_reader.h>

namespace vraudio {
// Stores the necessary components for the ResonanceAudio system. Methods called
// from the native implementation below must check the validity of this
// instance.
struct ResonanceAudioSystem {
  ResonanceAudioSystem(int sample_rate, size_t num_channels,
                       size_t frames_per_buffer)
      : api(CreateResonanceAudioApi(num_channels, frames_per_buffer,
                                    sample_rate)) {}

  // ResonanceAudio API instance to communicate with the internal system.
  std::unique_ptr<ResonanceAudioApi> api;

  // Default room properties, which effectively disable the room effects.
  ReflectionProperties null_reflection_properties;
  ReverbProperties null_reverb_properties;
};

// Singleton |ResonanceAudioSystem| instance to communicate with the internal
// API.
static std::shared_ptr<ResonanceAudioSystem> resonance_audio = nullptr;

}  // namespace vraudio

int main(int argc, char const* argv[]) {
  std::unique_ptr<vraudio::ResonanceAudioApi> api(vraudio::CreateResonanceAudioApi(1, 1000, 44100));

  api->SetMasterVolume(0.5);

  std::cout << "Hello World" << std::endl;
}
