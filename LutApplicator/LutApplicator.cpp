// LutApplicator.cpp: 定义应用程序的入口点。
//

#include "LutApplicator.h"
#include <iostream>
#include <filesystem>
#include <thread>

float toFloat[256]; //预计算 0-255 到 0.0-1.0 的映射，避免在千万次循环里做除法

namespace fs = std::filesystem;

bool runPipeline(const std::string& sourcePath, const std::string& outputPath, int quality) {

    std::thread thread1 = std::thread([&]() {
        for (int i = 0; i < 256; ++i) {
            toFloat[i] = static_cast<float>(i) / 255.0f;
        }
     });//多线程

    std::cout << "------------------------------------------------" << std::endl;
    std::cout << ">>> 开始处理文件: " << sourcePath << std::endl;

    ImageProcessor pixelProcessor;

    if (!pixelProcessor.load(sourcePath)) {
        std::cerr << "错误: 加载源图像失败: " << pixelProcessor.getLastError() << std::endl;
        return false;
    }
    std::cout << "尺寸: " << pixelProcessor.getWidth() << "x" << pixelProcessor.getHeight() << std::endl;

    Lut3D lut;
    std::string lutPath = "D:/S5/luts/std pt160h new.cube"; // 暂时硬编码
    std::cout << "正在加载 LUT: " << lutPath << std::endl;
    if (!lut.load(lutPath)) {
        std::cerr << "错误: LUT 加载失败！" << std::endl;
        return false;
    }

    std::cout << "正在应用 LUT..." << std::endl;
    unsigned char* pixels = pixelProcessor.getPixelData();
    int width = pixelProcessor.getWidth();
    int height = pixelProcessor.getHeight();
    int channels = 3; // RGB

    thread1.join();

    for (int i = 0; i < width * height; ++i) {
        int idx = i * channels;

        // A. 获取原始像素 (归一化)
        float r = toFloat[pixels[idx + 0]];
        float g = toFloat[pixels[idx + 1]];
        float b = toFloat[pixels[idx + 2]];

        // B. 计算插值
        RGB out = lut.apply(r, g, b);

        // C. 写回像素 (反归一化 + 限制范围)
        // 加 0.5f 是为了四舍五入
        pixels[idx + 0] = static_cast<unsigned char>(std::clamp(out.r * 255.0f + 0.5f, 0.0f, 255.0f));
        pixels[idx + 1] = static_cast<unsigned char>(std::clamp(out.g * 255.0f + 0.5f, 0.0f, 255.0f));
        pixels[idx + 2] = static_cast<unsigned char>(std::clamp(out.b * 255.0f + 0.5f, 0.0f, 255.0f));
    }

    std::string tempPath = outputPath + ".tmp_lut_proc";

    std::cout << "保存临时文件: " << tempPath << std::endl;
    if (!pixelProcessor.save(tempPath, quality)) {
        std::cerr << "错误: 保存临时文件失败: " << pixelProcessor.getLastError() << std::endl;
        // 确保文件关闭，但此处不再需要清理像素，因为它会被析构函数管理。
        return false;
    }

    //元数据协调
    MetadataProcessor metaProcessor;
    std::cout << "正在复制元数据..." << std::endl;
    if (!metaProcessor.copyMetadata(sourcePath, tempPath)) {
        std::cerr << "错误: 元数据复制失败: " << metaProcessor.getLastError() << std::endl;
        fs::remove(tempPath); // 失败了，删除临时文件
        return false;
    }

    //命名和清理
    try {
        fs::remove(outputPath); // 确保旧文件被删除
        fs::rename(tempPath, outputPath); // 临时文件重命名为最终文件
        std::cout << ">>> 成功处理并保存到: " << outputPath << std::endl;
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "错误: 文件重命名/清理失败: " << e.what() << std::endl;
        fs::remove(tempPath);
        return false;
    }

    return true;
}

int main()
{
    std::string source = "D:/S5/test/P1011157.jpg"; // 输入路径
    std::string output = "D:/S5/testout/result.jpg"; // 输出路径
      
	runPipeline(source, output, 90);

}
