/* Sockets Example
 * Copyright (c) 2016-2020 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "juliah_mqtt.hpp"
#include "stm32l475e_iot01_audio.h"

#include <cstdint>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <stdint.h>
#include <string>

#define NOISE_THRESHOLD 5000
#define FILE_CHUNK_SIZE 100

DigitalOut led(LED1);

EventQueue* ev_queue = mbed_event_queue();
InterruptIn blue_button(BUTTON1);
const auto juliah_mqtt = new JuLIAHMQTT();

// MIC STUFF BEGIN
static uint16_t PCM_Buffer[PCM_BUFFER_LEN / 2];
static BSP_AUDIO_Init_t MicParams;

// Place to store final audio (alloc on the heap), here two seconds...
static size_t TARGET_AUDIO_BUFFER_NB_SAMPLES = AUDIO_SAMPLING_FREQUENCY * 2;
static int16_t *TARGET_AUDIO_BUFFER = (int16_t*)calloc(TARGET_AUDIO_BUFFER_NB_SAMPLES, sizeof(int16_t));
static size_t TARGET_AUDIO_BUFFER_IX = 0;

// we skip the first 50 events (100 ms.) to not record the button click
static size_t SKIP_FIRST_EVENTS = 50;
static size_t half_transfer_events = 0;
static size_t transfer_complete_events = 0;

int16_t get_max_amplitude() {
    const size_t buffer_size = PCM_BUFFER_LEN / 2;
    int16_t max_amplitude = 0; // Track max amplitude

    for (int i = 0; i < buffer_size; i++) {
        int16_t sample = PCM_Buffer[i];
        /* Absolute value accounts for negative amplitudes
        (Range of -32000 to 32000, 0 means no sound) */
        sample = abs(sample);
        if (sample > max_amplitude) 
            max_amplitude = sample;
    }
    return max_amplitude;
}

void send_audio_chunk(uint8_t preamble, void* buf, size_t buf_length) {
    uint8_t *content_buf = (uint8_t*)buf;
    memmove(content_buf + 1, content_buf, buf_length - 1);
    *content_buf = preamble; // middle section
    juliah_mqtt->send_message(sound_topic, content_buf, buf_length + 1);
}

// callback that gets invoked when TARGET_AUDIO_BUFFER is full
void target_audio_buffer_full() {
    // pause audio stream
    int32_t ret = BSP_AUDIO_IN_Stop(AUDIO_INSTANCE);
    if (ret != BSP_ERROR_NONE) {
        printf("Error Audio Stop (%d)\n", ret);
        return;
    }

    // create WAV file
    size_t wavFreq = AUDIO_SAMPLING_FREQUENCY;
    size_t dataSize = (TARGET_AUDIO_BUFFER_NB_SAMPLES * 2);
    size_t fileSize = 44 + (TARGET_AUDIO_BUFFER_NB_SAMPLES * 2);

    const auto wav_header_length = 44;

    uint8_t wav_header[wav_header_length] = {
        0x52, 0x49, 0x46, 0x46, // RIFF
        static_cast<uint8_t>(fileSize & 0xff), static_cast<uint8_t>((fileSize >> 8) & 0xff), static_cast<uint8_t>((fileSize >> 16) & 0xff), static_cast<uint8_t>((fileSize >> 24) & 0xff),
        0x57, 0x41, 0x56, 0x45, // WAVE
        0x66, 0x6d, 0x74, 0x20, // fmt
        0x10, 0x00, 0x00, 0x00, // length of format data
        0x01, 0x00, // type of format (1=PCM)
        0x01, 0x00, // number of channels
        static_cast<uint8_t>(wavFreq & 0xff), static_cast<uint8_t>((wavFreq >> 8) & 0xff), static_cast<uint8_t>((wavFreq >> 16) & 0xff), static_cast<uint8_t>((wavFreq >> 24) & 0xff),
        0x00, 0x7d, 0x00, 0x00, // 	(Sample Rate * BitsPerSample * Channels) / 8
        0x02, 0x00, 0x10, 0x00,
        0x64, 0x61, 0x74, 0x61, // data
        static_cast<uint8_t>(dataSize & 0xff), static_cast<uint8_t>((dataSize >> 8) & 0xff), static_cast<uint8_t>((dataSize >> 16) & 0xff), static_cast<uint8_t>((dataSize >> 24) & 0xff),
    };

    printf("Total complete events: %lu, index is %lu\n", transfer_complete_events, TARGET_AUDIO_BUFFER_IX);

    const auto max_amplitude = get_max_amplitude();
    if (max_amplitude < NOISE_THRESHOLD) {
        printf("Audio not loud enough, discarding\n");
        // return;
    }

    // print both the WAV header and the audio buffer in HEX format to serial
    // you can use the script in `hex-to-buffer.js` to make a proper WAV file again
    printf("WAV file:\n");
    for (size_t ix = 0; ix < wav_header_length; ix++)
        printf("%02x", wav_header[ix]);
    printf("\n");

    uint8_t *buf = (uint8_t*)TARGET_AUDIO_BUFFER;
    for (size_t ix = 0; ix < dataSize; ix++)
        if (ix < 1000) // only print first 1000 bytes
            printf("%02x", buf[ix]);
    
    printf("\n");

    // Send header w preamble 0x00
    const auto header_message = "{\"timestamp\": 100000000, \"peakVolume\": 43.4}";
    auto header = (char *)malloc(strlen(header_message) + 1); // allocate on heap, +1 for null terminator
    strcpy(header, header_message);
    send_audio_chunk(0x00, (void*)header, strlen(header_message) + 1);
    free(header);
    // const uint8_t start = 0x00;
    // juliah_mqtt->send_message(sound_topic, (void*)&start, 1);

    // Send wav header w preamble 0x01
    send_audio_chunk(0x01, wav_header, wav_header_length);

    // Send wav contents w preamble 0x01
    size_t offset;
    for (offset = 0; offset < dataSize; offset += FILE_CHUNK_SIZE)
        send_audio_chunk(0x01, buf + offset, FILE_CHUNK_SIZE);

    const auto remaining_offset = offset - FILE_CHUNK_SIZE;
    const auto remaining_chunk_size = dataSize - remaining_offset;
    if (remaining_chunk_size > 0) {
        printf("Remaining chunk size: %d\n", remaining_chunk_size);
        send_audio_chunk(0x01, buf + remaining_offset, remaining_chunk_size);
    }

    // Send footer w preamble 0x02
    const uint8_t end = 0x02;
    juliah_mqtt->send_message(sound_topic, (void*)&end, 1);
    // const auto footer = "end of audio file";
    // send_audio_chunk(0x02, (void*)footer, 0);

    printf("Recorded and sent audio\n");
}


/**
* @brief  Half Transfer user callback, called by BSP functions.
* @param  None
* @retval None
*/
void BSP_AUDIO_IN_HalfTransfer_CallBack(uint32_t Instance) {
    half_transfer_events++;
    if (half_transfer_events < SKIP_FIRST_EVENTS) return;

    uint32_t buffer_size = PCM_BUFFER_LEN / 2; /* Half Transfer */
    uint32_t nb_samples = buffer_size / sizeof(int16_t); /* Bytes to Length */

    if ((TARGET_AUDIO_BUFFER_IX + nb_samples) > TARGET_AUDIO_BUFFER_NB_SAMPLES)
        return;

    /* Copy first half of PCM_Buffer from Microphones onto Fill_Buffer */
    memcpy(((uint8_t*)TARGET_AUDIO_BUFFER) + (TARGET_AUDIO_BUFFER_IX * 2), PCM_Buffer, buffer_size);
    TARGET_AUDIO_BUFFER_IX += nb_samples;

    if (TARGET_AUDIO_BUFFER_IX >= TARGET_AUDIO_BUFFER_NB_SAMPLES) {
        ev_queue->call(&target_audio_buffer_full);
        return;
    }
}

/**
* @brief  Transfer Complete user callback, called by BSP functions.
* @param  None
* @retval None
*/
void BSP_AUDIO_IN_TransferComplete_CallBack(uint32_t Instance) {
    transfer_complete_events++;
    if (transfer_complete_events < SKIP_FIRST_EVENTS) return;

    uint32_t buffer_size = PCM_BUFFER_LEN / 2; /* Half Transfer */
    uint32_t nb_samples = buffer_size / sizeof(int16_t); /* Bytes to Length */

    if ((TARGET_AUDIO_BUFFER_IX + nb_samples) > TARGET_AUDIO_BUFFER_NB_SAMPLES)
        return;

    /* Copy second half of PCM_Buffer from Microphones onto Fill_Buffer */
    memcpy(((uint8_t*)TARGET_AUDIO_BUFFER) + (TARGET_AUDIO_BUFFER_IX * 2),
        ((uint8_t*)PCM_Buffer) + (nb_samples * 2), buffer_size);
    TARGET_AUDIO_BUFFER_IX += nb_samples;

    if (TARGET_AUDIO_BUFFER_IX >= TARGET_AUDIO_BUFFER_NB_SAMPLES) {
        ev_queue->call(&target_audio_buffer_full);
        return;
    }
}

/**
  * @brief  Manages the BSP audio in error event.
  * @param  Instance Audio in instance.
  * @retval None.
  */
void BSP_AUDIO_IN_Error_CallBack(uint32_t Instance) {
    printf("BSP_AUDIO_IN_Error_CallBack\n");
}

void print_stats() {
    printf("Half %lu, Complete %lu, IX %lu\n", half_transfer_events, transfer_complete_events,
        TARGET_AUDIO_BUFFER_IX);
}

void start_recording() {
    uint32_t state;

    int32_t ret = BSP_AUDIO_IN_GetState(AUDIO_INSTANCE, &state);
    if (ret != BSP_ERROR_NONE) {
        printf("Cannot start recording: Error getting audio state (%d)\n", ret);
        return;
    }
    if (state == AUDIO_IN_STATE_RECORDING) {
        printf("Cannot start recording: Already recording\n");
        return;
    }

    // reset audio buffer location
    TARGET_AUDIO_BUFFER_IX = 0;
    transfer_complete_events = 0;
    half_transfer_events = 0;

    ret = BSP_AUDIO_IN_Record(AUDIO_INSTANCE, (uint8_t *) PCM_Buffer, PCM_BUFFER_LEN);
    if (ret != BSP_ERROR_NONE) {
        printf("Error Audio Record (%ld)\n", ret);
        return;
    }

    printf("OK Audio Record\n");
}



bool setup_audio() {
    // set up the microphone
    MicParams.BitsPerSample = 16;
    MicParams.ChannelsNbr = AUDIO_CHANNELS;
    MicParams.Device = AUDIO_IN_DIGITAL_MIC1;
    MicParams.SampleRate = AUDIO_SAMPLING_FREQUENCY;
    MicParams.Volume = 32;

    if (!TARGET_AUDIO_BUFFER) {
        printf("Failed to allocate TARGET_AUDIO_BUFFER buffer\n");
        return false;
    }

    int32_t ret = BSP_AUDIO_IN_Init(AUDIO_INSTANCE, &MicParams);

    if (ret != BSP_ERROR_NONE) {
        printf("Error Audio Init (%ld)\r\n", ret);
        return false;
    }
    printf("OK Audio Init\t(Audio Freq=%d)\r\n", AUDIO_SAMPLING_FREQUENCY);
    return true;
}
// MIC STUFF END

int main() {
    printf("========== JuLIAH: Hi ==========\n\n");

    while(!juliah_mqtt->setup())
        printf("Something went wrong when setting up wifi & mqtt connection! Retrying...\n");

    if (!setup_audio()) {
        printf("Something went wrong when setting up audio!\n");
        return 1;
    }

    // char* message = "{\"timestamp\": 100000000, \"peakVolume\": 43.4, \"audio\": \"\"}";
    // juliah_mqtt->send_message(sound_topic, message, strlen(message));

    printf("Press the BLUE button to record audio\n");
    blue_button.rise(ev_queue->event(&start_recording));

    printf("Receiving messages\n");
    while (juliah_mqtt->has_message())
        juliah_mqtt->listen_message();

    juliah_mqtt->cleanup();

    printf("========== JuLIAH: Bye ==========\n\n");
    return 0;
}
