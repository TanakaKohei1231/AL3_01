
#pragma once
#include "ICharactersBehavior.h"
#include "Vector3Util.h"

// 前方宣言
class Player;

//============================================================
// 通常状態
//============================================================
class PlayerRootBehavior : public ICharactersBehavior {
  public:
	explicit PlayerRootBehavior(Player* player);

	void Initialize() override;
	void Update() override;
	void UpdateMotion() override;

  private:
	Player* player_ = nullptr;

	// 歩行時の加速度
	static inline float kAcceleration = 0.02f;

	// 空中での左右加速度倍率
	static inline float kAirAccelerationRate = 0.7f;

	// 歩行の方向転換時の急ブレーキでのx速度減衰量
	static inline float kAttenuation = 0.1f;

	// 歩行の最高速
	static inline float kLimitRunSpeed = 0.3f;

	// ジャンプの初速
	static inline float kJumpAcceleration = 0.8f;
};

//============================================================
// 攻撃状態
//============================================================
class PlayerAttackBehavior : public ICharactersBehavior {
  public:
	explicit PlayerAttackBehavior(Player* player);

	void Initialize() override;
	void Update() override;
	void UpdateMotion() override;

	// 攻撃状態か
	bool IsAttack() const override { return true; }

  private:
	Player* player_ = nullptr;

	// 攻撃の進行段階
	enum class AttackPhase {
		kBefore,
		kAttack,
		kAfter
	};

	// 現在の攻撃段階
	AttackPhase attackPhase_ = AttackPhase::kBefore;

	// 攻撃の経過時間
	float attackParameter_ = 0.0f;

	// 前隙
	static inline float kAttackBeforeTime = 0.2f;

	// 攻撃の時間
	static inline float kAttackTime = 0.2f;

	// 後隙
	static inline float kAttackAfterTime = 0.2f;

	// 攻撃時の突進速度
	static inline float kAttackMoveSpeed = 0.5f;
};

//============================================================
// ノックバック状態
//============================================================
class PlayerKnockbackBehavior : public ICharactersBehavior {
  public:
	explicit PlayerKnockbackBehavior(Player* player);

	void Initialize() override;
	void Update() override;
	void UpdateMotion() override;

  private:
	// ノックバックの進行段階
	enum class KnockbackPhase {
		kKnockback, // 吹っ飛ばされる
		kAfter      // 後隙
	};

	Player* player_ = nullptr;

	// 現在の進行段階
	KnockbackPhase knockbackPhase_ = KnockbackPhase::kKnockback;

	// 経過時間
	float knockbackParameter_ = 0.0f;

	// ノックバックで吹っ飛ばされる時間
	static inline float kKnockbackTime = 0.15f;

	// ノックバック後の硬直時間
	static inline float kKnockbackAfterTime = 0.1f;

	// ノックバックの横速度
	static inline float kKnockbackSpeed = 0.3f;

	// ノックバック後の減速率
	static inline float kKnockbackAttenuation = 0.15f;
};

//============================================================
// 挟まれ死亡演出
//============================================================
class PlayerSqueezeDeathBehavior : public ICharactersBehavior {
  public:
	explicit PlayerSqueezeDeathBehavior(Player* player);

	void Initialize() override;
	void Update() override;
	void UpdateMotion() override;

	// 挟まれ死亡状態か
	bool IsSqueezeDeath() const override { return true; }

  private:
	Player* player_ = nullptr;

	// 挟まれ死亡演出の経過時間
	float squeezeDeathTimer_ = 0.0f;

	// 挟まれ死亡時の上方向初速
	static inline const float kSqueezeDeathJumpVelocity = 0.5f;

	// 挟まれ死亡時の重力
	static inline const float kSqueezeDeathGravity = 0.015f;

	// 挟まれ死亡時のX回転速度
	static inline const float kSqueezeDeathRotationSpeed = kPi * 2.0f;

	// 挟まれ死亡演出時間
	static inline const float kSqueezeDeathMotionTime = 1.5f;
};

class PlayerLadderBehavior : public ICharactersBehavior {
  public:
	explicit PlayerLadderBehavior(Player* player);

	void Initialize() override;
	void Update() override;
	void UpdateMotion() override;

  private:
	Player* player_ = nullptr;

	// はしごの昇降速度
	static inline const float kClimbSpeed = 0.15f;

	// はしごの左右移動速度
	static inline const float kLadderHorizontalSpeed = 0.03f;

	// はしごから離れる際のジャンプ初速
	static inline const float kJumpAcceleration = 0.8f;
};

//============================================================
// クリア演出状態
//============================================================
class PlayerClearBehavior : public ICharactersBehavior {
  public:
	explicit PlayerClearBehavior(Player* player);

	void Initialize() override;
	void Update() override;
	void UpdateMotion() override;

  private:
	// クリア演出の進行段階
	enum class ClearPhase { kJump, kTurn, kFinished };

	Player* player_ = nullptr;

	ClearPhase clearPhase_ = ClearPhase::kJump;

	// 一度空中へ出たか
	bool hasClearJumpStarted_ = false;

	// 旋回開始時の角度
	float clearTurnStartRotationY_ = 0.0f;

	// 最短経路で補正した旋回先の角度
	float clearTurnTargetRotationY_ = 0.0f;

	// 旋回の経過時間
	float clearTurnParameter_ = 0.0f;

	// クリアジャンプ初速
	static inline const float kClearJumpVelocity = 0.7f;

	// 正面を向くまでの時間
	static inline const float kClearTurnTime = 0.5f;

	// カメラ側を向く角度
	static inline const float kClearFrontRotationY = kPi;
};