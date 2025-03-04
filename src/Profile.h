#pragma once
#include <string>

class Profile {
public:
    Profile(const std::string& name = "Investigator", int maxActions = 3);
    
    // Getters
    const std::string& getName() const { return m_name; }
    int getMaxActions() const { return m_maxActions; }
    int getCurrentActions() const { return m_currentActions; }
    int getDamage() const { return m_damage; }
    int getHorror() const { return m_horror; }
    int getClues() const { return m_clues; }
    int getResources() const { return m_resources; }
    
    // Setters
    void setName(const std::string& name) { m_name = name; }
    void setMaxActions(int max);
    void setCurrentActions(int current);
    void resetActions() { m_currentActions = m_maxActions; }
    
    // Modifiers
    void incrementActions();
    void decrementActions();
    void incrementDamage() { m_damage++; }
    void decrementDamage() { if (m_damage > 0) m_damage--; }
    void incrementHorror() { m_horror++; }
    void decrementHorror() { if (m_horror > 0) m_horror--; }
    void incrementClues() { m_clues++; }
    void decrementClues() { if (m_clues > 0) m_clues--; }
    void incrementResources() { m_resources++; }
    void decrementResources() { if (m_resources > 0) m_resources--; }
    
private:
    std::string m_name;
    int m_maxActions;
    int m_currentActions;
    int m_damage;
    int m_horror;
    int m_clues;
    int m_resources;
};
