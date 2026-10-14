#include "ModelManager.h"

using namespace KamataEngine;

ModelManager* ModelManager::GetInstance() {
	static ModelManager instance;
	return &instance;
}

Model* ModelManager::Load(const std::string& modelName, bool smoothing) {

	// 既に読み込まれているモデルを探す
	auto iterator = models_.find(modelName);

	// 登録済みならそのモデルを返す
	if (iterator != models_.end()) {
		return iterator->second.get();
	}

	// モデルを新しく読み込む
	std::unique_ptr<Model> model(Model::CreateFromOBJ(modelName, smoothing));

	// 読み込みに失敗した場合
	if (model == nullptr) {
		return nullptr;
	}

	// モデルを管理対象として登録
	Model* modelPointer = model.get();
	models_.emplace(modelName, std::move(model));

	return modelPointer;
}

void ModelManager::Finalize() { models_.clear(); }