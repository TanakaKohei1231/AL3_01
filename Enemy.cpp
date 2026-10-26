#include "Enemy.h"
#include "MapChipField.h"
#include "Vector3Util.h"
#include "Matrix4x4Util.h"
#include<numbers>
#include<cmath>
#include "Player.h"
#include <algorithm>
#include "Ease.h"
#include"GameScene.h"
#include "ModelManager.h"
#include "CharacterEnemyBehavior.h"

using namespace KamataEngine;

void Enemy::InitializeInternal() {
	// モデルマネージャーからモデルを取得
	model_ = ModelManager::GetInstance()->Load("enemy", true);

	// モデルの向きに合わせてY軸回りに270度回転させる
	worldTransform_.rotation_.y = kDefaultAngle;

	// 最初の振る舞いを要求
	RequestBehaviorChange(std::make_unique<NormalEnemyWalk>(this));
}

void Enemy::OnCollision(Player* player) {
	// 死亡演出中は何もしない
	if (isCollisionDisabled_) {
		return;
	}

	// プレイヤーが攻撃中でなければ何もしない
	if (!player->IsAttack()) {
		return;
	}

	// ヒットフラッシュ開始
	StartHitFlash();

	// 死亡演出へ移行
	RequestBehaviorChange(std::make_unique<EnemyDeathBehavior>(this));

	// 衝突判定を無効にする
	isCollisionDisabled_ = true;

	// ヒットエフェクト
	Vector3 effectPosition = (GetWorldPosition() + player->GetWorldPosition()) / 2.0f;

	if (gameScene_) {
		gameScene_->CreateHitEffect(effectPosition);
	}
}