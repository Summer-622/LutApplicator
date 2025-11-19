#pragma once

#include <string>
#include <vector>
#include <turbojpeg.h>
#include <memory>

struct TjFreeDeleter {
    void operator()(unsigned char* ptr) const {
        tjFree(ptr);
    }
};

class ImageProcessor {
public:
    /**
     * @brief 构造函数：初始化 TurboJPEG "句柄"
     */
    ImageProcessor();

    /**
     * @brief 析构函数：释放资源
     */
    ~ImageProcessor();



    /**
     * @brief 从磁盘加载JPG文件
     * @param filePath JPG文件路径
     */
    bool load(const std::string& filePath);

    /**
     * @brief 将内存中的图像数据保存为JPG文件
     * @param filePath 保存路径
     * @param quality 压缩质量
     */
    bool save(const std::string& filePath, int quality = 90);

    /**
     * @brief 获取图像像素数据的“指针”
     * 格式为 [R, G, B, R, G, B, ...]
     */
    unsigned char* getPixelData() const { return m_pixelData.get(); }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    /**
     * @brief 获取色彩通道数（比如 3 = RGB）
     */
    int getComponents() const { return m_components; }

    /**
     * @brief 如果操作失败，获取错误信息
     */
    std::string getLastError() const { return m_lastError; }

private:
    /**
     * @brief 释放旧的像素数据，为加载新图做准备
     */
    void cleanup();

	std::unique_ptr<unsigned char[], TjFreeDeleter> m_pixelData; // 指向[R,G,B,R,G,B...]的内存
    int m_width;
    int m_height;
    int m_components;           // 色彩通道数
    std::string m_lastError;    // 存放错误信息

    tjhandle m_decompressHandle; // TurboJPEG 解压句柄
    tjhandle m_compressHandle;   // TurboJPEG 压缩句柄

    ImageProcessor(const ImageProcessor&) = delete;
    ImageProcessor& operator=(const ImageProcessor&) = delete;
    ImageProcessor(ImageProcessor&&) = delete;
    ImageProcessor& operator=(ImageProcessor&&) = delete;
};
