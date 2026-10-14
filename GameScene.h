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
	std::unique_ptr<Player> player_;

	// 敵
	std::list <std::unique_ptr<BaseEnemy>> enemies_;

	// 天球
	std::unique_ptr <Skydome> skydome_;

	// デスパーティクル
	std::unique_ptr<DeathParticles> deathParticles_;

	// マップチップフィールド
	std::unique_ptr<MapChipField> mapChipField_;

	// カメラ
	std::unique_ptr<KamataEngine::Camera> camera_;
	std::unique_ptr<CameraController> cameraController_;

	// デバッグカメラ
	std::unique_ptr<KamataEngine::DebugCamera> debugCamera_;

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
	std::unique_ptr<Fade> fade_;

	// 全てのあたり判定を行う
	void CheckAllCollisions();

	// ゲームプレイフェーズの更新
	void UpdatePlayPhase();

	// デス演出フェーズの更新
	void UpdateDeathPhase();

	// フェーズ切り替え
	void ChangePhase();

	// ヒットエフェクト
	std::list<std::unique_ptr<BaseEffect>> effects_;

	// ヒットエフェクトの更新
	void UpdateHitEffects();

	// ヒットエフェクトの描画
	void DrawHitEffects();

	// 敵を生成
	void GenerateEnemy(KamataEngine::Vector3 &position,uint8_t type);

	// フィールドの全オブジェクトを生成
	void GenerateFieldObjects();

	std::vector <std::vector<std::unique_ptr<KamataEngine::WorldTransform>>> worldTransformBlocks_;

	uint32_t blockTextureHandle_ = 0;
	std::unique_ptr<KamataEngine::Model> modelBlock_;

	// 画面サイズ
	const int kMaxWindowWidth = 1280;
	const int kMaxWindowHeight = 720;
};
