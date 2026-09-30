#pragma once
#include"KamataEngine.h"

class Fade {
  public:

	// フェードの状態
	enum class Status {
		kNone,    // フェードなし
		kFadeIn,  // フェードイン中
		kFadeOut, // フェードアウト中
	};

	void Initialize();
	void Update();
	void Draw();

	// フェード開始
	void Start(Status status, float duration);

	// フェード停止
	void Stop() { status_ = Status::kNone; }

	// フェードが終わっているか
	bool IsFinished() const;

	// 終了処理
	~Fade();

  private:
	KamataEngine::Sprite* sprite_ = nullptr;

	// フェードの状態
	Status status_ = Status::kNone;

	// フェードの持続時間
	float duration_ = 0.0f;

	// 経過時間カウンター
	float counter_ = 0.0f;

	// 色
	KamataEngine::Vector4 color_ = {0.0f, 0.0f, 0.0f, 0.0f};


	// 画面サイズ
	static inline const float kWindowWidth = 1280.0f;
	static inline const float kWindowHeight = 720.0f;
};
