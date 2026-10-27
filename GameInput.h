#pragma once
#include "KamataEngine.h"

// ゲーム内で使用する操作
enum class GameAction {
	// ゲーム
	kMoveLeft,     // 左移動
	kMoveRight, // 右移動
	kMoveUp, // 上移動
	kMoveDown, // 下移動
	kJump,         // ジャンプ
	kAttack,       // 攻撃
	kDash,         // ダッシュ

	// UI
	kUIUp, // 移動 上
	kUIDown, // 移動 下
	kUIRight, // 移動 右
	kUILeft,  // 移動 左
};

class GameInput final {
public:

	static GameInput* GetInstance();

	// キー情報の更新
	void Update();

	// 押している間
	bool IsPress(GameAction action);

	// 押した瞬間
	bool IsTrigger(GameAction action);

	// 離した瞬間
	bool IsRelease(GameAction action);



	// コピーコンストラクタと代入演算子を削除してシングルトンパターンにする
	GameInput(const GameInput&) = delete;
	GameInput& operator=(const GameInput&) = delete;

private:
	// キーの状態を保持する配列
	std::array<BYTE, 256> keys_{};
	std::array<BYTE, 256> preKeys_{};

	// シングルトンパターンにするためのコンストラクタとデストラクタをprivateにする
	GameInput() = default;
	~GameInput() = default;


	bool IsPressedRaw(std::array<BYTE, 256> keys, GameAction action);
};
