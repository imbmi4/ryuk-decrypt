#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <vector>

#define TARGET_FILE L"C:\\Users\\Alpha\\Desktop\\decoy.txt"

// 안전 프로세스 제외
bool isSafeProcess(const std::wstring& procName) {
    std::wstring lowerName = procName;
    for (auto& c : lowerName) c = towlower(c);
    return (lowerName == L"explorer.exe" || lowerName == L"notepad.exe");
}

int wmain() {
    std::wstring directory = L"C:\\Users\\Alpha\\Desktop";
    HANDLE hDir = CreateFile(
        directory.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        NULL
    );

    if (hDir == INVALID_HANDLE_VALUE) {
        std::wcerr << L"[x] Failed to open directory handle.\n";
        return 1;
    }

    std::wcout << L"[*] Monitoring started for: " << TARGET_FILE << L"\n";

    char buffer[1024];
    DWORD bytesReturned;
    std::wstring originalName = L"decoy.txt";

    while (true) {
        if (ReadDirectoryChangesW(
            hDir,
            &buffer,
            sizeof(buffer),
            FALSE,
            FILE_NOTIFY_CHANGE_FILE_NAME,
            &bytesReturned,
            NULL,
            NULL)) {

            FILE_NOTIFY_INFORMATION* fni = (FILE_NOTIFY_INFORMATION*)buffer;
            std::wstring fileName(fni->FileName, fni->FileNameLength / sizeof(WCHAR));

            if (fileName != originalName) {
                std::wstring fullPath = directory + L"\\" + fileName;
                std::wcout << L"[!] File name changed detected: " << TARGET_FILE
                    << L" -> " << fullPath << L"\n";

                // handle.exe 호출
                std::wstring command = L"handle.exe -accepteula " + fullPath;
                FILE* pipe = _wpopen(command.c_str(), L"r");
                if (!pipe) {
                    std::wcerr << L"[x] Failed to run handle.exe\n";
                    break;
                }

                wchar_t line[512];
                while (fgetws(line, sizeof(line) / sizeof(wchar_t), pipe)) {
                    std::wstring sline(line);
                    if (sline.find(L":") != std::wstring::npos) {
                        // 예:  pid: process_name
                        size_t pidPos = sline.find(L":");
                        std::wstring pidStr = sline.substr(0, pidPos);
                        std::wstring procName = sline.substr(pidPos + 1);
                        if (!isSafeProcess(procName))
                            std::wcout << L"[+] Process holding file: " << procName << L" (PID " << pidStr << L")\n";
                    }
                }
                _pclose(pipe);

                std::wcout << L"[*] Detection complete. Exiting monitor.\n";
                break;
            }
        }
        Sleep(100);
    }

    CloseHandle(hDir);
    return 0;
}
