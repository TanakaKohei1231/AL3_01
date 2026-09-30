#include "BaseEnemy.h"
#include "Player.h"

using namespace KamataEngine;

void BaseEnemy::Initialize(Model* model, Camera* camera, Vector3 position) {
	InitializeCommon(model, camera, position);
	InitializeInternal();
}

void BaseEnemy::Draw() {
	Model::PreDraw();

	model_->Draw(worldTransform_, *camera_);

	Model::PostDraw();
}

Vector3 BaseEnemy::GetWorldPosition() const {
	Vector3 worldPosition{};

	worldPosition.x = worldTransform_.matWorld_.m[3][0];
	worldPosition.y = worldTransform_.matWorld_.m[3][1];
	worldPosition.z = worldTransform_.matWorld_.m[3][2];

	return worldPosition;
}

void BaseEnemy::InitializeCommon(Model* model, Camera* camera, Vector3 position) {
	#ifdef _DEBUG

	assert(model);
	assert(camera);

	#endif // _DEBUG

	// 引数で渡されたモデルとテクスチャハンドルをメンバ変数に保存
	model_ = model;

	// ワールド変換の初期化
	worldTransform_.Initialize();

	// 初期配置座標
	worldTransform_.translation_ = position;

	// 引数で渡されたカメラをメンバ変数に保存
	camera_ = camera;

	isCollisionDisabled_ = false;
	isDead_ = false;
}