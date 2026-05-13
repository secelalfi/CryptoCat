import struct

with open("flag.enc", "rb") as f:
    seed_bytes = f.read(4)
    seed = struct.unpack("<I", seed_bytes)[0]

print(f"Extracted seed: {seed}")
