#include <ImageProcessor.h>
#include <fstream>
#include "Lut3D.h"

ImageProcessor::ImageProcessor()
    : m_pixelData(nullptr),
    m_width(0),
    m_height(0),
    m_components(0),
    m_decompressHandle(nullptr),
    m_compressHandle(nullptr)
{
    // 初始化 TurboJPEG 句柄
    m_decompressHandle = tjInitDecompress();
    m_compressHandle = tjInitCompress();

    if (!m_decompressHandle || !m_compressHandle) {
        m_lastError = "Failed to initialize TurboJPEG handles.";
    }
}

ImageProcessor::~ImageProcessor()
{
    cleanup();
    if (m_decompressHandle) {
        tjDestroy(m_decompressHandle);
    }
    if (m_compressHandle) {
        tjDestroy(m_compressHandle);
    }
}

bool ImageProcessor::load(const std::string& filePath)
{
    if (!m_decompressHandle) {
        m_lastError = "Decompress handle is not initialized.";
        return false;
	}

    cleanup();

    std::ifstream file(filePath, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        m_lastError = "Failed to open file: " + filePath;
        return false;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> jpegBuffer(fileSize);

	// 读取文件内容到缓冲区
    if (!file.read(reinterpret_cast<char*>(jpegBuffer.data()), fileSize)) {
        m_lastError = "Failed to read file: " + filePath;
        return false;
    }

    file.close();

    int width, height, subsamp, colorspace;

    // 先“读取头部”，获取图像尺寸
    int result = tjDecompressHeader3(m_decompressHandle,
        jpegBuffer.data(),
        fileSize,
        &width, &height, &subsamp, &colorspace);
    if (result != 0) {
        m_lastError = tj3GetErrorStr(m_decompressHandle);
        return false;
    }

    

    //RGB 一个像素3字节
    m_width = width;
    m_height = height;
    m_components = 3;

    int pixelSize = m_width * m_height * m_components;
    if (m_pixelData.get()) tjFree(m_pixelData.get()); // 确保释放旧的
    m_pixelData.reset(reinterpret_cast<unsigned char*>(tjAlloc(pixelSize)));

    result = tj3Decompress8(m_decompressHandle,
        jpegBuffer.data(),
        fileSize,
        m_pixelData.get(),
        0, // pitch = 0 (自动)
        TJPF_RGB); // 格式

    if (result != 0) {
        m_lastError = tj3GetErrorStr(m_decompressHandle);
        cleanup();
        return false;
    }

	return true;
}


bool ImageProcessor::save(const std::string& filePath, int quality)
{
    if (!m_compressHandle) {
        m_lastError = "Compress handle is not initialized.";
        return false;
    }
    if (!m_pixelData || m_width == 0 || m_height == 0) {
        m_lastError = "No pixel data to save.";
        return false;
    }

    std::ofstream file(filePath, std::ios::binary);  
    if (!file.is_open()) {
        m_lastError = "Failed to open file for writing: " + filePath;
        return false;
    }

    unsigned char* compressedBuffer = nullptr; 
	size_t jpegSize = 0;

    tj3Set(m_compressHandle, TJPARAM_QUALITY, quality);
    tj3Set(m_compressHandle, TJPARAM_SUBSAMP, TJSAMP_444); // 最高质量采样

    int result = tj3Compress8(m_compressHandle,
        m_pixelData.get(),
        m_width, 0, m_height,
        TJPF_RGB,
        &compressedBuffer,
        &jpegSize);

    if (result != 0) {
        m_lastError = tj3GetErrorStr(m_compressHandle);
        file.close();
        if (compressedBuffer) tjFree(compressedBuffer);
        return false;
    }

    file.write(reinterpret_cast<const char*>(compressedBuffer), jpegSize);
    file.close();

    tjFree(compressedBuffer);

    return true;
}

void ImageProcessor::cleanup()
{
    m_pixelData.reset();
	m_components = 0;
    m_width = 0;
    m_height = 0;
}
