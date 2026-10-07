#pragma once

enum class SCENE {
	TITLE,
	GAME,
	CLEAR,
	kCountOfScene
};

class IScene {
  public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;

	virtual ~IScene() = default;

	SCENE GetNextSceneNo() const { return nextScene_; }

	virtual bool GetReloadRequest() const { return false; }

  protected:
	SCENE nextScene_;
};
