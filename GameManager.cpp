#include "GameManager.h"
#include "StageManager.h"
#include "GameInput.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "GlobalVariables.h"
#include <fstream>
#include <string>

void GameManager::Initialize() {
	// ステージデータファイルを読み込む
	StageManager::GetInstance()->LoadStageDataCsv();

	#ifdef _DEBUG

	LoadDebugSettings();

	#endif // _DEBUG

	// 起動時のグローバル変数の読み込み
	GlobalVariables::GetInstance()->LoadFiles();

	// タイトルシーンを生成
	ChangeScene(SCENE::TITLE);
}

void GameManager::Update() {
	// ゲームインプットの更新
	GameInput::GetInstance()->Update();

	// 現在のシーンを更新
	scene_->Update();

	// シーンが変更リクエストを出している場合は、シーンを切り替える
	if (sceneNo_ != scene_->GetNextSceneNo()) {
		ChangeScene(scene_->GetNextSceneNo());

		// ホットリロードのリクエストがある場合
	} else if (scene_->GetReloadRequest()) {
		// グローバル変数の再読み込み
		GlobalVariables::GetInstance()->LoadFiles();

		// 現在のシーンを再生成
		ChangeScene(sceneNo_);
	}

	// グローバル変数の更新
	GlobalVariables::GetInstance()->Update();
}

void GameManager::Draw() {
	scene_->Draw();
}

void GameManager::ChangeScene(SCENE newScene) {

	sceneNo_ = newScene;

	switch (sceneNo_) {
	case SCENE::TITLE:

		scene_ = std::make_unique<TitleScene>();

		break;


	case SCENE::GAME:

		scene_ = std::make_unique<GameScene>();

		break;
	}

	scene_->Initialize();
}

void GameManager::LoadDebugSettings() {
	std::ifstream file("DebugSettings.ini");

	// 個人用ファイルなので、存在しなくても問題なし
	if (!file.is_open()) {
		return;
	}

	std::string key;
	std::string value;

	while (file >> key >> value) {
		if (key == "InitialStage") {
			StageManager::GetInstance()->SetCurrentStageIndexByName(value);
		}
	}
}
