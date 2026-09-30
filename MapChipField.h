#pragma once
#include<KamataEngine.h>
#include "Vector3Util.h"
#include"Rect.h"

// 1ブロックのサイズ
static inline const float kBlockWidth = 1.0f;
static inline const float kBlockHeight = 1.0f;

// ブロックの個数
static inline const uint32_t kNumBlockVertical = 20;
static inline const uint32_t kNumBlockHorizontal = 100;

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
	kPlayer,
	kEnemy
};

// マップチップCSVの文字番号
enum MAP_CHIP_CHAR_INDEX {
	kChipType = 0,   // マップチップタイプ
	kChipSubID = 1   // タイプごとのサブID
};

struct MapChipDataUnit {
	MapChipType type = MapChipType::kBlank; // マップチップの種別
	uint8_t subID = 0;						// 種類ごとのサブID
};

struct MapChipData {
	std::vector<std::vector<MapChipDataUnit>> data;
};

struct IndexSet {
	uint32_t xIndex;
	uint32_t yIndex;
};

class MapChipField {
  public:
	void LoadMapChipCsv(const std::string& filePath);
	void ResetMapChipData();
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);
	uint8_t GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex);

	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) {
		return KamataEngine::Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVertical - 1 - yIndex), 0.0f);
	}

	uint32_t GetNumBlockVertical();
	uint32_t GetNumBlockHorizontal();
	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);
	Rect RectByIndex(uint32_t xIndex, uint32_t yIndex);

  private:
	MapChipData mapChipData_;
};
