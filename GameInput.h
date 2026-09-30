#pragma once

#include "KamataEngine.h"

// ゲーム内で使用する操作
enum class GameAction {
	kMoveLeft,     // 左移動
	kMoveRight,    // 右移動
	kJump,         // ジャンプ
	kNormalAttack, // 通常攻撃
	kDash,         // ダッシュ
};

class GameInput {
public:
	// 押している間
	static bool IsPress(GameAction action);

	// 押した瞬間
	static bool IsTrigger(GameAction action);
};
