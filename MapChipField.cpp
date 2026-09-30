#include "MapChipField.h"
#include <map>
#include <fstream>
#include <sstream>

using namespace KamataEngine;

namespace {

	std::map<char, MapChipType> MapChipTypeTable = {
		{'B', MapChipType::kBlock},
		{'P', MapChipType::kPlayer},
		{'E', MapChipType::kEnemy},
	};

} // namespace

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	// マップチップデータをリセット
	ResetMapChipData();

	// ファイルを開く
	std::ifstream file;
	file.open(filePath);

	#ifdef _DEBUG

	assert(file.is_open());

	#endif // _DEBUG


	// マップチップCSV
	std::stringstream mapChipCsv;

	// ファイルの内容を文字列ストリームにコピー
	mapChipCsv << file.rdbuf();

	// ファイルを閉じる
	file.close();

	// CSVからマップチップデータを読み込む
	for (uint32_t i = 0; i < kNumBlockVertical; ++i) {
		std::string line;
		getline(mapChipCsv, line);

		// 1行分の文字列をストリームに変換して解析しやすくする
		std::istringstream lineStream(line);

		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			std::string word;
			std::getline(lineStream, word, ','); // カンマ区切りで1単語読み込む

			// 空白の場合はスキップ
			if (word.empty()) {
				continue;
			}

			// 先頭文字がいずれかのマップチップ種別に該当するか確認
			if (!MapChipTypeTable.contains(word[kChipType])) {
				continue;
			}

			// 先頭文字でマップチップのタイプを判別
			mapChipData_.data[i][j].type = MapChipTypeTable[word[kChipType]];

			// サブIDを含まない場合はスキップ
			if (word.size() <= kChipSubID) {
				continue;
			}

			// マップチップのサブIDを設定
			mapChipData_.data[i][j].subID = static_cast<uint8_t>(word[kChipSubID] - '0');
		}
	}
}

void MapChipField::ResetMapChipData() {
	// マップチップデータをリセット
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVertical);

	for (std::vector<MapChipDataUnit>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}

	if (kNumBlockVertical - 1 < yIndex) {
		return MapChipType::kBlank;
	}

	return mapChipData_.data[yIndex][xIndex].type;
}

uint8_t MapChipField::GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex) {
	if (kNumBlockHorizontal - 1 < xIndex) {
		return 0;
	}

	if (kNumBlockVertical - 1 < yIndex) {
		return 0;
	}

	return mapChipData_.data[yIndex][xIndex].subID;
}


uint32_t MapChipField::GetNumBlockVertical() { return kNumBlockVertical; }
uint32_t MapChipField::GetNumBlockHorizontal() { return kNumBlockHorizontal; }

IndexSet MapChipField::GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position) {
	Vector3 offset{kBlockWidth / 2.0f, kBlockHeight / 2.0f, 0.0f};
	Vector3 adjustedPosition = position + offset;

	IndexSet indexSet;
	indexSet.xIndex = static_cast<uint32_t>(adjustedPosition.x / kBlockWidth);
	indexSet.yIndex = kNumBlockVertical - 1 - static_cast<uint32_t>(adjustedPosition.y / kBlockHeight);

	return indexSet;
}

Rect MapChipField::RectByIndex(uint32_t xIndex, uint32_t yIndex) {
	// 指定ブロックの中心座標を取得する
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rect rect;
	rect.left = center.x - kBlockWidth / 2.0f;
	rect.right = center.x + kBlockWidth / 2.0f;
	rect.bottom = center.y - kBlockHeight / 2.0f;
	rect.top = center.y + kBlockHeight / 2.0f;
	return rect;
}
