#pragma once
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

struct StageData {
	std::string name;   // ステージ名(フィールドCSVファイル名)
	int32_t timeLimit;  // 制限時間(秒)
};

/// <summary>
/// ステージ管理
/// </summary>
class StageManager {
  public:
	/// <summary>
	/// ステージデータファイルの読み込み
	/// </summary>
	void LoadStageDataCsv();

	/// <summary>
	/// ステージデータの取得
	/// </summary>
	/// <param name="index">ステージ番号</param>
	/// <returns>ステージデータ</returns>
	const StageData& GetStageData(int32_t index) const {
		assert(0 <= index);
		assert(index < static_cast<int32_t>(stageDatas_.size()));

		return stageDatas_[index];
	}

	/// <summary>
	/// 現在ステージのステージデータ取得
	/// </summary>
	/// <returns>現在ステージのステージデータ</returns>
	const StageData& GetCurrentStageData() const { return GetStageData(currentStageIndex_); }

	/// <summary>
	/// 現在ステージ番号を設定
	/// </summary>
	/// <param name="index">ステージ番号</param>
	void SetCurrentStageIndex(int32_t index) {
		assert(0 <= index);
		assert(index < static_cast<int32_t>(stageDatas_.size()));

		currentStageIndex_ = index;
	}

	/// <summary>
	/// ステージ名指定で現在ステージ番号を設定
	/// </summary>
	/// <param name="name">ステージ名</param>
	void SetCurrentStageIndexByName(const std::string& name);

	/// <summary>
	/// 現在ステージ番号を取得
	/// </summary>
	/// <returns>現在ステージ番号</returns>
	int32_t GetCurrentStageIndex() const { return currentStageIndex_; }
  private:
	// 全ステージデータ
	std::vector<StageData> stageDatas_;

	// 現在のステージ番号
	int32_t currentStageIndex_ = 0;
};
