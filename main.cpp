#include <Windows.h>
#include"KamataEngine.h"
#include "GameManager.h"
#include "ModelManager.h"
#include "SoundManager.h"

using namespace KamataEngine;


// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// エンジンの初期化
	Initialize(L"LE2C_18_タナカ_コウヘイ_マジョ子の冒険");
	
	// DirectXCommonインスタンスの取得
	DirectXCommon* dXCommon = DirectXCommon::GetInstance();

	// ImGuiManagerインスタンスの取得
	ImGuiManager* imGuiManager = ImGuiManager::GetInstance();

	// サウンドの読み込み
	SoundManager::GetInstance()->Initialize();

	// ゲームマネージャーの生成
	std::unique_ptr<GameManager> gameManager = std::make_unique<GameManager>();
	gameManager->Initialize();


	// メインループ
	while (true) {
		// エンジンの更新
		if (Update()) {
			break;
		}

		// ImGui受付開始
		imGuiManager->Begin();

		// ゲームマネージャーの更新
		gameManager->Update();

		// ImGui受付終了
		imGuiManager->End();

		// 描画開始
		dXCommon->PreDraw();

		// ここに描画処理を記述

		// ゲームマネージャーの描画
		gameManager->Draw();

		// ImGui描画
		imGuiManager->Draw();

		// 描画終了
		dXCommon->PostDraw();
	}

	// GameManager以下をすべて破棄
	gameManager.reset();

	// モデルマネージャーの終了処理
	ModelManager::GetInstance()->Finalize();

	// エンジンの終了処理
	Finalize();

	return 0;
}