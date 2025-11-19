// MetadataProcessor.cpp
#include "MetadataProcessor.h"
#include <exiv2/exiv2.hpp>
#include <iostream>

bool MetadataProcessor::copyMetadata(const std::string& sourcePath, const std::string& destinationPath) {
    m_lastError.clear(); // 清空上次的错误信息

    try {
        //打开原始文件
        std::unique_ptr<Exiv2::Image> pSourceImage = Exiv2::ImageFactory::open(sourcePath);
        if (!pSourceImage) {
            m_lastError = "Exiv2 无法打开源文件或格式不支持: " + sourcePath;
            return false;
        }

        //打开目标文件
        std::unique_ptr<Exiv2::Image> pDestImage = Exiv2::ImageFactory::open(destinationPath);
        if (!pDestImage) {
            m_lastError = "Exiv2 无法打开目标文件或格式不支持: " + destinationPath;
            return false;
        }

        //读取原始文件的元数据
        pSourceImage->readMetadata();

        //将元数据块从源文件复制到目标文件对象
        pDestImage->setExifData(pSourceImage->exifData());

        //写入元数据
        pDestImage->writeMetadata();

    }
    catch (const Exiv2::Error& e) {
        // 捕获 Exiv2 抛出的任何异常
        m_lastError = "Exiv2 运行时错误: " + std::string(e.what());
        return false;
    }

    return true;
}