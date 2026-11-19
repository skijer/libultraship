#include <algorithm>
#include "ship/audio/Audio.h"

#ifdef __APPLE__
#include "ship/audio/CoreAudioAudioPlayer.h"
#endif

#include "ship/Context.h"
#include "ship/config/Config.h"
#include "ship/controller/controldeck/ControlDeck.h"

#include <cstdlib>

namespace Ship {

Audio::~Audio() {
    SPDLOG_TRACE("destruct audio");
}

void Audio::InitAudioPlayer() {
    switch (GetCurrentAudioBackend()) {
#ifdef _WIN32
        case AudioBackend::WASAPI:
            mAudioPlayer = std::make_shared<WasapiAudioPlayer>(this->mAudioSettings);
            break;
#endif
#ifdef __APPLE__
        case AudioBackend::COREAUDIO:
            mAudioPlayer = std::make_shared<CoreAudioAudioPlayer>(this->mAudioSettings);
            break;
#endif
#ifdef __EMSCRIPTEN__
        case AudioBackend::WEBAUDIO:
            mAudioPlayer = std::make_shared<WebAudioAudioPlayer>(this->mAudioSettings);
            break;
#endif
        case AudioBackend::SDL:
            mAudioPlayer = std::make_shared<SDLAudioPlayer>(this->mAudioSettings);
            break;
        default:
            mAudioPlayer = std::make_shared<NullAudioPlayer>(this->mAudioSettings);
            break;
    }

    if (mAudioPlayer && !mAudioPlayer->Init()) {
#ifdef __EMSCRIPTEN__
        if (GetCurrentAudioBackend() == AudioBackend::WEBAUDIO) {
            FallBackToSdl("no AudioContext with AudioWorklet in this browser");
            return;
        }
#endif
        // Failed to initialize system audio player.
        // Fallback to Null if the native system player does not work.
        SetCurrentAudioBackend(AudioBackend::NUL);
    }
}

void Audio::FallBackToSdl(const char* why) {
    SPDLOG_WARN("Web Audio player unavailable ({}); falling back to SDL audio for this session", why);
    mAudioBackend = AudioBackend::SDL;
    InitAudioPlayer();
}

void Audio::Init() {
    mConfig = Context::GetRawInstance()->GetHostConfig();

    mAvailableAudioBackends = std::make_shared<std::vector<AudioBackend>>();
#ifdef _WIN32
    mAvailableAudioBackends->push_back(AudioBackend::WASAPI);
#endif
#ifdef __APPLE__
    mAvailableAudioBackends->push_back(AudioBackend::COREAUDIO);
#endif
#ifdef __EMSCRIPTEN__
    mAvailableAudioBackends->push_back(AudioBackend::WEBAUDIO);
#endif
    mAvailableAudioBackends->push_back(AudioBackend::SDL);
    mAvailableAudioBackends->push_back(AudioBackend::NUL);

    AudioBackend backend = GetSavedAudioBackend();
    if (std::find(mAvailableAudioBackends->begin(), mAvailableAudioBackends->end(), backend) ==
        mAvailableAudioBackends->end()) {
        SPDLOG_WARN("The configured audio backend is not available in this build; using the default");
        backend = mAvailableAudioBackends->front();
    }
    SetCurrentAudioBackend(backend);
    SetAudioChannels(GetSavedAudioChannelsSetting());
}

std::shared_ptr<AudioPlayer> Audio::GetAudioPlayer() {
#ifdef __EMSCRIPTEN__
    if (mAudioPlayer && mAudioPlayer->HasFailed()) {
        FallBackToSdl("the AudioWorklet could not be loaded");
    }
#endif
    return mAudioPlayer;
}

AudioBackend Audio::GetCurrentAudioBackend() {
    return mAudioBackend;
}

AudioBackend Audio::GetSavedAudioBackend() {
    std::string backendName = mConfig->GetString("Window.AudioBackend");

    const char* env = std::getenv("SHIP_AUDIO_BACKEND");
    const bool fromEnv = env != nullptr && *env != '\0';
    if (fromEnv) {
        backendName = env;
    }
    if (backendName == "wasapi") {
        return AudioBackend::WASAPI;
    }

    // Migrate pulse player in config to sdl
    if (backendName == "pulse") {
        mConfig->SetString("Window.AudioBackend", "sdl");
        mConfig->Save();
        return AudioBackend::SDL;
    }

    if (backendName == "coreaudio") {
#ifdef __APPLE__
        // The CoreAudio player persistently changes the device's system-wide sample rate.
        if (!fromEnv) {
            mConfig->SetString("Window.AudioBackend", "sdl");
            mConfig->Save();
            SPDLOG_WARN("macOS: forcing Window.AudioBackend sdl (was coreaudio). The CoreAudio player "
                        "reconfigures the output device's sample rate system-wide and persistently. "
                        "Set SHIP_AUDIO_BACKEND=coreaudio to override.");
            return AudioBackend::SDL;
        }
#endif
        return AudioBackend::COREAUDIO;
    }

    if (backendName == "sdl") {
        return AudioBackend::SDL;
    }

    if (backendName == "webaudio") {
        return AudioBackend::WEBAUDIO;
    }

    if (backendName == "null") {
        return AudioBackend::NUL;
    }

    SPDLOG_TRACE("Could not find AudioBackend matching value from config file ({}). Returning default AudioBackend.",
                 backendName);
#ifdef _WIN32
    return AudioBackend::WASAPI;
#endif

#ifdef __APPLE__
    return AudioBackend::SDL;
#endif

#ifdef __EMSCRIPTEN__
    return AudioBackend::WEBAUDIO;
#endif

    return AudioBackend::SDL;
}

void Audio::SetCurrentAudioBackend(AudioBackend backend) {
    mAudioBackend = backend;

    switch (backend) {
        case AudioBackend::WASAPI:
            mConfig->SetString("Window.AudioBackend", "wasapi");
            break;
        case AudioBackend::COREAUDIO:
            mConfig->SetString("Window.AudioBackend", "coreaudio");
            break;
        case AudioBackend::SDL:
            mConfig->SetString("Window.AudioBackend", "sdl");
            break;
        case AudioBackend::WEBAUDIO:
            mConfig->SetString("Window.AudioBackend", "webaudio");
            break;
        case AudioBackend::NUL:
            mConfig->SetString("Window.AudioBackend", "null");
            break;
        default:
            mConfig->SetString("Window.AudioBackend", "");
    }
    mConfig->Save();

    InitAudioPlayer();
}

std::shared_ptr<std::vector<AudioBackend>> Audio::GetAvailableAudioBackends() {
    return mAvailableAudioBackends;
}

void Audio::SetAudioChannels(AudioChannelsSetting channels) {
    if (mAudioSettings.ChannelSetting != channels) {
        mAudioSettings.ChannelSetting = channels;
        // Reinitialize the existing audio player with the new channel configuration
        if (mAudioPlayer) {
            mAudioPlayer->SetAudioChannels(channels);
        }
    }
}

AudioChannelsSetting Audio::GetAudioChannels() const {
    return mAudioSettings.ChannelSetting;
}

AudioChannelsSetting Audio::GetSavedAudioChannelsSetting() {
    int32_t channelsSetting =
        mConfig->GetInt("CVars." CVAR_AUDIO_CHANNELS_SETTING, static_cast<int32_t>(AudioChannelsSetting::audioMax));
    switch (channelsSetting) {
        case AudioChannelsSetting::audioMatrix51:
            return AudioChannelsSetting::audioMatrix51;
        case AudioChannelsSetting::audioRaw51:
            return AudioChannelsSetting::audioRaw51;
        case AudioChannelsSetting::audioStereo:
        case AudioChannelsSetting::audioMax:
        default:
            return AudioChannelsSetting::audioStereo;
    }
}

} // namespace Ship
