#pragma once
#include "FileChangeNotification.h"
#include <string>
#include <thread>
#include <atomic>
#include <functional>

typedef void* HANDLE;

class FolderWatcher {
public:
    FolderWatcher();
    ~FolderWatcher();

    /**
     * @brief 开始监听指定文件夹
     * @param directoryPath 文件夹路径
     * @param callback 回调函数
     * @param userData 用户数据指针
     */
    bool start(const std::wstring& directoryPath, FileChangeCallback callback, void* userData = nullptr);

    /**
     * @brief 停止监听
     */
    void stop();

    /**
     * @brief 检查是否正在运行
     */
    bool isRunning() const { return m_running; }

private:
    /**
     * @brief 监听循环（独立线程）
     */
    void monitorLoop();

private:
    std::wstring m_directoryPath;
    FileChangeCallback m_callback;
    void* m_userData;

    std::atomic<bool> m_running;
    std::thread m_watchThread;
    HANDLE m_hDir; // 目录句柄
};