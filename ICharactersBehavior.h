#pragma once
class ICharactersBehavior {
  public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void UpdateMotion() = 0;

	// 攻撃状態か
	virtual bool IsAttack() const { return false; }

	// 挟まれ死亡状態か
	virtual bool IsSqueezeDeath() const { return false; }

	virtual ~ICharactersBehavior() = default;
};
