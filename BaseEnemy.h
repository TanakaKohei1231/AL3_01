#pragma once
#include<KamataEngine.h>
#include"Collision.h"
#include "BaseCharacter.h"

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

  protected:
	// ゲームシーン
	GameScene* gameScene_ = nullptr; 
};
