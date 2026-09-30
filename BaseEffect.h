#pragma once
#include"KamataEngine.h"

class BaseEffect{
public:
	virtual ~BaseEffect() = default;

	// 初期化処理
	void Initialize(const KamataEngine::Vector3& position);

	// 更新処理
	virtual void Update() = 0;

	// 描画処理
	virtual void Draw() = 0;

	// 死亡状態か
	virtual bool IsDead() const = 0;

	static void SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

  protected:
	// 初期化処理の個別部分
	virtual void InitializeInternal(const KamataEngine::Vector3& position) = 0;

	static KamataEngine::Camera* camera_;

	// 色変更オブジェクト
	KamataEngine::ObjectColor objectColor_;

	// 色
	KamataEngine::Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	// 経過時間
	float counter_ = 0.0f;

	// 1フレーム分の秒数
	static inline const float kFrameTime = 1.0f / 60.0f;

  private:
	// 初期化処理の共通部分
	void InitializeCommon();
};
