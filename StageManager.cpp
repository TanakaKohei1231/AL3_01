#include "StageManager.h"
#include <fstream>
#include <sstream>

void StageManager::LoadStageDataCsv() {
	// ステージデータファイルのパス
	const std::string filePath = "fieldsData/stageDatas.csv";

	// ifstreamでステージデータファイルを開く
	std::ifstream stageDataFile(filePath);
	assert(stageDataFile.is_open() && "ステージデータファイルが存在しません");

	// ファイルの内容を格納するstringstream
	std::stringstream stageDataCsv;

	// ファイルの内容をstringstreamにコピーする
	stageDataCsv << stageDataFile.rdbuf();

	// ファイルを閉じる
	stageDataFile.close();

	// 再読み込みに備えて一度クリア
	stageDatas_.clear();

	// ステージデータを最終行まで1行ずつ読み込む
	std::string line;
	while (std::getline(stageDataCsv, line)) {
		// 空行ならスキップ
		if (line.empty()) {
			continue;
		}

		// 1行分の内容を解析する
		std::istringstream lineStream(line);

		// ステージデータを格納する構造体
		StageData stageData{};

		// カンマ区切りのデータ
		std::string stageName;
		std::string timeLimitString;

		// ステージ名を取得
		std::getline(lineStream, stageName, ',');

		// 制限時間を取得
		std::getline(lineStream, timeLimitString, ',');

		// データが足りない行はスキップ
		if (stageName.empty() || timeLimitString.empty()) {
			continue;
		}

		// ステージデータに格納
		stageData.name = stageName;
		stageData.timeLimit = std::stoi(timeLimitString);

		// ステージデータテーブルに追加
		stageDatas_.push_back(stageData);
	}

	assert(!stageDatas_.empty() && "Stage data is empty.");
}

void StageManager::SetCurrentStageIndexByName(const std::string& name) {
	for (size_t index = 0; index < stageDatas_.size(); ++index) {
		if (stageDatas_[index].name == name) {
			currentStageIndex_ = static_cast<int32_t>(index);
			return;
		}
	}

	assert(false && "指定されたステージ名は存在しません");

	// Releaseビルドでassertが無効な場合の保険
	currentStageIndex_ = 0;
}
