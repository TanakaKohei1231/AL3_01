#define NOMINMAX
#include "BaseCharacter.h"
#include <utility>
#include "MapChipField.h"
#include <algorithm>
#include <array>
#include <cstdint>

using namespace KamataEngine;

void BaseCharacter::Initialize(KamataEngine::Camera* camera, KamataEngine::Vector3 position, MapChipField* mapChipField) {

#ifdef _DEBUG

	assert(camera);

#endif // _DEBUG

	SetMapChipField(mapChipField);

	// ワールド変換の初期化
	worldTransform_.Initialize();

	// 初期配置座標
	worldTransform_.translation_ = position;

	// 引数で渡されたカメラをメンバ変数に保存
	camera_ = camera;

	isCollisionDisabled_ = false;
	isDead_ = false;

	// ヒットフラッシュの初期化
	objectColor_.Initialize();
	objectColor_.SetColor(kNormalColor);
	hitFlashTimer_ = 0;

	// キャラクター固有の初期化
	InitializeInternal();
}

void BaseCharacter::Update() {
	// 振る舞い更新前の個別処理
	BeforeBehaviorUpdate();

	// 振る舞い変更リクエストを反映
	ChangeBehavior();

	// 振る舞いの更新
	if (behavior_) {
		behavior_->Update();
		behavior_->UpdateMotion();
	}

	// 振る舞い更新後の個別処理
	AfterBehaviorUpdate();

	// ワールド変換の更新
	WorldTransformUpdate(worldTransform_);

	// ワールド変換更新後の個別処理
	AfterTransformUpdate();
}

void BaseCharacter::Draw() {
	Model::PreDraw();

	model_->Draw(worldTransform_, *camera_, &objectColor_);

	Model::PostDraw();
}

void BaseCharacter::StartHitFlash() {
	// タイマーを設定
	hitFlashTimer_ = kHitFlashFrames;

	// ヒット時の色を適用
	objectColor_.SetColor(kHitFlashColor);
}

void BaseCharacter::UpdateHitFlash() {
	// フラッシュ中でなければ何もしない
	if (hitFlashTimer_ <= 0) {
		return;
	}

	// タイマー更新
	--hitFlashTimer_;

	// フラッシュが終わったら通常色へ戻す
	if (hitFlashTimer_ <= 0) {
		objectColor_.SetColor(kNormalColor);
	}
}

void BaseCharacter::CheckFallDeath(float deathLineY) {
	// すでに死亡していたら何もしない
	if (isDead_) {
		return;
	}

	// デスラインより上なら何もしない
	if (GetWorldPosition().y >= deathLineY) {
		return;
	}

	// 落下死亡処理
	OnFallDeath();
}

AABB BaseCharacter::GetAABB() const {
	Vector3 worldPos = GetWorldPosition();
	AABB aabb{};
	aabb.min = {worldPos.x - width_ / 2.0f, worldPos.y - height_ / 2.0f, worldPos.z - width_ / 2.0f};
	aabb.max = {worldPos.x + width_ / 2.0f, worldPos.y + height_ / 2.0f, worldPos.z + width_ / 2.0f};

	return aabb;
}

KamataEngine::Vector3 BaseCharacter::GetWorldPosition() const {
	Vector3 worldPosition{};

	worldPosition.x = worldTransform_.matWorld_.m[3][0];
	worldPosition.y = worldTransform_.matWorld_.m[3][1];
	worldPosition.z = worldTransform_.matWorld_.m[3][2];

	return worldPosition;
}

void BaseCharacter::RequestBehaviorChange(std::unique_ptr<ICharactersBehavior> nextBehavior) {

	// 振る舞い変更を予約
	behaviorRequest_ = std::move(nextBehavior);
}

void BaseCharacter::ChangeBehavior() {
	// 変更リクエストがなければ何もしない
	if (!behaviorRequest_) {
		return;
	}

	// 新しい振る舞いを適用
	behavior_ = std::move(behaviorRequest_);

	// 振る舞いの初期化
	behavior_->Initialize();
}


void BaseCharacter::ResolveMapChipCollision(const Vector3& moveDistance) {
	// マップが未設定の場合は通常移動
	if (!mapChipField_) {
		Move(moveDistance);
		return;
	}

	// 衝突判定情報
	CollisionMapInfo collisionMapInfo{};

	// 移動量を設定
	collisionMapInfo.moveDistance = moveDistance;

	// 上下左右のマップ衝突判定
	MapChipCollision(collisionMapInfo);

	// 補正後の移動量で移動
	ApplyCollisionResult(collisionMapInfo);

	// 天井に接触したとき
	if (collisionMapInfo.isHitCeiling) {
		OnCollisionCeiling();
	}

	// 壁に接触したとき
	if (collisionMapInfo.isHitWall) {
		OnCollisionWall();
	}

	// プレイヤーなどの個別処理
	AfterMapChipCollision(collisionMapInfo);

	// 地面に接触したとき
	if (collisionMapInfo.onGround) {
		OnCollisionGround();
	}
}

void BaseCharacter::MapChipCollision(CollisionMapInfo& info) {
	MapChipCollisionTop(info);
	MapChipCollisionBottom(info);
	MapChipCollisionLeft(info);
	MapChipCollisionRight(info);
}

void BaseCharacter::MapChipCollisionTop(CollisionMapInfo& info) {
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

void BaseCharacter::MapChipCollisionBottom(CollisionMapInfo& info) {
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

	// 着地できる通常ブロックか
	bool isBlockGround = mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock;

	// はしご最上段に着地できるか
	bool isLadderGround = CanLandOnLadder() && mapChipField_->IsLadderTop(indexSet.xIndex, indexSet.yIndex);

	if (isBlockGround || isLadderGround) {
		isHit = true;
		hitIndexSet = indexSet;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	// 着地できる通常ブロックか
	isBlockGround = mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock;

	// はしご最上段に着地できるか
	isLadderGround = CanLandOnLadder() && mapChipField_->IsLadderTop(indexSet.xIndex, indexSet.yIndex);

	if (isBlockGround || isLadderGround) {
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

void BaseCharacter::MapChipCollisionLeft(CollisionMapInfo& info) {
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

void BaseCharacter::MapChipCollisionRight(CollisionMapInfo& info) {
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

void BaseCharacter::ApplyCollisionResult(const CollisionMapInfo& info) { worldTransform_.translation_ += info.moveDistance; }

Vector3 BaseCharacter::CornerPosition(const Vector3& center, Corner corner) {
	Vector3 offsetTable[static_cast<uint32_t>(Corner::kNumCorner)] = {
	    Vector3(width_ / 2.0f, -height_ / 2.0f, 0.0f),
		Vector3(-width_ / 2.0f, -height_ / 2.0f, 0.0f),
		Vector3(width_ / 2.0f, height_ / 2.0f, 0.0f),
		Vector3(-width_ / 2.0f, height_ / 2.0f, 0.0f)
	};

	return center + offsetTable[static_cast<uint32_t>(corner)];
}