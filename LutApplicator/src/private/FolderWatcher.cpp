#include "FolderWatcher.h"
#include <windows.h>
#include <iostream>
#include <vector>

FolderWatcher::FolderWatcher()
    : m_callback(nullptr)
    , m_userData(nullptr)
    , m_running(false)
    , m_hDir(INVALID_HANDLE_VALUE)
{
}

FolderWatcher::~FolderWatcher() {
    stop();
}

bool FolderWatcher::start(const std::wstring& directoryPath, FileChangeCallback callback, void* userData) {
    if (m_running) {
        return false; // 已经在运行中
    }

    m_directoryPath = directoryPath;
    m_callback = callback;
    m_userData = userData;

    // 打开目录句柄
    m_hDir = CreateFileW(
        directoryPath.c_str(),
        FILE_LIST_DIRECTORY,                // 需要列出目录的权限
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, // 异步/目录标志
        NULL
    );

    if (m_hDir == INVALID_HANDLE_VALUE) {
        std::cerr << "无法打开目录进行监听: " << GetLastError() << std::endl;
        return false;
    }

    m_running = true;
    // 启动后台线程
    m_watchThread = std::thread(&FolderWatcher::monitorLoop, this);

    return true;
}

void FolderWatcher::stop() {
    if (!m_running) return;

    m_running = false;

    //取消 IO 操作，使得 ReadDirectoryChangesW 退出阻塞状态
    if (m_hDir != INVALID_HANDLE_VALUE) {
        CancelIoEx(m_hDir, NULL);
        CloseHandle(m_hDir);
        m_hDir = INVALID_HANDLE_VALUE;
    }

    if (m_watchThread.joinable()) {
        m_watchThread.join();
    }
}

void FolderWatcher::monitorLoop() {
    const int BUFFER_SIZE = 1024 * 64;  //缓冲区
    std::vector<BYTE> buffer(BUFFER_SIZE);
    DWORD bytesReturned = 0;
    OVERLAPPED overlapped = { 0 };

    // 创建一个事件用于等待
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    while (m_running) {
        // 重置 overlapped
        ResetEvent(overlapped.hEvent);

        // 监听文件名称变更、大小变更、最后写入时间变更
        BOOL success = ReadDirectoryChangesW(
            m_hDir,
            buffer.data(),
            static_cast<DWORD>(buffer.size()),
            FALSE, // 是否监听子目录 (FALSE = 仅当前目录)
            FILE_NOTIFY_CHANGE_FILE_NAME |
            FILE_NOTIFY_CHANGE_DIR_NAME |
            FILE_NOTIFY_CHANGE_LAST_WRITE |
            FILE_NOTIFY_CHANGE_SIZE,
            &bytesReturned,
            &overlapped,
            NULL
        );

        if (!success) {
            break;
        }

        //阻塞等待结果。
        if (GetOverlappedResult(m_hDir, &overlapped, &bytesReturned, TRUE)) {
            if (bytesReturned == 0) continue;

            // 解析数据
            FILE_NOTIFY_INFORMATION* pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer.data());

            while (true) {
                // 获取文件名
                std::wstring fileName(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));

                // 构造完整路径
                std::wstring fullPath = m_directoryPath + L"/" + fileName;

                FileAction action;
                bool isValidAction = true;

                switch (pNotify->Action) {
                case FILE_ACTION_ADDED:            action = FileAction::Added; break;
                case FILE_ACTION_REMOVED:          action = FileAction::Removed; break;
                case FILE_ACTION_MODIFIED:         action = FileAction::Modified; break;
                case FILE_ACTION_RENAMED_OLD_NAME: action = FileAction::RenamedOld; break;
                case FILE_ACTION_RENAMED_NEW_NAME: action = FileAction::RenamedNew; break;
                default: isValidAction = false; break;
                }

                if (isValidAction && m_callback) {
                    // 注意：Modified 事件在文件写入时可能会频繁触发多次
                    // 实际生产中通常需要做一个 "Debounce" (防抖) 处理
                    FileChangeEvent event(fullPath, action, 0); // Size 暂时无法直接从通知获取，需单独查询
                    m_callback(event, m_userData);
                }

                if (pNotify->NextEntryOffset == 0) break;
                pNotify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                    reinterpret_cast<BYTE*>(pNotify) + pNotify->NextEntryOffset
                    );
            }
        }
        else {
            // 操作失败或被取消
            break;
        }
    }

    CloseHandle(overlapped.hEvent);
}