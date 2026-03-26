import numpy as np
from stable_baselines3 import PPO

model = PPO.load("fight_agent_v2_checkpoint_400000_steps")
policy = model.policy

layers = []
for name, param in policy.mlp_extractor.policy_net.named_parameters():
    layers.append((name, param.detach().numpy()))
for name, param in policy.action_net.named_parameters():
    layers.append(("action_" + name, param.detach().numpy()))

def arr_to_c(name, arr):
    flat = arr.flatten()
    vals = ", ".join(f"{v:.6f}f" for v in flat)
    return f"/* shape: {arr.shape} */\nstatic const float {name}[{arr.size}] = {{{vals}}};\n"

# Detect actual input size from first weight matrix shape
first_w = next(p for n,p in policy.mlp_extractor.policy_net.named_parameters() if 'weight' in n)
input_size = first_w.detach().numpy().shape[1]  # e.g. 16

with open("../src/ai/weights.h", "w") as f:
    f.write("#pragma once\n/* Auto-generated - do not edit */\n\n")
    f.write(f"#define NN_INPUT_SIZE  {input_size}\n")
    f.write("#define NN_HIDDEN_SIZE 64\n")
    f.write("#define NN_OUTPUT_SIZE 10\n\n")
    for name, arr in layers:
        f.write(arr_to_c(name.replace(".", "_"), arr))
        f.write("\n")
print(f"Wrote weights.h with NN_INPUT_SIZE={input_size}")