

#ifndef WebRTC_AudioPacketModule_H
#define WebRTC_AudioPacketModule_H


#include "base/base.h"

#ifdef HAVE_FFMPEG

#include "ff/ff.h"
#include "ff/audiobuffer.h"
#include "ff/packet.h"

#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/ref_counted_object.h"

#include "rtc_base/thread.h"
#include "common_types.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_device/audio_device_buffer.h"

#define DIAGNOSTICK 0
 #if DIAGNOSTICK
    #include "rtc_base/system/file_wrapper.h"
 #endif   

#include "rtc_base/platform_thread.h"

namespace base {
    namespace wrtc {

        class AudioPacketModule : public webrtc::AudioDeviceModule
 {
        public:
            //typedef uint16_t Sample;
            //std::vector<Sample> _sendSamples;

            static rtc::scoped_refptr<AudioPacketModule> Create();
            void onAudioCaptured(IPacket& packet);


            /*
            int32_t ActiveAudioLayer(AudioLayer* audioLayer) const override;
            int32_t RegisterAudioCallback(webrtc::AudioTransport* audioCallback) override;

                // Main initialization and termination
            int32_t Init() override;
            int32_t Terminate() override;
            bool Initialized() const override;
    
             */

        protected:
            /// The constructor is protected because the class needs to be created as a
            /// reference counted object (for memory managment reasons). It could be
            /// exposed in which case the burden of proper instantiation would be put on
            /// the creator of a AudioPacketModule instance. To create an instance of
            /// this class use the `Create()` API.
            explicit AudioPacketModule();

            /// The destructor is protected because it is reference counted and should
            /// not be deleted directly.
            virtual ~AudioPacketModule();


            //////////////////////////////override///
            //
            int32_t ActiveAudioLayer(AudioLayer* audioLayer) const override;
            int32_t RegisterAudioCallback(webrtc::AudioTransport* audioCallback) override;

            // Main initialization and termination
            int32_t Init() override;
            int32_t Terminate() override;
            bool Initialized() const override;

            // Device enumeration
            int16_t PlayoutDevices() override;
            int16_t RecordingDevices() override;
            int32_t PlayoutDeviceName(uint16_t index, char name[webrtc::kAdmMaxDeviceNameSize], char guid[webrtc::kAdmMaxGuidSize]) override;
            int32_t RecordingDeviceName(uint16_t index, char name[webrtc::kAdmMaxDeviceNameSize], char guid[webrtc::kAdmMaxGuidSize]) override;

            // Device selection
            int32_t SetPlayoutDevice(uint16_t index) override;
            int32_t SetPlayoutDevice(WindowsDeviceType device) override;
            int32_t SetRecordingDevice(uint16_t index) override;
            int32_t SetRecordingDevice(WindowsDeviceType device) override;

            // Audio transport initialization
            int32_t PlayoutIsAvailable(bool* available) override;
            int32_t InitPlayout() override;
            bool PlayoutIsInitialized() const override;
            int32_t RecordingIsAvailable(bool* available) override;
            int32_t InitRecording() override;
            bool RecordingIsInitialized() const override;

            // Audio transport control
            virtual int32_t StartPlayout() override;
            virtual int32_t StopPlayout() override;
            virtual bool Playing() const override;
            virtual int32_t StartRecording() override;
            virtual int32_t StopRecording() override;
            virtual bool Recording() const override;

            // Audio mixer initialization
            virtual int32_t InitSpeaker() override;
            virtual bool SpeakerIsInitialized() const override;
            virtual int32_t InitMicrophone() override;
            virtual bool MicrophoneIsInitialized() const override;

            // Speaker volume controls

            virtual int32_t SpeakerVolumeIsAvailable(bool* available) override {
                return -1;
            }

            virtual int32_t SetSpeakerVolume(uint32_t volume) override {
                return -1;
            }

            virtual int32_t SpeakerVolume(uint32_t* volume) const override {
                return -1;
            }

            virtual int32_t MaxSpeakerVolume(uint32_t* maxVolume) const override {
                return -1;
            }

            virtual int32_t MinSpeakerVolume(uint32_t* minVolume) const override {
                return -1;
            }

            // Microphone volume controls

            virtual int32_t MicrophoneVolumeIsAvailable(bool* available) override {
                return -1;
            }

            virtual int32_t SetMicrophoneVolume(uint32_t volume) override {
                return -1;
            }

            virtual int32_t MicrophoneVolume(uint32_t* volume) const override {
                return -1;
            }

            virtual int32_t MaxMicrophoneVolume(uint32_t* maxVolume) const override {
                return -1;
            }

            virtual int32_t MinMicrophoneVolume(uint32_t* minVolume) const override {
                return -1;
            }

            // Speaker mute control

            virtual int32_t SpeakerMuteIsAvailable(bool* available) override {
                return -1;
            }

            virtual int32_t SetSpeakerMute(bool enable) override {
                return -1;
            }

            virtual int32_t SpeakerMute(bool* enabled) const override {
                return -1;
            }

            // Microphone mute control

            virtual int32_t MicrophoneMuteIsAvailable(bool* available) override {
                return -1;
            }

            virtual int32_t SetMicrophoneMute(bool enable) override {
                return -1;
            }

            virtual int32_t MicrophoneMute(bool* enabled) const override {
                return -1;
            }

            // Stereo support
            virtual int32_t StereoPlayoutIsAvailable(bool* available) const override;
            virtual int32_t SetStereoPlayout(bool enable) override;
            virtual int32_t StereoPlayout(bool* enabled) const override;
            virtual int32_t StereoRecordingIsAvailable(bool* available) const override;
            virtual int32_t SetStereoRecording(bool enable) override;
            virtual int32_t StereoRecording(bool* enabled) const override;

            // Playout delay

            virtual int32_t PlayoutDelay(uint16_t* delayMS) const override {
                return -1;
            }

            // Only supported on Android.

            virtual bool BuiltInAECIsAvailable() const override {
                return false;
            }

            virtual bool BuiltInAGCIsAvailable() const override {
                return false;
            }

            virtual bool BuiltInNSIsAvailable() const override {
                return false;
            }

            // Enables the built-in audio effects. Only supported on Android.

            virtual int32_t EnableBuiltInAEC(bool enable) override {
                return -1;
            }

            virtual int32_t EnableBuiltInAGC(bool enable) override {
                return -1;
            }

            virtual int32_t EnableBuiltInNS(bool enable) override {
                return -1;
            }
            //////////////////////////////

        private:
            /// Initializes the state of the `AudioPacketModule`. This API is called on
            /// creation by the `Create()` API.
            bool Initialize();


            /// Protects variables that are accessed from `_processThread` and
            /// the main thread.
            // rtc::CriticalSection _crit;

            /// Protects |_audioCallback| that is accessed from `_processThread` and
            /// the main thread.
            rtc::CriticalSection _critCallback;


            std::atomic<bool> bInitialized{ false};
            std::atomic<bool> _recIsInitialized{ false};
            std::atomic<bool> _recording{ false};


            //TUniquePtr<webrtc::AudioDeviceBuffer> DeviceBuffer;
            std::unique_ptr<webrtc::AudioDeviceBuffer> DeviceBuffer;

            //TArray<uint8_t> RecordingBuffer;
            int RecordingBufferSize = 0;


            static constexpr int SampleRate = 48000;
            static constexpr int NumChannels = 2;

            bool bFormatChecked = false;

            std::vector<int8_t> RecordingBuffer;

            ///// arvind file debug

            size_t _recordingBufferSizeIn10MS;
            size_t _recordingFramesIn10MS;
            int8_t* _recordingBuffer; // In bytes.

  #if DIAGNOSTICK
            webrtc::FileWrapper _inputFile;
            webrtc::FileWrapper _outputFile;
   #endif           
            std::unique_ptr<rtc::PlatformThread> _ptrThreadRec;
            
            static void RecThreadFunc(void* pThis);
            
            
            bool RecThreadProcess();
            int64_t _lastCallRecordMillis{0};
            


        };

#if 0
        /// This class implements an `AudioDeviceModule` that can be used to process
        /// arbitrary audio packets. This class is send only, recording is not implemented
        ///
        /// Note P postfix of a function indicates that it should only be called by the
        /// processing thread.

        class AudioPacketModule : public webrtc::AudioDeviceModule,
        public rtc::MessageHandler {
        public:
            typedef uint16_t Sample;

            /// Creates a `AudioPacketModule` or returns NULL on failure.
            static rtc::scoped_refptr<AudioPacketModule> Create();

            /// Handles input packets from the capture for sending.
            void onAudioCaptured(IPacket& packet);

            /// Following functions are inherited from `webrtc::AudioDeviceModule`.
            /// Only functions called by `Peer` are implemented, the rest do
            /// nothing and return success. If a function is not expected to be called
            /// by `Peer` an assertion is triggered if it is in fact called.
            // int64_t TimeUntilNextProcess() override;
            // void Process() override;

            int32_t ActiveAudioLayer(AudioLayer* audio_layer) const override;

            // ErrorCode LastError() const override;
            // int32_t RegisterEventObserver(webrtc::AudioDeviceObserver* event_callback) override;

            /// Note: Calling this method from a callback may result in deadlock.
            int32_t RegisterAudioCallback(webrtc::AudioTransport* audio_callback) override;

            int32_t Init() override;
            int32_t Terminate() override;
            bool Initialized() const override;

            int16_t PlayoutDevices() override;
            int16_t RecordingDevices() override;
            int32_t PlayoutDeviceName(uint16_t index,
                    char name[webrtc::kAdmMaxDeviceNameSize],
                    char guid[webrtc::kAdmMaxGuidSize]) override;
            int32_t RecordingDeviceName(uint16_t index,
                    char name[webrtc::kAdmMaxDeviceNameSize],
                    char guid[webrtc::kAdmMaxGuidSize]) override;

            int32_t SetPlayoutDevice(uint16_t index) override;
            int32_t SetPlayoutDevice(WindowsDeviceType device) override;
            int32_t SetRecordingDevice(uint16_t index) override;
            int32_t SetRecordingDevice(WindowsDeviceType device) override;

            int32_t PlayoutIsAvailable(bool* available) override;
            int32_t InitPlayout() override;
            bool PlayoutIsInitialized() const override;
            int32_t RecordingIsAvailable(bool* available) override;
            int32_t InitRecording() override;
            bool RecordingIsInitialized() const override;

            int32_t StartPlayout() override;
            int32_t StopPlayout() override;
            bool Playing() const override;
            int32_t StartRecording() override;
            int32_t StopRecording() override;
            bool Recording() const override;

            // int32_t SetAGC(bool enable) override;
            // bool AGC() const override;

            // int32_t SetWaveOutVolume(uint16_t volume_left, uint16_t volume_right) override;
            // int32_t WaveOutVolume(uint16_t* volume_left, uint16_t* volume_right) const override;

            int32_t InitSpeaker() override;
            bool SpeakerIsInitialized() const override;
            int32_t InitMicrophone() override;
            bool MicrophoneIsInitialized() const override;

            int32_t SpeakerVolumeIsAvailable(bool* available) override;
            int32_t SetSpeakerVolume(uint32_t volume) override;
            int32_t SpeakerVolume(uint32_t* volume) const override;
            int32_t MaxSpeakerVolume(uint32_t* max_volume) const override;
            int32_t MinSpeakerVolume(uint32_t* min_volume) const override;
            // int32_t SpeakerVolumeStepSize(uint16_t* step_size) const override;

            int32_t MicrophoneVolumeIsAvailable(bool* available) override;
            int32_t SetMicrophoneVolume(uint32_t volume) override;
            int32_t MicrophoneVolume(uint32_t* volume) const override;
            int32_t MaxMicrophoneVolume(uint32_t* max_volume) const override;

            int32_t MinMicrophoneVolume(uint32_t* min_volume) const override;
            // int32_t MicrophoneVolumeStepSize(uint16_t* step_size) const override;

            int32_t SpeakerMuteIsAvailable(bool* available) override;
            int32_t SetSpeakerMute(bool enable) override;
            int32_t SpeakerMute(bool* enabled) const override;

            int32_t MicrophoneMuteIsAvailable(bool* available) override;
            int32_t SetMicrophoneMute(bool enable) override;
            int32_t MicrophoneMute(bool* enabled) const override;

            // int32_t MicrophoneBoostIsAvailable(bool* available) override;
            // int32_t SetMicrophoneBoost(bool enable) override;
            // int32_t MicrophoneBoost(bool* enabled) const override;

            int32_t StereoPlayoutIsAvailable(bool* available) const override;
            int32_t SetStereoPlayout(bool enable) override;
            int32_t StereoPlayout(bool* enabled) const override;
            int32_t StereoRecordingIsAvailable(bool* available) const override;
            int32_t SetStereoRecording(bool enable) override;
            int32_t StereoRecording(bool* enabled) const override;
            // int32_t SetRecordingChannel(const ChannelType channel) override;
            // int32_t RecordingChannel(ChannelType* channel) const override;

            // int32_t SetPlayoutBuffer(const BufferType type, uint16_t size_ms = 0) override;
            // int32_t PlayoutBuffer(BufferType* type, uint16_t* size_ms) const override;
            int32_t PlayoutDelay(uint16_t* delay_ms) const override;
            // int32_t RecordingDelay(uint16_t* delay_ms) const override;

            // int32_t CPULoad(uint16_t* load) const override;

            // int32_t StartRawOutputFileRecording(
            //     const char pcm_file_name_utf8[webrtc::kAdmMaxFileNameSize]) override;
            // int32_t StopRawOutputFileRecording() override;
            // int32_t StartRawInputFileRecording(
            //     const char pcm_file_name_utf8[webrtc::kAdmMaxFileNameSize]) override;
            // int32_t StopRawInputFileRecording() override;

            // int32_t SetRecordingSampleRate(const uint32_t samples_per_sec) override;
            // int32_t RecordingSampleRate(uint32_t* samples_per_sec) const override;
            // int32_t SetPlayoutSampleRate(const uint32_t samples_per_sec) override;
            // int32_t PlayoutSampleRate(uint32_t* samples_per_sec) const override;

            // int32_t ResetAudioDevice() override;
            // int32_t SetLoudspeakerStatus(bool enable) override;
            // int32_t GetLoudspeakerStatus(bool* enabled) const override;

            bool BuiltInAECIsAvailable() const override {
                return false;
            }

            int32_t EnableBuiltInAEC(bool enable) override {
                return -1;
            }

            bool BuiltInAGCIsAvailable() const override {
                return false;
            }

            int32_t EnableBuiltInAGC(bool enable) override {
                return -1;
            }

            bool BuiltInNSIsAvailable() const override {
                return false;
            }

            int32_t EnableBuiltInNS(bool enable) override {
                return -1;
            }
#if defined(WEBRTC_IOS)

            int GetPlayoutAudioParameters(webrtc::AudioParameters* params) const override {
                return -1;
            }

            int GetRecordAudioParameters(webrtc::AudioParameters* params) const override {
                return -1;
            }
#endif // WEBRTC_IOS

            /// End of functions inherited from `webrtc::AudioDeviceModule`.

            /// The following function is inherited from `rtc::MessageHandler`.
            void OnMessage(rtc::Message* msg) override;

        protected:
            /// The constructor is protected because the class needs to be created as a
            /// reference counted object (for memory managment reasons). It could be
            /// exposed in which case the burden of proper instantiation would be put on
            /// the creator of a AudioPacketModule instance. To create an instance of
            /// this class use the `Create()` API.
            explicit AudioPacketModule();

            /// The destructor is protected because it is reference counted and should
            /// not be deleted directly.
            virtual ~AudioPacketModule();

        private:
            /// Initializes the state of the `AudioPacketModule`. This API is called on
            /// creation by the `Create()` API.
            bool Initialize();

            /// Returns true/false depending on if recording or playback has been
            /// enabled/started.
            bool shouldStartProcessing();

            /// Starts or stops the pushing and pulling of audio frames.
            void updateProcessing(bool start);

            /// Starts the periodic calling of `ProcessFrame()` in a thread safe way.
            void startProcessP();

            /// Periodcally called function that ensures that frames are pulled and
            /// pushed
            /// periodically if enabled/started.
            void processFrameP();

            /// Pulls frames from the registered webrtc::AudioTransport.
            void receiveFrameP();

            /// Pushes frames to the registered webrtc::AudioTransport.
            void sendFrameP();

            /// The time in milliseconds when Process() was last called or 0 if no call
            /// has been made.
            int64_t _lastProcessTimeMS;

            /// Callback for playout and recording.
            webrtc::AudioTransport* _audioCallback;

            bool _recording; ///< True when audio is being pushed from the instance.
            bool _playing; ///< True when audio is being pulled by the instance.

            bool _playIsInitialized; ///< True when the instance is ready to pull audio.
            bool _recIsInitialized; ///< True when the instance is ready to push audio.

            /// Input to and output from RecordedDataIsAvailable(..) makes it possible
            /// to modify the current mic level. The implementation does not care about
            /// the mic level so it just feeds back what it receives.
            uint32_t _currentMicLevel;

            /// `_nextFrameTime` is updated in a non-drifting manner to indicate the
            /// next wall clock time the next frame should be generated and received.
            /// `_started` ensures that _nextFrameTime can be initialized properly on first call.
            bool _started;
            int64_t _nextFrameTime;

            std::unique_ptr<rtc::Thread> _processThread;

            /// A FIFO buffer that stores samples from the audio source to be sent.
            ff::AudioBuffer _sendFifo;

            /// A buffer with enough storage for a 10ms of samples to send.
            std::vector<Sample> _sendSamples;

            /// Protects variables that are accessed from `_processThread` and
            /// the main thread.
            rtc::CriticalSection _crit;

            /// Protects |_audioCallback| that is accessed from `_processThread` and
            /// the main thread.
            rtc::CriticalSection _critCallback;
        };


#endif


    }

} // namespace wrtc


#endif // HAVE_FFMPEG
#endif // WebRTC_AudioPacketModule_H



/*
 *  Copyright 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
