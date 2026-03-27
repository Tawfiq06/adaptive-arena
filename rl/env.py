import gymnasium as gym
import numpy as np

SCREEN_W, SCREEN_H = 320, 240
MAX_STEPS = 3600  # 60s at 60fps

# ===========================================================================
# REWARD CONSTANTS — change these, nothing else, to tune behavior
# ===========================================================================

# Terminal
R_WIN  =  30.0
R_LOSE = -30.0

# Combat — per event
R_HIT_MELEE_ATK1  =  3.0   # landed atk1 (10 dmg)
R_HIT_MELEE_ATK2  =  5.0   # landed atk2 (18 dmg)
R_MISS_MELEE      = -4.0   # swung melee out of range (flat, not distance-scaled)
R_HIT_ARROW       =  2.0   # agent arrow hit enemy
R_EAT_MELEE       = -3.0   # took enemy melee hit
R_EAT_ARROW       = -2.0   # took enemy arrow hit
R_BLOCK_HIT       =  1.5   # successfully blocked (melee or arrow)

# Ranged attack quality
R_SHOOT_GOOD      =  0.3   # shot from 50–180 px (ideal range)
R_SHOOT_POINTBLANK = -0.5  # shot at < 30 px (wasteful, melee better)
SHOOT_NEAR_THRESH  = 30    # px below which shooting is penalised
SHOOT_FAR_THRESH   = 180   # px above which no bonus (far but acceptable)

# Positioning — per step (fires constantly, creates movement pressure)
R_IN_MELEE_RANGE  =  0.08  # dist < MELEE_DIST: reward for being close
R_APPROACHING     =  0.04  # dist decreasing AND not yet in melee range
R_FAR_PENALTY     = -0.06  # dist > FAR_DIST: constant pull toward enemy
R_IDLE_FAR        = -0.08  # not moving AND not attacking AND not in range

MELEE_DIST = 40   # px — "in melee range" threshold (generous, atk checks < 30)
FAR_DIST   = 150  # px — "too far" threshold

# Survival — contextual
R_DODGE_SUCCESS   =  0.4   # moved while enemy arrow was incoming
R_ARROW_NEAR      = -0.08  # enemy arrow < 15px away
R_ARROW_MED       = -0.04  # enemy arrow 15–40px away
R_POTION_PER_HP   =  0.05  # per HP healed from potion
R_TIME_PENALTY    = -0.001 # per step (tiny — just breaks ties)

# ===========================================================================


class Arrow:
    def __init__(self, x, y, facing, owner):
        self.x = x
        self.y = y
        self.dx = 4 if facing == 1 else -4
        self.owner = owner  # "agent" or "enemy"
        self.active = True

    def update(self):
        self.x += self.dx
        if self.x < 0 or self.x > SCREEN_W:
            self.active = False


class Potion:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.active = True


class FightEnv(gym.Env):
    def __init__(self):
        super().__init__()
        self.observation_space = gym.spaces.Box(
            low=-2.0, high=2.0, shape=(17,), dtype=np.float32
        )
        self.action_space = gym.spaces.Discrete(10)

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)

        side = self.np_random.integers(0, 2)
        ax = int(self.np_random.integers(20, 80))
        ex = int(self.np_random.integers(240, 300))
        if side == 1:
            ax, ex = ex, ax

        ay = int(self.np_random.integers(40, SCREEN_H - 40))
        ey = int(self.np_random.integers(40, SCREEN_H - 40))

        self.agent = {
            "x": ax, "y": ay, "hp": 100,
            "atk1_cd": 0, "atk2_cd": 0, "shoot_cd": 0,
            "dash_cd": 0, "dash_timer": 0,
            "block_cd": 0, "block_timer": 0, "blocking": False,
            "facing": 1 if ex > ax else -1, "dx": 0, "dy": 0
        }
        self.enemy = {
            "x": ex, "y": ey, "hp": 100,
            "atk1_cd": 0, "shoot_cd": 0,
            "dash_cd": 0, "dash_timer": 0,
            "dodge_cd": 0, "dodge_timer": 0, "dodge_dy": 0,
            "facing": -1 if ex > ax else 1, "dx": 0, "dy": 0
        }

        self.arrows = []
        self.step_count = 0

        px = int(self.np_random.integers(60, SCREEN_W - 60))
        py = int(self.np_random.integers(40, SCREEN_H - 40))
        self.potion = Potion(px, py)

        return self._obs(), {}

    # ------------------------------------------------------------------
    # Observation (unchanged)
    # ------------------------------------------------------------------

    def _nearest_potion_rel(self, entity):
        if self.potion.active:
            return ((self.potion.x - entity["x"]) / SCREEN_W,
                    (self.potion.y - entity["y"]) / SCREEN_H)
        return (0.0, 0.0)

    def _arrow_danger(self):
        a = self.agent
        closest = 2.0
        for arr in self.arrows:
            if not arr.active or arr.owner != "enemy":
                continue
            moving_toward = (arr.dx > 0 and arr.x < a["x"]) or \
                            (arr.dx < 0 and arr.x > a["x"])
            if not moving_toward:
                continue
            dist = abs(arr.x - a["x"]) / SCREEN_W
            if dist < closest:
                closest = dist
        return closest if closest < 2.0 else 0.0

    def _obs(self):
        a, e = self.agent, self.enemy
        pot_rx, pot_ry = self._nearest_potion_rel(a)
        return np.array([
            a["x"] / SCREEN_W * 2 - 1,
            a["y"] / SCREEN_H * 2 - 1,
            e["x"] / SCREEN_W * 2 - 1,
            e["y"] / SCREEN_H * 2 - 1,
            np.clip(a["hp"] / 100.0, 0.0, 1.0),
            np.clip(e["hp"] / 100.0, 0.0, 1.0),
            a["atk1_cd"]  / 20.0,
            a["atk2_cd"]  / 30.0,
            a["shoot_cd"] / 45.0,
            float(a["facing"]),
            a["dash_cd"]  / 60.0,
            float(1 if a["dash_timer"] > 0 else 0),
            a["block_cd"] / 30.0,
            pot_rx,
            pot_ry,
            float(self.potion.active),
            self._arrow_danger(),
        ], dtype=np.float32)

    # ------------------------------------------------------------------
    # Enemy AI (unchanged)
    # ------------------------------------------------------------------
    def _update_enemy(self):
        a, e = self.agent, self.enemy

        for cd in ["atk1_cd", "shoot_cd", "dash_cd", "dodge_cd"]:
            if e[cd] > 0:
                e[cd] -= 1

        if e["dodge_timer"] > 0:
            e["y"] = np.clip(e["y"] + e["dodge_dy"], 0, SCREEN_H - 18)
            e["dodge_timer"] -= 1

        e["dx"] = 0
        e["dy"] = 0

        if e["dash_timer"] > 0:
            e["dx"] = 3 if e["facing"] == 1 else -3
            e["dash_timer"] -= 1

        dx = a["x"] - e["x"]
        dist = abs(dx)
        e["facing"] = 1 if dx > 0 else -1

        if e["hp"] < 30 and dist < 80:
            e["dx"] = -2 if e["facing"] == 1 else 2
        elif e["hp"] < 40 and self.potion.active:
            pdx = self.potion.x - e["x"]
            pdy = self.potion.y - e["y"]
            e["dx"] = 1 if pdx > 0 else -1
            e["dy"] = 1 if pdy > 0 else -1
        elif dist > 120 and e["dash_cd"] == 0 and e["dash_timer"] == 0:
            e["dash_timer"] = 10
            e["dash_cd"] = 90
        elif 60 < dist < 160 and e["shoot_cd"] == 0:
            self.arrows.append(Arrow(e["x"], e["y"] + 10, e["facing"], "enemy"))
            e["shoot_cd"] = 60
        elif dist < 50:
            if e["dodge_cd"] == 0 and self.np_random.random() < 0.02:
                e["dodge_dy"] = 2 if self.np_random.random() < 0.5 else -2
                e["dodge_timer"] = 12
                e["dodge_cd"] = 40
            elif e["atk1_cd"] == 0:
                e["atk1_cd"] = 20
        else:
            e["dx"] = 1 if dx > 0 else -1

        e["x"] = np.clip(e["x"] + e["dx"], 0, SCREEN_W - 13)
        e["y"] = np.clip(e["y"] + e["dy"], 0, SCREEN_H - 18)

    # ------------------------------------------------------------------
    # Step
    # ------------------------------------------------------------------
    def step(self, action):
        a, e = self.agent, self.enemy
        self.step_count += 1

        # --- Tick cooldowns ---
        a["dx"] = 0
        a["dy"] = 0

        for cd in ["atk1_cd", "atk2_cd", "shoot_cd", "dash_cd", "block_cd"]:
            if a[cd] > 0:
                a[cd] -= 1

        if a["dash_timer"] > 0:
            a["dash_timer"] -= 1

        if a["block_timer"] > 0:
            a["block_timer"] -= 1
            if a["block_timer"] == 0:
                a["blocking"] = False

        # --- Snapshot position BEFORE moving ---
        prev_x    = a["x"]
        prev_y    = a["y"]
        prev_dist = ((prev_x - e["x"])**2 + (prev_y - e["y"])**2) ** 0.5

        # --- Apply action ---
        SPEED = 2
        did_move   = False
        did_act    = False   # attacked or blocked this step

        if action == 0:
            a["dy"] = -SPEED
        elif action == 1:
            a["dy"] = SPEED
        elif action == 2:
            a["dx"] = -SPEED
            a["facing"] = -1
        elif action == 3:
            a["dx"] = SPEED
            a["facing"] = 1
        elif action in (4, 5, 6):
            did_act = True
        elif action == 7:
            if a["dash_cd"] == 0:
                a["dash_timer"] = 10
                a["dash_cd"] = 60
        elif action == 8:
            if a["block_cd"] == 0:
                a["blocking"] = True
                a["block_timer"] = 15
                a["block_cd"] = 30
                did_act = True

        if a["dash_timer"] > 0:
            a["dx"] = 3 if a["facing"] == 1 else -3

        a["x"] = np.clip(a["x"] + a["dx"], 0, SCREEN_W - 13)
        a["y"] = np.clip(a["y"] + a["dy"], 0, SCREEN_H - 18)

        did_move = (a["x"] != prev_x or a["y"] != prev_y)

        # --- Update enemy and arrows ---
        self._update_enemy()
        for arr in self.arrows:
            arr.update()
        self.arrows = [arr for arr in self.arrows if arr.active]

        # --- Geometry after moving ---
        curr_dist    = ((a["x"] - e["x"])**2 + (a["y"] - e["y"])**2) ** 0.5
        dist_x       = abs(a["x"] - e["x"])
        dist_y       = abs(a["y"] - e["y"])
        in_melee_range = dist_x < 30 and dist_y < 30

        # Snapshot incoming arrows (for dodge reward)
        incoming_arrow_close = any(
            arr for arr in self.arrows
            if arr.active and arr.owner == "enemy"
            and ((arr.dx > 0 and arr.x < a["x"]) or (arr.dx < 0 and arr.x > a["x"]))
            and abs(arr.x - a["x"]) < 50
        )

        # ==================================================================
        # REWARD BLOCK — edit constants at top of file, not here
        # ==================================================================
        reward = R_TIME_PENALTY

        # ------------------------------------------------------------------
        # 2. COMBAT — fires only when something actually happens
        # ------------------------------------------------------------------

        # Melee atk1
        if action == 4 and a["atk1_cd"] == 0:
            if in_melee_range:
                e["hp"] -= 10
                reward += R_HIT_MELEE_ATK1
            else:
                reward += R_MISS_MELEE      # flat penalty, no distance scaling
            a["atk1_cd"] = 20

        # Melee atk2
        if action == 5 and a["atk2_cd"] == 0:
            if in_melee_range:
                e["hp"] -= 18
                reward += R_HIT_MELEE_ATK2
            else:
                reward += R_MISS_MELEE
            a["atk2_cd"] = 30

        # Ranged attack — reward depends on range quality
        if action == 6 and a["shoot_cd"] == 0:
            self.arrows.append(Arrow(a["x"], a["y"] + 10, a["facing"], "agent"))
            a["shoot_cd"] = 45
            if curr_dist < SHOOT_NEAR_THRESH:
                reward += R_SHOOT_POINTBLANK
            elif curr_dist <= SHOOT_FAR_THRESH:
                reward += R_SHOOT_GOOD
            # beyond FAR_THRESH: no bonus (neutral — arrow still fires)

        # Enemy melee lands
        if e["atk1_cd"] == 0 and in_melee_range:
            if a["blocking"]:
                reward += R_BLOCK_HIT
            else:
                a["hp"] -= 10
                reward += R_EAT_MELEE
            e["atk1_cd"] = 20

        # Arrow resolution
        new_arrows = []
        for arr in self.arrows:
            if not arr.active:
                continue
            hit_agent = abs(arr.x - a["x"]) < 8 and abs(arr.y - a["y"]) < 10
            hit_enemy = abs(arr.x - e["x"]) < 8 and abs(arr.y - e["y"]) < 10

            if arr.owner == "enemy" and hit_agent:
                if a["blocking"]:
                    reward += R_BLOCK_HIT
                else:
                    a["hp"] -= 12
                    reward += R_EAT_ARROW
                arr.active = False
            elif arr.owner == "agent" and hit_enemy:
                e["hp"] -= 12
                reward += R_HIT_ARROW
                arr.active = False

            if arr.active:
                new_arrows.append(arr)
        self.arrows = new_arrows

        # ------------------------------------------------------------------
        # 3. POSITIONING — fires every step (the movement engine)
        # ------------------------------------------------------------------

        if in_melee_range or curr_dist < MELEE_DIST:
            # Already where we want to be
            reward += R_IN_MELEE_RANGE
        else:
            # Not close enough
            if curr_dist > FAR_DIST:
                reward += R_FAR_PENALTY   # constant pull toward enemy

            if curr_dist < prev_dist:
                reward += R_APPROACHING  # bonus for closing gap

            # Idle far from enemy — this is the anti-camping signal
            if not did_move and not did_act:
                reward += R_IDLE_FAR

        # ------------------------------------------------------------------
        # 4. SURVIVAL — contextual, small
        # ------------------------------------------------------------------

        # Potion pickup
        if self.potion.active:
            if abs(a["x"] - self.potion.x) < 12 and abs(a["y"] - self.potion.y) < 12:
                heal = min(25, 100 - a["hp"])
                a["hp"] += heal
                reward += heal * R_POTION_PER_HP
                self.potion.active = False

        if self.potion.active:
            if abs(e["x"] - self.potion.x) < 12 and abs(e["y"] - self.potion.y) < 12:
                e["hp"] = min(100, e["hp"] + 25)
                self.potion.active = False

        # Dodge reward
        if incoming_arrow_close and did_move:
            reward += R_DODGE_SUCCESS

        # Arrow danger proximity
        for arr in self.arrows:
            if not arr.active or arr.owner != "enemy":
                continue
            moving_toward = (arr.dx > 0 and arr.x < a["x"]) or \
                            (arr.dx < 0 and arr.x > a["x"])
            if not moving_toward:
                continue
            d = abs(arr.x - a["x"])
            if d < 15:
                reward += R_ARROW_NEAR
            elif d < 40:
                reward += R_ARROW_MED

        # ------------------------------------------------------------------
        # 1. TERMINAL — always last so it can't be drowned out
        # ------------------------------------------------------------------
        terminated = False
        if a["hp"] <= 0:
            reward += R_LOSE
            terminated = True
        elif e["hp"] <= 0:
            reward += R_WIN
            terminated = True

        truncated = self.step_count >= MAX_STEPS
        return self._obs(), reward, terminated, truncated, {}