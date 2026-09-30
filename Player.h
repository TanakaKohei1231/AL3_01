#pragma once
#include "KamataEngine.h"
#include "Collision.h"
#include "LRDirection.h"

// 前方宣言
class MapChipField;
class BaseEnemy;

enum Corner {
	kRightBottom,
	kLeftBottom,
	kRightTop,
	kLeftTop,
	kNumCorner
};

struct CollisionMapInfo {
	bool isHitCeiling = false;
	bool onGround = false;
	bool isHitWall = false;
	KamataEngine::Vector3 moveDistance = {};
};

class Player {
  public:

	// 初期化処理
	void Initialize(KamataEngine::Model* model, KamataEngine::Model* modelAttack, KamataEngine::Camera* camera, KamataEngine::Vector3 position);

	// 更新処理
	void Update();

	// 描画処理
	void Draw();

	// 敵と当たった時の処理
	void OnCollision(const BaseEnemy* enemy);

	// ノックバックを要求
	void RequestKnockback () {
		isKnockbackRequested_ = true;
	}

	// 調整項目を登録
	static void RegisterGlobalVariables();

	// 調整項目を適用
	static void ApplyGlobalVariables();


	//============================================================
	// ゲッター・セッター
	//============================================================

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// 画面端挟まれ死亡判定を有効にするか
	void SetScreenSqueezeDeathEnabled(bool enabled) { isScreenSqueezeDeathEnabled_ = enabled; }

	const KamataEngine::WorldTransform& GetWorldTransform();
	KamataEngine::Vector3 GetWorldPosition() const;
	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	// 敵とのあたり判定
	AABB GetAABB();

	// 死亡しているか
	bool IsDead() const { return isDead_; }

	// 攻撃中か
	bool IsAttack() const { return behavior_ == Behavior::kAttack; }

	LRDirection GetLRDirection() const { return lrDirection_; }



  private:

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// 向き
	LRDirection lrDirection_ = LRDirection::kRight;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;

	// 旋回タイマー
	float turnTimer_ = 0.0f;

	// 接地フラグ
	bool isGround_ = true;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	// 重力加速度
	static inline const float kGravityAcceleration = 0.07f;

	// 最大落下速度
	static inline const float kLimitFallSpeed = 1.0f;

	// 接地確認用の微小な下方向オフセット
	static inline const float kGroundCheckOffset = 0.1f;



	//============================================================
	// 振る舞い
	//============================================================

	enum class Behavior {
		kRoot,         // 通常状態
		kAttack,       // 攻撃中
		kKnockback,    // ノックバック
		kSqueezeDeath, // 挟まれ死亡演出
		kUnknown
	};

	Behavior behavior_ = Behavior::kRoot;
	Behavior behaviorRequest_ = Behavior::kUnknown;



	//============================================================
	// 死亡周り
	//============================================================

	// 死亡の種類
	enum class DeathType {
		kNone,
		kNormal,   // 通常死
		kSqueeze,  // 挟まれ
	};

	DeathType deathType_ = DeathType::kNone;

	// デスフラグ
	bool isDead_ = false;

	// 死亡演出が終わったか
	bool isDeathMotionFinished_ = false;

	// 描画するべきか
	bool ShouldDraw() const;

	//============================================================
	// 通常状態
	//============================================================

	// 通常行動の初期化
	void BehaviorRootInitialize();

	// 通常行動更新
	void BehaviorRootUpdate();

	// 歩行時の加速度
	static inline float kAcceleration = 0.02f;

	// 空中での左右加速度倍率
	static inline float kAirAccelerationRate = 0.7f;

	// 歩行の方向転換時の急ブレーキでのx速度減衰量
	static inline float kAttenuation = 0.1f;

	// 歩行の最高速
	static inline float kLimitRunSpeed = 0.3f;

	// 接地時に減衰するx速度
	static inline float kAttenuationLanding = 0.05f;

	// 壁接触時に減衰するx速度
	static inline float kAttenuationWall = 0.2f;

	// 旋回にかかる時間<秒>
	static inline float kTurnTime = 0.3f;

	// ジャンプの初速
	static inline float kJumpAcceleration = 0.8f;



	//============================================================
	// ノックバック
	//============================================================


	// ノックバックのフェーズ
	enum class KnockbackPhase {
		kKnockback, // 吹っ飛ばされる
		kAfter      // 後隙
	};

	// ノックバックの初期化
	void BehaviorKnockInitialize();

	// ノックバックの更新
	void BehaviorKnockUpdate();

	// ノックバックの吹っ飛ばされている時間かどうか
	bool IsKnockbackInvincible() const { return (behavior_ == Behavior::kKnockback && knockbackPhase_ == KnockbackPhase::kKnockback); }


	// ノックバック要求フラグ
	bool isKnockbackRequested_ = false;

	// ノックバックのフェーズ
	KnockbackPhase knockbackPhase_ = KnockbackPhase::kKnockback;

	// ノックバック経過時間
	float knockbackParameter_ = 0.0f;

	// ノックバックで吹っ飛ばされる時間
	static inline float kKnockbackTime = 0.15f;

	// ノックバック後の硬直時間
	static inline float kKnockbackAfterTime = 0.1f;

	// ノックバックの横速度
	static inline float kKnockbackSpeed = 0.3f;

	// ノックバック後の減速率
	static inline float kKnockbackAttenuation = 0.15f;



	//============================================================
	// 攻撃行動
	//============================================================

	// 攻撃のフェーズ
	enum class AttackPhase {
		kBefore,    // 前隙
		kAttack,    // 攻撃
		kAfter      // 後隙
	};

	// 攻撃のフェーズ
	AttackPhase attackPhase_ = AttackPhase::kBefore;


	// 攻撃行動の初期化
	void BehaviorAttackInitialize();

	// 攻撃行動更新
	void BehaviorAttackUpdate();

	// 攻撃の時間
	float attackParameter_ = 0.0f;

	// 攻撃エフェクト用モデル
	KamataEngine::Model* modelAttack_ = nullptr;

	// 攻撃エフェクト用ワールド変換データ
	KamataEngine::WorldTransform worldTransformAttack_;

	// 前隙
	static inline float kAttackBeforeTime = 0.2f;

	// 攻撃の時間
	static inline float kAttackTime = 0.2f;

	// 後隙
	static inline float kAttackAfterTime = 0.2f;

	// 攻撃時の突進速度
	static inline float kAttackMoveSpeed = 0.5f;



	//============================================================
	// マップチップ回り
	//============================================================


	// マップチップとの衝突を解決する
	void ResolveMapChipCollision(const KamataEngine::Vector3& velocity);

	void MapChipCollision(CollisionMapInfo& info);
	void MapChipCollisionTop(CollisionMapInfo& info);
	void MapChipCollisionBottom(CollisionMapInfo& info);
	void MapChipCollisionLeft(CollisionMapInfo& info);
	void MapChipCollisionRight(CollisionMapInfo& info);

	// 衝突結果を反映して移動させる
	void ApplyCollisionResult(const CollisionMapInfo& info);

	// 天井に接触している時の処理
	void HandleCeilingCollision(const CollisionMapInfo& info);

	// 地面に接触している時の処理
	void HandleGroundCollision(const CollisionMapInfo& info);

	// 壁に接触してるときの処理
	void HandleWallCollision(const CollisionMapInfo& info);

	// 地面に触れてるときの処理
	void ChangeGroundState(const CollisionMapInfo& info);

	// キャラクターの当たり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// めり込み防止用の微小な余白
	static inline const float kBlank = 0.05f;



	//============================================================
	// 強制スクロール
	//============================================================

	// 画面端からはみ出さないように補正する
	void CorrectPositionInScreen();

	// 画面端と壁に挟まれたか確認する
	void CheckScreenSqueezeDeath();

	// 画面左端に接触しているか
	bool IsTouchingLeftScreenEdge();

	// 右側にブロックがあるか
	bool IsBlockOnRightSide();

	// 挟まれ死亡を開始する
	void StartSqueezeDeath();

	// 挟まれ死亡演出の初期化
	void BehaviorSqueezeDeathInitialize();

	// 挟まれ死亡演出の更新
	void BehaviorSqueezeDeathUpdate();



	// 画面端挟まれ死亡判定を有効にするか
	bool isScreenSqueezeDeathEnabled_ = false;

	// 挟まれ死亡演出タイマー
	float squeezeDeathTimer_ = 0.0f;



	// 画面端接触判定の許容誤差
	static inline const float kScreenEdgeCheckTolerance = 0.05f;

	// 壁確認用の微小なオフセット
	static inline const float kScreenSqueezeCheckOffset = 0.05f;

	// 挟まれ死亡時の上方向初速
	static inline const float kSqueezeDeathJumpVelocity = 0.5f;

	// 挟まれ死亡時の重力
	static inline const float kSqueezeDeathGravity = 0.015f;

	// 挟まれ死亡時のX回転速度
	static inline const float kSqueezeDeathRotationSpeed = kPi * 2.0f;

	// 挟まれ死亡演出時間
	static inline const float kSqueezeDeathMotionTime = 1.5f;

	// 画面端補正用の範囲
	static inline const float kScreenLeftLimit = -11.0f;
	static inline const float kScreenRightLimit = 11.0f;
	static inline const float kScreenBottomLimit = -8.0f;
	static inline const float kScreenTopLimit = 10.0f;
};