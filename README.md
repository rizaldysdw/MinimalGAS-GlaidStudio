# GAS Test Project

A minimal Unreal Engine 4.27 project demonstrating the **Gameplay Ability System (GAS)** with a Fireball, Normal Attack, and Berserk buff. Includes AttributeSets, GameplayAbilities, GameplayEffects, GameplayCues, and a simple world-space UI for Health, Mana, and Berserk status.

---

## Features

### Core Abilities
- **Normal Attack (LMB)**  
  - Spawns a projectile.  
  - Applies damage equal to `AttackPower` (default 10).  
  - **No explosion effect.**

- **Fireball (Q)**  
  - Costs **20 Mana**.  
  - Spawns a projectile.  
  - On hit: applies **40 damage** and triggers the `GameplayCue.Explosion` VFX.  
  - Disabled while Berserk is active.

- **Berserk (E)**  
  - Grants `AttackPower × 1.5` (e.g. 10 → 15).  
  - Duration: **5 seconds**.  
  - Grants the tag `State.Berserk.Active`.  
  - Blocks Fireball while active.

- **Restore (R)**  
  - Restores Health and Mana to their **Max** values.

### Attributes
Defined in `USimpleAttributeSet`:
- `Health`, `MaxHealth`
- `Mana`, `MaxMana`
- `AttackPower`

### Projectile
- `ASimpleProjectile` spawns from the character’s gun muzzle.
- Carries references to the Source ASC + Damage GE.  
- On hit:
  - Optionally executes `GameplayCue.Explosion`.  
  - Applies the associated GameplayEffect to the target’s ASC.  

### TargetTest Actor
- Dummy actor with its own ASC + AttributeSet.  
- On Health reaching **0**, waits **3 seconds** then automatically restores Health & Mana to Max.  
- Useful for testing Fireball/Normal Attack damage and Berserk interactions.

### UI
- World-space **Widget Blueprints** attached above each actor:
  - Health & Mana bars.  
  - Berserk status text (`Active` / `Inactive`).

---

## Controls

- **Mouse** - Aim
- **LMB** - Normal Attack  
- **Q** - Fireball  
- **E** - Berserk (buff, blocks Fireball)  
- **R** - Restore Health & Mana  
- **Alt + F4** - Exit Application

---

## Downloads
Windows Build - https://mega.nz/file/rgZhnZoK#6Le5OX1lVmmlfKorlXC619lODhCMPYG_f6BD-2LjrI4

---
