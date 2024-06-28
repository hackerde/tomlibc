import toml

t = dict()

for i in range(100000):
    t[f'k{i}'] = f"v{i}"

for i in range(1000):
    t[f'k{i}-{i}'] = dict()
    for j in range(100):
        t[f'k{i}-{i}'][f'k{i}-{i}-{j}'] = f"v{i}-{i}-{j}"

t['arr'] = []
for i in range(100000):
    t['arr'].append(f"{i}")

with open("keys.toml", 'w') as f:
    toml.dump(t, f)
