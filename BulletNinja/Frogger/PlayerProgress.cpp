#include "PlayerProgress.h"

int LevelStats::countKillsByType(const std::string& enemyType) const {
    return std::count_if(kills.begin(), kills.end(),
        [&](const auto& kill) { return kill.enemyType == enemyType; });
}


PlayerProgress::PlayerProgress() {
    m_levelStats[0] = LevelStats{};
    m_allWeapons = { "Katana", "Shuriken" };
    m_playerArsenal = { "Katana" };
}

PlayerProgress& PlayerProgress::getInstance() {
    static PlayerProgress instance;
    return instance;
}

void PlayerProgress::registerWeapon(const std::string& weapon) {
    m_allWeapons.insert(weapon);
}

void PlayerProgress::unlockWeapon(const std::string& weapon) {
    m_levelStats[m_currentLevel].weaponsGained.insert(weapon);
}

std::vector<std::string> PlayerProgress::getAllWeapons() const {
    return { m_allWeapons.begin(), m_allWeapons.end() };
}

std::vector<std::string> PlayerProgress::getPlayerWeapons() const {
    return { m_playerArsenal.begin(), m_playerArsenal.end() };
}

std::vector<std::string> PlayerProgress::getWeaponsGained() const {
    
    if (auto it = m_levelStats.find(m_currentLevel); it != m_levelStats.end()) {
        return { it->second.weaponsGained.begin(), it->second.weaponsGained.end() };
    }
    return {};
}

void PlayerProgress::recordKill(const std::string& enemyType, const std::string& weaponUsed, sf::Time time) {
    
    auto& stats = m_levelStats[m_currentLevel];
    stats.kills.push_back({ enemyType, weaponUsed, time });
}

void PlayerProgress::addScore( int points) {
   
    m_levelStats[m_currentLevel].score += points;
}

QuestState PlayerProgress::getLevelStatus() const {
	
	return m_levelStats.at(m_currentLevel).completed;
}

void PlayerProgress::startLevel(sf::Time time) {
	
    setLevelStatus(QuestState::InProgress);
	m_levelStats[m_currentLevel].m_levelStartTime = time;
    notifyStatusChanged();
}

void PlayerProgress::endLevel(sf::Time time) {
	
    setLevelStatus(QuestState::Completed);
    m_levelStats[m_currentLevel].m_levelEndTime = time;
    m_currentLevel++;
    m_levelStats[m_currentLevel] = LevelStats{};
    notifyStatusChanged();
}

void PlayerProgress::setLevelStatus(QuestState status) {
    
    m_levelStats[m_currentLevel].completed = status;
}

LevelStats PlayerProgress::getCurrentLevelStats() const {
    
    return m_levelStats.count(m_currentLevel) ? m_levelStats.at(m_currentLevel) : LevelStats{};
}

void PlayerProgress::resetAllProgress() {
   
    m_levelStats.clear();
    m_playerArsenal = { "Katana" };
}

void PlayerProgress::resetCurrentLevel() {
    

    //auto weaponsGained = m_levelStats[m_currentLevel].weaponsGained;
    m_levelStats[m_currentLevel] = LevelStats{};
    //m_levelStats[m_currentLevel].weaponsGained = weaponsGained;

    m_resetLevelCallBack(); // respawn/reset in the game scene
    setPaused(false);
    notifyStatusChanged();
    

}

void PlayerProgress::setLevelStartTime(sf::Time time) {
    m_levelStats[m_currentLevel].m_levelStartTime = time;
} 

void PlayerProgress::setLevelEndTime(sf::Time time) {
    m_levelStats[m_currentLevel].m_levelEndTime = time;
}

sf::Time PlayerProgress::getLevelDuration() {
    sf::Time levelDuration = m_levelStats[m_currentLevel].m_levelEndTime - m_levelStats[m_currentLevel].m_levelStartTime;
    return levelDuration;
}

std::string PlayerProgress::getLevelFormatTime() {
    sf::Time duration = getLevelDuration();
    int totalSeconds = static_cast<int>(duration.asSeconds());
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds;

    return oss.str();
}

void PlayerProgress::notifyStatusChanged() {
    std::cout << "Adding listener (total: " << m_statusListeners.size() << ")\n";
    for (auto& listener : m_statusListeners) {
        listener();
    }
}

void PlayerProgress::addStatusListener(std::function<void()> listener) {
    m_statusListeners.push_back(listener);
    std::cout << "Adding listener (total: " << m_statusListeners.size() << ")\n";
}

size_t	PlayerProgress::getCurrentLevel() const {
    return m_currentLevel; 
}

void PlayerProgress::setLives(size_t level) {
	m_lives = level;
}

void PlayerProgress::incrementLastLevel()
{
	m_lastLevel++;
}

bool PlayerProgress::isLastLevel()
{
    if (m_currentLevel == m_lastLevel) {
		return true;
	}
	else {
		return false;
	}
}

bool PlayerProgress::isKeysCollected() 
{
    	return m_levelStats[m_currentLevel].keysCollected;
    
}

void PlayerProgress::setKeysCollected(bool collected)
{
	m_levelStats[m_currentLevel].keysCollected = collected;
}

int PlayerProgress::getCurrentLevelScore()
{
	return m_levelStats[m_currentLevel].score;
}

int PlayerProgress::getTotalScore()
{
	int totalScore = 0;
	for (const auto& levelStat : m_levelStats) {
		totalScore += levelStat.second.score;
	}
	return totalScore;

}
void PlayerProgress::setCurrentLevelScore(int score)
{
    m_levelStats[m_currentLevel].score += score;
}

void PlayerProgress::setResetCurrentLevelCallBack(std::function<void()> listener) {
    
    m_resetLevelCallBack = listener;
}

void PlayerProgress::setResetAllProgressCallBack(std::function<void()> listener) {
    m_resetGameCallBack = listener;
}

size_t	 PlayerProgress::getLives() const { return m_lives; }

bool PlayerProgress::isInGameMenu() const { return m_isInGameMenu; }