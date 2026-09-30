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

	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, KamataEngine::Vector3 position);
	virtual void Update() = 0;
	virtual void Draw();
	virtual void OnCollision(Player* player) = 0;

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

  private:
	void InitializeCommon(KamataEngine::Model* model, KamataEngine::Camera* camera, KamataEngine::Vector3 position);
};
