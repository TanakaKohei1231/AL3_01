#include "GameInput.h"

using namespace KamataEngine;

GameInput* GameInput::GetInstance() {
	static GameInput instance;

	return &instance;
}

void GameInput::Update() {
	preKeys_ = keys_;
	keys_ = Input::GetInstance()->GetAllKey();
}

bool GameInput::IsPress(GameAction action) { return IsPressedRaw(keys_, action); }

bool GameInput::IsTrigger(GameAction action) {
	bool nowKey = IsPressedRaw(keys_, action);
	bool preKey = IsPressedRaw(preKeys_, action);

	return nowKey && !preKey;
}

bool GameInput::IsRelease(GameAction action) {
	bool nowKey = IsPressedRaw(keys_, action);
	bool preKey = IsPressedRaw(preKeys_, action);

	return !nowKey && preKey;
}

bool GameInput::IsPressedRaw(std::array<BYTE, 256> keys, GameAction action) {
	switch (action) {
	// ゲーム
	case GameAction::kMoveLeft:
		return keys[DIK_A];

	case GameAction::kMoveRight:
		return keys[DIK_D];

	case GameAction::kMoveUp:
		return keys[DIK_W];

	case GameAction::kMoveDown:
		return keys[DIK_S];

	case GameAction::kJump:
		return keys[DIK_SPACE];

	case GameAction::kAttack:
		return keys[DIK_J];

	case GameAction::kDash:
		return keys[DIK_LSHIFT];

	// UI
	case GameAction::kUIUp:
		return keys[DIK_UP];

	case GameAction::kUIDown:
		return keys[DIK_DOWN];

	case GameAction::kUIRight:
		return keys[DIK_RIGHT];

	case GameAction::kUILeft:
		return keys[DIK_LEFT];

	default:
		return false;
	}
}
