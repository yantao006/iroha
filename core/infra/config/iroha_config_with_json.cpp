#include "iroha_config_with_json.hpp"

using IrohaConfigManager = config::IrohaConfigManager;

IrohaConfigManager::IrohaConfigManager(){}

IrohaConfigManager& IrohaConfigManager::getInstance() {
    static IrohaConfigManager instance;
    return instance;
}

std::string IrohaConfigManager::getConfigName() {
  return "config/config.json";
}

std::string IrohaConfigManager::getDatabasePath(const std::string& defaultValue) {
  return getParam("database_path", defaultValue);
}

size_t IrohaConfigManager::getConcurrency(size_t defaultValue) {
  return getParam("concurrency", defaultValue);
}

size_t IrohaConfigManager::getMaxFaultyPeers(size_t defaultValue) {
  return getParam("maxFaultyPeers", defaultValue);
}

size_t IrohaConfigManager::getPoolWorkerQueueSize(size_t defaultValue) {
  return getParam("pool_worker_queue_size", defaultValue);
}
