import os
from stable_baselines3 import PPO
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.callbacks import CheckpointCallback, EvalCallback
from stable_baselines3.common.monitor import Monitor
from env import FightEnv

# ------------------------------------------------------------------
# Paths
# ------------------------------------------------------------------
CHECKPOINT_PREFIX = "fight_agent_v2_checkpoint"
SAVE_PATH         = "fight_agent_v2"
BEST_PATH         = "fight_agent_v2_best"

# ------------------------------------------------------------------
# Environments
# ------------------------------------------------------------------
env      = make_vec_env(FightEnv, n_envs=8)
eval_env = Monitor(FightEnv())

# ------------------------------------------------------------------
# Load checkpoint if resuming, otherwise start fresh
# ------------------------------------------------------------------
latest_checkpoint = CHECKPOINT_PREFIX + ".zip"

if os.path.exists(latest_checkpoint):
    print(f"Resuming from {latest_checkpoint} ...")
    model = PPO.load(latest_checkpoint, env=env)
else:
    print("Starting fresh against harder enemy...")
    model = PPO(
        "MlpPolicy",
        env,
        verbose=1,
        learning_rate=3e-4,
        n_steps=2048,
        batch_size=64,
        n_epochs=10,
        ent_coef=0.02,
        tensorboard_log="./tb_logs/",
        policy_kwargs=dict(net_arch=[64, 64])
    )

model.verbose = 1

# ------------------------------------------------------------------
# Callbacks
# ------------------------------------------------------------------
checkpoint_cb = CheckpointCallback(
    save_freq=50_000,
    save_path=".",
    name_prefix=CHECKPOINT_PREFIX,
    verbose=1
)

eval_cb = EvalCallback(
    eval_env,
    best_model_save_path=f"./{BEST_PATH}",
    log_path="./eval_logs/",
    eval_freq=25_000,
    n_eval_episodes=20,
    deterministic=True,
    verbose=1
)

# ------------------------------------------------------------------
# Train
# ------------------------------------------------------------------
model.learn(
    total_timesteps=3_000_000,
    reset_num_timesteps=True,
    callback=[checkpoint_cb, eval_cb]
)

model.save(SAVE_PATH)
print(f"Done! Saved {SAVE_PATH}.zip")
print(f"Best model saved to {BEST_PATH}/best_model.zip")