#include "TitleScene.h"
#include "Matrix4x4Util.h"
#include "Vector3Util.h"
#include <cassert>
#include <cmath>
#include <numbers>

using namespace KamataEngine;

void TitleScene::Initialize() {
	finished_ = false;
	phase_ = Phase::kFadeIn;
	animationTimer_ = 0.0f;

	// カメラ初期化
	camera_.Initialize();
	camera_.translation_ = {0.0f, 1.0f, -10.0f};
	camera_.rotation_ = {0.0f, 0.0f, 0.0f};
	camera_.UpdateMatrix();

	// モデル読み込み
	// タイトル文字モデルがある場合
	modelTitle_ = Model::CreateFromOBJ("titleFont", true);

	// プレイヤーモデル
	modelPlayer_ = Model::CreateFromOBJ("player", true);

	// タイトル文字のワールド変換
	worldTransformTitle_.Initialize();
	worldTransformTitle_.translation_ = {0.0f, 3.0f, 0.0f};
	worldTransformTitle_.scale_ = {0.5f, 0.5f, 0.5f};

	// プレイヤーモデルのワールド変換
	worldTransformPlayer_.Initialize();
	worldTransformPlayer_.translation_ = {0.0f, -1.0f, 0.0f};
	worldTransformPlayer_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// フェード
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::kFadeIn, 1.0f);
}

void TitleScene::Update() {
	// アニメーション時間を進める
	animationTimer_ += 1.0f / 60.0f;

	// プレイヤーを上下にふわふわさせる
	float floating = std::sin(2.0f * kPi * animationTimer_) * 0.2f;
	worldTransformPlayer_.translation_.y = -1.0f + floating;

	// プレイヤーをゆっくり回転させる
	worldTransformPlayer_.rotation_.y += 0.02f;

	// タイトル文字も少しだけ上下させる
	float titleFloating = std::sin(2.0f * kPi * animationTimer_ * 0.5f) * 0.1f;
	worldTransformTitle_.translation_.y = 3.0f + titleFloating;

	// ワールド行列更新
	WorldTransformUpdate(worldTransformTitle_);
	WorldTransformUpdate(worldTransformPlayer_);

	// カメラ行列更新
	camera_.UpdateMatrix();

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();

		if (fade_->IsFinished()) {
			fade_->Stop();
			phase_ = Phase::kMain;
		}
		break;

	case Phase::kMain:
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::kFadeOut, 1.0f);
		}
		break;

	case Phase::kFadeOut:
		fade_->Update();

		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}
}

void TitleScene::Draw() {
	Model::PreDraw();

	if (modelTitle_) {
		modelTitle_->Draw(worldTransformTitle_, camera_);
	}

	if (modelPlayer_) {
		modelPlayer_->Draw(worldTransformPlayer_, camera_);
	}

	Model::PostDraw();

	if (fade_) {
		fade_->Draw();
	}
}

TitleScene::~TitleScene() {
	delete modelTitle_;
	modelTitle_ = nullptr;

	delete modelPlayer_;
	modelPlayer_ = nullptr;

	delete fade_;
	fade_ = nullptr;
}
