#pragma once
#include "BaseEnemy.h"
#include<KamataEngine.h>
#include"Collision.h"
#include"LRDirection.h"

// 前方宣言
class GameScene;

class ShieldEnemy : public BaseEnemy {
  public:
	// 更新処理
	void Update() override;

	void OnCollision(Player* player) override;

	AABB GetAABB() override;


  private:
	// 初期化の共通処理
	void InitializeInternal() override;

	// 振る舞い
	enum class Behavior {
		kWalk,           // 歩行
		kDeathDirection, // 死亡演出
		kGuard,          // ガード
		kUnknown
	};

	Behavior behavior_ = Behavior::kWalk;
	Behavior behaviorRequest_ = Behavior::kUnknown;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// 向き
	LRDirection lrDirection_ = LRDirection::kLeft;


	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// モデルの向きに合わせたY軸の回転量(270度)
	static inline const float kDefaultAngle = 3.0f * (kPi / 2.0f);


	//============================================================
	// 歩行時
	//============================================================

	float walkTimer_ = 0.0f;

	// 歩行の初期化
	void BehaviorWalkInitialize();
	// 歩行更新
	void BehaviorWalkUpdate();

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.01f;

	// 歩行モーションの最初の角度
	static inline const float kWalkMotionAngleStart = kPi / 5.0f;

	// 歩行モーション終わりの角度
	static inline const float kWalkMotionAngleEnd = -kWalkMotionAngleStart;

	// 歩行アニメーションの周期[秒]
	static inline const float kWalkMotionTime = 2.0f;


	//============================================================
	// 死亡時
	//============================================================

	// 死亡演出タイマー
	float deathTimer_ = 0.0f;

	// 死亡演出開始時のX回転
	float deathStartRotationX_ = 0.0f;

	// 死亡演出の初期化
	void BehaviorDeathDirectionInitialize();
	// 死亡演出更新
	void BehaviorDeathDirectionUpdate();

	// 死亡演出時間
	static inline const float kDeathMotionTime = 1.0f;

	// 死亡演出で倒れ切る角度
	static inline const float kDeathMotionAngleEnd = -kPi / 2.0f;

	// 死亡演出中の回転速度
	static inline const float kDeathSpinSpeed = kPi * 6.0f;


	//============================================================
	// ガード時
	//============================================================

	// ガード演出タイマー
	float guardTimer_ = 0.0f;

	// ガード演出開始時のZ回転
	float guardStartRotationZ_ = 0.0f;

	// ガードの初期化
	void BehaviorGuardInitialize();
	// ガード更新
	void BehaviorGuardUpdate();

	// ガード演出時間
	static inline const float kGuardMotionTime = 0.5f;

	// ガード時ののけぞり角度
	static inline const float kGuardMotionAngle = kPi / 6.0f;
};
