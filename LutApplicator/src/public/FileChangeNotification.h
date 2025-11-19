#pragma once
#include <string>



enum class FileAction {
    Added,           // 文件添加
    Removed,         // 文件删除
    Modified,        // 文件修改
    RenamedOld,      // 文件重命名旧名
    RenamedNew       // 文件重命名新名
};

struct FileChangeEvent {
    std::wstring filePath;  // 文件路径
    FileAction action;       // 文件操作类型
    uint64_t fileSize;      // 文件大小（字节）

    FileChangeEvent(const std::wstring& path, FileAction act, uint64_t size = 0)
        : filePath(path), action(act), fileSize(size) {
    }
};

// 文件变化回调函数类型
typedef void (*FileChangeCallback)(const FileChangeEvent& event, void* userData);
