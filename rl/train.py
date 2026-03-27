import os
from stable_baselines3 import PPO
from stable_baselines3.common.env_util import make_vec_env
from stable_baselines3.common.callbacks import CheckpointCallback, EvalCallback
from stable_baselines3.common.monitor import Monitor
from env import FightEnv

CHECKPOINT_PREFIX = "fight_agent_v3_checkpoint"
SAVE_PATH         = "fight_agent_v3"
BEST_PATH         = "fight_agent_v3_best"

env      = make_vec_env(FightEnv, n_envs=16)   # more envs = more diverse experience
eval_env = Monitor(FightEnv())

latest_checkpoint = None
for steps in [5_000_000, 4_000_000, 3_000_000, 2_000_000, 1_000_000, 500_000]:
    path = f"{CHECKPOINT_PREFIX}_{steps}_steps.zip"
    if os.path.exists(path):
        latest_checkpoint = path
        break

if latest_checkpoint:
    print(f"Resuming from {latest_checkpoint} ...")
    model = PPO.load(latest_checkpoint, env=env)
    resuming = True
else:
    print("Starting fresh...")
    model = PPO(
        "MlpPolicy",
        env,
        verbose=1,
        learning_rate=3e-4,
        n_steps=2048,
        batch_size=256,         # larger batch for stability
        n_epochs=10,
        ent_coef=0.05,          # higher entropy — stops early action collapse
        clip_range=0.2,
        vf_coef=0.5,
        max_grad_norm=0.5,
        tensorboard_log="./tb_logs/",
        policy_kwargs=dict(net_arch=[128, 128])  # bigger network for 17 obs
    )
    resuming = False

model.verbose = 1

checkpoint_cb = CheckpointCallback(
    save_freq=500_000,
    save_path=".",
    name_prefix=CHECKPOINT_PREFIX,
    verbose=1
)

eval_cb = EvalCallback(
    eval_env,
    best_model_save_path=f"./{BEST_PATH}",
    log_path="./eval_logs/",
    eval_freq=50_000,
    n_eval_episodes=30,
    deterministic=True,
    verbose=1
)

model.learn(
    total_timesteps=10_000_000,
    reset_num_timesteps=not resuming,   # don't restart LR schedule on resume
    callback=[checkpoint_cb, eval_cb]
)

model.save(SAVE_PATH)
print(f"Done! Saved {SAVE_PATH}.zip")