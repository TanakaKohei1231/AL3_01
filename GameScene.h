#pragma once
#include"KamataEngine.h"
#include "IScene.h"
#include"Player.h"
#include "BaseEnemy.h"
#include "Skydome.h"
#include<vector>
#include "MapChipField.h"
#include "CameraController.h"
#include "DeathParticles.h"
#include "Fade.h"
#include <list>
#include"BaseEffect.h"

// 前方宣言
class HitEffect;

// ゲームシーン
class GameScene : public IScene {
 public:
	// 初期化処理
	void Initialize() override;

	// 更新処理
	void Update() override;

	// 描画処理
	void Draw() override;

	GameScene();
	~GameScene();

	// ヒットエフェクトを生成
	void CreateHitEffect(const KamataEngine::Vector3& position);

	// ガードエフェクトを生成
	void CreateGuardEffect(const KamataEngine::Vector3& position);

  private:

	// プレイヤー
	Player* player_ = nullptr;
	KamataEngine::Model* mPlayerModel_ = nullptr;

	// プレイヤー攻撃エフェクト用モデル
	KamataEngine::Model* modelAttack_ = nullptr;

	// ヒットエフェクト用モデル
	KamataEngine::Model* modelHitEffect_ = nullptr;

	// ガードエフェクト用モデル
	KamataEngine::Model* modelGuardEffect_ = nullptr;

	// 敵
	std::list<BaseEnemy*> enemies_;
	KamataEngine::Model* mEnemyModel_ = nullptr;
	KamataEngine::Model* mShieldEnemyModel_ = nullptr;

	// 天球
	Skydome* skydome_ = nullptr;
	KamataEngine::Model* modelSkydome_ = nullptr;

	// デスパーティクル
	DeathParticles* deathParticles_ = nullptr;
	KamataEngine::Model* modelDeathParticles_ = nullptr;

	// マップチップフィールド
	MapChipField *mapChipField_ = nullptr;

	// カメラ
	KamataEngine::Camera camera_;

	CameraController* cameraController_ = nullptr;

	// デバッグカメラ
	KamataEngine::DebugCamera *debugCamera_ = nullptr;

	// デバッグカメラの有効化フラグ
	bool isDebugCameraActive_ = false;

	// ゲームのフェーズ
	enum class Phase {
		kFadeIn,  // フェードイン
		kPlay,    // ゲームプレイ
		kDeath,   // デス演出
		kFadeOut, // フェードアウト
	};

	// 現在のフェーズ
	Phase phase_ = Phase::kFadeIn;

	// フェード
	Fade* fade_ = nullptr;

	// 全てのあたり判定を行う
	void CheckAllCollisions();

	// ゲームプレイフェーズの更新
	void UpdatePlayPhase();

	// デス演出フェーズの更新
	void UpdateDeathPhase();

	// フェーズ切り替え
	void ChangePhase();

	// ヒットエフェクト
	std::list<BaseEffect*> effects_;

	// ヒットエフェクトの更新
	void UpdateHitEffects();

	// ヒットエフェクトの描画
	void DrawHitEffects();

	// 敵を生成
	void GenerateEnemy(KamataEngine::Vector3 &position,uint8_t type);

	// フィールドの全オブジェクトを生成
	void GenerateFieldObjects();

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	uint32_t mBlockTextureHandle_ = 0;
	KamataEngine::Model* mBlockModel_ = nullptr;

	// 画面サイズ
	const int kMaxWindowWidth = 1280;
	const int kMaxWindowHeight = 720;
};
