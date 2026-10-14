#pragma once
#include<KamataEngine.h>

class ModelManager {
  public:
	static ModelManager* GetInstance();

	KamataEngine::Model* Load(const std::string& modelName, bool smoothing = false);

	void Finalize();

  private:
	std::unordered_map<std::string, std::unique_ptr<KamataEngine::Model>> models_;

	// シングルトンパターンにする
	ModelManager() = default;
	~ModelManager() = default;
	ModelManager(const ModelManager&) = delete;
	ModelManager& operator=(const ModelManager&) = delete;
};
