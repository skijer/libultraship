#pragma once
#include "AudioPlayer.h"

namespace Ship {
class WebAudioAudioPlayer final : public AudioPlayer {
  public:
    WebAudioAudioPlayer(AudioSettings settings) : AudioPlayer(settings) {
    }
    ~WebAudioAudioPlayer();

    int32_t Buffered() override;
    bool HasFailed() override;

  protected:
    bool DoInit() override;
    void DoClose() override;
    void DoPlay(const uint8_t* buf, size_t len) override;

  private:
    int32_t mNumChannels = 2;
};
} // namespace Ship
