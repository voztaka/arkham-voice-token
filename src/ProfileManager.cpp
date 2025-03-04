#include "ProfileManager.h"

ProfileManager::ProfileManager() {
    // 기본 프로필 하나 생성
    addProfile("Investigator 1");
}

bool ProfileManager::addProfile(const std::string& name) {
    if (m_profiles.size() >= MAX_PROFILES) {
        return false;
    }
    
    m_profiles.push_back(std::make_unique<Profile>(name));
    return true;
}

bool ProfileManager::removeProfile(size_t index) {
    if (index >= m_profiles.size() || m_profiles.size() <= 1) {
        return false; // 최소 하나의 프로필은 유지
    }
    
    m_profiles.erase(m_profiles.begin() + index);
    return true;
}

Profile* ProfileManager::getProfile(size_t index) {
    if (index >= m_profiles.size()) {
        return nullptr;
    }
    return m_profiles[index].get();
}

const Profile* ProfileManager::getProfile(size_t index) const {
    if (index >= m_profiles.size()) {
        return nullptr;
    }
    return m_profiles[index].get();
}
