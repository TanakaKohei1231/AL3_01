#pragma once
#include "KamataEngine.h"
#include "Collision.h"
#include "LRDirection.h"
#include "BaseCharacter.h"
#include "PlayerBehavior.h"

// 前方宣言
class MapChipField;
class BaseEnemy;

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

	// はしごの上にいるか
	bool IsOnLadder() const;

	// 指定したY軸角度へ滑らかに旋回する
	void TurnToAngle(float targetRotationY);

	// はしごを掴むときの旋回
	void TurnToLadder();

	// はしごから離れたときの旋回
	void TurnFromLadder();

	// クリア演出を開始
	void StartClearDirection();

	// クリア演出が終了したか
	bool IsClearDirectionFinished() const { return isClearDirectionFinished_; }

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

	// 重力加速度
	static inline const float kGravityAcceleration = 0.07f;

	// 最大落下速度
	static inline const float kLimitFallSpeed = 1.0f;

	// 接地確認用の微小な下方向オフセット
	static inline const float kGroundCheckOffset = 0.1f;

	// はしごを昇降しているか
	bool isLadderClimbing_ = false;

	// はしご最上段へ着地できるか
	bool CanLandOnLadder() const override { return !isLadderClimbing_; }

	// はしごを登り切った場合、上面へ移動する
	bool TryFinishLadderClimb();

	// 旋回先のY軸角度
	float turnTargetRotationY_ = 0.0f;

	// はしご中の背面角度
	static inline const float kLadderRotationY = 0.0f;

	// クリア演出終了フラグ
	bool isClearDirectionFinished_ = false;

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

	// 落下死亡時
	void OnFallDeath() override;


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

	// 地面に接触している時の処理
	void HandleGroundCollision(const CollisionMapInfo& info);

	// 地面に触れてるときの処理
	void ChangeGroundState(const CollisionMapInfo& info);

	// 天井への衝突時の処理
	void OnCollisionCeiling() override;

	// 壁への衝突時の処理
	void OnCollisionWall() override;

	// マップ衝突後の個別処理
	void AfterMapChipCollision(const CollisionMapInfo& info) override;


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
	friend class PlayerLadderBehavior;
	friend class PlayerClearBehavior;
};