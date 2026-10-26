#pragma once
#include "KamataEngine.h"
#include "Collision.h"
#include "LRDirection.h"
#include "BaseCharacter.h"
#include "PlayerBehavior.h"

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

class Player : public BaseCharacter {
  public:
	// 描画処理
	void Draw() override;

	// 敵と当たった時の処理
	void OnCollision(const BaseEnemy* enemy);

	// ノックバックを要求
	void RequestKnockback () {
		isKnockbackRequested_ = true;
	}


	//============================================================
	// ゲッター・セッター
	//============================================================

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// 画面端挟まれ死亡判定を有効にするか
	void SetScreenSqueezeDeathEnabled(bool enabled) { isScreenSqueezeDeathEnabled_ = enabled; }

	const KamataEngine::WorldTransform& GetWorldTransform();

	// 死亡しているか
	bool IsDead() const { return isDead_; }

	LRDirection GetLRDirection() const { return lrDirection_; }

	// 接地しているか
	bool IsGround() const { return isGround_; }

	// プレイヤーの向きを変更
	void TurnTo(LRDirection direction);

	// ノックバックの無敵状態を設定
	void SetKnockbackInvincible(bool isInvincible) { isKnockbackInvincible_ = isInvincible; }

  private:

	// プレイヤー固有の初期化
	void InitializeInternal() override;

	// 振る舞い更新前の処理
	void BeforeBehaviorUpdate() override;

	// 振る舞い更新後の処理
	void AfterBehaviorUpdate() override;

	// ワールド変換更新後の処理
	void AfterTransformUpdate() override;

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
	// 死亡周り
	//============================================================

	// 死亡の種類
	enum class DeathType {
		kNone,
		kNormal,   // 通常死
		kSqueeze,  // 挟まれ
	};

	DeathType deathType_ = DeathType::kNone;

	// 死亡演出が終わったか
	bool isDeathMotionFinished_ = false;

	// 描画するべきか
	bool ShouldDraw() const;


	// 接地時に減衰するx速度
	static inline float kAttenuationLanding = 0.05f;

	// 壁接触時に減衰するx速度
	static inline float kAttenuationWall = 0.2f;

	// 旋回にかかる時間<秒>
	static inline float kTurnTime = 0.3f;


	// ノックバックの吹っ飛ばされている時間かどうか
	bool IsKnockbackInvincible() const { return isKnockbackInvincible_; }

	// ノックバック要求フラグ
	bool isKnockbackRequested_ = false;

	// ノックバックによる無敵状態
	bool isKnockbackInvincible_ = false;

	// 攻撃エフェクトを表示するか
	bool isAttackEffectVisible_ = false;

	// 攻撃エフェクト用モデル
	KamataEngine::Model* modelAttack_ = nullptr;

	// 攻撃エフェクト用ワールド変換データ
	KamataEngine::WorldTransform worldTransformAttack_;



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

	// 画面端挟まれ死亡判定を有効にするか
	bool isScreenSqueezeDeathEnabled_ = false;

	// 画面端接触判定の許容誤差
	static inline const float kScreenEdgeCheckTolerance = 0.05f;

	// 壁確認用の微小なオフセット
	static inline const float kScreenSqueezeCheckOffset = 0.05f;


	// 画面端補正用の範囲
	static inline const float kScreenLeftLimit = -11.0f;
	static inline const float kScreenRightLimit = 11.0f;
	static inline const float kScreenBottomLimit = -8.0f;
	static inline const float kScreenTopLimit = 10.0f;


	// ステート移行中の既存処理へのアクセス
	friend class PlayerRootBehavior;
	friend class PlayerAttackBehavior;
	friend class PlayerKnockbackBehavior;
	friend class PlayerSqueezeDeathBehavior;
};