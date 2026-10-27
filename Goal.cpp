#include "Goal.h"
#include "Matrix4x4Util.h"
#include <cassert>
#include <cmath>
#include <numbers>
#include "ModelManager.h"

using namespace KamataEngine;

void Goal::Initialize(Camera* camera, const Vector3& position) {
	assert(camera);

	// 仮置きでプレイヤーモデル
	model_ = ModelManager::GetInstance()->Load("goal", true);
	camera_ = camera;

	assert(model_);

	worldTransform_.Initialize();

	// モデルの向きに合わせてY軸回りに90度回転させる
	worldTransform_.rotation_.y = kPi;

	basePosition_ = position;
	worldTransform_.translation_ = position;

	worldTransform_.scale_ = {kScale, kScale, kScale};

	animationTimer_ = 0.0f;
	isCollected_ = false;

	WorldTransformUpdate(worldTransform_);
}

void Goal::Update() {
	if (isCollected_) {
		return;
	}

	animationTimer_ += 1.0f / 60.0f;

	// 上下に浮遊
	worldTransform_.translation_.y = basePosition_.y + std::sin(animationTimer_ * 2.0f * std::numbers::pi_v<float>) * kFloatAmplitude;

	WorldTransformUpdate(worldTransform_);
}

void Goal::Draw() {
	if (isCollected_) {
		return;
	}

	Model::PreDraw();

	model_->Draw(worldTransform_, *camera_);

	Model::PostDraw();
}

AABB Goal::GetAABB() const {
	Vector3 position = GetWorldPosition();

	AABB aabb{};

	aabb.min = {position.x - kWidth / 2.0f, position.y - kHeight / 2.0f, position.z - kWidth / 2.0f};

	aabb.max = {position.x + kWidth / 2.0f, position.y + kHeight / 2.0f, position.z + kWidth / 2.0f};

	return aabb;
}

Vector3 Goal::GetWorldPosition() const {
	Vector3 worldPosition{};

	worldPosition.x = worldTransform_.matWorld_.m[3][0];

	worldPosition.y = worldTransform_.matWorld_.m[3][1];

	worldPosition.z = worldTransform_.matWorld_.m[3][2];

	return worldPosition;
}