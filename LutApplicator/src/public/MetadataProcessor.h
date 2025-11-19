#pragma once


#include <string>

/**
 * @brief 专门负责在两个图像文件之间复制元数据（EXIF, IPTC, XMP等）的类。
 */
class MetadataProcessor {
public:
    MetadataProcessor() = default;
    ~MetadataProcessor() = default;

    /**
     * @brief 将源文件中的所有元数据复制到目标文件。
     * * @param sourcePath 元数据来源的原始图像文件路径。
     * @param destinationPath 待注入元数据的新生成图像文件路径。
     * @return 成功返回 true，失败返回 false。
     */
    bool copyMetadata(const std::string& sourcePath, const std::string& destinationPath);

    /**
     * @brief 获取最近一次操作的错误信息。
     */
    std::string getLastError() const { return m_lastError; }

private:
    std::string m_lastError;
};