#include "DeathParticles.h"
#include"Vector3Util.h"
#include "Matrix4x4Util.h"
#include <cassert>
#include <algorithm>

using namespace KamataEngine;

void DeathParticles::Initialize(Model* model, Camera* camera, const Vector3& position) {
	if (model == nullptr) {
		assert(model);
	}

	if (camera == nullptr) {
		assert(camera);
	}

	model_ = model;
	camera_ = camera;
	isFinished_ = false;
	counter_ = 0.0f;
	objectColor_.Initialize();
	color_ = {1.0f, 1.0f, 1.0f, 1.0f};
	objectColor_.SetColor(color_);

	// ワールド変換の初期化
	for (WorldTransform& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position;
		WorldTransformUpdate(worldTransform);
	}
}

void DeathParticles::Update() {
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	// 経過時間を進める
	counter_ += 1.0f / 60.0f;

	// フェードアウト
	color_.w = std::clamp(1.0f - counter_ / kDuration, 0.0f, 1.0f);
	objectColor_.SetColor(color_);

	// 存続時間を過ぎたら終了
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		isFinished_ = true;
		return;
	}

	for (uint32_t i = 0; i < worldTransforms_.size(); ++i) {
		// 基本となる速度ベクトル
		Vector3 velocity = {kSpeed, 0.0f, 0.0f};

		// 回転角
		float angle = kAngleUnit * static_cast<float>(i);

		// Z軸まわりの回転行列
		Matrix4x4 matrixRotation = MakeRotateZMatrix(angle);

		// 基本ベクトルを回転させて速度ベクトルを得る
		velocity = Transform(velocity, matrixRotation);

		// 移動処理
		worldTransforms_[i].translation_ += velocity;

		// ワールド変換の更新
		WorldTransformUpdate(worldTransforms_[i]);
	}
}

void DeathParticles::Draw() {
	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	Model::PreDraw();

	// モデルの描画
	for (WorldTransform& worldTransform : worldTransforms_) {
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}

	Model::PostDraw();
}
