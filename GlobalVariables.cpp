#include "GlobalVariables.h"
#include <json.hpp>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <format>
#include <iomanip>

using namespace KamataEngine;
using json = nlohmann::json;


GlobalVariables *GlobalVariables::GetInstance() {
	static GlobalVariables instance;
	return &instance;
}

void GlobalVariables::Update() {
	if (!ImGui::Begin("GlobalVariables", nullptr, ImGuiWindowFlags_MenuBar)) {
		ImGui::End();
		return;
	}

	//ImGui::SetNextWindowSize(ImVec2(500, 500));

	if (!ImGui::BeginMenuBar()) {
		ImGui::End();
		return;
	}

	// 各グループについて
	for (std::map<std::string, Group>::iterator itGroup = datas_.begin(); itGroup != datas_.end(); ++itGroup) {
		// グループ名を取得
		const std::string& groupName = itGroup->first;

		// グループの参照を取得
		Group& group = itGroup->second;

		if (!ImGui::BeginMenu(groupName.c_str())) {
			continue;
		}

		// 各項目について
		for (std::map<std::string, Item>::iterator itItem = group.items.begin(); itItem != group.items.end(); ++itItem) {
			// 項目名を取得
			const std::string& itemName = itItem->first;

			// 項目の参照を取得
			Item& item = itItem->second;

			// int32_t型の値を保持していれば
			if (std::holds_alternative<int32_t>(item.value)) {
				int32_t* ptr = std::get_if<int32_t>(&item.value);
				ImGui::SliderInt(itemName.c_str(), ptr, 0, 100);

				// float型の値を保持していれば
			} else if (std::holds_alternative<float>(item.value)){
				float* ptr = std::get_if<float>(&item.value);
				ImGui::SliderFloat(itemName.c_str(), ptr, 0, 100);

				// Vector3型の値を保持していれば
			} else if (std::holds_alternative<Vector3>(item.value)) {
				Vector3* ptr = std::get_if<Vector3>(&item.value);
				ImGui::SliderFloat3(itemName.c_str(), reinterpret_cast<float*>(ptr), 0, 100);
			}
		}

		// 改行
		ImGui::Text("\n");

		if (ImGui::Button("Save")) {
			SaveFile(groupName);
			std::string message = std::format("{}.json saved.", groupName);
			MessageBoxA(nullptr, message.c_str(), "GlobalVariables", 0);
		}


		ImGui::EndMenu();
	}

	ImGui::EndMenuBar();
	ImGui::End();
}

void GlobalVariables::CreateGroup(const std::string& groupName) {
	// 指定名のオブジェクトがなければ追加する
	datas_[groupName];
}

void GlobalVariables::SetValue(const std::string& groupName, const std::string& key, int32_t value) {

	// グループの参照を取得
	Group& group = datas_[groupName];

	// 新しい項目のデータを設定
	Item newItem{};
	newItem.value = value;

	// 設定した項目をstd::mapに追加
	group.items[key] = newItem;
}

void GlobalVariables::SetValue(const std::string& groupName, const std::string& key, float value) {

	// グループの参照を取得
	Group& group = datas_[groupName];

	// 新しい項目のデータを設定
	Item newItem{};
	newItem.value = value;

	// 設定した項目をstd::mapに追加
	group.items[key] = newItem;
}

void GlobalVariables::SetValue(const std::string& groupName, const std::string& key, const Vector3 value) {

	// グループの参照を取得
	Group& group = datas_[groupName];

	// 新しい項目のデータを設定
	Item newItem{};
	newItem.value = value;

	// 設定した項目をstd::mapに追加
	group.items[key] = newItem;
}

void GlobalVariables::SaveFile(const std::string& groupName) {
	// グループを検索
	std::map<std::string, Group>::iterator itGroup = datas_.find(groupName);

	// 未登録チェック
	assert(itGroup != datas_.end());

	json root;

	root = json::object();

	root[groupName] = json::object();


	// 各項目について
	for (std::map<std::string, Item>::iterator itItem = itGroup->second.items.begin(); itItem != itGroup->second.items.end(); ++itItem) {
		// 項目名を取得
		const std::string& itemName = itItem->first;

		// 項目の参照を取得
		Item& item = itItem->second;

		// int32_t型の値を保持していれば
		if (std::holds_alternative<int32_t>(item.value)) {
			// 値を登録
			root[groupName][itemName] = std::get<int32_t>(item.value);

			// float型の値を保持していれば
		} else if (std::holds_alternative<float>(item.value)) {
			// 値を登録
			root[groupName][itemName] = std::get<float>(item.value);

			// Vector3型の値を保持していれば
		} else if (std::holds_alternative<Vector3>(item.value)) {
			// float型のjson配列登録
			Vector3 value = std::get<Vector3>(item.value);
			root[groupName][itemName] = json::array({value.x, value.y, value.z});
		}
	}

	// ディレクトリがなければ作成する
	std::filesystem::path dir(kDirectoryPath);

	if (!std::filesystem::exists(kDirectoryPath)) {
		std::filesystem::create_directories(kDirectoryPath);
	}

	// 書き込むJSONファイルのフルパスを合成する
	std::string filePath = kDirectoryPath + groupName + ".json";

	// 書き込み用ファイルストリーム
	std::ofstream ofs;

	// ファイルを書き込み用に開く
	ofs.open(filePath);

	// ファイルオープン失敗？
	if (ofs.fail()) {
		std::string message = "Failed open data file for write.";
		MessageBoxA(nullptr, message.c_str(), "GlobalVariables", 0);
		assert(0);
		return;
	}

	// ファイルにjson文字列を書き込む(インデント幅4)
	ofs << std::setw(4) << root << std::endl;

	// ファイルを閉じる
	ofs.close();
}

void GlobalVariables::LoadFiles() {
	// 保存先ディレクトリのパス
	const std::filesystem::path directoryPath(kDirectoryPath);

	// ディレクトリがなければ何もしない
	if (!std::filesystem::exists(directoryPath)) {
		return;
	}

	// ディレクトリ内のファイルを走査
	std::filesystem::directory_iterator directoryIterator(directoryPath);
	for (const std::filesystem::directory_entry& entry : directoryIterator) {
		// ファイルパスを取得
		const std::filesystem::path& filePath = entry.path();

		// 拡張子が.json以外ならスキップ
		if (filePath.extension().string() != ".json") {
			continue;
		}

		// 拡張子を除いたファイル名をグループ名として読み込む
		LoadFile(filePath.stem().string());
	}
}

void GlobalVariables::LoadFile(const std::string& groupName) {
	// 読み込むJSONファイルのフルパスを合成する
	std::string filePath = kDirectoryPath + groupName + ".json";

	// 読み込み用ファイルストリーム
	std::ifstream ifs;

	// ファイルを読み込み用に開く
	ifs.open(filePath);

	// ファイルオープン失敗？
	if (ifs.fail()) {
		std::string message = "Failed open data file for read.";
		MessageBoxA(nullptr, message.c_str(), "GlobalVariables", 0);
		assert(0);
		return;
	}

	json root;

	// json文字列からjsonのデータ構造に展開
	ifs >> root;

	// ファイルを閉じる
	ifs.close();

	// グループを検索
	json::iterator itGroup = root.find(groupName);

	// 未登録チェック
	assert(itGroup != root.end());

	// 各アイテムについて
	for (json::iterator itItem = itGroup->begin(); itItem != itGroup->end(); ++itItem) {
		// アイテム名を取得
		const std::string& itemName = itItem.key();

		// int32_t型の値を保持していれば
		if (itItem->is_number_integer()) {
			// int型の値を登録
			int32_t value = itItem->get<int32_t>();
			SetValue(groupName, itemName, value);
		}
		// float型の値を保持していれば
		else if (itItem->is_number_float()) {
			// JSONの小数はdoubleとして取得してfloatへ変換
			double value = itItem->get<double>();
			SetValue(groupName, itemName, static_cast<float>(value));
		}
		// Vector3型の値を保持していれば
		else if (itItem->is_array() && itItem->size() == 3) {
			// float型のjson配列をVector3に変換
			Vector3 value = {static_cast<float>(itItem->at(0).get<double>()), static_cast<float>(itItem->at(1).get<double>()), static_cast<float>(itItem->at(2).get<double>())};

			SetValue(groupName, itemName, value);
		}
	}
}

void GlobalVariables::AddItem(const std::string& groupName, const std::string& key, int32_t value) {
	// グループの参照を取得
	Group& group = datas_[groupName];

	// 既に項目があれば何もしない
	if (group.items.find(key) != group.items.end()) {
		return;
	}

	// 項目を追加
	SetValue(groupName, key, value);
}

void GlobalVariables::AddItem(const std::string& groupName, const std::string& key, float value) {
	// グループの参照を取得
	Group& group = datas_[groupName];

	// 既に項目があれば何もしない
	if (group.items.find(key) != group.items.end()) {
		return;
	}

	// 項目を追加
	SetValue(groupName, key, value);
}

void GlobalVariables::AddItem(const std::string& groupName, const std::string& key, const Vector3& value) {
	// グループの参照を取得
	Group& group = datas_[groupName];

	// 既に項目があれば何もしない
	if (group.items.find(key) != group.items.end()) {
		return;
	}

	// 項目を追加
	SetValue(groupName, key, value);
}

int32_t GlobalVariables::GetIntValue(const std::string& groupName, const std::string& key) const {
	// グループが存在するか確認
	std::map<std::string, Group>::const_iterator itGroup = datas_.find(groupName);
	assert(itGroup != datas_.end());

	// グループの参照を取得
	const Group& group = itGroup->second;

	// キーが存在するか確認
	std::map<std::string, Item>::const_iterator itItem = group.items.find(key);
	assert(itItem != group.items.end());

	// int32_t型として取得
	return std::get<int32_t>(itItem->second.value);
}

float GlobalVariables::GetFloatValue(const std::string& groupName, const std::string& key) const {
	// グループが存在するか確認
	std::map<std::string, Group>::const_iterator itGroup = datas_.find(groupName);
	assert(itGroup != datas_.end());

	// グループの参照を取得
	const Group& group = itGroup->second;

	// キーが存在するか確認
	std::map<std::string, Item>::const_iterator itItem = group.items.find(key);
	assert(itItem != group.items.end());

	// float型として取得
	return std::get<float>(itItem->second.value);
}

Vector3 GlobalVariables::GetVector3Value(const std::string& groupName, const std::string& key) const {
	// グループが存在するか確認
	std::map<std::string, Group>::const_iterator itGroup = datas_.find(groupName);
	assert(itGroup != datas_.end());

	// グループの参照を取得
	const Group& group = itGroup->second;

	// キーが存在するか確認
	std::map<std::string, Item>::const_iterator itItem = group.items.find(key);
	assert(itItem != group.items.end());

	// Vector3型として取得
	return std::get<Vector3>(itItem->second.value);
}