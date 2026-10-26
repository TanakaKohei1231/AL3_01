#include "ShieldEnemy.h"
#include "Player.h"
#include "GameScene.h"
#include "Vector3Util.h"
#include "Matrix4x4Util.h"
#include "Ease.h"
#include<numbers>
#include<cmath>
#include <algorithm>
#include "ModelManager.h"
#include "CharacterEnemyBehavior.h"

using namespace KamataEngine;

void ShieldEnemy::InitializeInternal () {
	// モデルマネージャーからモデルを取得
	model_ = ModelManager::GetInstance()->Load("shieldEnemy", true);

	// モデルの向きに合わせてY軸回りに回転させる
	worldTransform_.rotation_.y = kDefaultAngle;

	// 最初の振る舞いを要求
	RequestBehaviorChange(std::make_unique<ShieldEnemyWalk>(this));
}

void ShieldEnemy::OnCollision(Player* player) {
	// 死んでいるなら何もしない
	if (isCollisionDisabled_) {
		return;
	}

	if (!player->IsAttack()) {
		return;
	}

	// 正面から攻撃された場合はガード
	if (lrDirection_ != player->GetLRDirection()) {
		if (gameScene_) {
			gameScene_->CreateGuardEffect(GetWorldPosition());
		}

		// ガードステートへ移行
		RequestBehaviorChange(std::make_unique<ShieldEnemyGuard>(this));

		// プレイヤーをノックバック
		player->RequestKnockback();

		return;
	}

	// ヒットフラッシュ
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