#pragma once

#include <SFML/Audio.hpp>

#include <array>
#include <cstddef>
#include <list>
#include <string>
#include <string_view>
#include <unordered_map>

struct SoundAssetDefinition {
    std::string_view key;
    std::string_view relativePath;
    float minimumInterval;
};

class AudioManager {
public:
    static constexpr std::size_t MAX_VOICES = 28;
    static const std::array<SoundAssetDefinition, 11>& soundAssets();

    explicit AudioManager(bool playbackEnabled = true);

    bool playSound(std::string_view key);
    bool playSfx(const std::string& name, const std::string& relativePath);
    bool playMusic(const std::string& relativePath);
    void update(float deltaTime);

    void stopMusic();
    void pauseMusic();
    void resumeMusic();
    void toggleMute();
    void setMuted(bool value);
    bool isMuted() const { return muted; }

    bool handleMuteKeyPressed();
    void handleMuteKeyReleased();

    std::size_t activeVoiceCount() const { return voices.size(); }
    std::size_t loadedBufferCount() const { return soundBuffers.size(); }
    std::size_t acceptedPlayCount(std::string_view key) const;
    bool isPlaybackEnabled() const { return playbackEnabled; }

private:
    bool playResolved(std::string_view key, std::string_view relativePath,
                      float minimumInterval);
    const SoundAssetDefinition* findDefinition(std::string_view key) const;
    void removeStoppedVoices();

    bool playbackEnabled = true;
    bool muted = false;
    bool muteKeyHeld = false;
    sf::Music music;
    std::string currentMusic;
    std::unordered_map<std::string, sf::SoundBuffer> soundBuffers;
    std::list<sf::Sound> voices;
    std::unordered_map<std::string, float> cooldowns;
    std::unordered_map<std::string, std::size_t> acceptedCounts;
};
