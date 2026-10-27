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

	// ヒットストップを要求
	void RequestHitStop();

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

	// 落下死亡判定のY座標
	static inline const float kFallDeathY = -2.0f;

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

	// ヒットストップの残りフレーム数
	int32_t hitStopTimer_ = 0;

	// ヒットストップの持続フレーム数
	static inline const int32_t kHitStopFrames = 5;

	// 敵を生成
	void GenerateEnemy(KamataEngine::Vector3 &position,uint8_t type);

	// フィールドの全オブジェクトを生成
	void GenerateFieldObjects();

	std::vector <std::vector<std::unique_ptr<KamataEngine::WorldTransform>>> worldTransformBlocks_;

	// ブロックモデル
	KamataEngine::Model* modelBlock_ = nullptr;

	// はしごモデル
	KamataEngine::Model* modelLadder_ = nullptr;

	// はしごのワールド変換
	std::vector<std::unique_ptr<KamataEngine::WorldTransform>> worldTransformLadders_;

	// 画面サイズ
	const int kMaxWindowWidth = 1280;
	const int kMaxWindowHeight = 720;
};
