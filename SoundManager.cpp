#include "SoundManager.h"

using namespace KamataEngine;

SoundManager* SoundManager::GetInstance() {
	static SoundManager instance;

	return &instance;
}

void SoundManager::Initialize() {
	Audio* audio = Audio::GetInstance();

	//============================================================
	// BGM
	//============================================================

	bgmSoundHandles_[static_cast<size_t>(BgmType::kTitle)] = audio->LoadWave("sound/bgm/title.mp3");

	bgmSoundHandles_[static_cast<size_t>(BgmType::kStageSelect)] = audio->LoadWave("sound/bgm/stage_select.mp3");

	bgmSoundHandles_[static_cast<size_t>(BgmType::kGame)] = audio->LoadWave("sound/bgm/game.mp3");

	//============================================================
	// SE
	//============================================================

	seSoundHandles_[static_cast<size_t>(SeType::kAttack)] = audio->LoadWave("sound/se/attack.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kHit)] = audio->LoadWave("sound/se/hit.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kDamage)] = audio->LoadWave("sound/se/damage.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kJump)] = audio->LoadWave("sound/se/jump.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kStep)] = audio->LoadWave("sound/se/step.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kShot)] = audio->LoadWave("sound/se/shot.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kHeal)] = audio->LoadWave("sound/se/heal.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kGuard)] = audio->LoadWave("sound/se/guard.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kPerfectDodge)] = audio->LoadWave("sound/se/perfect_dodge.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kClear)] = audio->LoadWave("sound/se/clear.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kMenuMove)] = audio->LoadWave("sound/se/menu_move.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kMenuConfirm)] = audio->LoadWave("sound/se/menu_confirm.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kMenuCancel)] = audio->LoadWave("sound/se/menu_cancel.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kPlayerDeath)] = audio->LoadWave("sound/se/player_death.mp3");

	seSoundHandles_[static_cast<size_t>(SeType::kEnemyDeath)] = audio->LoadWave("sound/se/enemy_death.mp3");
}

void SoundManager::PlayBgm(BgmType type) {
	Audio* audio = Audio::GetInstance();

	//============================================================
	// 同じBGMが現在再生中なら何もしない
	//============================================================

	if (currentBgm_ == type && bgmVoiceHandle_ != 0 && audio->IsPlaying(bgmVoiceHandle_)) {
		return;
	}

	// 現在のBGMを停止
	StopBgm();

	size_t index = static_cast<size_t>(type);

	// 総括音量と個別音量から実際の再生音量を計算
	float volume = kBgmMasterVolume * kBgmIndividualVolumes[index];

	bgmVoiceHandle_ = audio->PlayWave(bgmSoundHandles_[index], true, volume);

	currentBgm_ = type;
}

void SoundManager::StopBgm() {
	// BGMを再生していなければ何もしない
	if (currentBgm_ == BgmType::kCount) {
		return;
	}

	// 現在のBGMを停止
	Audio::GetInstance()->StopWave(bgmVoiceHandle_);

	bgmVoiceHandle_ = 0;

	currentBgm_ = BgmType::kCount;
}

void SoundManager::PlaySe(SeType type) {
	size_t index = static_cast<size_t>(type);

	Audio::GetInstance()->PlayWave(seSoundHandles_[index], false, kSeVolume);
}