#pragma once

enum class SCENE {
	kTitle,
	kGame,
	KClear,
	kCountOfScene,
	kUnknown // nextSceneがkUnknownの間はシーン変更リクエストが来ていない扱い
};

class IScene {
  public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual ~IScene() = default;

	SCENE GetNextSceneNo() const { return nextScene_; }

  protected:
	SCENE nextScene_ = SCENE::kUnknown;
};
