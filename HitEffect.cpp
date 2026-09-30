#include "HitEffect.h"
#include "Matrix4x4Util.h"
#include <algorithm>
#include <cassert>
#include <random>
#include"Ease.h"
#include"Vector3Util.h"

using namespace KamataEngine;

Model* HitEffect::model_ = nullptr;

namespace {

std::random_device seedGenerator;
std::mt19937_64 randomEngine(seedGenerator());

float RandomFloat(float min, float max) {
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(randomEngine);
}

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



void HitEffect::InitializeInternal(const Vector3& position) {
	assert(model_);

	state_ = State::kSpread;

	// 円形エフェクト
	circleWorldTransform_.Initialize();
	circleWorldTransform_.translation_ = position;
	circleWorldTransform_.scale_ = {kCircleStartScale, kCircleStartScale, kCircleStartScale};

	// 楕円エフェクト
	for (WorldTransform& ellipseWorldTransform : ellipseWorldTransforms_) {
		ellipseWorldTransform.Initialize();
		ellipseWorldTransform.translation_ = position;
		ellipseWorldTransform.scale_ = {0.0f, 0.0f, 1.0f};

		// Z軸まわりにランダム回転
		ellipseWorldTransform.rotation_ = {0.0f, 0.0f, RandomFloat(0.0f, 2.0f * kPi)};
	}

	WorldTransformUpdate(circleWorldTransform_);

	for (WorldTransform& ellipseWorldTransform : ellipseWorldTransforms_) {
		WorldTransformUpdate(ellipseWorldTransform);
	}

}

void HitEffect::Update() {
	if (state_ == State::kDead) {
		return;
	}

	counter_ += kFrameTime;

	switch (state_) {
	case State::kSpread: {
		float t = std::clamp(counter_ / kSpreadDuration, 0.0f, 1.0f);

		// 円を拡大
		float circleScale = EaseOutFloat(kCircleStartScale, kCircleEndScale, t);
		circleWorldTransform_.scale_ = {circleScale, circleScale, circleScale};

		// 楕円を細長く拡大
		for (WorldTransform& ellipseWorldTransform : ellipseWorldTransforms_) {
			ellipseWorldTransform.scale_ = {
			    EaseOutFloat(0.0f, kEllipseLength, t),
			    EaseOutFloat(0.0f, kEllipseThickness, t),
			    1.0f,
			};
		}

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

	WorldTransformUpdate(circleWorldTransform_);

	for (WorldTransform& ellipseWorldTransform : ellipseWorldTransforms_) {
		WorldTransformUpdate(ellipseWorldTransform);
	}
}

void HitEffect::Draw() {
	if (state_ == State::kDead) {
		return;
	}

	if (model_ == nullptr || camera_ == nullptr) {
		return;
	}
	Model::PreDraw();

	// 円形エフェクト
	model_->Draw(circleWorldTransform_, *camera_, &objectColor_);

	// 楕円エフェクト
	for (WorldTransform& ellipseWorldTransform : ellipseWorldTransforms_) {
		model_->Draw(ellipseWorldTransform, *camera_, &objectColor_);
	}

	Model::PostDraw();
}

HitEffect* HitEffect::Create(const Vector3& position) {
	// インスタンス生成
	HitEffect* instance = new HitEffect();

	// newの失敗を検出
	assert(instance);

	// インスタンスの初期化
	instance->Initialize(position);

	// 初期化したインスタンスを返す
	return instance;
}
