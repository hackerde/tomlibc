import toml
import sys
import os

if len(sys.argv) < 2:
    print("Not enough arguments")
    sys.exit(1)

filename = sys.argv[1]

if not os.path.exists(filename):
    print("File not found")
    sys.exit(1)

with open(filename) as f:
    data = toml.load(f)

sys.exit(0)
