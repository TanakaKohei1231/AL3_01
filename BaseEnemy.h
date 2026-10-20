#pragma once
#include<KamataEngine.h>
#include"Collision.h"

// 前方宣言
class Player;
class GameScene;

class BaseEnemy {
  public:
	// 仮想デストラクタ
	virtual ~BaseEnemy() = default;

	void Initialize(KamataEngine::Camera* camera, KamataEngine::Vector3 position);
	virtual void Update() = 0;
	virtual void Draw();
	virtual void OnCollision(Player* player) = 0;

	// ヒットフラッシュ開始
	void StartHitFlash();

	// ヒットフラッシュ更新
	void UpdateHitFlash();

	virtual AABB GetAABB() = 0;
	KamataEngine::Vector3 GetWorldPosition() const;
	// あたり判定無効かどうか
	bool IsCollisionDisabled() const { return isCollisionDisabled_; }
	// 死亡しているか
	bool IsDead() const { return isDead_; }

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

  protected:
	// 初期化の共通処理
	virtual void InitializeInternal() = 0;

	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// ゲームシーン
	GameScene* gameScene_ = nullptr;

	// あたり判定無効(死亡演出中)
	bool isCollisionDisabled_ = false;

	// デスフラグ
	bool isDead_ = false;

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
	void InitializeCommon(KamataEngine::Camera* camera, KamataEngine::Vector3 position);
};
