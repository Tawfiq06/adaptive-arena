import gymnasium as gym
import numpy as np

SCREEN_W, SCREEN_H = 320, 240
MAX_STEPS = 3600  # 60s at 60fps

# --- Projectile helper ---
class Arrow:
    def __init__(self, x, y, facing):
        self.x = x
        self.y = y
        self.dx = 4 if facing == 1 else -4
        self.active = True

    def update(self):
        self.x += self.dx
        if self.x < 0 or self.x > SCREEN_W:
            self.active = False

# --- Potion helper ---
class Potion:
    def __init__(self, x, y):
        self.x = x
        self.y = y
        self.active = True


class FightEnv(gym.Env):
    def __init__(self):
        super().__init__()

        # 16 floats: positions, hp, cooldowns, facing, dash, potion info, arrow danger
        self.observation_space = gym.spaces.Box(
            low=-2.0, high=2.0, shape=(16,), dtype=np.float32
        )
        self.action_space = gym.spaces.Discrete(10)

    def reset(self, seed=None, options=None):
        super().reset(seed=seed)

        # Randomise starting positions and sides
        side = self.np_random.integers(0, 2)  # 0 = agent left, 1 = agent right
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
   
    def _nearest_potion_rel(self, entity):
        """Relative normalised vector to potion, or (0,0) if none."""
        if self.potion.active:
            return ((self.potion.x - entity["x"]) / SCREEN_W,
                    (self.potion.y - entity["y"]) / SCREEN_H)
        return (0.0, 0.0)

    def _arrow_danger(self):
        """Closest enemy arrow normalised x-distance to agent (0 if none)."""
        a = self.agent
        closest = 2.0
        for arr in self.arrows:
            if arr.active:
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
            np.clip(a["hp"] / 100.0, -1.0, 1.0),
            np.clip(e["hp"] / 100.0, -1.0, 1.0),
            a["atk1_cd"]  / 20.0,
            a["atk2_cd"]  / 30.0,
            a["shoot_cd"] / 45.0,
            float(a["facing"]),
            a["dash_cd"]  / 60.0,
            float(1 if a["dash_timer"] > 0 else 0),
            pot_rx,
            pot_ry,
            float(self.potion.active),
            self._arrow_danger(),
        ], dtype=np.float32)

    # ------------------------------------------------------------------
    # Enemy AI
    # ------------------------------------------------------------------
    def _update_enemy(self):
        a, e = self.agent, self.enemy

        for cd in ["atk1_cd", "shoot_cd", "dash_cd", "dodge_cd"]:
            if e[cd] > 0:
                e[cd] -= 1

        # Tick active dodge
        if e["dodge_timer"] > 0:
            e["y"] = np.clip(e["y"] + e["dodge_dy"], 0, SCREEN_H - 18)
            e["dodge_timer"] -= 1

        e["dx"] = 0
        e["dy"] = 0

        # Tick active dash
        if e["dash_timer"] > 0:
            e["dx"] = 3 if e["facing"] == 1 else -3
            e["dash_timer"] -= 1

        dx = a["x"] - e["x"]
        dist = abs(dx)
        e["facing"] = 1 if dx > 0 else -1

        # --- Behavior tree ---

        # 1. Retreat if low HP and agent is close
        if e["hp"] < 30 and dist < 80:
            e["dx"] = -2 if e["facing"] == 1 else 2

        # 2. Go for potion if low HP and potion exists
        elif e["hp"] < 40 and self.potion.active:
            pdx = self.potion.x - e["x"]
            pdy = self.potion.y - e["y"]
            e["dx"] = 1 if pdx > 0 else -1
            e["dy"] = 1 if pdy > 0 else -1

        # 3. Dash toward agent if far away and dash is ready
        elif dist > 120 and e["dash_cd"] == 0 and e["dash_timer"] == 0:
            e["dash_timer"] = 10
            e["dash_cd"] = 90

        # 4. Shoot if at medium range
        elif 60 < dist < 160 and e["shoot_cd"] == 0:
            self.arrows.append(Arrow(e["x"], e["y"] + 10, e["facing"]))
            e["shoot_cd"] = 60

        # 5. Melee if in range — occasionally dodge to reposition
        elif dist < 50:
            if e["dodge_cd"] == 0 and self.np_random.random() < 0.02:
                e["dodge_dy"] = 2 if self.np_random.random() < 0.5 else -2
                e["dodge_timer"] = 12
                e["dodge_cd"] = 40
            elif e["atk1_cd"] == 0:
                e["atk1_cd"] = 20

        # 6. Walk toward agent
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

        a["dx"] = 0
        a["dy"] = 0

        for cd in ["atk1_cd", "atk2_cd", "shoot_cd", "dash_cd"]:
            if a[cd] > 0:
                a[cd] -= 1

        if a["dash_timer"] > 0:
            a["dash_timer"] -= 1

        SPEED = 2
        if   action == 0: a["dy"] = -SPEED
        elif action == 1: a["dy"] =  SPEED
        elif action == 2: a["dx"] = -SPEED; a["facing"] = -1
        elif action == 3: a["dx"] =  SPEED; a["facing"] =  1
        elif action == 7:
            if a["dash_cd"] == 0:
                a["dash_timer"] = 10
                a["dash_cd"] = 60

        if a["dash_timer"] > 0:
            a["dx"] = 3 if a["facing"] == 1 else -3

        prev_x = a["x"]
        prev_y = a["y"]

        a["x"] = np.clip(a["x"] + a["dx"], 0, SCREEN_W - 13)
        a["y"] = np.clip(a["y"] + a["dy"], 0, SCREEN_H - 18)

        self._update_enemy()

        for arr in self.arrows:
            arr.update()
        self.arrows = [arr for arr in self.arrows if arr.active]

        reward = -0.005  # smaller time penalty so it doesn't rush blindly
        agent_dealt = 0
        enemy_dealt = 0

        # Agent melee — fix: check the correct cooldown per action
        if action == 4 and a["atk1_cd"] == 0:
            if abs(a["x"] - e["x"]) < 30 and abs(a["y"] - e["y"]) < 30:
                e["hp"] -= 10
                agent_dealt = 10
            a["atk1_cd"] = 20

        if action == 5 and a["atk2_cd"] == 0:  # fix: was checking atk1_cd
            if abs(a["x"] - e["x"]) < 30 and abs(a["y"] - e["y"]) < 30:
                e["hp"] -= 18
                agent_dealt = 18
            a["atk2_cd"] = 30

        # Agent shoot — fix: spawn a real arrow, don't deal instant damage
        if action == 6 and a["shoot_cd"] == 0:
            self.arrows.append(Arrow(a["x"], a["y"] + 10, a["facing"]))
            a["shoot_cd"] = 45

        # Enemy melee
        if e["atk1_cd"] == 0 and abs(a["x"] - e["x"]) < 30 and abs(a["y"] - e["y"]) < 30:
            a["hp"] -= 10
            enemy_dealt += 10
            e["atk1_cd"] = 20

        # Arrow hits (now includes agent's own arrows hitting enemy too)
        remaining = []
        for arr in self.arrows:
            if not arr.active:
                continue
            # Enemy arrows hitting agent
            if arr.dx < 0 or True:  # check all arrows
                hit_agent = abs(arr.x - a["x"]) < 8 and abs(arr.y - a["y"]) < 10
                hit_enemy = abs(arr.x - e["x"]) < 8 and abs(arr.y - e["y"]) < 10
                if hit_agent:
                    a["hp"] -= 12
                    enemy_dealt += 12
                    arr.active = False
                elif hit_enemy:
                    e["hp"] -= 12
                    agent_dealt += 12
                    arr.active = False
            remaining.append(arr)
        self.arrows = [arr for arr in remaining if arr.active]

        # Potion pickup
        if self.potion.active:
            if abs(a["x"] - self.potion.x) < 12 and abs(a["y"] - self.potion.y) < 12:
                heal = min(25, 100 - a["hp"])
                a["hp"] += heal
                reward += heal * 0.05
                self.potion.active = False
        if self.potion.active:
            if abs(e["x"] - self.potion.x) < 12 and abs(e["y"] - self.potion.y) < 12:
                e["hp"] = min(100, e["hp"] + 25)
                self.potion.active = False

        # Reward shaping
        reward += agent_dealt * 0.1
        reward -= enemy_dealt * 0.05

        # Fix: use actual 2D distance for approach reward
        prev_dist = ((prev_x - e["x"])**2 + (prev_y - e["y"])**2) ** 0.5
        curr_dist = ((a["x"]  - e["x"])**2 + (a["y"]  - e["y"])**2) ** 0.5
        if curr_dist < prev_dist and curr_dist > 30:
            reward += 0.01

        # Fix: punish being near arrows (not reward it)
        for arr in self.arrows:
            if arr.active and abs(arr.x - a["x"]) < 40:
                reward -= 0.01

        # Termination
        terminated = False
        if a["hp"] <= 0:
            reward -= 10.0
            terminated = True
        elif e["hp"] <= 0:
            reward += 10.0
            terminated = True

        truncated = self.step_count >= MAX_STEPS
        return self._obs(), reward, terminated, truncated, {}

