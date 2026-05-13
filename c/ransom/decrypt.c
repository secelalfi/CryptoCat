#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

#define AES_BLOCK_SIZE 16
#define KEY_SIZE 32  // AES-256 requires a 32-byte key

// Function to convert hex string to binary
void hex_to_bytes(const char *hex, unsigned char *bytes, size_t len) {
    for (size_t i = 0; i < len; i++) {
        sscanf(hex + 2 * i, "%2hhx", &bytes[i]);
    }
}

// Function to decrypt files using AES-256
void decrypt_file(const char *filename, unsigned char *key) {
    FILE *input = fopen(filename, "rb");
    if (!input) {
        printf("Error opening file: %s\n", filename);
        return;
    }

    // Read IV from file
    unsigned char iv[AES_BLOCK_SIZE];
    if (fread(iv, 1, AES_BLOCK_SIZE, input) != AES_BLOCK_SIZE) {
        printf("Error reading IV from file: %s\n", filename);
        fclose(input);
        return;
    }

    // Generate output filename
    char output_filename[MAX_PATH];
    strncpy(output_filename, filename, MAX_PATH);
    char *ext = strstr(output_filename, ".locked");
    if (ext) {
        *ext = '\0';
    } else {
        printf("File does not have .locked extension: %s\n", filename);
        fclose(input);
        return;
    }

    FILE *output = fopen(output_filename, "wb");
    if (!output) {
        printf("Error creating output file: %s\n", output_filename);
        fclose(input);
        return;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    unsigned char buffer[1024 + AES_BLOCK_SIZE];
    unsigned char decrypted[1024];
    int len, outlen;

    while ((len = fread(buffer, 1, sizeof(buffer), input)) > 0) {
        if (!EVP_DecryptUpdate(ctx, decrypted, &outlen, buffer, len)) {
            printf("Error decrypting file: %s\n", filename);
            fclose(input);
            fclose(output);
            EVP_CIPHER_CTX_free(ctx);
            remove(output_filename);
            return;
        }
        fwrite(decrypted, 1, outlen, output);
    }

    if (!EVP_DecryptFinal_ex(ctx, decrypted, &outlen)) {
        printf("Error finalizing decryption for file: %s\n", filename);
        fclose(input);
        fclose(output);
        EVP_CIPHER_CTX_free(ctx);
        remove(output_filename);
        return;
    }
    fwrite(decrypted, 1, outlen, output);

    EVP_CIPHER_CTX_free(ctx);
    fclose(input);
    fclose(output);

    remove(filename);
    printf("Decrypted: %s -> %s\n", filename, output_filename);
}

// Function to decrypt all .locked files in directory
void decrypt_directory(const char *directory, unsigned char *key) {
    WIN32_FIND_DATA findData;
    char searchPath[MAX_PATH];
    snprintf(searchPath, MAX_PATH, "%s\\*.locked", directory);

    HANDLE hFind = FindFirstFile(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No .locked files found in directory: %s\n", directory);
        return;
    }

    do {
        if (strcmp(findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) continue;

        char full_path[MAX_PATH];
        snprintf(full_path, MAX_PATH, "%s\\%s", directory, findData.cFileName);

        decrypt_file(full_path, key);
    } while (FindNextFile(hFind, &findData) != 0);

    FindClose(hFind);
}

int main() {
    char key_hex[65]; // 64 chars + null terminator
    unsigned char aes_key[KEY_SIZE];

    printf("Enter AES-256 key (64 hex characters): ");
    scanf("%64s", key_hex);

    if (strlen(key_hex) != 64) {
        printf("Invalid key length! Expected 64 hex characters.\n");
        return 1;
    }

    hex_to_bytes(key_hex, aes_key, KEY_SIZE);

    char current_dir[MAX_PATH];
    if (GetCurrentDirectory(MAX_PATH, current_dir) == 0) {
        printf("Error getting current directory!\n");
        return 1;
    }

    decrypt_directory(current_dir, aes_key);

    printf("Decryption complete!\n");
    return 0;
}
