import struct
import random

# Read the encrypted file
with open("flag.enc", "rb") as f:
    seed_bytes = f.read(4)  # First 4 bytes contain the seed
    encrypted_data = bytearray(f.read())  # Read the rest as encrypted content

# Extract the seed (timestamp) from the first 4 bytes
seed = struct.unpack("<I", seed_bytes)[0]
print(f"Extracted seed (timestamp): {seed}")

# Set the PRNG to the same state used during encryption
random.seed(seed)

# Decrypt the data
decrypted_data = bytearray()
for byte in encrypted_data:
    shift = random.randint(0, 7)  # Get the same shift used during encryption
    byte = (byte >> shift | byte << (8 - shift)) & 0xFF  # Reverse the shift
    byte ^= random.randint(0, 255)  # Reverse the XOR operation
    decrypted_data.append(byte)

# Save the decrypted flag
with open("flag_decrypted", "wb") as f:
    f.write(decrypted_data)

print("Decryption complete! Check the file: flag_decrypted")
