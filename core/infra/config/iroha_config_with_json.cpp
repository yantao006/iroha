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