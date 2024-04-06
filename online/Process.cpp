#include "pch.h"
#include "Process.h"

DWORD EnumerateThreads(DWORD processId) {
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        std::cout << "CreateToolhelp32Snapshot failed" << std::endl;
        return 0;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    if (!Thread32First(hThreadSnap, &te32)) {
        std::cout << "Thread32First failed" << std::endl;
        CloseHandle(hThreadSnap);
        return 0;
    }

    std::cout << "Threads in process " << processId << ":" << std::endl;
    do {
        if (te32.th32OwnerProcessID == processId) {
            std::cout << "Thread ID: " << te32.th32ThreadID << std::endl;
            CloseHandle(hThreadSnap);
            return te32.th32ThreadID;
        }
    } while (Thread32Next(hThreadSnap, &te32));

    CloseHandle(hThreadSnap);
    return 0;
}