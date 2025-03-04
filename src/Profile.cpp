#include "Profile.h"
#include <algorithm>

Profile::Profile(const std::string& name, int maxActions)
    : m_name(name), 
      m_maxActions(std::max(1, maxActions)),
      m_currentActions(maxActions),
      m_damage(0),
      m_horror(0),
      m_clues(0),
      m_resources(0)
{
}

void Profile::setMaxActions(int max) {
    m_maxActions = std::max(1, max);
    m_currentActions = std::min(m_currentActions, m_maxActions);
}

void Profile::setCurrentActions(int current) {
    m_currentActions = std::clamp(current, 0, m_maxActions);
}

void Profile::incrementActions() {
    if (m_currentActions < m_maxActions) {
        m_currentActions++;
    }
}

void Profile::decrementActions() {
    if (m_currentActions > 0) {
        m_currentActions--;
    }
}
