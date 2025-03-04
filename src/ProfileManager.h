#pragma once
#include "Profile.h"
#include <vector>
#include <memory>

class ProfileManager {
public:
    ProfileManager();
    
    // Profile management
    bool addProfile(const std::string& name = "Investigator");
    bool removeProfile(size_t index);
    size_t getProfileCount() const { return m_profiles.size(); }
    static constexpr size_t MAX_PROFILES = 4;
    
    // Profile access
    Profile* getProfile(size_t index);
    const Profile* getProfile(size_t index) const;
    
private:
    std::vector<std::unique_ptr<Profile>> m_profiles;
};
