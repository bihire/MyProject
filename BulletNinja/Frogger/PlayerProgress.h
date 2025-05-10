

#pragma once
#include <SFML/System.hpp>
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <mutex>
#include <iostream>
#include <sstream>
#include <iomanip> 

enum QuestState { NotStarted, InProgress, Completed };

struct KillRecord {
    std::string enemyType;
    std::string weaponUsed;
    sf::Time timestamp;
};

struct LevelStats {
    sf::Time m_levelStartTime{ sf::Time::Zero };
    sf::Time m_levelEndTime{ sf::Time::Zero };
    int score{ 0 };
    std::vector<KillRecord> kills;
    bool keysCollected{ false };
    QuestState completed{ QuestState::NotStarted };
    std::unordered_set<std::string> weaponsGained;

    int countKillsByType(const std::string& enemyType) const;
};

class PlayerProgress {
private:
    
    std::unordered_map<size_t, LevelStats> m_levelStats;
    std::unordered_set<std::string>     m_allWeapons;
    std::unordered_set<std::string>     m_playerArsenal;
    size_t                              m_currentLevel{ 0 };
    size_t                              m_lastLevel{ 0 };
    size_t                              m_lives{ 0 };
    std::atomic<bool>                   m_paused{ false };
    std::atomic<bool>                   m_isInGameMenu{ true };
    std::vector<std::function<void()>>  m_statusListeners;
    std::function<void()>               m_resetLevelCallBack;
    std::function<void()>               m_resetGameCallBack;

    PlayerProgress();


    void                        setLevelStatus(QuestState status);
    void			            notifyStatusChanged();

public:
    // Singleton control
    PlayerProgress(const PlayerProgress&) = delete;
    PlayerProgress& operator=(const PlayerProgress&) = delete;
    static PlayerProgress&      getInstance();

    //getters
    size_t			            getCurrentLevel() const;
    size_t			            getLives() const;
    bool			            isInGameMenu() const;

    //listeners
    void				        addStatusListener(std::function<void()> listener);

    // Weapon system
    void registerWeapon(const std::string& weapon);
    void unlockWeapon(const std::string& weapon);
    std::vector<std::string>    getAllWeapons() const;
    std::vector<std::string>    getPlayerWeapons() const;
    std::vector<std::string>    getWeaponsGained() const;

    // Timing controls
    void                        setLevelStartTime(sf::Time time);
    void                        setLevelEndTime(sf::Time time);
    sf::Time                    getLevelDuration();
    std::string                 getLevelFormatTime();

    // Pause controls
    void                        setPaused(bool paused) { m_paused = paused; }
    bool                        isPaused() const { return m_paused.load(std::memory_order_relaxed); }

    // Level progress
    void                        recordKill(const std::string& enemyType, const std::string& weaponUsed, sf::Time);
    void                        addScore(int points);
    LevelStats                  getCurrentLevelStats() const;

    // Utility
    void                        resetAllProgress();
    void                        resetCurrentLevel();
    void                        setResetCurrentLevelCallBack(std::function<void()> listener);
    void                        setResetAllProgressCallBack(std::function<void()> listener);

    // Level Status
    void                        startLevel(sf::Time time);
    void                        endLevel(sf::Time time);
    QuestState                  getLevelStatus() const;

    //setters
    void                        setLives(size_t l);
    void                        setIsInGameMenu() { m_isInGameMenu =!m_isInGameMenu; }
    void                        incrementLastLevel();
    bool                        isLastLevel();
    bool						isKeysCollected();
    void						setKeysCollected(bool collected);

    // Score
    void                        setCurrentLevelScore(int score);
    int                         getCurrentLevelScore();
    int                         getTotalScore();
};