from env import FightEnv

env = FightEnv()
obs, _ = env.reset()
print("obs:", obs)
print("obs shape:", obs.shape)

for action in [3, 3, 4, 0, 5]:
    obs, reward, term, trunc, _ = env.step(action)
    print(f"action={action}  reward={reward:.3f}  terminated={term}")

print("\nEnv looks good!" if True else "")