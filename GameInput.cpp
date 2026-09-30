#include "GameInput.h"

using namespace KamataEngine;

bool GameInput::IsPress(GameAction action) {
	switch (action) {
	case GameAction::kMoveLeft:
		return Input::GetInstance()->PushKey(DIK_LEFT);

	case GameAction::kMoveRight:
		return Input::GetInstance()->PushKey(DIK_RIGHT);

	case GameAction::kJump:
		return Input::GetInstance()->PushKey(DIK_UP);

	case GameAction::kNormalAttack:
		return Input::GetInstance()->PushKey(DIK_SPACE);

	case GameAction::kDash:
		return Input::GetInstance()->PushKey(DIK_LSHIFT);

	default:
		return false;
	}
}

bool GameInput::IsTrigger(GameAction action) {
	switch (action) {
	case GameAction::kMoveLeft:
		return Input::GetInstance()->TriggerKey(DIK_LEFT);

	case GameAction::kMoveRight:
		return Input::GetInstance()->TriggerKey(DIK_RIGHT);

	case GameAction::kJump:
		return Input::GetInstance()->TriggerKey(DIK_UP);

	case GameAction::kNormalAttack:
		return Input::GetInstance()->TriggerKey(DIK_SPACE);

	case GameAction::kDash:
		return Input::GetInstance()->TriggerKey(DIK_LSHIFT);

	default:
		return false;
	}
}