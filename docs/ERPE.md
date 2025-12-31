# 《代码余烬》统一范式：ERPE + 原子化 Process + 制作 + 机会/GOAP（简版整合）

> 一句话：
**Entity 使用/转化 Resource，通过一系列“原子化 Process”产生 Effect；
同一套范式同时驱动物理/技能/制作系统，以及 NPC 的机会识别与 GOAP 规划。**

---

## 0. 四个核心概念：ERPE

1. **Entity：谁在做事**
    - 角色 / 怪物 / NPC / 派系
    - 投射物、陷阱、火堆、机关
    - 工作台、熔炉、工厂、塔
    - JobSlot / Quest / Shop（长期机会）
    - 环境实体（区域、地块……，如果需要追踪）

    > 本质：**带 ID 的状态容器**（组件集合），逻辑尽量下沉到 Process。
2. **Resource：它有什么、能消耗什么**
    - 能量：HP、MP、体力、热量、电力、燃料
    - 物质量：雪、土、水、木、金属……
    - 状态量：耐久度、冷却值、弹药数量、温度、熟练度、时间进度……

    > 本质：**可计量、可增减的数值或状态**，是 Process 运算的主要输入/输出。
3. **Process：事情怎么发生（原子化 + 可组合）**
    - 输入：若干 Entity / Resource / 环境信息
    - 内部：一串 **原子 Process 节点**（检查、计算、消耗、生成……）
    - 输出：**Effect（世界状态 diff）**

    典型原子 Process 类型：

    - 条件判定：`CheckResourceEnough`, `CheckInRange`, `CheckTag`…
    - 数值计算：`ComputeDamage`, `ComputeHeatExchange`, `ComputeCraftTime`…
    - 资源变更：`ConsumeResource`, `AddResource`
    - 状态修饰：`ApplyModifier`, `RemoveModifier`, `ApplyTag`
    - 实体/环境操作：`SpawnEntity`, `DestroyEntity`, `ChangeMaterial`, `ChangeTerrain`
    - 时序/调度：`Wait`, `TickBuff`, `ScheduleNextEvent`, `UpdateCraftProgress`

    > 高层行为（技能、制作、AI 动作）= **流程图/脚本，只是把原子节点串起来**。
4. **Effect：世界发生了什么（状态 diff）**
    - 生成/移除 Entity：生成投射物、建筑、地面物品；销毁死亡单位、破碎物体
    - 修改 Resource：HP/MP/体力变化，雪量/燃料变化，温度/速度变化
    - 修改环境：地形改变、局部温度变化、环境资源变化（氧气、积雪等）

    > 推荐：**Process 只产出 Effect，由统一 EffectSystem / WorldStateApplier 应用**
方便回放、网络同步、调试。

---

## 1. 长期运行的 Process：Buff / Job / 制作工单

> Buff / DOT / HOT / Aura / 建筑持续效果 / 制作任务 / 工厂工单
= **挂在 Entity 上的长期 Process 实例，由时间/事件驱动，周期地产生 Effect**。

- Buff 例：
    - OnApply：`ApplyModifier(攻击+X)`, `ApplyTag("Burning")`
    - OnTick：`ConsumeResource(HP)` → 持续伤害
    - OnExpire：`RemoveModifier`, `RemoveTag`
- 制作工单（CraftJob）例：
    - 初始化：`ConsumeIngredients`, `ConsumeCrafterResource`, `ConsumeFuelOrPower`
    - 过程：`UpdateCraftProgress`（每 Tick），中途可插 `RandomEventCheck`
    - 完成：`RollQualityAndByproducts`, `CreateOutputItems`, `ApplyExperience`

调度方式：
统一的 **ProcessScheduler / BuffSystem / JobSystem** 每帧/每步遍历这些实例，执行对应原子 Process，集中产出 Effect。

---

## 2. 制作系统：ERPE 视角下的“Craft”

> 制作 = **多个 Entity 围绕材料/能量等 Resource，
通过一串原子 Process（检查→消耗→进度→产出），产生 Effect（物品、经验、状态变化）。**

### 2.1 参与角色映射

- **Entity**
    - 制作者：玩家 / NPC / 机器人 / 工厂
    - 制作载体：工作台、熔炉、炼金台、3D 打印机…
    - 材料容器：背包、箱子、料仓、传送带
    - CraftJob：一个“正在进行的制作任务”的实体/记录
- **Resource**
    - 材料：木、石、铁锭、雪、土、水、药草……
    - 能量：体力、魔力、电力、燃料
    - 状态：工具耐久、温度、熟练度、制作时间/进度

### 2.2 通用高层流程：`Craft_Generic`

可以即时，也可以生成 CraftJob 异步完成。

典型原子节点（可按需删减）：

1. 检查阶段
    - `CheckRecipeUnlocked`
    - `CheckIngredientsEnough`（输出 IngredientPlan）
    - `CheckEnvironmentCondition`（温度、在熔炉旁、室内等）
    - `CheckToolAndDurability`
2. 消耗阶段
    - `ConsumeIngredients(IngredientPlan)`
    - `ConsumeCrafterResource`（体力/魔力等）
    - `ConsumeFuelOrPower`
    - `AdjustToolDurability`
3. 时间 / 进度阶段
    - 即时：直接跳到产出
    - 异步：
        - `ScheduleCraftTime` → 生成 CraftJob
        - Job 每 Tick：`UpdateCraftProgress`（可中断/加速/事件）
4. 产出阶段
    - `RollQualityAndByproducts`（读材料质量、环境、Buff…）
    - `CreateOutputItems`（产物 + 副产物）
    - `ApplyExperienceAndSkillUp`

### 2.3 三种典型形态

- **即时制作（背包合成）**
    - 不创建 CraftJob；只跑一遍检查+消耗+产出。
- **工作台制作（带时间条）**
    - 生成 CraftJob；由系统持续 `UpdateCraftProgress`，完成时再产物。
- **自动化工厂**
    - “制作者”是机器/生产线；材料来自输送带/料仓；输出到下一环节。
    - 本质仍是同一套原子 Process，只是调用者/容器不同。

---

## 3. “机会”在这套范式中的位置

> “机会”（Opportunity）= **当前 Actor 在当前世界状态下可用的“Action 实例”**，
可以来自长期存在的 Entity，也可以由现有状态临时推导。

### 3.1 长期存在的机会：用 Entity 表示

- **JobSlot（工作位）**
    - 例：工厂“流水线 X/10 名额”
    - 字段：位置、雇主派系、工资、要求技能、剩余名额…
- **Quest / Task（任务）**
    - 发布方、报酬、限制条件、执行状态
- **Shop / MarketOffer（商店/固定交易接口）**
    - 某地长期收购/贩卖某物，维护库存、价格、位置

> 这些是“世界里的机会” → **Entity**。
多个 Actor 可以同时感知、竞争、改变它们的状态。

### 3.2 瞬时机会：ActionOption（非 Entity）

由当前 Entity + Resource + 环境推导出的临时“可行动选项”：

- 抢劫机会：
    - 来源：目标 Actor、当前地点治安、自身战力/性格
    - 生成：
        - `action = "Rob"`
        - `target_id = 某Actor`
        - `success_prob = f(力量, 治安, traits)`
        - `expected_gain = f(target_wealth)`
- 临时交易机会：
    - 同地有 A（多余食物缺钱）、B（有钱缺食物）
    - 生成：
        - `action = "Trade"`
        - `partner_id = B`
        - `item = food`
        - `price = world_rule(...)`

> 这类“机会”只是 **决策层的临时结构（ActionOption）**：
- 引用相关 Entity（目标、商店、JobSlot）
- 加上预估收益/风险/成本
- 不登记为 Entity，只在 AI 感知/规划阶段短暂存在。

### 3.3 术语分层

- 世界模型层：
    - Entity：Actor / Faction / Location / JobSlot / Quest / Shop…
- AI 决策层：
    - Opportunity / ActionOption：
        - = `(ActionType + 具体参数 + 评估数值 + 关联 EntityId)`
        - 起源：长期机会 Entity + 临时推导

---

## 4. GOAP 在 ERPE / Process 框架中的嵌入

### 4.1 GOAP 用我们的语言描述

GOAP（Goal-Oriented Action Planning）：

> 给定一个目标（Goal）和一组可选 Action（行动模板），  
每个 Action 拥有 precondition（前置条件）和 effect（抽象效果），  
通过搜索找到一串 Action，使世界从当前状态达到目标。

在本项目中：

- **GOAP Action** ⇔ **高层 Process / 计划步骤**
    - 如：`GetJob`, `WorkForMoney`, `Rob`, `BuyFood`, `MoveToSafeLocation`…
    - 真正执行时，再拆成多个原子 Process（Check + Consume + Spawn…）
- **GOAP precondition**
    - 直接对应我们已有的检查类原子 Process 抽象：
        - `CheckHasMoney(actor, x)`
        - `CheckLocationSecurityBelowThreshold(location)`
        - `CheckJobSlotAvailable(jobslot)`
        - `CheckHasWeapon(actor)` …
- **GOAP effect（规划用）**
    - 对应我们 Effect 系统的“抽象版”：
        - `wealth += 50`
        - `hunger -= 30`
        - `location = SafePlace`
    - 仅在规划时使用；执行时走真实的 Process → Effect。

> 规划期：GOAP 看的是简化状态（主要是 Resource/标志位）；  
执行期：用同名/等价的原子 Process 和 Effect 真正改变世界。

### 4.2 插入 Actor AI 主循环

简化的 Actor AI：

```text
Actor_AI_Tick(actor):
  SenseWorld(actor)          # 感知实体/资源/长期机会，生成若干 ActionOption
  UpdateInternalState(actor) # 记忆、情绪、偏好等
  UpdateOrSelectGoal(actor)  # 决定当前目标（吃饭/赚钱/安全/逃离）
  UpdateOrSelectPlan(actor)  # 在这里调用 GOAP
  ExecutePlanStep(actor)     # 每 Tick 执行计划中的一个高层 Action
```

**UpdateOrSelectPlan(actor)** 中：

1. 抽象当前局部 state（和该 Actor 相关的资源/标志）
2. 提供 Action 库给 GOAP（= 可用的高层 Process 列表）
3. 调用 GOAP：
    - 输入：Goal + 抽象 state + Action 库
    - 输出：Action 序列，如 `[GetJob, Work, BuyFood]`
4. 缓存为 `actor.current_plan`

**ExecutePlanStep(actor)**：

- 取当前 plan 的第一个/当前 Action（例如 `Work`）
- 实际调用对应高层 Process（内部是原子 Process 序列）：
    - `CheckPreconditions`（再校验一次）
    - `ConsumeResource / SpawnEntity / ChangeWealth / ChangeLocation`…
- 若前置不再满足（世界变了）：
    - 放弃/重算计划，或局部修正

### 4.3 GOAP + ERPE 的价值

- **统一语义**  
    - precondition/effect 与我们的 `CheckXxx` / Effect 类型一一对应
    - 不会出现“规划层语义和执行层不一致”的双轨问题
- **易扩展**  
    - 新增一个行为：
        - 提供一套 precondition / effect（规划用）
        - 提供一个高层 Process（执行用）
    - GOAP 自动在规划中把它考虑进去，产生新组合
- **分层使用**  
    - 小 NPC：简单规则/手写 Plan  
    - 重要 NPC / 势力：启用 GOAP，做中长期多步规划（找工作→赚钱→买武器→抢地盘→谈判）

---

## 5. 设计与实现时的“固定提问模板”

无论是技能、物理、制作，还是 AI 行为/机会/GOAP，都可以用同一套问题审视：

1. **谁在做事？**（Entity）
    - 玩家、NPC、工厂、哨戒塔、JobSlot、Shop…
2. **他/它拥有什么可计量的量？**（Resource）
    - HP/MP、材料、燃料、钱、关系、温度、熟练度、时间进度…
3. **可以拆成哪些原子 Process？如何组合？**（Process）
    - 检查：`CheckXxx`
    - 计算：`ComputeXxx`
    - 消耗：`ConsumeXxx`
    - 生成：`SpawnXxx`
    - 修改：`ChangeXxx`
    - 调度：`Schedule/UpdateXxx`
4. **组合执行后，世界发生了哪些具体变化？**（Effect）
    - 新实体？资源变化？状态/地形/关系变化？  
    - 这些 Effect 是否都能被一个统一系统收集和应用？

> 总结：
**ERPE + 原子化 Process 是统一底层；
制作系统只是“围绕材料/能量的 Process 拼装”；
AI 的“机会”是对现有 Entity/Resource 的视角抽象；
GOAP 则在同一套 precondition/effect 语义之上做搜索规划。**

