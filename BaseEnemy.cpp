#include "BaseEnemy.h"
#include "Player.h"

using namespace KamataEngine;

void BaseEnemy::Initialize(Camera* camera, Vector3 position) {
	InitializeCommon(camera, position);
	InitializeInternal();
}

void BaseEnemy::Draw() {
	Model::PreDraw();

	model_->Draw(worldTransform_, *camera_, &objectColor_);

	Model::PostDraw();
}

void BaseEnemy::StartHitFlash() {
	// タイマーを設定
	hitFlashTimer_ = kHitFlashFrames;

	// ヒット時の色を適用
	objectColor_.SetColor(kHitFlashColor);
}

void BaseEnemy::UpdateHitFlash() {
	// フラッシュ中でなければ何もしない
	if (hitFlashTimer_ <= 0) {
		return;
	}

	// タイマー更新
	--hitFlashTimer_;

	// フラッシュが終わったら通常色へ戻す
	if (hitFlashTimer_ <= 0) {
		objectColor_.SetColor(kNormalColor);
	}
}

Vector3 BaseEnemy::GetWorldPosition() const {
	Vector3 worldPosition{};

	worldPosition.x = worldTransform_.matWorld_.m[3][0];
	worldPosition.y = worldTransform_.matWorld_.m[3][1];
	worldPosition.z = worldTransform_.matWorld_.m[3][2];

	return worldPosition;
}

void BaseEnemy::InitializeCommon(Camera* camera, Vector3 position) {
	#ifdef _DEBUG

	assert(camera);

	#endif // _DEBUG

	// ワールド変換の初期化
	worldTransform_.Initialize();

	// 初期配置座標
	worldTransform_.translation_ = position;

	// 引数で渡されたカメラをメンバ変数に保存
	camera_ = camera;

	isCollisionDisabled_ = false;
	isDead_ = false;

	// ヒットフラッシュの初期化
	objectColor_.Initialize();
	objectColor_.SetColor(kNormalColor);
	hitFlashTimer_ = 0;
}