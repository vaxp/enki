#include "core/process.hpp"
#include <iostream>
#include <array>
#include <memory>
#include <sstream>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace enki::cli {

#if defined(_WIN32)
static std::wstring utf8ToWide(const std::string& str) {
    if (str.empty()) return L"";
    int count = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0);
    if (count <= 0) return L"";
    std::wstring wstr(count, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), &wstr[0], count);
    return wstr;
}

ProcessResult Process::run(
    const std::string& command_line,
    const std::string& working_dir,
    bool stream_output,
    LineCallback on_stdout_line,
    const std::vector<std::pair<std::string, std::string>>& extra_env
) {
    ProcessResult result;

    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) {
        result.exit_code = -1;
        result.stderr_str = "Failed to create pipe";
        return result;
    }
    SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW siStartInfo;
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = hChildStd_OUT_Wr;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    PROCESS_INFORMATION piProcInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    std::wstring wcmd = utf8ToWide(command_line);
    std::wstring wwd = working_dir.empty() ? L"" : utf8ToWide(working_dir);
    LPCWSTR lpWorkingDir = wwd.empty() ? NULL : wwd.c_str();

    // Prepare command line (CreateProcessW requires a mutable buffer)
    std::vector<wchar_t> cmdBuf(wcmd.begin(), wcmd.end());
    cmdBuf.push_back(L'\0');

    BOOL bSuccess = CreateProcessW(
        NULL,
        cmdBuf.data(),
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        lpWorkingDir,
        &siStartInfo,
        &piProcInfo
    );

    CloseHandle(hChildStd_OUT_Wr);

    if (!bSuccess) {
        CloseHandle(hChildStd_OUT_Rd);
        result.exit_code = -1;
        result.stderr_str = "Failed to launch process: " + command_line;
        return result;
    }

    // Read output in real time
    DWORD dwRead;
    CHAR chBuf[4096];
    std::string current_line;

    while (ReadFile(hChildStd_OUT_Rd, chBuf, sizeof(chBuf) - 1, &dwRead, NULL) && dwRead != 0) {
        chBuf[dwRead] = '\0';
        result.stdout_str.append(chBuf, dwRead);

        if (stream_output || on_stdout_line) {
            for (DWORD i = 0; i < dwRead; ++i) {
                char c = chBuf[i];
                if (c == '\n') {
                    if (!current_line.empty() && current_line.back() == '\r') {
                        current_line.pop_back();
                    }
                    if (stream_output) {
                        std::cout << current_line << "\n";
                    }
                    if (on_stdout_line) {
                        on_stdout_line(current_line);
                    }
                    current_line.clear();
                } else {
                    current_line.push_back(c);
                }
            }
        }
    }

    if (!current_line.empty()) {
        if (stream_output) {
            std::cout << current_line << "\n";
        }
        if (on_stdout_line) {
            on_stdout_line(current_line);
        }
    }

    CloseHandle(hChildStd_OUT_Rd);

    WaitForSingleObject(piProcInfo.hProcess, INFINITE);

    DWORD exitCode = 0;
    if (GetExitCodeProcess(piProcInfo.hProcess, &exitCode)) {
        result.exit_code = static_cast<int>(exitCode);
    }

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    return result;
}

bool Process::spawnDetached(const std::string& command_line, const std::string& working_dir) {
    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    std::wstring wcmd = utf8ToWide(command_line);
    std::vector<wchar_t> cmdBuf(wcmd.begin(), wcmd.end());
    cmdBuf.push_back(L'\0');

    std::wstring wwd = working_dir.empty() ? L"" : utf8ToWide(working_dir);
    LPCWSTR lpWorkingDir = wwd.empty() ? NULL : wwd.c_str();

    BOOL success = CreateProcessW(
        NULL,
        cmdBuf.data(),
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_PROCESS_GROUP | DETACHED_PROCESS,
        NULL,
        lpWorkingDir,
        &si,
        &pi
    );

    if (success) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    return false;
}

#else
// POSIX implementation
ProcessResult Process::run(
    const std::string& command_line,
    const std::string& working_dir,
    bool stream_output,
    LineCallback on_stdout_line,
    const std::vector<std::pair<std::string, std::string>>&
) {
    ProcessResult result;
    std::string cmd = command_line + " 2>&1";
    if (!working_dir.empty()) {
        cmd = "cd \"" + working_dir + "\" && " + cmd;
    }

    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        result.exit_code = -1;
        result.stderr_str = "Failed to run popen";
        return result;
    }

    char buffer[4096];
    std::string current_line;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result.stdout_str += buffer;
        std::string s = buffer;
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
        if (stream_output) {
            std::cout << s << "\n";
        }
        if (on_stdout_line) {
            on_stdout_line(s);
        }
    }

    int st = pclose(pipe);
    result.exit_code = WEXITSTATUS(st);
    return result;
}

bool Process::spawnDetached(const std::string& command_line, const std::string& working_dir) {
    pid_t pid = fork();
    if (pid == 0) {
        if (!working_dir.empty()) {
            if (chdir(working_dir.c_str()) != 0) {}
        }
        execl("/bin/sh", "sh", "-c", command_line.c_str(), (char*)NULL);
        _exit(127);
    }
    return pid > 0;
}
#endif

} // namespace enki::cli
