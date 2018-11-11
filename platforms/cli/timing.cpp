#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include <boost/program_options.hpp>

#include <api/resonance_audio_api.h>
#include <platforms/common/room_effects_utils.h>
#include <platforms/common/room_properties.h>
#include <utils/wav.h>

namespace po = boost::program_options;

constexpr size_t kNumFramesPerBuffer = 256;

template <typename Word>
std::ostream& little_endian_write(std::ostream& out_stream, const Word value,
                                  const unsigned num_bytes) {
  for (unsigned i = 0; i < num_bytes; ++i) {
    out_stream.put(value >> (i * 8));
  }
  return out_stream;
}

void write_wav_file(const std::string& file_name,
                    const std::vector<int16_t>& interleaved_samples,
                    const uint16_t num_channels,
                    const uint32_t sample_rate_hz) {
  const unsigned bytes_per_sample = 2;
  const uint32_t bytes_of_samples =
      interleaved_samples.size() * bytes_per_sample;

  std::ofstream f(file_name,
                  std::ios::out | std::ios::trunc | std::ios::binary);
  f.imbue(std::locale::classic());

  // RIFF CHUNK
  f.write("RIFF", 4);                                // Chunk ID
  little_endian_write(f, 36 + bytes_of_samples, 4);  // Chunk Size
  f.write("WAVE", 4);                                // WAVE ID
  // fmt Chunk
  f.write("fmt ", 4);             // Chunk ID
  little_endian_write(f, 16, 4);  // Chunk size
  little_endian_write(f, 1, 2);   // Wave format: PCM - integer samples
  little_endian_write(f, num_channels, 2);    // Num channels
  little_endian_write(f, sample_rate_hz, 4);  // Samples per second (Hz)
  // Average Bytes per second
  little_endian_write(f, sample_rate_hz * bytes_per_sample * num_channels, 4);
  little_endian_write(f, bytes_per_sample * num_channels, 2);  // Block Align
  little_endian_write(f, bytes_per_sample * 8, 2);  // Bits per sample

  // data Chunk
  f.write("data", 4);
  little_endian_write(f, bytes_of_samples, 4);

  // Write the audio samples
  for (const int16_t& sample : interleaved_samples) {
    little_endian_write(f, sample, 2);
  }
  f.close();
}

bool verify_material_names(std::vector<int>& material_names) {
  for (const auto& n : material_names) {
    if (n < 0 or n >= vraudio::kNumMaterialNames) return false;
  }
  return true;
}

std::vector<std::string> time_point_names;
std::vector<std::chrono::high_resolution_clock::time_point> time_points;

bool write_time_deltas(
    const std::vector<std::string>& time_point_names,
    const std::vector<std::chrono::high_resolution_clock::time_point>&
        time_points) {
  const size_t kNumericalWidth = 15;
  const size_t kNameWidth = 30;
  if (time_point_names.size() != time_points.size()) {
    return false;
  } else {
    // Header
    std::cout << std::setw(kNameWidth + 2) << "Interval Name |";
    std::cout << std::setw(kNumericalWidth + 2) << "last section |";
    std::cout << std::setw(kNumericalWidth + 2) << "from start |";
    std::cout << std::setw(kNumericalWidth + 2) << "percentage" << std::endl;

    // Data
    auto total_time = std::chrono::duration_cast<std::chrono::microseconds>(
                          time_points.back() - time_points.front())
                          .count();
    for (size_t i = 1; i < time_point_names.size(); ++i) {
      auto last_section = std::chrono::duration_cast<std::chrono::microseconds>(
                              time_points[i] - time_points[i - 1])
                              .count();
      auto until_now = std::chrono::duration_cast<std::chrono::microseconds>(
                           time_points[i] - time_points[0])
                           .count();
      double percentage =
          static_cast<double>(last_section) / static_cast<double>(total_time);
      std::cout << std::setw(kNameWidth) << time_point_names[i] << " |";
      std::cout << std::setw(kNumericalWidth) << last_section << " |";
      std::cout << std::setw(kNumericalWidth) << until_now << " |";
      std::cout << std::setw(kNumericalWidth) << std::fixed << percentage * 100 << std::endl;
    }
  }
  return true;
}

int main(int argc, char const* argv[]) {
  time_point_names.reserve(100);
  time_points.reserve(100);
  time_point_names.push_back("main start");
  time_points.push_back(std::chrono::high_resolution_clock::now());

  std::string input_wav_path;
  std::string output_wav_path;
  std::vector<float> position;
  std::vector<float> dimensions;
  std::vector<int> material_names;

  // Declare the supported options.
  po::options_description desc("Usage");
  // clang-format off
  desc.add_options()
      ("help,h", "produce help message")
      ("input-file,i", po::value<std::string>(&input_wav_path)->required(), "input wav file")
      ("output-file,o", po::value<std::string>()->required(), "output wav file")
      ("position,p", po::value<std::vector<float>>(&position)->required()->multitoken(), "position of sound and head: x y z")
      ("dimensions,d", po::value<std::vector<float>>(&dimensions)->required()->multitoken(), "dimensions of room: x y z")
      ("materials,m", po::value<std::vector<int>>(&material_names)->required()->multitoken(), "wall materials: x-pos x-neg y-pos y-neg z-pos z-neg")
      ;
  // clang-format on

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  if (vm.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }
  po::notify(vm);
  std::cout << input_wav_path << std::endl;
  for (const auto& m : position) {
    std::cout << m << " ";
  }
  std::cout << std::endl;
  for (const auto& m : dimensions) {
    std::cout << m << " ";
  }
  std::cout << std::endl;
  for (const auto& m : material_names) {
    std::cout << m << " ";
  }
  std::cout << std::endl;
  if (position.size() != 3) {
    std::cout << "Need 3 floats (x y z) for position." << std::endl;
    std::cout << desc << std::endl;
    return 1;
  } else if (dimensions.size() != 3) {
    std::cout << "Need 3 floats (x y z) for dimensions." << std::endl;
    std::cout << desc << std::endl;
    return 1;
  } else if (material_names.size() != 6 or
             not verify_material_names(material_names)) {
    std::cout << "Need 6 ints (x-pos x-neg y-pos y-neg z-pos z-neg) for "
                 "materials of each wall. The materials are ints between 0 and "
              << vraudio::kNumMaterialNames << "." << std::endl;
    std::cout << desc << std::endl;
    return 1;
  }

  // Load wav file
  time_point_names.push_back("start wav construction");
  time_points.push_back(std::chrono::high_resolution_clock::now());
  std::ifstream input_wav_stream(input_wav_path, std::ios::binary);
  std::unique_ptr<const vraudio::Wav> wav =
      vraudio::Wav::CreateOrNull(&input_wav_stream);
  const size_t kNumInputFrames =
      wav->interleaved_samples().size() / wav->GetNumChannels();
  time_point_names.push_back("end wav construction");
  time_points.push_back(std::chrono::high_resolution_clock::now());

  // Print wav file info
  std::cout << "Channels: " << wav->GetNumChannels() << std::endl;
  std::cout << "SampleRateHz: " << wav->GetSampleRateHz() << std::endl;

  // Initialize Resonance Audio
  time_point_names.push_back("start ra api initialization");
  time_points.push_back(std::chrono::high_resolution_clock::now());
  std::unique_ptr<vraudio::ResonanceAudioApi> api(
      vraudio::CreateResonanceAudioApi(vraudio::kNumStereoChannels,
                                       kNumFramesPerBuffer,
                                       wav->GetSampleRateHz()));
  vraudio::RoomProperties room_properties;
  std::copy(dimensions.begin(), dimensions.end(), room_properties.dimensions);
  for (size_t i = 0; i < material_names.size(); ++i)
    room_properties.material_names[i] =
        static_cast<vraudio::MaterialName>(material_names[i]);
  api->SetReverbProperties(vraudio::ComputeReverbProperties(room_properties));
  api->SetReflectionProperties(
      vraudio::ComputeReflectionProperties(room_properties));

  vraudio::SourceId stereo_id =
      api->CreateSoundObjectSource(vraudio::RenderingMode());
  api->SetSourcePosition(stereo_id, position[0], position[1], position[2]);
  api->SetHeadPosition(position[0], position[1], position[2]);
  time_point_names.push_back("end ra api initialization");
  time_points.push_back(std::chrono::high_resolution_clock::now());

  //// Render output sound
  // Number of buffers needed (rounded up)
  time_point_names.push_back("start render");
  time_points.push_back(std::chrono::high_resolution_clock::now());
  const size_t kNumBuffersToRender =
      (kNumInputFrames + kNumFramesPerBuffer - 1) / kNumFramesPerBuffer;

  // Render each buffer
  std::vector<int16_t> input_buffer(wav->GetNumChannels() *
                                    kNumFramesPerBuffer);
  std::array<int16_t, vraudio::kNumStereoChannels * kNumFramesPerBuffer>
      output_buffer;
  std::vector<int16_t> output_samples;
  output_samples.reserve(vraudio::kNumStereoChannels * kNumBuffersToRender);
  auto input_sample_start_it = wav->interleaved_samples().begin();
  for (size_t i = 0; i < kNumBuffersToRender;
       ++i, input_sample_start_it +=
            (kNumFramesPerBuffer * wav->GetNumChannels())) {
    // Fill input buffer
    input_buffer.insert(
        input_buffer.begin(), input_sample_start_it,
        input_sample_start_it + kNumFramesPerBuffer * wav->GetNumChannels());

    // Add input buffer
    api->SetInterleavedBuffer(stereo_id, input_buffer.data(),
                              wav->GetNumChannels(), kNumFramesPerBuffer);

    // Fill output buffer
    bool rendered = api->FillInterleavedOutputBuffer(
        vraudio::kNumStereoChannels, kNumFramesPerBuffer, output_buffer.data());

    // Append output bugger to output samples
    if (not rendered) LOG() << "Render failed on frame: " << i;
    output_samples.insert(output_samples.end(), output_buffer.begin(),
                          output_buffer.end());
  }
  // Remove extra frames from last buffer
  output_samples.resize(kNumInputFrames * vraudio::kNumStereoChannels);
  time_point_names.push_back("end render");
  time_points.push_back(std::chrono::high_resolution_clock::now());

  write_wav_file(output_wav_path, output_samples, vraudio::kNumStereoChannels,
                 wav->GetSampleRateHz());
  time_point_names.push_back("end write wav");
  time_points.push_back(std::chrono::high_resolution_clock::now());

  write_time_deltas(time_point_names, time_points);
}
