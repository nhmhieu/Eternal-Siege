#include "AudioManager.h"

#include "AssetLocator.h"

#include <algorithm>
#include <iostream>

namespace {
const std::array<SoundAssetDefinition, 11> SOUND_ASSETS{{
    {"ui_click", "assets/audio/sfx/ui_click.wav", 0.05f},
    {"spirit_bolt", "assets/audio/sfx/spirit_bolt.wav", 0.04f},
    {"melee_hit", "assets/audio/sfx/melee_hit.wav", 0.045f},
    {"bow_shot", "assets/audio/sfx/bow_shot.wav", 0.045f},
    {"enemy_death", "assets/audio/sfx/enemy_death.wav", 0.065f},
    {"radiant_pulse", "assets/audio/sfx/radiant_pulse.wav", 0.15f},
    {"ally_skill", "assets/audio/sfx/ally_skill.wav", 0.10f},
    {"wave_start", "assets/audio/sfx/wave_start.wav", 0.35f},
    {"boss_spawn", "assets/audio/sfx/boss_spawn.wav", 0.75f},
    {"victory", "assets/audio/sfx/victory.wav", 1.50f},
    {"defeat", "assets/audio/sfx/defeat.wav", 1.50f},
}};
}

const std::array<SoundAssetDefinition, 11>& AudioManager::soundAssets() {
    return SOUND_ASSETS;
}

AudioManager::AudioManager(bool enablePlayback)
    : playbackEnabled(enablePlayback) {}

const SoundAssetDefinition* AudioManager::findDefinition(
    std::string_view key) const {
    const auto found = std::find_if(
        SOUND_ASSETS.begin(), SOUND_ASSETS.end(),
        [key](const SoundAssetDefinition& asset) {
            return asset.key == key;
        });
    return found == SOUND_ASSETS.end() ? nullptr : &*found;
}

bool AudioManager::playSound(std::string_view key) {
    const SoundAssetDefinition* definition = findDefinition(key);
    if (!definition) {
        std::cerr << "Unknown sound key: " << key << '\n';
        return false;
    }
    return playResolved(
        definition->key, definition->relativePath,
        definition->minimumInterval);
}

bool AudioManager::playSfx(const std::string& name,
                           const std::string& relativePath) {
    if (const SoundAssetDefinition* definition = findDefinition(name)) {
        return playResolved(
            definition->key, definition->relativePath,
            definition->minimumInterval);
    }
    return playResolved(name, relativePath, 0.05f);
}

bool AudioManager::playResolved(std::string_view key,
                                std::string_view relativePath,
                                float minimumInterval) {
    if (muted) return false;
    const std::string keyString(key);
    if (const auto found = cooldowns.find(keyString);
        found != cooldowns.end() && found->second > 0.f) {
        return false;
    }

    const auto path = AssetLocator::find(std::string(relativePath));
    if (!path) {
        std::cerr << "Missing audio asset: " << relativePath << '\n';
        return false;
    }

    removeStoppedVoices();
    if (playbackEnabled && voices.size() >= MAX_VOICES) return false;

    if (playbackEnabled) {
        auto found = soundBuffers.find(keyString);
        if (found == soundBuffers.end()) {
            sf::SoundBuffer buffer;
            if (!buffer.loadFromFile(*path)) {
                std::cerr << "Failed to load audio asset: "
                          << relativePath << '\n';
                return false;
            }
            found = soundBuffers.emplace(keyString, std::move(buffer)).first;
        }
        try {
            voices.emplace_back(found->second);
            voices.back().play();
        } catch (const std::exception& error) {
            std::cerr << "Audio playback unavailable for " << relativePath
                      << ": " << error.what() << '\n';
            voices.clear();
            return false;
        }
    }

    cooldowns[keyString] = std::max(0.f, minimumInterval);
    ++acceptedCounts[keyString];
    return true;
}

bool AudioManager::playMusic(const std::string& relativePath) {
    if (muted || !playbackEnabled) {
        if (muted) {
            musicBeforeMute = relativePath;
        }
        return false;
    }
    if (currentMusic == relativePath &&
        music.getStatus() == sf::SoundSource::Status::Playing) {
        return true;
    }
    const auto path = AssetLocator::find(relativePath);
    if (!path) {
        std::cerr << "Missing music asset: " << relativePath << '\n';
        return false;
    }
    if (!music.openFromFile(*path)) {
        std::cerr << "Failed to load music asset: " << relativePath << '\n';
        return false;
    }
    currentMusic = relativePath;
    musicBeforeMute.clear();
    music.setLooping(true);
    music.play();
    return true;
}

void AudioManager::update(float deltaTime) {
    for (auto& entry : cooldowns) {
        entry.second = std::max(0.f, entry.second - deltaTime);
    }
    removeStoppedVoices();
}

void AudioManager::removeStoppedVoices() {
    if (!playbackEnabled) return;
    voices.remove_if([](const sf::Sound& sound) {
        return sound.getStatus() == sf::SoundSource::Status::Stopped;
    });
}

void AudioManager::stopMusic() {
    if (playbackEnabled) music.stop();
    currentMusic.clear();
    musicBeforeMute.clear();
}

void AudioManager::pauseMusic() {
    if (playbackEnabled &&
        music.getStatus() == sf::SoundSource::Status::Playing) {
        music.pause();
    }
}

void AudioManager::resumeMusic() {
    if (!muted && playbackEnabled &&
        music.getStatus() == sf::SoundSource::Status::Paused) {
        music.play();
    }
}

void AudioManager::toggleMute() {
    setMuted(!muted);
}

void AudioManager::setMuted(bool value) {
    if (muted == value) return;
    muted = value;
    if (muted) {
        musicBeforeMute = currentMusic;
        if (playbackEnabled) {
            for (sf::Sound& voice : voices) voice.stop();
            music.stop();
        }
        voices.clear();
        currentMusic.clear();
    } else {
        if (!musicBeforeMute.empty()) {
            std::string restoreTrack = std::move(musicBeforeMute);
            musicBeforeMute.clear();
            playMusic(restoreTrack);
        }
    }
}

bool AudioManager::handleMuteKeyPressed() {
    if (muteKeyHeld) return false;
    muteKeyHeld = true;
    toggleMute();
    return true;
}

void AudioManager::handleMuteKeyReleased() {
    muteKeyHeld = false;
}

std::size_t AudioManager::acceptedPlayCount(std::string_view key) const {
    const auto found = acceptedCounts.find(std::string(key));
    return found == acceptedCounts.end() ? 0 : found->second;
}
