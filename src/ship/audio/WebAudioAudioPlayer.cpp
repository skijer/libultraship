#ifdef __EMSCRIPTEN__
#include "ship/audio/WebAudioAudioPlayer.h"
#include <spdlog/spdlog.h>

extern "C" {
int lus_webaudio_init(int sampleRate, int channels, int ringCapacityFrames);
void lus_webaudio_close(void);
int lus_webaudio_buffered(void);
int lus_webaudio_failed(void);
void lus_webaudio_play(const uint8_t* buf, size_t len, int maxQueuedFrames);
}

namespace Ship {

static const int32_t kRingCapacityFrames = 16384;

static const int32_t kMaxQueuedFrames = 6000;

WebAudioAudioPlayer::~WebAudioAudioPlayer() {
    SPDLOG_TRACE("destruct Web Audio player");
    DoClose();
}

bool WebAudioAudioPlayer::DoInit() {
    mNumChannels = this->GetNumOutputChannels();
    if (!lus_webaudio_init(this->GetSampleRate(), mNumChannels, kRingCapacityFrames)) {
        SPDLOG_ERROR("Web Audio: no AudioContext with AudioWorklet support in this browser");
        return false;
    }
    SPDLOG_INFO("Web Audio initialized: {} channels, {} Hz requested", mNumChannels, this->GetSampleRate());
    return true;
}

void WebAudioAudioPlayer::DoClose() {
    lus_webaudio_close();
}

int32_t WebAudioAudioPlayer::Buffered() {
    return lus_webaudio_buffered();
}

bool WebAudioAudioPlayer::HasFailed() {
    return lus_webaudio_failed() != 0;
}

void WebAudioAudioPlayer::DoPlay(const uint8_t* buf, size_t len) {
    lus_webaudio_play(buf, len, kMaxQueuedFrames);
}

} // namespace Ship
#endif // __EMSCRIPTEN__
