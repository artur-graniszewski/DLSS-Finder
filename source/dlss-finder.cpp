#include <filesystem>
#include <string>
#include <windows.h>
#include <shellapi.h>

#define MINIMUM_FILESIZE    32000000
#define ORIGINAL_FILENAME   L"nvngx_dlss.dll"
#define TARGET_FILENAME     L"_nvngx.dll"
#define SEARCH_DEPTH        3
#define ERROR_CAPTION       L"Fatal Error"
#define ERRNO_GENERAL_FAIL  99
#define ERRNO_INV_ARG       98
#define ERRNO_COPY_FAILED   4
#define ERRNO_INVALID_FILE  3
#define ERRNO_MISSING_FILE  2
#define ERRNO_INVALID_DIR   1
#define ERRNO_SUCCESS       0

namespace fs = std::filesystem;

bool reportSuccess = true;  // when set to true, shows the pop-up informing about success
bool reportErrors = true;   // when set to false, hides all error pop-ups if operation fails

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int ShowErrorMessage(const wchar_t* errorMessage, int errorCode) {
    if (reportErrors) {
        std::wstring fullMessage = std::wstring(errorMessage)
            + L"\r\n"
            + L"\r\nError code: " + std::to_wstring(errorCode)
            + L"\r\nCurrent path: " + fs::current_path().wstring();
        MessageBoxW(nullptr, fullMessage.c_str(), ERROR_CAPTION, MB_ICONERROR);
    }

    return errorCode;
}

bool searchFileInDirectory(const fs::path& dir, const std::wstring& filename, fs::path& foundPath) {
    for (const auto& entry : fs::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().filename() == filename) {
            foundPath = entry.path();
            return true;
        }
    }
    return false;
}

bool searchFile(const std::wstring& filename, fs::path& foundPath) {
    fs::path currentPath = fs::current_path();
    fs::path previousPath;

    // Search current directory and its subdirectories
    if (searchFileInDirectory(currentPath, filename, foundPath)) {
        return true;
    }

    // Search up to X levels above, including subdirectories
    for (int i = 0; i < SEARCH_DEPTH; ++i) {
        if (!currentPath.has_parent_path()) {
            break;
        }
        else {
            previousPath = currentPath;
            currentPath = currentPath.parent_path();
            auto currentFilename = currentPath.filename();

            if (currentFilename == L"Program Files"
                || currentFilename == L"Program Files (x86)"
                || currentFilename == L"steamapps"
                || currentFilename == L"Users"
                || currentFilename == L"Downloads" // we don't want to fetch anything random from the Internet... (also AVs would be unhappy with that)
                || !currentPath.has_parent_path()) {
                break;
            }

            if (searchFileInDirectory(currentPath, filename, foundPath)) {
                return true;
            }

            // Ensure we do not search the same directory again
            for (const auto& entry : fs::directory_iterator(currentPath)) {
                if (entry.is_directory() && entry != previousPath) {
                    if (searchFileInDirectory(entry.path(), filename, foundPath)) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

int copyFileToStartingDirectory(const std::wstring& filename) {
    fs::path foundPath;
    fs::path startingPath = fs::current_path();
    fs::path destination = startingPath / TARGET_FILENAME;

    if (startingPath.filename() != L"Win64" || startingPath.parent_path().filename() != L"Binaries") {
        // its not UE game, maybe at least it has the file in the cwd?
        fs::path dlssPath = startingPath / ORIGINAL_FILENAME;
        if (!fs::exists(dlssPath)) {
            return ShowErrorMessage(L"DLSS file not found in current directory and the game directory structure is unsupported", ERRNO_INVALID_DIR);
        }
    }
    // Check if the file already exists in the starting directory and is not a dummy file (which would be very small in size)
    // We could try to check the file properties instead, but every additional operation on a file may increase the risk of false positives reported by various AVs...
    if (fs::exists(destination) && std::filesystem::file_size(destination) > MINIMUM_FILESIZE) {
        // Yes, no further steps are needed
        if (reportSuccess) {
            MessageBoxW(nullptr, L"DLSS file is already installed", L"Success", MB_ICONINFORMATION);
        }
        return ERRNO_SUCCESS;
    }

    if (!searchFile(filename, foundPath)) {
        return ShowErrorMessage(L"DLSS file not found", ERRNO_MISSING_FILE);
    }
    else {
        try {
            if (std::filesystem::file_size(foundPath) < MINIMUM_FILESIZE) {
                return ShowErrorMessage(L"DLSS file is invalid", ERRNO_INVALID_FILE);
            }
            fs::copy_file(foundPath, destination, fs::copy_options::overwrite_existing);
            if (reportSuccess) {
                MessageBoxW(nullptr, L"DLSS file installed successfully", L"Success", MB_ICONINFORMATION);
            }
        }
        catch (const fs::filesystem_error& e) {
            return ShowErrorMessage(L"Error copying DLSS file", ERRNO_COPY_FAILED);
        }
    }

    return ERRNO_SUCCESS;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "DLSSFinder";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        "Searching for DLSS file...",   // Window text
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_CAPTION, // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, 350, 30,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
    );

    if (hwnd == NULL) {
        return ERRNO_GENERAL_FAIL;
    }

    ShowWindow(hwnd, SW_SHOWMINIMIZED);
    UpdateWindow(hwnd);

    // Run the message loop. (maybe I will reenable it when switching to multithreaded version of the code...)
    /*
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    */

    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    for (int i = 1; i < argc; ++i) { // Start from 1 to skip the program name
        std::wstring arg(argv[i]);

        if (arg == L"/s") {
            reportSuccess = false;
        }
        else if (arg == L"/q") {
            reportErrors = false;
        }
        else {
            // Show error window and the invalid argument
            return ShowErrorMessage((L"Invalid argument detected: " + arg + L"\r\n\r\nAvailable arguments:\r\n/q - do not report errors\r\n/s  - do not report success").c_str(), ERRNO_INV_ARG);
        }
    }

    return copyFileToStartingDirectory(ORIGINAL_FILENAME);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}
