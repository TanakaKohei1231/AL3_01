#include "BaseEffect.h"

using namespace KamataEngine;

Camera* BaseEffect::camera_ = nullptr;

void BaseEffect::Initialize(const Vector3& position) {
	InitializeCommon();
	InitializeInternal(position);
}

void BaseEffect::InitializeCommon() {
	// カメラがセットされているか確認
	assert(camera_);

	// 経過時間の初期化
	counter_ = 0.0f;

	// 色変更オブジェクトの初期化
	objectColor_.Initialize();
	color_ = {1.0f, 1.0f, 1.0f, 1.0f};
	objectColor_.SetColor(color_);
}

