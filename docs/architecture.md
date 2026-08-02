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
    class IntroState
    class GameplayState
    class MenuState
    class SetupState
    class GameOverState
    class WinState

    State <|-- IntroState
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
    class Wand
    class SpiritStaff
    class Projectiles
    class Arrow
    class MagicBolt
    class SpiritBolt
    Weapon <|-- Bow
    Weapon <|-- Sword
    Weapon <|-- Wand
    Weapon <|-- SpiritStaff
    Entity *-- Weapon
    Projectiles <|-- Arrow
    Projectiles <|-- MagicBolt
    Projectiles <|-- SpiritBolt

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
    class EntityCollision
    class EntityLifecycle

    GameplayState *-- Player
    GameplayState *-- Ally
    GameplayState *-- Monster
    GameplayState *-- Map
    GameplayState *-- WaveManager
    GameplayState *-- CombatManager
    GameplayState *-- UpgradeManager
    GameplayState *-- HUD
    GameplayState --> GameContext
    GameContext *-- Projectiles
    GameplayState ..> EntityCollision : separates
    GameplayState ..> EntityLifecycle : cleans up
    WaveManager ..> Monster : creates
    WaveManager ..> Elite : creates
    WaveManager ..> Boss : creates
    CombatManager ..> Weapon : resolves hits
    CombatManager ..> Projectiles : resolves hits
    Player --> SpiritStaff
    Ally --> Bow
    Ally --> Wand
    Ally --> Sword
    Monster --> Sword
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
    participant Staff as SpiritStaff
    participant Context as GameContext
    participant Combat as CombatManager
    participant Monster

    User->>Gameplay: Nhấn chuột trái
    Gameplay->>Player: setAttackDirection(), startAttacking()
    Gameplay->>Player: update(context)
    Player->>Staff: triggerAction(player, context, combat)
    Staff->>Context: thêm SpiritBolt (unique_ptr, snapshot damage)
    Gameplay->>Combat: processProjectiles(context, entities)
    Combat->>Monster: takeDamage(damage)
    Monster-->>Combat: cập nhật health/death state
    Gameplay->>Gameplay: thu thập entity chết logic
    Gameplay->>Monster: forgetEntity(victim)
    Gameplay->>Context: gỡ victim khỏi target views
    Gameplay->>Gameplay: erase unique_ptr sau death timer
```

## Áp dụng OOP

- **Đóng gói:** trạng thái máu, cooldown, vũ khí và dữ liệu wave được giữ trong
  lớp tương ứng, truy cập qua phương thức công khai.
- **Kế thừa:** các state kế thừa `State`; nhân vật kế thừa `Entity`; `Elite`
  và `Boss` kế thừa `Monster`; `Arrow`, `MagicBolt`, `SpiritBolt` kế thừa `Projectiles`; vũ khí kế thừa
  `Weapon`.
- **Đa hình:** `update`, `draw`, `triggerAction`, `isHitting` được gọi qua lớp
  cơ sở nhưng thực thi theo đúng lớp con.
- **Trừu tượng hóa:** `State`, `Entity` và `Weapon` định nghĩa hợp đồng chung
  bằng các hàm thuần ảo.

## Luồng gameplay

1. `Game` chuyển event cho `StateMachine`.
2. `GameplayState` dựng các view không sở hữu trong `GameContext`.
3. `WaveManager` có thể trả về một Monster mới cho owner container.
4. Player, Ally và Monster cập nhật bằng cùng `deltaTime`.
5. `EntityCollision` tách các entity sống; `CombatManager` xử lý vũ khí và đạn.
6. `EntityLifecycle` xóa target/cache trước khi owner erase entity đã hết
   death timer.
7. `RadiantPulse`, Effects, Boss Beam/Enrage và HUD chỉ cập nhật khi gameplay
   thực sự chạy; pause đóng băng timer mà không reset.
8. `GameplayTransitionGate` bảo vệ reward, intermission và Victory khỏi phát
   transition lặp; gameplay sau đó kiểm tra Game Over hoặc Win.
