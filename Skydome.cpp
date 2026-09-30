#include "Skydome.h"

using namespace KamataEngine;

void Skydome::Initialize(Model* model, Camera* camera) {

	#ifdef DEBUG

	assert(model);
	assert(camera);

	#endif // DEBUG


	// 引数で渡されたモデルをメンバ変数に保存
	model_ = model;

	// ワールド変換の初期化
	worldTransform_.Initialize();

	// 引数で渡されたカメラをメンバ変数に保存
	camera_ = camera;
}

void Skydome::Update() {
	worldTransform_.TransferMatrix();
}

void Skydome::Draw() {

	Model::PreDraw();

	model_->Draw(worldTransform_, *camera_);

	Model::PostDraw();
}