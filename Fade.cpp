#include "Fade.h"
#include <algorithm>

using namespace KamataEngine;

void Fade::Initialize() {
	// 白画像を黒く塗って画面全体に表示する
	uint32_t textureHandle = TextureManager::Load("white1x1.png");

	sprite_ = Sprite::Create(textureHandle, {0.0f, 0.0f});
	sprite_->SetSize({kWindowWidth, kWindowHeight});

	color_ = {0.0f, 0.0f, 0.0f, 0.0f};
	sprite_->SetColor(color_);
}

void Fade::Update() {
	switch (status_) {
	case Status::kNone:
		break;

	case Status::kFadeIn:
		counter_ += 1.0f / 60.0f;
		counter_ = (std::min)(counter_, duration_);

		// 1.0fから0.0fへ下げる
		color_.w = std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f);
		sprite_->SetColor(color_);
		break;

	case Status::kFadeOut:
		counter_ += 1.0f / 60.0f;
		counter_ = (std::min)(counter_, duration_);

		// 0.0fから1.0fへ上げる
		color_.w = std::clamp(counter_ / duration_, 0.0f, 1.0f);
		sprite_->SetColor(color_);
		break;
	}
}

void Fade::Draw() {
	if (status_ == Status::kNone) {
		return;
	}

	Sprite::PreDraw();

	sprite_->Draw();

	Sprite::PostDraw();
}

void Fade::Start(Status status, float duration) {
	status_ = status;
	duration_ = (std::max)(duration, 0.001f);
	counter_ = 0.0f;

	if (status_ == Status::kFadeIn) {
		color_.w = 1.0f;
	} else if (status_ == Status::kFadeOut) {
		color_.w = 0.0f;
	}

	sprite_->SetColor(color_);
}

Fade::~Fade() {
	delete sprite_;
	sprite_ = nullptr;
}

bool Fade::IsFinished() const {
	switch (status_) {
	case Status::kFadeIn:
	case Status::kFadeOut:
		return counter_ >= duration_;

	case Status::kNone:
	default:
		return true;
	}
}