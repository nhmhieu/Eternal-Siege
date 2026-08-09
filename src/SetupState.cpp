#include "SetupState.h"

#include "AllyConfig.h"
#include "AssetLocator.h"
#include "Constants.h"
#include "GameplayState.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {
constexpr sf::Color PANEL_COLOR{8, 18, 28, 244};
constexpr sf::Color CARD_COLOR{15, 31, 43, 245};
constexpr sf::Color GOLD{235, 191, 96};
constexpr sf::Color CYAN{90, 225, 215};
constexpr sf::Color GREEN{100, 235, 170};
constexpr sf::Color RED{235, 92, 100};

std::size_t allyIndex(AllyType type) {
    return static_cast<std::size_t>(type);
}

sf::Color allyColor(AllyType type) {
    switch (type) {
    case AllyType::Damian:     return {238, 192, 84};
    case AllyType::Evangeline: return {105, 205, 245};
    case AllyType::Junior:     return {187, 105, 245};
    case AllyType::Lucas:      return {235, 105, 63};
    }
    return CYAN;
}

void centerText(sf::Text& text, sf::Vector2f center) {
    const sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin({bounds.position.x + bounds.size.x * 0.5f,
                    bounds.position.y + bounds.size.y * 0.5f});
    text.setPosition(center);
}

sf::Vector2f fittedSize(const sf::IntRect& visible, float maximum) {
    if (visible.size.x <= 0 || visible.size.y <= 0) {
        return {maximum, maximum};
    }
    const float aspect = static_cast<float>(visible.size.x) /
                         static_cast<float>(visible.size.y);
    return aspect >= 1.f
        ? sf::Vector2f{maximum, maximum / aspect}
        : sf::Vector2f{maximum * aspect, maximum};
}
}

SetupState::SetupState(StateMachine& machine, sf::RenderWindow& gameWindow,
                       TextureManager& textures, AudioManager& audio,
                       GameProgress& gameProgress, LevelId levelId)
    : stateMachine(machine), window(gameWindow), map(15, 15),
      textureManager(textures), audioManager(audio), progress(gameProgress),
      selectedLevelId(levelId) {
    map.setTextureManager(textureManager);
}

void SetupState::loadResources() {
    const auto fontPath = AssetLocator::find("assets/fonts/Font.ttf");
    fontLoaded = fontPath && font.openFromFile(*fontPath);
    if (!fontLoaded) {
        std::cerr << "Setup UI font failed to load: assets/fonts/Font.ttf\n";
    }

    for (const AllyType type : AllyPlacementModel::ALLY_ORDER) {
        const std::size_t index = allyIndex(type);
        const std::string name = getAllyName(type);
        if (!textureManager.loadTexture(
                name, "assets/images/characters/" + name + ".png")) {
            std::cerr << "Setup portrait failed to load: " << name << '\n';
        }
        heroTextures[index] = textureManager.findTexture(name);
        heroVisibleBounds[index] = textureManager.getVisibleBounds(name);
    }
}

void SetupState::createTextUi() {
    titleText.reset();
    instructionText.reset();
    deployedText.reset();
    controlsText.reset();
    feedbackText.reset();
    startText.reset();
    startHintText.reset();
    for (HeroCardUi& card : heroCards) {
        card.name.reset();
        card.details.reset();
        card.status.reset();
    }
    if (!fontLoaded) return;

    titleText = std::make_unique<sf::Text>(font, "DEPLOY YOUR SQUAD", 25);
    titleText->setFillColor(GOLD);
    instructionText = std::make_unique<sf::Text>(
        font, "Choose a hero, then select a position.", 14);
    instructionText->setFillColor({164, 183, 191});
    deployedText = std::make_unique<sf::Text>(
        font, "0 / 4 HEROES DEPLOYED", 17);
    controlsText = std::make_unique<sf::Text>(
        font,
        "LEFT CLICK     Select / Deploy / Move\n"
        "RIGHT CLICK    Remove placed hero     R    Reset formation",
        12);
    controlsText->setFillColor({145, 163, 173});
    controlsText->setLineSpacing(1.25f);
    feedbackText = std::make_unique<sf::Text>(font, "", 13);
    startText = std::make_unique<sf::Text>(font, "START BATTLE", 18);
    startHintText = std::make_unique<sf::Text>(
        font, "Deploy all four heroes to continue.", 12);

    for (const AllyType type : AllyPlacementModel::ALLY_ORDER) {
        const std::size_t index = allyIndex(type);
        const AllyStats& stats = getAllyStats(type);
        const AllySkillConfig& skill = getAllySkillConfig(type);
        HeroCardUi& card = heroCards[index];
        card.name = std::make_unique<sf::Text>(
            font, getAllyName(type), 18);
        card.name->setFillColor(allyColor(type));
        const std::string details =
            std::string(getAllyRole(type)) + "  |  " +
            getWeaponName(stats.weaponType) + "\n" + skill.name +
            "  |  Range " + std::to_string(static_cast<int>(stats.attackRange));
        card.details = std::make_unique<sf::Text>(font, details, 12);
        card.details->setFillColor({184, 198, 205});
        card.details->setLineSpacing(1.15f);
        card.status = std::make_unique<sf::Text>(font, "READY", 11);
    }
}

void SetupState::onEnter() {
    placements.reset();
    selectedAlly = AllyType::Damian;
    inputArmed = false;
    transitionRequested = false;
    feedbackRemaining = 0.f;
    loadResources();
    createTextUi();
    updateLayout();
    refreshUi();
}

void SetupState::onExit() {
}

void SetupState::updateLayout() {
    const sf::View& view = window.getView();
    const sf::Vector2f viewSize = view.getSize();
    const sf::Vector2f origin = view.getCenter() - viewSize * 0.5f;
    const float mapWidth = static_cast<float>(map.getWidth()) *
                           GameConfig::TILE_SIZE;
    const float panelLeft = origin.x + mapWidth;
    const float panelWidth = std::max(320.f, origin.x + viewSize.x - panelLeft);

    sidePanel.setPosition({panelLeft, origin.y});
    sidePanel.setSize({panelWidth, viewSize.y});
    sidePanel.setFillColor(PANEL_COLOR);
    sidePanel.setOutlineColor({195, 147, 72, 170});
    sidePanel.setOutlineThickness(2.f);

    const float contentWidth = std::min(560.f, panelWidth - 36.f);
    const float contentLeft = panelLeft + (panelWidth - contentWidth) * 0.5f;
    const float top = origin.y;
    if (titleText) titleText->setPosition({contentLeft, top + 18.f});
    if (instructionText) {
        instructionText->setPosition({contentLeft, top + 51.f});
    }
    if (deployedText) deployedText->setPosition({contentLeft, top + 80.f});

    constexpr float cardHeight = 88.f;
    constexpr float cardGap = 7.f;
    const float cardsTop = top + 112.f;
    for (std::size_t index = 0; index < heroCards.size(); ++index) {
        HeroCardUi& card = heroCards[index];
        const float y = cardsTop + index * (cardHeight + cardGap);
        card.panel.setPosition({contentLeft, y});
        card.panel.setSize({contentWidth, cardHeight});
        card.panel.setOutlineThickness(2.f);

        const sf::Vector2f portraitSize = fittedSize(
            heroVisibleBounds[index], 62.f);
        card.portrait.setSize(portraitSize);
        card.portrait.setOrigin(portraitSize * 0.5f);
        card.portrait.setPosition({contentLeft + 40.f, y + cardHeight * 0.5f});
        card.portrait.setTexture(heroTextures[index], true);
        if (heroVisibleBounds[index].size.x > 0 &&
            heroVisibleBounds[index].size.y > 0) {
            card.portrait.setTextureRect(heroVisibleBounds[index]);
        }
        card.portrait.setFillColor(heroTextures[index]
            ? sf::Color::White : sf::Color(90, 130, 140));

        if (card.name) card.name->setPosition({contentLeft + 80.f, y + 8.f});
        if (card.details) {
            card.details->setPosition({contentLeft + 80.f, y + 34.f});
        }
        if (card.status) {
            card.status->setPosition({contentLeft + contentWidth - 67.f,
                                      y + 10.f});
        }
    }

    if (controlsText) controlsText->setPosition({contentLeft, top + 505.f});
    if (feedbackText) feedbackText->setPosition({contentLeft, top + 555.f});

    const float buttonWidth = std::min(310.f, contentWidth);
    const float buttonY = origin.y + viewSize.y - 67.f;
    startButton.setSize({buttonWidth, 50.f});
    startButton.setPosition({panelLeft + (panelWidth - buttonWidth) * 0.5f,
                             buttonY});
    startButton.setOutlineThickness(2.f);
    if (startText) {
        centerText(*startText,
            startButton.getPosition() + startButton.getSize() * 0.5f);
    }
    if (startHintText) {
        centerText(*startHintText,
            {panelLeft + panelWidth * 0.5f, buttonY - 17.f});
    }
}

void SetupState::refreshUi() {
    const bool complete = placements.isComplete();
    if (deployedText) {
        deployedText->setString(std::to_string(placements.deployedCount()) +
                                " / 4 HEROES DEPLOYED");
        deployedText->setFillColor(complete ? GREEN : sf::Color(185, 198, 204));
    }

    for (const AllyType type : AllyPlacementModel::ALLY_ORDER) {
        const std::size_t index = allyIndex(type);
        HeroCardUi& card = heroCards[index];
        const bool selected = selectedAlly && *selectedAlly == type;
        const bool placed = placements.placement(type).has_value();
        card.panel.setFillColor(selected
            ? sf::Color(25, 59, 68, 250) : CARD_COLOR);
        card.panel.setOutlineColor(selected
            ? CYAN : (placed ? sf::Color(63, 146, 104) : sf::Color(92, 112, 122)));
        if (card.status) {
            card.status->setString(placed ? "PLACED" : "READY");
            card.status->setFillColor(placed ? GREEN : sf::Color(155, 174, 184));
        }
    }

    startButton.setFillColor(complete
        ? sf::Color(36, 104, 91) : sf::Color(31, 42, 49));
    startButton.setOutlineColor(complete ? GOLD : sf::Color(102, 104, 100));
    if (startText) {
        startText->setFillColor(complete ? sf::Color::White
                                         : sf::Color(112, 121, 126));
    }
    if (startHintText) {
        startHintText->setString(complete
            ? "Formation ready." : "Deploy all four heroes to continue.");
        startHintText->setFillColor(complete ? GREEN
                                             : sf::Color(135, 148, 155));
    }
}

bool SetupState::isDeployableCell(sf::Vector2i cell) const {
    return map.isInside(cell) && map.isWalkable(cell) &&
           map.getTileType(cell.x, cell.y) != TILE_SPAWN;
}

std::optional<sf::Vector2i> SetupState::mouseCell(sf::Vector2i pixel) const {
    const sf::Vector2f world = window.mapPixelToCoords(pixel);
    if (!map.getWorldBounds().contains(world)) return std::nullopt;
    const sf::Vector2i cell = map.worldToGrid(world);
    return map.isInside(cell) ? std::optional<sf::Vector2i>(cell)
                              : std::nullopt;
}

void SetupState::selectAlly(AllyType type) {
    selectedAlly = type;
    audioManager.playSound("ui_click");
    refreshUi();
}

void SetupState::deploySelected(sf::Vector2i cell) {
    if (!selectedAlly) return;
    if (!isDeployableCell(cell)) {
        setFeedback("Allies cannot be deployed here.", RED);
        return;
    }

    const AllyPlacementModel::PlaceResult result =
        placements.place(*selectedAlly, cell);
    if (result == AllyPlacementModel::PlaceResult::Occupied) {
        setFeedback("That position is already occupied.", RED);
        return;
    }
    if (result == AllyPlacementModel::PlaceResult::AlreadyThere) return;

    audioManager.playSound("ui_click");
    const AllyType justPlaced = *selectedAlly;
    if (const auto next = placements.firstUnplaced()) {
        selectedAlly = *next;
    } else {
        selectedAlly = justPlaced;
    }
    setFeedback(result == AllyPlacementModel::PlaceResult::Moved
        ? "Hero moved to the new position."
        : "Hero deployed.", GREEN);
    refreshUi();
}

void SetupState::removeAt(sf::Vector2i cell) {
    const auto removed = placements.removeAt(cell);
    if (!removed) return;
    selectedAlly = *removed;
    audioManager.playSound("ui_click");
    setFeedback("Hero returned to the squad.", sf::Color(180, 205, 215));
    refreshUi();
}

void SetupState::resetFormation() {
    placements.reset();
    selectedAlly = AllyType::Damian;
    audioManager.playSound("ui_click");
    setFeedback("Formation reset.", sf::Color(180, 205, 215));
    refreshUi();
}

void SetupState::setFeedback(const char* message, sf::Color color) {
    feedbackRemaining = 2.f;
    if (feedbackText) {
        feedbackText->setString(message);
        feedbackText->setFillColor(color);
    }
}

void SetupState::startBattle() {
    if (transitionRequested) return;
    const auto ordered = placements.orderedPositions();
    if (!ordered) return;

    transitionRequested = true;
    audioManager.playSound("ui_click");
    const std::vector<sf::Vector2i> positions(ordered->begin(), ordered->end());
    stateMachine.changeState(std::make_unique<GameplayState>(
        stateMachine, window, textureManager, audioManager, progress,
        selectedLevelId, map, positions));
}

void SetupState::handleEvent(const sf::Event& event) {
    if (const auto* key = event.getIf<sf::Event::KeyPressed>()) {
        if (key->code == sf::Keyboard::Key::R) resetFormation();
        return;
    }

    const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>();
    if (!mousePress || !inputArmed || transitionRequested) return;
    const sf::Vector2f mouseWorld = window.mapPixelToCoords(mousePress->position);

    if (mousePress->button == sf::Mouse::Button::Left) {
        if (placements.isComplete() &&
            startButton.getGlobalBounds().contains(mouseWorld)) {
            startBattle();
            return;
        }
        for (const AllyType type : AllyPlacementModel::ALLY_ORDER) {
            if (heroCards[allyIndex(type)].panel.getGlobalBounds().contains(
                    mouseWorld)) {
                selectAlly(type);
                return;
            }
        }
        if (const auto cell = mouseCell(mousePress->position)) {
            deploySelected(*cell);
        }
    } else if (mousePress->button == sf::Mouse::Button::Right) {
        if (const auto cell = mouseCell(mousePress->position)) {
            removeAt(*cell);
        }
    }
}

void SetupState::update(float dt) {
    updateLayout();
    if (!inputArmed &&
        !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) &&
        !sf::Mouse::isButtonPressed(sf::Mouse::Button::Right)) {
        inputArmed = true;
    }
    if (feedbackRemaining > 0.f) {
        feedbackRemaining = std::max(0.f, feedbackRemaining - dt);
        if (feedbackRemaining == 0.f && feedbackText) {
            feedbackText->setString("");
        }
    }
}

void SetupState::drawPlacementHints(sf::RenderWindow& target) const {
    if (!selectedAlly) return;

    for (int y = 0; y < map.getHeight(); ++y) {
        for (int x = 0; x < map.getWidth(); ++x) {
            const sf::Vector2i cell{x, y};
            if (!isDeployableCell(cell) || placements.allyAt(cell)) continue;
            sf::RectangleShape hint({GameConfig::TILE_SIZE - 4.f,
                                     GameConfig::TILE_SIZE - 4.f});
            hint.setPosition({x * GameConfig::TILE_SIZE + 2.f,
                              y * GameConfig::TILE_SIZE + 2.f});
            hint.setFillColor({70, 220, 205, 13});
            target.draw(hint);
        }
    }

    const sf::Vector2i mousePixel = sf::Mouse::getPosition(window);
    const auto hovered = mouseCell(mousePixel);
    bool validHover = false;
    if (hovered) {
        const auto occupant = placements.allyAt(*hovered);
        validHover = isDeployableCell(*hovered) &&
                     (!occupant || *occupant == *selectedAlly);
        sf::RectangleShape hover({GameConfig::TILE_SIZE - 4.f,
                                  GameConfig::TILE_SIZE - 4.f});
        hover.setPosition({hovered->x * GameConfig::TILE_SIZE + 2.f,
                           hovered->y * GameConfig::TILE_SIZE + 2.f});
        hover.setFillColor(validHover ? sf::Color(70, 225, 210, 58)
                                      : sf::Color(235, 70, 80, 58));
        hover.setOutlineColor(validHover ? CYAN : RED);
        hover.setOutlineThickness(2.f);
        target.draw(hover);
    }

    std::optional<sf::Vector2f> previewCenter;
    if (hovered && validHover) {
        previewCenter = map.gridToWorld(*hovered);
    } else if (placements.placement(*selectedAlly)) {
        previewCenter = map.gridToWorld(*placements.placement(*selectedAlly));
    }
    if (!previewCenter) return;

    const float range = getAllyStats(*selectedAlly).attackRange;
    sf::CircleShape rangePreview(range);
    rangePreview.setOrigin({range, range});
    rangePreview.setPosition(*previewCenter);
    rangePreview.setFillColor({allyColor(*selectedAlly).r,
                               allyColor(*selectedAlly).g,
                               allyColor(*selectedAlly).b, 12});
    rangePreview.setOutlineColor({allyColor(*selectedAlly).r,
                                  allyColor(*selectedAlly).g,
                                  allyColor(*selectedAlly).b, 105});
    rangePreview.setOutlineThickness(2.f);
    target.draw(rangePreview);

    if (hovered && validHover) {
        const std::size_t index = allyIndex(*selectedAlly);
        sf::RectangleShape ghost;
        const sf::Vector2f size = fittedSize(heroVisibleBounds[index], 38.f);
        ghost.setSize(size);
        ghost.setOrigin(size * 0.5f);
        ghost.setPosition(*previewCenter);
        ghost.setTexture(heroTextures[index], true);
        if (heroVisibleBounds[index].size.x > 0) {
            ghost.setTextureRect(heroVisibleBounds[index]);
        }
        ghost.setFillColor({255, 255, 255, 125});
        target.draw(ghost);
    }
}

void SetupState::drawPlacedHeroes(sf::RenderWindow& target) const {
    for (std::size_t index = 0;
         index < AllyPlacementModel::ALLY_ORDER.size(); ++index) {
        const AllyType type = AllyPlacementModel::ALLY_ORDER[index];
        const auto& placement = placements.placement(type);
        if (!placement) continue;
        const sf::Vector2f center = map.gridToWorld(*placement);
        const bool selected = selectedAlly && *selectedAlly == type;

        sf::CircleShape ring(selected ? 23.f : 21.f);
        const float radius = ring.getRadius();
        ring.setOrigin({radius, radius});
        ring.setPosition(center);
        ring.setFillColor({8, 20, 28, 185});
        ring.setOutlineColor(selected ? sf::Color::White : allyColor(type));
        ring.setOutlineThickness(selected ? 4.f : 3.f);
        target.draw(ring);

        sf::RectangleShape portrait;
        const sf::Vector2f size = fittedSize(heroVisibleBounds[index], 35.f);
        portrait.setSize(size);
        portrait.setOrigin(size * 0.5f);
        portrait.setPosition(center);
        portrait.setTexture(heroTextures[index], true);
        if (heroVisibleBounds[index].size.x > 0) {
            portrait.setTextureRect(heroVisibleBounds[index]);
        }
        portrait.setFillColor(heroTextures[index]
            ? sf::Color::White : allyColor(type));
        target.draw(portrait);

        if (fontLoaded) {
            sf::Text label(font, std::to_string(index + 1), 11);
            label.setFillColor(sf::Color::White);
            label.setOutlineColor(sf::Color::Black);
            label.setOutlineThickness(1.5f);
            centerText(label, center + sf::Vector2f{17.f, 17.f});
            target.draw(label);
        }
    }
}

void SetupState::drawPanel(sf::RenderWindow& target) {
    target.draw(sidePanel);
    const sf::Vector2f mouse = window.mapPixelToCoords(
        sf::Mouse::getPosition(window));
    for (std::size_t index = 0; index < heroCards.size(); ++index) {
        HeroCardUi& card = heroCards[index];
        const AllyType type = AllyPlacementModel::ALLY_ORDER[index];
        const bool selected = selectedAlly && *selectedAlly == type;
        const bool hovered = card.panel.getGlobalBounds().contains(mouse);
        if (hovered && !selected) {
            card.panel.setFillColor({23, 45, 56, 250});
        } else if (!selected) {
            card.panel.setFillColor(CARD_COLOR);
        }
        target.draw(card.panel);
        target.draw(card.portrait);
        if (card.name) target.draw(*card.name);
        if (card.details) target.draw(*card.details);
        if (card.status) target.draw(*card.status);
    }

    target.draw(startButton);
    if (titleText) target.draw(*titleText);
    if (instructionText) target.draw(*instructionText);
    if (deployedText) target.draw(*deployedText);
    if (controlsText) target.draw(*controlsText);
    if (feedbackText) target.draw(*feedbackText);
    if (startHintText) target.draw(*startHintText);
    if (startText) target.draw(*startText);
}

void SetupState::render(sf::RenderWindow& target) {
    map.draw(target);
    drawPlacementHints(target);
    drawPlacedHeroes(target);
    drawPanel(target);
}
