#pragma once
#include<iostream>
#include <variant>
#include <string>
#include <map>
#include <cstdint>
#include "KamataEngine.h"

// 項目
struct Item {
	std::variant<int32_t, float, KamataEngine::Vector3> value;
};

struct Group {
	std::map<std::string, Item> items;
};

class GlobalVariables final{
  public:
	static GlobalVariables* GetInstance();

	void Update();

	/// <summary>
	/// グループの作成
	/// </summary>
	/// <param name="groupName">グループ名</param>
	void CreateGroup(const std::string& groupName);

	// 値のセット
	void SetValue(const std::string& groupName, const std::string& key, int32_t value);
	void SetValue(const std::string& groupName, const std::string& key, float value);
	void SetValue(const std::string& groupName, const std::string& key, const KamataEngine::Vector3 value);

	/// <summary>
	/// ファイルに書き出し
	/// </summary>
	/// <param name="groupName">グループ</param>
	void SaveFile(const std::string& groupName);


	/// <summary>
	/// ディレクトリの全ファイル読み込み
	/// </summary>
	void LoadFiles();

	/// <summary>
	/// ファイルから読み込む
	/// </summary>
	/// <param name="groupName">グループ</param>
	void LoadFile(const std::string& groupName);

	// 項目の追加
	void AddItem(const std::string& groupName, const std::string& key, int32_t value);
	void AddItem(const std::string& groupName, const std::string& key, float value);
	void AddItem(const std::string& groupName, const std::string& key, const KamataEngine::Vector3& value);

	// 値の取得
	int32_t GetIntValue(const std::string& groupName, const std::string& key) const;
	float GetFloatValue(const std::string& groupName, const std::string& key) const;
	KamataEngine::Vector3 GetVector3Value(const std::string& groupName, const std::string& key) const;


  private:
	// 全データ
	std::map<std::string, Group> datas_;

	GlobalVariables() = default;
	~GlobalVariables() = default;
	GlobalVariables(const GlobalVariables& obj) = delete;
	GlobalVariables& operator=(const GlobalVariables& obj) = delete;

	const std::string kDirectoryPath = "GlobalVariables/";
};
