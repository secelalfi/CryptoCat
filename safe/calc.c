#include <stdio.h>
#include <string.h>

// Function to check the password
void check_password(char *input) {
    char saved_password[] = "secret123"; // Correct password
    char buffer[16]; // Small buffer vulnerable to overflow
    int access_granted = 0; // Flag to indicate access

    // Copy user input into the buffer without checking length
    strcpy(buffer, input);

    // Compare user input with the saved password
    if (strcmp(buffer, saved_password) == 0) {
        access_granted = 1; // Grant access if passwords match
    }

    // Check if access is granted
    if (access_granted) {
        printf("Access granted!\n");
    } else {
        printf("Access denied!\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <password>\n", argv[0]);
        return 1;
    }

    // Call the vulnerable function with user input
    check_password(argv[1]);

    return 0;
}