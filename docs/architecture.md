# Kiến trúc Eternal Siege

## Sơ đồ lớp

```mermaid
classDiagram
    class State {
        <<abstract>>
        +onEnter()
        +onExit()
        +handleEvent(event)
        +update(dt)
        +render(window)
    }
    class StateMachine {
        -states
        -pendingState
        +changeState(state)
        +handleEvent(event)
        +update(dt)
        +render(window)
    }
    class GameplayState
    class MenuState
    class SetupState
    class GameOverState
    class WinState

    State <|-- GameplayState
    State <|-- MenuState
    State <|-- SetupState
    State <|-- GameOverState
    State <|-- WinState
    StateMachine o-- State

    class Entity {
        <<abstract>>
        -health
        -position
        -currentWeapon
        +takeDamage(damage)
        +update(context)
        +draw(window)*
    }
    class Player
    class Ally
    class Monster
    class Elite
    class Boss
    Entity <|-- Player
    Entity <|-- Ally
    Entity <|-- Monster
    Monster <|-- Elite
    Monster <|-- Boss

    class Weapon {
        <<abstract>>
        +getHitbox()*
        +isHitting()*
        +triggerAction()*
    }
    class Bow
    class Sword
    Weapon <|-- Bow
    Weapon <|-- Sword
    Entity *-- Weapon

    class GameplayState {
        -player
        -allies
        -monsters
        -map
        -waveManager
        -combatManager
        -upgradeManager
        -hud
    }
    class GameContext {
        +allEntity
        +players
        +enemies
        +projectiles
    }
    class WaveManager
    class CombatManager
    class UpgradeManager
    class HUD
    class Map

    GameplayState *-- Player
    GameplayState *-- Ally
    GameplayState *-- Monster
    GameplayState *-- Map
    GameplayState *-- WaveManager
    GameplayState *-- CombatManager
    GameplayState *-- UpgradeManager
    GameplayState *-- HUD
    GameplayState --> GameContext
    WaveManager ..> Boss : creates
```

`GameplayState` là chủ sở hữu duy nhất của player, ally và monster.
`GameContext` chỉ giữ các con trỏ quan sát trong một frame, đồng thời sở hữu
projectile. Cấu trúc này tránh `delete` thủ công và tránh quyền sở hữu mơ hồ.

## Sơ đồ trình tự: người chơi bắn quái

```mermaid
sequenceDiagram
    actor User
    participant Gameplay as GameplayState
    participant Player
    participant Bow
    participant Context as GameContext
    participant Combat as CombatManager
    participant Monster

    User->>Gameplay: Nhấn chuột trái
    Gameplay->>Player: setAttackDirection(), startAttacking()
    Gameplay->>Player: update(context)
    Player->>Bow: triggerAction(player, context, combat)
    Bow->>Context: thêm Arrow (unique_ptr)
    Gameplay->>Combat: processProjectiles(context, entities)
    Combat->>Monster: takeDamage(damage)
    Monster-->>Combat: cập nhật health/death state
```

## Áp dụng OOP

- **Đóng gói:** trạng thái máu, cooldown, vũ khí và dữ liệu wave được giữ trong
  lớp tương ứng, truy cập qua phương thức công khai.
- **Kế thừa:** các state kế thừa `State`; nhân vật kế thừa `Entity`; boss kế
  thừa `Monster`; vũ khí kế thừa `Weapon`.
- **Đa hình:** `update`, `draw`, `triggerAction`, `isHitting` được gọi qua lớp
  cơ sở nhưng thực thi theo đúng lớp con.
- **Trừu tượng hóa:** `State`, `Entity` và `Weapon` định nghĩa hợp đồng chung
  bằng các hàm thuần ảo.
