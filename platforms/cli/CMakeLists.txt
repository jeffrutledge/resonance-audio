# Copyright 2018 Google Inc. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS-IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

find_package(Boost COMPONENTS program_options REQUIRED)

add_executable(resonance_audio_cli resonance_audio_cli.cpp $<TARGET_OBJECTS:ResonanceAudioObj>
                                                           $<TARGET_OBJECTS:SadieHrtfsObj>
                                                           $<TARGET_OBJECTS:PffftObj>)
target_link_libraries(resonance_audio_cli ${Boost_LIBRARIES})

target_include_directories(resonance_audio_cli PRIVATE "${EIGEN3_INCLUDE_DIR}/")
target_include_directories(resonance_audio_cli PRIVATE "${PFFFT_INCLUDE_DIR}/")
target_include_directories(resonance_audio_cli PRIVATE "${PROJECT_SOURCE_DIR}/resonance_audio/")
target_include_directories(resonance_audio_cli PRIVATE "${Boost_INCLUDE_DIR}")

add_executable(timing timing.cpp $<TARGET_OBJECTS:ResonanceAudioObj>
                                                           $<TARGET_OBJECTS:SadieHrtfsObj>
                                                           $<TARGET_OBJECTS:PffftObj>)
target_link_libraries(timing ${Boost_LIBRARIES})

target_include_directories(timing PRIVATE "${EIGEN3_INCLUDE_DIR}/")
target_include_directories(timing PRIVATE "${PFFFT_INCLUDE_DIR}/")
target_include_directories(timing PRIVATE "${PROJECT_SOURCE_DIR}/resonance_audio/")
target_include_directories(timing PRIVATE "${Boost_INCLUDE_DIR}")
