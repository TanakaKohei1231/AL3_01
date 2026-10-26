#pragma once
#include<KamataEngine.h>
#include"Collision.h"
#include "BaseCharacter.h"
#include "LRDirection.h"

// 前方宣言
class Player;
class GameScene;

class BaseEnemy : public BaseCharacter {
  public:
	// 仮想デストラクタ
	virtual ~BaseEnemy() = default;

	// プレイヤーと当たった際の処理
	virtual void OnCollision(Player* player) = 0;

	// あたり判定無効かどうか
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }
	// 死亡しているか
	bool IsDead() const { return isDead_; }

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

	LRDirection GetLRDirection() const { return lrDirection_; }

	void SetWalkMotionY(float angle) { walkMotionRotationY_ = angle; }

  protected:
	// ゲームシーン
	GameScene* gameScene_ = nullptr;

	// 敵共通の更新後処理
	void AfterBehaviorUpdate() override;

	// 落下死亡時
	void OnFallDeath() override;

	// 地面に接触したとき
	void OnCollisionGround() override;

	// 天井に接触したとき
	void OnCollisionCeiling() override;

	// 重力加速度
	static inline const float kGravityAcceleration = 0.07f;

	// 最大落下速度
	static inline const float kLimitFallSpeed = 1.0f;

	// 敵の向き
	LRDirection lrDirection_ = LRDirection::kLeft;

	// 基本の向き
	float baseRotationY_ = 3.0f * kPi / 2.0f;

	// 歩行モーションのY軸回転量
	float walkMotionRotationY_ = 0.0f;

	// 旋回開始時の角度
	float turnStartRotationY_ = 0.0f;

	// 旋回終了時の角度
	float turnTargetRotationY_ = 0.0f;

	// 旋回タイマー
	float turnTimer_ = 0.0f;

	// 旋回中か
	bool isTurning_ = false;

	// 旋回時間
	static inline const float kTurnTime = 0.5f;

	// 壁との衝突
	void OnCollisionWall() override;

	// 旋回開始
	void StartTurn(float targetRotationY);

	// 旋回更新
	void UpdateTurn();

	// 歩行モーションの回転量を設定
	void SetWalkMotionRotationY(float angle) { walkMotionRotationY_ = angle; }
};
