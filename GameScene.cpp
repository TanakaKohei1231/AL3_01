#include "GameScene.h"
#include "Matrix4x4Util.h"
#include "MapChipField.h"
#include "HitEffect.h"
#include "Enemy.h"
#include "ShieldEnemy.h"
#include "GuardEffect.h"
#include "StageManager.h"

using namespace KamataEngine;

namespace {

enum class EnemySpawnType : uint8_t {
	kNormal = 0,
	kShield = 1,
};

} // namespace

void GameScene::Initialize(StageManager* stageManager) {
	// 引数をメンバ変数に記録する
	stageManager_ = stageManager;
	assert(stageManager_);

	// フェーズ初期化
	isFinished_ = false;
	phase_ = Phase::kFadeIn;

	// カメラの初期化
	camera_.farZ = 1100.0f;
	camera_.Initialize();

	cameraController_ = new CameraController();
	cameraController_->Initialize(&camera_);
	cameraController_->SetMode(CameraMode::kFollow);


	// 天球の生成
	skydome_ = new Skydome();
	modelSkydome_ = Model::CreateFromOBJ("SkyDome", true);
	skydome_->Initialize(modelSkydome_, &camera_);

	// マップチップフィールドの生成
	mapChipField_ = new MapChipField();

	// 現在のステージデータを取得する
	const StageData& stageData = stageManager_->GetCurrentStageData();

	// ステージファイルパスの生成
	std::string stageFileName = "fieldsData/" + stageData.name + ".csv";

	// ステージファイルの読み込み
	mapChipField_->LoadMapChipCsv(stageFileName);

	// プレイヤー
	mPlayerModel_ = Model::CreateFromOBJ("player",true);
	modelAttack_ = KamataEngine::Model::CreateFromOBJ("hit_effect", true);

	// デスパーティクル
	modelDeathParticles_ = Model::CreateFromOBJ("deathParticle", true);

	// フェード
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::kFadeIn, 1.0f);

	// 敵
	mEnemyModel_ = Model::CreateFromOBJ("enemy", true);
	mShieldEnemyModel_ = Model::CreateFromOBJ("shieldEnemy", true);

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(kMaxWindowWidth, kMaxWindowHeight);
	debugCamera_->SetFarZ(camera_.farZ);

	// ブロック
	mBlockModel_ = Model::CreateFromOBJ("block", true);
	mBlockTextureHandle_ = TextureManager::Load("block/block.png");

	modelHitEffect_ = Model::CreateFromOBJ("particle", true);
	HitEffect::SetModel(modelHitEffect_);
	HitEffect::SetCamera(&camera_);

	modelGuardEffect_ = Model::CreateFromOBJ("ring", true);
	GuardEffect::SetModel(modelGuardEffect_);
	GuardEffect::SetCamera(&camera_);

	// 調整項目の登録
	Player::RegisterGlobalVariables();

	// 調整項目の適用
	Player::ApplyGlobalVariables();

	// フィールドのオブジェクトを生成
	GenerateFieldObjects();

	assert(player_ != nullptr && "プレイヤーがCSV内に配置されていません");

	// 追従カメラをプレイヤーにセット
	cameraController_->SetTarget(player_);
	cameraController_->Reset();
}

void GameScene::Update() {
	// フェーズ切り替え
	ChangePhase();

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

	case Phase::kFadeOut:
		UpdateDeathPhase();
		fade_->Update();
		break;
	}

	#ifdef _DEBUG

	ImGui::Begin("Debug");

	// リロードボタン
	if (ImGui::Button("Reload")) {
		reloadRequested_ = true;
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

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}

			mBlockModel_->Draw(*worldTransformBlock, camera_);
		}
	}

	Model::PostDraw();

	player_->Draw();

	for (BaseEnemy* enemy : enemies_) {
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
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapPosition;
				break;
			}

				// プレイヤー
			case MapChipType::kPlayer: {
				// 既にプレイヤーが生成されているなら停止
				assert(player_ == nullptr && "自キャラを二重に配置しようとしています");

				player_ = new Player();
				player_->Initialize(mPlayerModel_, modelAttack_, &camera_, mapPosition);
				player_->SetMapChipField(mapChipField_);

				break;
			}

			case MapChipType::kEnemy: {
				GenerateEnemy(mapPosition, mapChipField_->GetMapChipSubIDByIndex(j, i));
			}
			}
		}
	}
}

void GameScene::GenerateEnemy(Vector3 &position,uint8_t type) {
	switch (static_cast<EnemySpawnType>(type)) {
		// 歩行敵の生成
	case EnemySpawnType::kNormal: {
	
		Enemy* newEnemy = new Enemy();
		newEnemy->Initialize(mEnemyModel_, &camera_, position);
		newEnemy->SetGameScene(this);
		enemies_.push_back(newEnemy);

		break;
	}

		// 盾敵の生成
	case EnemySpawnType::kShield: {
	
		ShieldEnemy* newShieldEnemy = new ShieldEnemy();
		newShieldEnemy->Initialize(mShieldEnemyModel_, &camera_, position);
		newShieldEnemy->SetGameScene(this);
		enemies_.push_back(newShieldEnemy);

		break;
	}
	}
}

GameScene::GameScene() {}

GameScene::~GameScene() {
	delete player_;
	delete mPlayerModel_;
	delete modelAttack_;
	modelAttack_ = nullptr;
	delete deathParticles_;
	delete modelDeathParticles_;

	for (BaseEnemy* enemy : enemies_) {
		delete enemy;
	}

	delete mEnemyModel_;
	delete mShieldEnemyModel_;
	delete modelGuardEffect_;
	delete mBlockModel_;
	delete skydome_;
	delete modelSkydome_;

	for (BaseEffect* effect : effects_) {
		delete effect;
	}

	effects_.clear();

	delete modelHitEffect_;
	delete debugCamera_;
	delete mapChipField_;
	delete cameraController_;
	delete fade_;
	fade_ = nullptr;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}

	worldTransformBlocks_.clear();
}

void GameScene::CheckAllCollisions() {
	// プレイヤーのあたり判定
	AABB playerAABB = player_->GetAABB();

	// 敵とプレイヤーのあたり判定
	for (BaseEnemy* enemy : enemies_) {

		// あたり判定無効の敵との処理はスキップ
		if (enemy->IsCollisionDisabled()) {
			continue;
		}

		if (IsCollision(playerAABB, enemy->GetAABB())) {
			player_->OnCollision(enemy);
			enemy->OnCollision(player_);
		}
	}
}

void GameScene::UpdatePlayPhase() {
	// 強制スクロール中だけ、画面端挟まれ死亡判定を有効にする
	player_->SetScreenSqueezeDeathEnabled(cameraController_->GetMode() == CameraMode::kForcedScroll);

	player_->Update();

	for (BaseEnemy* enemy : enemies_) {
		enemy->Update();
	}

	// 死んだ敵を削除
	enemies_.remove_if([] (BaseEnemy * enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
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
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;

		// ビュープロジェクション行列の転送
		camera_.TransferMatrix();

	} else {
		// ビュープロジェクション行列の更新と転送
		cameraController_->Update();
	}

	// ブロックの更新
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
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
	for (BaseEnemy* enemy : enemies_) {
		enemy->Update();
	}

	skydome_->Update();

	if (deathParticles_) {
		deathParticles_->Update();
	}

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock) {
				continue;
			}

			WorldTransformUpdate(*worldTransformBlock);
		}
	}

	// ヒットエフェクトの更新
	UpdateHitEffects();
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

			// 自キャラの座標を取得
			Vector3 deathParticlesPosition = player_->GetWorldPosition();

			// 自キャラの座標にデスパーティクルを発生
			deathParticles_ = new DeathParticles();
			deathParticles_->Initialize(modelDeathParticles_, &camera_, deathParticlesPosition);
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
			isFinished_ = true;
		}
		break;
	}
}

void GameScene::CreateHitEffect(const Vector3& position) {
	HitEffect* newHitEffect = HitEffect::Create(position);
	effects_.push_back(newHitEffect);
}

void GameScene::CreateGuardEffect(const Vector3& position) {
	GuardEffect* newHitEffect = GuardEffect::Create(position);
	effects_.push_back(newHitEffect);
}

void GameScene::UpdateHitEffects() {
	for (BaseEffect* effect : effects_) {
		effect->Update();
	}

	effects_.remove_if([](BaseEffect* effect) {
		if (effect->IsDead()) {
			delete effect;
			return true;
		}

		return false;
	});
}

void GameScene::DrawHitEffects() {
	for (BaseEffect* effect : effects_) {
		effect->Draw();
	}
}