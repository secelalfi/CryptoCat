import struct
import random

# Read the encrypted file
with open("flag.enc", "rb") as f:
    seed = struct.unpack("<I", f.read(4))[0]  # Read the first 4 bytes (timestamp)
    encrypted_data = bytearray(f.read())  # Read the rest of the file

print(f"Extracted seed (timestamp): {seed}")  # Debugging step

# Initialize random with the extracted seed
random.seed(seed)

# Decryption process
for i in range(len(encrypted_data)):
    shift = random.randint(0, 7)  # Get the same shift value
    encrypted_data[i] = (encrypted_data[i] >> shift | encrypted_data[i] << (8 - shift)) & 0xFF  # Undo rotation
    encrypted_data[i] ^= random.randint(0, 255)  # XOR with the same random number

# Save the decrypted flag
with open("flag_decrypted", "wb") as f:
    f.write(encrypted_data)

print("Decryption complete. Check 'flag_decrypted'.")
