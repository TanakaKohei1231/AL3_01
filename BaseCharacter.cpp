#include "BaseCharacter.h"
#include <utility>

using namespace KamataEngine;

void BaseCharacter::Initialize(KamataEngine::Camera* camera, KamataEngine::Vector3 position) {

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

	// キャラクター固有の初期化
	InitializeInternal();
}

void BaseCharacter::Update() {
	// 振る舞い更新前の個別処理
	BeforeBehaviorUpdate();

	// 振る舞い変更リクエストを反映
	ChangeBehavior();

	// 振る舞いの更新
	if (behavior_) {
		behavior_->Update();
		behavior_->UpdateMotion();
	}

	// 振る舞い更新後の個別処理
	AfterBehaviorUpdate();

	// ワールド変換の更新
	WorldTransformUpdate(worldTransform_);

	// ワールド変換更新後の個別処理
	AfterTransformUpdate();
}

void BaseCharacter::Draw() {
	Model::PreDraw();

	model_->Draw(worldTransform_, *camera_, &objectColor_);

	Model::PostDraw();
}

void BaseCharacter::StartHitFlash() {
	// タイマーを設定
	hitFlashTimer_ = kHitFlashFrames;

	// ヒット時の色を適用
	objectColor_.SetColor(kHitFlashColor);
}

void BaseCharacter::UpdateHitFlash() {
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

AABB BaseCharacter::GetAABB() const {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb{};
	aabb.min = {worldPos.x - width_ / 2.0f, worldPos.y - height_ / 2.0f, worldPos.z - width_ / 2.0f};
	aabb.max = {worldPos.x + width_ / 2.0f, worldPos.y + height_ / 2.0f, worldPos.z + width_ / 2.0f};

	return aabb;
}

KamataEngine::Vector3 BaseCharacter::GetWorldPosition() const {
	Vector3 worldPosition{};

	worldPosition.x = worldTransform_.matWorld_.m[3][0];
	worldPosition.y = worldTransform_.matWorld_.m[3][1];
	worldPosition.z = worldTransform_.matWorld_.m[3][2];

	return worldPosition;
}

void BaseCharacter::RequestBehaviorChange(std::unique_ptr<ICharactersBehavior> nextBehavior) {

	// 振る舞い変更を予約
	behaviorRequest_ = std::move(nextBehavior);
}

void BaseCharacter::ChangeBehavior() {
	// 変更リクエストがなければ何もしない
	if (!behaviorRequest_) {
		return;
	}

	// 新しい振る舞いを適用
	behavior_ = std::move(behaviorRequest_);

	// 振る舞いの初期化
	behavior_->Initialize();
}