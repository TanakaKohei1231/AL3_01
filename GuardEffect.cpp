#include "GuardEffect.h"
#include "Matrix4x4Util.h"
#include <algorithm>
#include <cassert>
#include"Ease.h"
#include"Vector3Util.h"

using namespace KamataEngine;

Model* GuardEffect::model_ = nullptr;

namespace {

float LerpFloat(float start, float end, float t) { return start + (end - start) * t; }

float EaseOutFloat(float start, float end, float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	float easedT = 1.0f - (1.0f - t) * (1.0f - t);
	return LerpFloat(start, end, easedT);
}

float EaseInFloat(float start, float end, float t) {
	t = std::clamp(t, 0.0f, 1.0f);
	float easedT = t * t;
	return LerpFloat(start, end, easedT);
}


} // namespace



void GuardEffect::InitializeInternal(const Vector3& position) {
	assert(model_);

	state_ = State::kSpread;

	// エフェクト
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {kCircleStartScale, kCircleStartScale, kCircleStartScale};

	WorldTransformUpdate(worldTransform_);
}

void GuardEffect::Update() {
	if (state_ == State::kDead) {
		return;
	}

	counter_ += kFrameTime;

	switch (state_) {
	case State::kSpread: {
		float t = std::clamp(counter_ / kSpreadDuration, 0.0f, 1.0f);

		// 円を拡大
		float circleScale = EaseOutFloat(kCircleStartScale, kCircleEndScale, t);
		worldTransform_.scale_ = {circleScale, circleScale, circleScale};

		if (counter_ >= kSpreadDuration) {
			counter_ = 0.0f;
			state_ = State::kFadeOut;
		}

		break;
	}

	case State::kFadeOut: {
		float t = std::clamp(counter_ / kFadeDuration, 0.0f, 1.0f);

		// アルファ値を下げる
		color_.w = EaseInFloat(1.0f, 0.0f, t);
		objectColor_.SetColor(color_);

		if (counter_ >= kFadeDuration) {
			state_ = State::kDead;
		}

		break;
	}

	case State::kDead:
		break;
	}

	WorldTransformUpdate(worldTransform_);
}

void GuardEffect::Draw() {
	if (state_ == State::kDead) {
		return;
	}

	if (model_ == nullptr || camera_ == nullptr) {
		return;
	}
	Model::PreDraw();

	// 円形エフェクト
	model_->Draw(worldTransform_, *camera_, &objectColor_);

	Model::PostDraw();
}

GuardEffect* GuardEffect::Create(const Vector3& position) {
	// インスタンス生成
	GuardEffect* instance = new GuardEffect();

	// newの失敗を検出
	assert(instance);

	// インスタンスの初期化
	instance->Initialize(position);

	// 初期化したインスタンスを返す
	return instance;
}
