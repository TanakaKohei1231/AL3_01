#include "GameScene.h"
#include "Matrix4x4Util.h"
#include "MapChipField.h"
#include "HitEffect.h"
#include "Enemy.h"
#include "ShieldEnemy.h"
#include "GuardEffect.h"
#include "StageManager.h"
#include "ModelManager.h"
#include <algorithm>
#include "SoundManager.h"

using namespace KamataEngine;

namespace {

enum class EnemySpawnType : uint8_t {
	kNormal = 0,
	kShield = 1,
};

} // namespace

void GameScene::Initialize() {
	// フェーズ初期化
	phase_ = Phase::kFadeIn;

	isCleared_ = false;

	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->farZ = 1100.0f;
	camera_->Initialize();

	cameraController_ = std::make_unique<CameraController>();
	cameraController_->Initialize(camera_.get());
	cameraController_->SetMode(CameraMode::kFollow);

	// ゲームBGMを再生
	SoundManager::GetInstance()->PlayBgm(BgmType::kGame);

	ModelManager *modelManager = ModelManager::GetInstance();

	// 天球の生成
	skydome_ = std::make_unique<Skydome>();
	skydome_->Initialize(modelManager->Load("SkyDome", true), camera_.get());

	// マップチップフィールドの生成
	mapChipField_ = std::make_unique<MapChipField>();

	// 現在のステージデータを取得する
	const StageData& stageData = StageManager::GetInstance()->GetCurrentStageData();

	// ステージファイルパスの生成
	std::string stageFileName = "fieldsData/" + stageData.name + ".csv";

	// ステージファイルの読み込み
	mapChipField_->LoadMapChipCsv(stageFileName);

	// フェード
	fade_ = std::make_unique<Fade>();
	fade_->Initialize();
	fade_->Start(Fade::Status::kFadeIn, 1.0f);

	// デバッグカメラの生成
	debugCamera_ = std::make_unique<DebugCamera>(kMaxWindowWidth, kMaxWindowHeight);
	debugCamera_->SetFarZ(camera_->farZ);

	// ブロック
	modelBlock_ = modelManager->Load("block", true);

	// はしご
	modelLadder_ = modelManager->Load("Ladder", true);

	HitEffect::SetModel(modelManager->Load("particle", true));
	HitEffect::SetCamera(camera_.get());

	GuardEffect::SetModel(modelManager->Load("ring", true));
	GuardEffect::SetCamera(camera_.get());

	// フィールドのオブジェクトを生成
	GenerateFieldObjects();

	assert(player_ != nullptr && "プレイヤーがCSV内に配置されていません");

	// 追従カメラをプレイヤーにセット
	cameraController_->SetTarget(player_.get());
	cameraController_->Reset();
}

void GameScene::Update() {
	// フェーズ切り替え
	ChangePhase();

	// 敵のヒットフラッシュ更新
	for (const auto& enemy : enemies_) {
		enemy->UpdateHitFlash();
	}

	switch (phase_) {
	case Phase::kFadeIn:
		UpdatePlayPhase();
		fade_->Update();
		break;

	case Phase::kPlay:
		UpdatePlayPhase();
		break;

	case Phase::kDeath:
		UpdateDeathPhase();
		break;

	case Phase::kClear:
		UpdateClearPhase();
		break;

	case Phase::kFadeOut:
		if (isCleared_) {
			// クリア後はプレイヤーを動かさない
			skydome_->Update();

			if (!isDebugCameraActive_) {
				cameraController_->Update();
			}
		} else {
			UpdateDeathPhase();
		}

		fade_->Update();
		break;
	}

	#ifdef _DEBUG

	ImGui::Begin("Debug");

	// リロードボタン
	if (ImGui::Button("Reload")) {
		nextScene_ = SCENE::kGame;
	}

	// 追従カメラ起動ボタン
	if (ImGui::Button("Follow Camera")) {
		cameraController_->SetMode(CameraMode::kFollow);
	}

	// 強制スクロール起動ボタン
	if (ImGui::Button("Forced Scroll Camera")) {
		cameraController_->SetMode(CameraMode::kForcedScroll);
	}

	ImGui::End();

	#endif // _DEBUG
}

void GameScene::Draw() {
	skydome_->Draw();

	Model::PreDraw();

	for (const auto& worldTransformBlockLine : worldTransformBlocks_) {
		for (const auto& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}

			modelBlock_->Draw(*worldTransformBlock, *camera_);
		}
	}

	// はしご描画
	for (const auto& worldTransform : worldTransformLadders_) {
		modelLadder_->Draw(*worldTransform, *camera_);
	}


	Model::PostDraw();

	// ゴール描画
	if (goal_) {
		goal_->Draw();
	}

	player_->Draw();

	for (const auto& enemy : enemies_) {
		enemy->Draw();
	}

	// ヒットエフェクトの描画
	DrawHitEffects();

	if (deathParticles_) {
		deathParticles_->Draw();
	}

	if (fade_) {
		fade_->Draw();
	}
}

void GameScene::GenerateFieldObjects() {
	// 要素数
	uint32_t numBlockVertical = mapChipField_->GetNumBlockVertical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	// 要素数を変更する
	// 列数の設定
	worldTransformBlocks_.resize(numBlockVertical);
	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		// 1列の要素数の設定
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}
	
	// 生成
	for (uint32_t i = 0; i < numBlockVertical; ++i) {
		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {

			Vector3 mapPosition = mapChipField_->GetMapChipPositionByIndex(j, i);

			switch (mapChipField_->GetMapChipTypeByIndex(j, i)) {
				// ブロック
			case MapChipType::kBlock: {
				worldTransformBlocks_[i][j] = std::make_unique<WorldTransform>();

				worldTransformBlocks_[i][j]->Initialize();
				worldTransformBlocks_[i][j]->translation_ = mapPosition;
				break;
			}

				// プレイヤー
			case MapChipType::kPlayer: {
				// 既にプレイヤーが生成されているなら停止
				assert(player_ == nullptr && "自キャラを二重に配置しようとしています");

				player_ = std::make_unique<Player>();
				player_->Initialize(camera_.get(), mapPosition, mapChipField_.get());

				break;
			}

			case MapChipType::kEnemy: {
				GenerateEnemy(mapPosition, mapChipField_->GetMapChipSubIDByIndex(j, i));

				break;
			}

			case MapChipType::kLadder: {
				// はしごのワールド変換を生成
				auto worldTransform = std::make_unique<WorldTransform>();

				worldTransform->Initialize();
				worldTransform->translation_ = mapPosition;
				worldTransform->rotation_.y = kPi;
				worldTransform->scale_ = {0.5f, 0.5f, 0.5f};

				// ワールド行列を更新
				WorldTransformUpdate(*worldTransform);

				// リストに追加
				worldTransformLadders_.push_back(std::move(worldTransform));

				break;
			}

			// ゴール
			case MapChipType::kGoal: {
				// ゴールの二重配置を防止
				assert(goal_ == nullptr && "ゴールを二重に配置しようとしています");

				// ゴールを生成
				goal_ = std::make_unique<Goal>();
				goal_->Initialize(camera_.get(), mapPosition);

				break;
			}
			}
		}
	}
}

void GameScene::GenerateEnemy(Vector3 &position,uint8_t type) {
	switch (static_cast<EnemySpawnType>(type)) {
		// 歩行敵の生成
	case EnemySpawnType::kNormal: {
	
		auto newEnemy = std::make_unique<Enemy>();
		newEnemy->Initialize(camera_.get(), position, mapChipField_.get());
		newEnemy->SetGameScene(this);
		enemies_.push_back(std::move(newEnemy));

		break;
	}

		// 盾敵の生成
	case EnemySpawnType::kShield: {
	
		auto newShieldEnemy = std::make_unique<ShieldEnemy>();
		newShieldEnemy->Initialize(camera_.get(), position, mapChipField_.get());
		newShieldEnemy->SetGameScene(this);
		enemies_.push_back(std::move(newShieldEnemy));

		break;
	}
	}
}

GameScene::GameScene() {}

GameScene::~GameScene() {

	effects_.clear();

	worldTransformBlocks_.clear();
}

void GameScene::CheckAllCollisions() {
	// プレイヤーのあたり判定
	AABB playerAABB = player_->GetAABB();

	// 敵とプレイヤーのあたり判定
	for (const auto& enemy : enemies_) {

		// あたり判定無効の敵との処理はスキップ
		if (enemy->IsCollisionDisabled()) {
			continue;
		}

		if (IsCollision(playerAABB, enemy->GetAABB())) {
			player_->OnCollision(enemy.get());
			enemy->OnCollision(player_.get());
		}
	}

	// ゴールとの接触判定
	CheckGoalCollision();
}

void GameScene::UpdatePlayPhase() {
	// ヒットストップ中はゲームの更新を停止
	if (hitStopTimer_ > 0) {
		--hitStopTimer_;

		// カメラシェイクだけ更新
		if (!isDebugCameraActive_) {
			cameraController_->UpdateShake();
		}

		// ヒットエフェクトは更新する
		UpdateHitEffects();

		return;
	}

	// 強制スクロール中だけ、画面端挟まれ死亡判定を有効にする
	player_->SetScreenSqueezeDeathEnabled(cameraController_->GetMode() == CameraMode::kForcedScroll);

	player_->Update();

	// ゴールの浮遊・回転アニメーションを更新
	if (goal_) {
		goal_->Update();
	}

	// プレイヤーの落下死亡判定
	player_->CheckFallDeath(kFallDeathY);

	for (const auto& enemy : enemies_) {
		enemy->Update();

		// 敵の落下死亡判定
		enemy->CheckFallDeath(kFallDeathY);
	}

	// 死んだ敵を削除
	enemies_.remove_if([](const std::unique_ptr<BaseEnemy>& enemy) {
		return enemy->IsDead();
	});

	skydome_->Update();

	#ifdef _DEBUG

	// Cを押すとデバッグカメラの有効化フラグをトグル
	if (Input::GetInstance()->TriggerKey(DIK_C)) {
		isDebugCameraActive_ = !isDebugCameraActive_;
	}

	#endif // _DEBUG

	// カメラの処理
	if (isDebugCameraActive_) {
		debugCamera_->Update();

		// デバッグカメラが有効ならデバッグカメラのビュー行列とプロジェクション行列をカメラにコピーする
		camera_->matView = debugCamera_->GetCamera().matView;
		camera_->matProjection = debugCamera_->GetCamera().matProjection;

		// ビュープロジェクション行列の転送
		camera_->TransferMatrix();

	} else {
		// ビュープロジェクション行列の更新と転送
		cameraController_->Update();
	}

	// ブロックの更新
	for (const auto& worldTransformBlockLine : worldTransformBlocks_) {
		for (const auto& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}

			WorldTransformUpdate(*worldTransformBlock);
		}
	}

	// あたり判定の更新
	CheckAllCollisions();

	// ヒットエフェクトの更新
	UpdateHitEffects();
}


void GameScene::UpdateDeathPhase() {
	player_->Update();

	// ゴールの浮遊・回転アニメーションを更新
	if (goal_) {
		goal_->Update();
	}

	for (const auto& enemy : enemies_) {
		enemy->Update();
	}

	skydome_->Update();

	if (deathParticles_) {
		deathParticles_->Update();
	}

	for (const auto& worldTransformBlockLine : worldTransformBlocks_) {
		for (const auto& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}

			WorldTransformUpdate(*worldTransformBlock);
		}
	}

	// ヒットエフェクトの更新
	UpdateHitEffects();
}

void GameScene::UpdateClearPhase() {
	// プレイヤーのクリア演出を更新
	player_->Update();

	// 背景を更新
	skydome_->Update();

	// カメラを更新
	if (isDebugCameraActive_) {
		debugCamera_->Update();

		camera_->matView = debugCamera_->GetCamera().matView;
		camera_->matProjection = debugCamera_->GetCamera().matProjection;
		camera_->TransferMatrix();
	} else {
		cameraController_->Update();
	}

	// クリア演出が終わったらフェードアウト
	if (player_->IsClearDirectionFinished()) {
		phase_ = Phase::kFadeOut;
		fade_->Start(Fade::Status::kFadeOut, 1.0f);
	}
}

void GameScene::ChangePhase() {
	switch (phase_) {
	case Phase::kFadeIn:
		if (fade_->IsFinished()) {
			fade_->Stop();
			phase_ = Phase::kPlay;
		}
		break;

	case Phase::kPlay:
		if (player_->IsDead()) {
			// デス演出フェーズに切り替え
			phase_ = Phase::kDeath;

			// 死亡SE
			SoundManager::GetInstance()->PlaySe(SeType::kPlayerDeath);

			// 自キャラの座標を取得
			Vector3 deathParticlesPosition = player_->GetWorldPosition();

			// 自キャラの座標にデスパーティクルを発生
			deathParticles_ = std::make_unique<DeathParticles>();
			deathParticles_->Initialize(camera_.get(), deathParticlesPosition);
		}
		break;

	case Phase::kDeath:
		if (deathParticles_ && deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
			fade_->Start(Fade::Status::kFadeOut, 1.0f);
		}
		break;

	case Phase::kFadeOut:
		if (fade_->IsFinished()) {
			nextScene_ = SCENE::kTitle;
		}
		break;
	}
}

void GameScene::CreateHitEffect(const Vector3& position) {
	effects_.push_back(std::unique_ptr<BaseEffect>(HitEffect::Create(position)));

	// 命中SE
	SoundManager::GetInstance()->PlaySe(SeType::kHit);

	// ヒットストップを要求
	RequestHitStop();

	// カメラシェイクを開始
	cameraController_->StartShake(0.2f, 12);
}

void GameScene::CreateGuardEffect(const Vector3& position) {
	// ガードSE
	SoundManager::GetInstance()->PlaySe(SeType::kGuard);

	effects_.push_back(std::unique_ptr<BaseEffect>(GuardEffect::Create(position)));
}

void GameScene::RequestHitStop() {
	// 現在の残り時間より長い場合だけ更新
	hitStopTimer_ = (std::max)(hitStopTimer_, kHitStopFrames);
}

void GameScene::UpdateHitEffects() {
	for (const auto &effect : effects_) {
		effect->Update();
	}

	effects_.remove_if([](const std::unique_ptr<BaseEffect>& effect) {
		return effect->IsDead();
		});
}

void GameScene::DrawHitEffects() {
	for (const auto& effect : effects_) {
		effect->Draw();
	}
}

void GameScene::CheckGoalCollision() {
	// ゴールがない、または取得済みなら判定しない
	if (!goal_ || goal_->IsCollected()) {
		return;
	}

	// 死亡しているプレイヤーはゴールできない
	if (player_->IsDead()) {
		return;
	}

	// プレイヤーとゴールの接触判定
	if (IsCollision(player_->GetAABB(), goal_->GetAABB())) {
		StartClear();
	}
}

void GameScene::StartClear() {
	// すでにクリアしていれば何もしない
	if (isCleared_) {
		return;
	}

	isCleared_ = true;

	// ゴールを取得済みにする
	goal_->Collect();

	// SEを流す
	SoundManager::GetInstance()->PlaySe(SeType::kClear);

	// プレイヤーのクリア演出を開始
	player_->StartClearDirection();

	// クリアフェーズへ
	phase_ = Phase::kClear;
}