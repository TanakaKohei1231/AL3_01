#pragma once
#include<KamataEngine.h>
#include "BaseEnemy.h"
#include"Collision.h"

// 前方宣言
class MapChipField;
class Player;

class Enemy final : public BaseEnemy {
public:
	void OnCollision(Player *player) override;

private:
	// 初期化の共通処理
	void InitializeInternal() override;

	// モデルの向きに合わせたY軸の回転量(270度)
	static inline const float kDefaultAngle = 3.0f * (kPi / 2.0f);
};
