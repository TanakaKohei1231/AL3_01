#pragma once
#include "BaseEnemy.h"
#include<KamataEngine.h>
#include"Collision.h"
#include"LRDirection.h"

// 前方宣言
class GameScene;

class ShieldEnemy : public BaseEnemy {
  public:
	void OnCollision(Player* player) override;

	LRDirection GetLRDirection() const { return lrDirection_; }

  private:
	// 初期化の個別処理
	void InitializeInternal() override;

	// 向き
	LRDirection lrDirection_ = LRDirection::kLeft;

	// モデルの向きに合わせたY軸の回転量(270度)
	static inline const float kDefaultAngle = 3.0f * (kPi / 2.0f);
};
