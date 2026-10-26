#pragma once
#include <KamataEngine.h>
#include "ICharactersBehavior.h"
#include"Collision.h"

enum Corner { kRightBottom, kLeftBottom, kRightTop, kLeftTop, kNumCorner };

struct CollisionMapInfo {
	bool isHitCeiling = false;
	bool onGround = false;
	bool isHitWall = false;
	KamataEngine::Vector3 moveDistance = {};
};

// 前方宣言
class GameScene;
class MapChipField;

class BaseCharacter {
  public:
	// 仮想デストラクタ
	virtual ~BaseCharacter() = default;

	void Initialize(KamataEngine::Camera* camera, KamataEngine::Vector3 position, MapChipField* mapChipField);
	void Update();
	virtual void Draw();

	// ヒットフラッシュ開始
	void StartHitFlash();

	// ヒットフラッシュ更新
	void UpdateHitFlash();

	// デスラインより下に落ちたか判定
	void CheckFallDeath(float deathLineY);

	AABB GetAABB() const;

	KamataEngine::Vector3 GetWorldPosition() const;

	// 振る舞い変更を要求
	void RequestBehaviorChange(std::unique_ptr<ICharactersBehavior> nextBehavior);

	// 移動
	void Move(const KamataEngine::Vector3& moveDistance) { worldTransform_.translation_ += moveDistance; }

	// X軸回転
	void SetRotationX(float angle) { worldTransform_.rotation_.x = angle; }

	float GetRotationX() const { return worldTransform_.rotation_.x; }

	// Y軸回転
	void SetRotationY(float angle) { worldTransform_.rotation_.y = angle; }

	float GetRotationY() const { return worldTransform_.rotation_.y; }

	// Z軸回転
	void SetRotationZ(float angle) { worldTransform_.rotation_.z = angle; }

	float GetRotationZ() const { return worldTransform_.rotation_.z; }

	// 移動速度
	void SetVelocity(const KamataEngine::Vector3& velocity) { velocity_ = velocity; }

	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	// 死亡フラグ
	void SetDead(bool isDead) { isDead_ = isDead; }

	// マップチップフィールドを設定
	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// マップチップとの衝突を解決して移動する
	void ResolveMapChipCollision(const KamataEngine::Vector3& moveDistance);

	// 攻撃状態か
	bool IsAttack() const {
		if (!behavior_) {
			return false;
		}

		return behavior_->IsAttack();
	}

	// 挟まれ死亡状態か
	bool IsSqueezeDeath() const {
		if (!behavior_) {
			return false;
		}

		return behavior_->IsSqueezeDeath();
	}

  protected:
	// 初期化の個別処理
	virtual void InitializeInternal() = 0;

	// 振る舞い
	std::unique_ptr<ICharactersBehavior> behavior_;

	// 次の振る舞い
	std::unique_ptr<ICharactersBehavior> behaviorRequest_;

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	// キャラクターの当たり判定サイズ
	float width_ = 0.8f;
	float height_ = 0.8f;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// あたり判定無効(死亡演出中)
	bool isCollisionDisabled_ = false;

	// デスフラグ
	bool isDead_ = false;

	// 振る舞い更新前の個別処理
	virtual void BeforeBehaviorUpdate() {}

	// 振る舞い更新後の個別処理
	virtual void AfterBehaviorUpdate() {}

	// ワールド変換更新後の個別処理
	virtual void AfterTransformUpdate() {}

	// 落下死亡時の個別処理
	virtual void OnFallDeath() {}

	// 天井に接触したとき
	virtual void OnCollisionCeiling() {}

	// 地面に接触したとき
	virtual void OnCollisionGround() {}

	// 壁に接触したとき
	virtual void OnCollisionWall() {}

	// マップチップとの衝突判定
	void MapChipCollision(CollisionMapInfo& info);
	void MapChipCollisionTop(CollisionMapInfo& info);
	void MapChipCollisionBottom(CollisionMapInfo& info);
	void MapChipCollisionLeft(CollisionMapInfo& info);
	void MapChipCollisionRight(CollisionMapInfo& info);

	// 衝突結果を反映して移動
	void ApplyCollisionResult(const CollisionMapInfo& info);

	// キャラクターの四隅の座標
	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	// 衝突解決後に呼び出す個別処理
	virtual void AfterMapChipCollision(const CollisionMapInfo& info) { (void)info; }

	// めり込み防止用の微小な余白
	static inline const float kBlank = 0.05f;

	//============================================================
	// ヒットフラッシュ
	//============================================================

	// 色変更オブジェクト
	KamataEngine::ObjectColor objectColor_;

	// ヒットフラッシュの残りフレーム数
	int32_t hitFlashTimer_ = 0;

	// ヒットフラッシュの持続フレーム数
	static inline const int32_t kHitFlashFrames = 12;

	// 通常の描画色
	static inline const KamataEngine::Vector4 kNormalColor = {1.0f, 1.0f, 1.0f, 1.0f};

	// ヒット時の描画色
	static inline const KamataEngine::Vector4 kHitFlashColor = {1.0f, 0.2f, 0.2f, 1.0f};

  private:
	// 振る舞い変更リクエストを反映
	void ChangeBehavior();
};
