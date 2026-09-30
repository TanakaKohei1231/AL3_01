#include <Windows.h>
#include"KamataEngine.h"
#include"GameScene.h"
#include "TitleScene.h"
#include "StageManager.h"
#include "GlobalVariables.h"
#include <fstream>
#include <string>

using namespace KamataEngine;

// シーン
enum class Scene {
	kUnknown = 0,
	kTitle,
	kGame,
};

// 現在シーン
Scene scene = Scene::kUnknown;

// ゲームシーン
GameScene* gameScene = nullptr;

// タイトルシーン
TitleScene* titleScene = nullptr;

// ステージマネージャー
StageManager* stageManager = nullptr;

// シーン切り替え
void ChangeScene();

// シーン更新
void UpdateScene();

// シーン描画
void DrawScene();

// デバッグ設定ファイルの読み込み
void LoadDebugSettings();

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// エンジンの初期化
	::Initialize(L"LE2C_14_タナカ_コウヘイ");
	
	// DirectXCommonインスタンスの取得
	DirectXCommon* dXCommon = DirectXCommon::GetInstance();

	// ImGuiManagerインスタンスの取得
	ImGuiManager* imGuiManager = ImGuiManager::GetInstance();

	// ステージマネージャの生成
	stageManager = new StageManager();

	// ステージデータファイルを読み込む
	stageManager->LoadStageDataCsv();

	#ifdef _DEBUG

	LoadDebugSettings();

	#endif // _DEBUG


	// 起動時のグローバル変数の読み込み
	GlobalVariables::GetInstance()->LoadFiles();

	// 最初のシーン
	scene = Scene::kTitle;
	titleScene = new TitleScene();
	titleScene->Initialize();

	// メインループ
	while (true) {
		// エンジンの更新
		if (Update()) {
			break;
		}

		// ImGui受付開始
		imGuiManager->Begin();

		// シーン切り替え
		ChangeScene();

		// 現在シーンの更新
		UpdateScene();

		GlobalVariables::GetInstance()->Update();

		// ImGui受付終了
		imGuiManager->End();


		// 描画開始
		dXCommon->PreDraw();

		// ここに描画処理を記述

		// 現在シーンの描画
		DrawScene();

		// ImGui描画
		imGuiManager->Draw();

		// 描画終了
		dXCommon->PostDraw();
	}

	// シーンの解放
	delete titleScene;
	titleScene = nullptr;

	delete gameScene;
	gameScene = nullptr;

	// ステージマネージャーの開放
	delete stageManager;
	stageManager = nullptr;


	// エンジンの終了処理
	::Finalize();

	return 0;
}

void ChangeScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene && titleScene->IsFinished()) {
			// シーン変更
			scene = Scene::kGame;

			// 旧シーンの解放
			delete titleScene;
			titleScene = nullptr;

			// 新シーンの生成
			gameScene = new GameScene();
			gameScene->Initialize(stageManager);
		}
		break;

	case Scene::kGame:

		// シーン終了処理
		if (gameScene && gameScene->IsFinished()) {
			// シーン変更
			scene = Scene::kTitle;

			// 旧シーンの解放
			delete gameScene;
			gameScene = nullptr;

			// 新シーンの生成と初期化
			titleScene = new TitleScene();
			titleScene->Initialize();


			// ホットリロード
		} else if (gameScene && gameScene->GetReloadRequest()) {
			// シーンの解放
			delete gameScene;
			gameScene = nullptr;

			// グローバル変数の再読み込み
			GlobalVariables::GetInstance()->LoadFiles();

			// 新シーンの生成
			gameScene = new GameScene();
			gameScene->Initialize(stageManager);
		}

		break;

	case Scene::kUnknown:
	default:
		break;
	}
}

void UpdateScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene) {
			titleScene->Update();
		}
		break;

	case Scene::kGame:
		if (gameScene) {
			gameScene->Update();
		}
		break;

	case Scene::kUnknown:
	default:
		break;
	}
}

void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		if (titleScene) {
			titleScene->Draw();
		}
		break;

	case Scene::kGame:
		if (gameScene) {
			gameScene->Draw();
		}
		break;

	case Scene::kUnknown:
	default:
		break;
	}
}

void LoadDebugSettings() {
	std::ifstream file("DebugSettings.ini");

	// 個人用ファイルなので、存在しなくても問題なし
	if (!file.is_open()) {
		return;
	}

	std::string key;
	std::string value;

	while (file >> key >> value) {
		if (key == "InitialStage") {
			stageManager->SetCurrentStageIndexByName(value);
		}
	}
}