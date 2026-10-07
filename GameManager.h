#pragma once
#include <memory>
#include "IScene.h"

class GameManager {
public:
	void Initialize();
	void Update();
	void Draw();

private:
	std::unique_ptr<IScene> scene_;
	SCENE sceneNo_ = SCENE::kTitle;

	// シーンを切り替える関数
	void ChangeScene(SCENE newSceneNo);

	// デバッグ設定ファイルの読み込み
	void LoadDebugSettings();
};