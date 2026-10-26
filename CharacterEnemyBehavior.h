
#pragma once
#include "ICharactersBehavior.h"

// 前方宣言
class BaseEnemy;
class ShieldEnemy;

//============================================================
// 通常敵の歩行
//============================================================
class NormalEnemyWalk : public ICharactersBehavior {
public:
	explicit NormalEnemyWalk(BaseEnemy* enemy);

	void Initialize() override;
	void Update() override;
	void UpdateMotion() override;

private:
	BaseEnemy* enemy_ = nullptr;

	float walkTimer_ = 0.0f;
};

//============================================================
// 盾敵の歩行
//============================================================
class ShieldEnemyWalk : public ICharactersBehavior {
public:
	explicit ShieldEnemyWalk(BaseEnemy* enemy);

	void Initialize() override;
	void Update() override;
	void UpdateMotion() override;

private:
	BaseEnemy* enemy_ = nullptr;

	float walkTimer_ = 0.0f;
};

//============================================================
// 敵共通の死亡演出
//============================================================
class EnemyDeathBehavior : public ICharactersBehavior {
public:
	explicit EnemyDeathBehavior(BaseEnemy* enemy);

	void Initialize() override;
	void Update() override;
	void UpdateMotion() override;

private:
	BaseEnemy* enemy_ = nullptr;

	float deathTimer_ = 0.0f;
	float deathStartRotationX_ = 0.0f;
};

//============================================================
// 盾敵のガード
//============================================================
class ShieldEnemyGuard : public ICharactersBehavior {
public:
	explicit ShieldEnemyGuard(ShieldEnemy* enemy);

	void Initialize() override;
	void Update() override;
	void UpdateMotion() override;

private:
	ShieldEnemy* enemy_ = nullptr;

	float guardTimer_ = 0.0f;
	float guardStartRotationZ_ = 0.0f;
};
