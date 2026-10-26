#define NOMINMAX
#include"Player.h"
#include<assert.h>
#include"Matrix4x4Util.h"
#include<numbers>
#include "Vector3Util.h"
#include<algorithm>
#include"Ease.h"
#include"MapChipField.h"
#include "GlobalVariables.h"
#include "GameInput.h"
#include "ModelManager.h"
#include <memory>

using namespace KamataEngine;

void Player::TurnTo(LRDirection direction) {
	// すでに指定方向を向いているなら何もしない
	if (lrDirection_ == direction) {
		return;
	}

	// 向きを変更
	lrDirection_ = direction;

	// 旋回開始時の角度を保存
	turnFirstRotationY_ = worldTransform_.rotation_.y;

	// 旋回タイマーを設定
	turnTimer_ = kTurnTime;
}

void Player::InitializeInternal() {

	// モデルマネージャーからモデルを取得
	model_ = ModelManager::GetInstance()->Load("player", true);
	modelAttack_ = ModelManager::GetInstance()->Load("hit_effect", true);

	assert(model_ != nullptr);
	assert(modelAttack_ != nullptr);

	// 死亡フラグを初期化
	isDead_ = false;
	deathType_ = DeathType::kNone;
	isDeathMotionFinished_ = false;

	// モデルの向きに合わせてY軸回りに90度回転させる
	worldTransform_.rotation_.y = kPi / 2.0f;

	worldTransformAttack_.Initialize();
	worldTransformAttack_.translation_ = worldTransform_.translation_;

	// 振る舞いの初期化
	RequestBehaviorChange(std::make_unique<PlayerRootBehavior>(this));

	// 攻撃エフェクトを非表示に初期化
	isAttackEffectVisible_ = false;

	// ノックバック無敵状態の初期化
	isKnockbackInvincible_ = false;

	// 外部からのノックバック要求を初期化
	isKnockbackRequested_ = false;

	width_ = 0.8f;
	height_ = 0.8f;
}

void Player::BeforeBehaviorUpdate() {
	// 外部からのノックバック要求を処理
	if (isKnockbackRequested_) {
		if (!isDead_) {
			// ノックバックへの切り替えを要求
			RequestBehaviorChange(std::make_unique<PlayerKnockbackBehavior>(this));
		}

		isKnockbackRequested_ = false;
	}
}

void Player::AfterBehaviorUpdate() {
	// 強制スクロール中かつ生存中は画面端補正と挟まれ死亡判定を行う
	if (!isDead_) {
		if (isScreenSqueezeDeathEnabled_) {
			// 画面端からはみ出さないように補正
			CorrectPositionInScreen();

			// 画面端とブロックに挟まれていたら死亡
			CheckScreenSqueezeDeath();
		}
	}

	// 旋回制御
	if (turnTimer_ > 0.0f) {
		// 旋回タイマーを1/60秒カウントダウンする
		turnTimer_ -= 1.0f / 60.0f;

		// 左右の自キャラ角度テーブル
		float destinationRotationYTable[] = {
		    kPi / 2.0f,        // 右
		    kPi * 3.0f / 2.0f, // 左
		};

		// 状態に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		// 0.0f から 1.0f に進む補間割合
		float t = 1.0f - std::clamp(turnTimer_ / kTurnTime, 0.0f, 1.0f);

		// イージングをかける
		float easedT = Ease::EaseInOut(t);

		// 自キャラの角度を設定する
		worldTransform_.rotation_.y = std::lerp(turnFirstRotationY_, destinationRotationY, easedT);
	}
}

void Player::AfterTransformUpdate() {
	// 攻撃エフェクト用ワールド変換の更新
	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;

	WorldTransformUpdate(worldTransformAttack_);
}

void Player::Draw() {
	// 描画すべきでないなら描画しない
	if (!ShouldDraw()) {
		return;
	}

	Model::PreDraw();

	// 攻撃エフェクトの描画
	if (IsAttack()) {
		if (isAttackEffectVisible_) {
			modelAttack_->Draw(worldTransformAttack_, *camera_);
		}
	}

	model_->Draw(worldTransform_, *camera_, &objectColor_);

	Model::PostDraw();
}

void Player::ResolveMapChipCollision(const Vector3& moveDistance) {
	// 衝突判定の初期化
	CollisionMapInfo collisionMapInfo{};

	// 移動量のコピー
	collisionMapInfo.moveDistance = moveDistance;

	// マップ衝突チェック
	MapChipCollision(collisionMapInfo);

	// 移動
	ApplyCollisionResult(collisionMapInfo);

	// 天井に接触している時の処理
	HandleCeilingCollision(collisionMapInfo);

	// 壁に接触している時の処理
	HandleWallCollision(collisionMapInfo);

	// 接地状態の切り替え処理
	ChangeGroundState(collisionMapInfo);
}

void Player::MapChipCollision(CollisionMapInfo& info) {
	MapChipCollisionTop(info);
	MapChipCollisionBottom(info);
	MapChipCollisionLeft(info);
	MapChipCollisionRight(info);
}

void Player::MapChipCollisionTop(CollisionMapInfo& info) {
	// 上昇あり？
	if (info.moveDistance.y <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew{};

	for (uint32_t i = 0; i < positionsNew.size(); i++) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveDistance, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 真上の判定を行う
	bool isHit = false;
	IndexSet indexSet{};
	IndexSet hitIndexSet{};

	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	if (isHit) {
		IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftTop));

		// 移動前と移動後でY方向のセル番号が変わった場合だけ、天井として処理する
		if (indexSetNow.yIndex != hitIndexSet.yIndex) {
			Rect rect = mapChipField_->RectByIndex(hitIndexSet.xIndex, hitIndexSet.yIndex);

			info.moveDistance.y = std::max(0.0f, rect.bottom - worldTransform_.translation_.y - height_ / 2.0f - kBlank);

			info.isHitCeiling = true;
		}
	}
}

void Player::MapChipCollisionBottom(CollisionMapInfo& info) {
	// 下降あり？
	if (info.moveDistance.y >= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew{};

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveDistance, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 真下の当たり判定を行う
	bool isHit = false;
	IndexSet indexSet{};
	IndexSet hitIndexSet{};

	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	if (isHit) {
		IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftBottom));

		// 移動前と移動後でY方向のセル番号が変わった場合だけ、地面として処理する
		if (indexSetNow.yIndex != hitIndexSet.yIndex) {
			Rect rect = mapChipField_->RectByIndex(hitIndexSet.xIndex, hitIndexSet.yIndex);

			info.moveDistance.y = std::min(0.0f, rect.top - worldTransform_.translation_.y + height_ / 2.0f + kBlank);

			info.onGround = true;
		}
	}
}

void Player::MapChipCollisionLeft(CollisionMapInfo& info) {
	// 左移動あり？
	if (info.moveDistance.x >= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew{};

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveDistance, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 真左の当たり判定を行う
	bool isHit = false;
	IndexSet indexSet{};
	IndexSet hitIndexSet{};

	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);

	// 隣接セルがどちらもブロックなら床や天井扱いなので、壁としては扱わない
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex + 1, indexSet.yIndex);

	// 隣接セルがどちらもブロックなら床や天井扱いなので、壁としては扱わない
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	if (isHit) {
		// 現在座標が属するマップチップ番号
		IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kLeftTop));

		// 移動前と移動後でX方向のセル番号が変わった場合だけ、壁として処理する
		if (indexSetNow.xIndex != hitIndexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			Rect rect = mapChipField_->RectByIndex(hitIndexSet.xIndex, hitIndexSet.yIndex);

			// めり込みを排除する方向に移動量を設定する
			info.moveDistance.x = std::min(0.0f, rect.right - worldTransform_.translation_.x + width_ / 2.0f + kBlank);

			// 壁に当たったことを記録する
			info.isHitWall = true;
		}
	}
}

void Player::MapChipCollisionRight(CollisionMapInfo& info) {
	// 右移動あり？
	if (info.moveDistance.x <= 0.0f) {
		return;
	}

	// 移動後の4つの角の座標
	std::array<Vector3, kNumCorner> positionsNew{};

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveDistance, static_cast<Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;

	// 真右の当たり判定を行う
	bool isHit = false;
	IndexSet indexSet{};
	IndexSet hitIndexSet{};

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);

	// 隣接セルがどちらもブロックなら床や天井扱いなので、壁としては扱わない
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex - 1, indexSet.yIndex);

	// 隣接セルがどちらもブロックなら床や天井扱いなので、壁としては扱わない
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	if (isHit) {
		// 現在座標が属するマップチップ番号
		IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(CornerPosition(worldTransform_.translation_, kRightTop));

		// 移動前と移動後でX方向のセル番号が変わった場合だけ、壁として処理する
		if (indexSetNow.xIndex != hitIndexSet.xIndex) {
			// めり込み先ブロックの範囲矩形
			Rect rect = mapChipField_->RectByIndex(hitIndexSet.xIndex, hitIndexSet.yIndex);

			// めり込みを排除する方向に移動量を設定する
			info.moveDistance.x = std::max(0.0f, rect.left - worldTransform_.translation_.x - width_ / 2.0f - kBlank);

			// 壁に当たったことを記録する
			info.isHitWall = true;
		}
	}
}

void Player::ApplyCollisionResult(const CollisionMapInfo& info) {
	worldTransform_.translation_ += info.moveDistance;
}

void Player::HandleCeilingCollision(const CollisionMapInfo& info) {
	if (info.isHitCeiling) {
		velocity_.y = 0.0f;
	}
}

// 地面に接触してる場合の処理
void Player::HandleGroundCollision(const CollisionMapInfo& info) {
	if (info.onGround) {
		// 着地状態に移行
		isGround_ = true;

		// 接地時にx速度を減衰
		velocity_.x *= (1.0f - kAttenuationLanding);

		velocity_.y = 0.0f;
	}
}

void Player::HandleWallCollision(const CollisionMapInfo& info) {
	if (info.isHitWall) {
		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

// 接地状態の切り替え処理
void Player::ChangeGroundState(const CollisionMapInfo& info) {
	if (isGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			isGround_ = false;
			return;
		}

		// 落下開始判定
		MapChipType mapChipType;
		bool isHit = false;

		// 左下点の判定
		Vector3 leftBottom = CornerPosition(worldTransform_.translation_, kLeftBottom);
		leftBottom += Vector3(0.0f, -kGroundCheckOffset, 0.0f);

		IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(leftBottom);
		mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

		if (mapChipType == MapChipType::kBlock) {
			isHit = true;
		}

		// 右下点の判定
		Vector3 rightBottom = CornerPosition(worldTransform_.translation_, kRightBottom);
		rightBottom += Vector3(0.0f, -kGroundCheckOffset, 0.0f);

		indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);
		mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

		if (mapChipType == MapChipType::kBlock) {
			isHit = true;
		}

		// 床がなくなったら空中状態に移行
		if (!isHit) {
			isGround_ = false;
		}

	} else {
		// 空中から地面に接地する際の処理
		HandleGroundCollision(info);
	}
}

const WorldTransform& Player::GetWorldTransform() {
	return worldTransform_;
}

void Player::OnCollision(const BaseEnemy* enemy) {
	// すでに死んでいたら早期リターン
	if (isDead_) {
		return;
	}

	// 現状は敵に当たった時、敵に対して行いたい処理がない
	// そのため、警告を抑制する意味で無意味な処理を書く
	(void)enemy;

	// 攻撃中はダメージ無効
	if (IsAttack()) {
		return;
	}

	// ノックバック初めの時間に無敵でないと、そのまま敵にめり込んで死ぬ場合があるので、吹っ飛ばされている間は無敵
	if (IsKnockbackInvincible()) {
		return;
	}

	// 通常死亡状態にする
	deathType_ = DeathType::kNormal;
	isDead_ = true;
}

Vector3 Player::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[static_cast<uint32_t>(Corner::kNumCorner)] = {
		Vector3(width_ / 2.0f,	 -height_ / 2.0f,	 0.0f),  // 右下
		Vector3(-width_ / 2.0f,	 -height_ / 2.0f,	 0.0f), // 左下
		Vector3(width_ / 2.0f,	 height_ / 2.0f,	 0.0f),   // 右上
		Vector3(-width_ / 2.0f,	 height_ / 2.0f,	 0.0f)   // 左上
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}

void Player::CorrectPositionInScreen() {
	// カメラが未設定なら何もしない
	if (!camera_) {
		return;
	}

	// カメラ位置を基準に画面端のワールド座標を計算
	const float leftLimit = camera_->translation_.x + kScreenLeftLimit + width_ / 2.0f;
	const float rightLimit = camera_->translation_.x + kScreenRightLimit - width_ / 2.0f;
	const float bottomLimit = camera_->translation_.y + kScreenBottomLimit + height_ / 2.0f;
	const float topLimit = camera_->translation_.y + kScreenTopLimit - height_ / 2.0f;

	// 画面外に出ないように座標を補正
	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, leftLimit, rightLimit);
	worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, bottomLimit, topLimit);
}

void Player::CheckScreenSqueezeDeath() {
	// 強制スクロール中でなければ判定しない
	if (!isScreenSqueezeDeathEnabled_) {
		return;
	}

	// すでに死亡しているなら何もしない
	if (isDead_) {
		return;
	}

	// 画面左端にいなければ挟まれていない
	if (!IsTouchingLeftScreenEdge()) {
		return;
	}

	// 右側にブロックがあれば、画面端とブロックに挟まれて死亡
	if (IsBlockOnRightSide()) {
		StartSqueezeDeath();
	}
}

bool Player::IsTouchingLeftScreenEdge() {
	if (!camera_) {
		return false;
	}

	// プレイヤー中心座標として許される左端
	const float leftLimit = camera_->translation_.x + kScreenLeftLimit + width_ / 2.0f;

	return worldTransform_.translation_.x <= leftLimit + kScreenEdgeCheckTolerance;
}

bool Player::IsBlockOnRightSide() {
	if (!mapChipField_) {
		return false;
	}

	// 右上と右下を少しだけ右側にずらして、壁があるか調べる
	KamataEngine::Vector3 rightTop = {
		worldTransform_.translation_.x + width_ / 2.0f + kScreenSqueezeCheckOffset,
		worldTransform_.translation_.y + height_ / 2.0f - kScreenSqueezeCheckOffset,
		worldTransform_.translation_.z
	};

	KamataEngine::Vector3 rightBottom = {
		worldTransform_.translation_.x + width_ / 2.0f + kScreenSqueezeCheckOffset,
		worldTransform_.translation_.y - height_ / 2.0f + kScreenSqueezeCheckOffset,
		worldTransform_.translation_.z
	};

	IndexSet indexSet{};

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightTop);

	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		return true;
	}

	indexSet = mapChipField_->GetMapChipIndexSetByPosition(rightBottom);

	if (mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex) == MapChipType::kBlock) {
		return true;
	}

	return false;
}

void Player::StartSqueezeDeath() {
	deathType_ = DeathType::kSqueeze;
	isDead_ = true;
	isDeathMotionFinished_ = false;

	RequestBehaviorChange(std::make_unique<PlayerSqueezeDeathBehavior>(this));

	// 通常移動の速度を一旦止める
	velocity_ = {0.0f, 0.0f, 0.0f};
}

bool Player::ShouldDraw() const {
	// 生きているなら描画する
	if (!isDead_) {
		return true;
	}

	// 挟まれ死亡中はプレイヤー本体を描画する
	if (deathType_ == DeathType::kSqueeze) {
		return true;
	}

	// 通常死亡では描画しない
	return false;
}