#include "Lut3D.h"
#include <fstream>
#include <sstream>
#include <iostream>

//线性插值，t 是权重 (0.0 - 1.0)
inline float lerp(float v0, float v1, float t) {
    return v0 + t * (v1 - v0);
}

// 辅助函数：结构体插值
inline RGB lerpRGB(const RGB& v0, const RGB& v1, float t) {
    return {
        lerp(v0.r, v1.r, t),
        lerp(v0.g, v1.g, t),
        lerp(v0.b, v1.b, t)
    };
}

bool Lut3D::load(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) return false;

    m_table.clear();
    m_size = 0;

    std::string line;
    while (std::getline(file, line)) {
        // 跳过注释
        if (line.empty() || line[0] == '#') continue;

        // 解析尺寸
        if (line.find("LUT_3D_SIZE") != std::string::npos) {
            std::stringstream ss(line);
            std::string temp;
            ss >> temp >> m_size;
            // 预分配内存 (Size * Size * Size)
            m_table.reserve(m_size * m_size * m_size);
            continue;
        }

        // 解析 RGB 数据行
        // 只要开头是数字或负号，就认为是数据行
        if (isdigit(line[0]) || line[0] == '-' || line[0] == '.') {
            float r, g, b;
            std::stringstream ss(line);
            if (ss >> r >> g >> b) {
                m_table.push_back({ r, g, b });
            }
        }
    }

    return m_size > 0 && m_table.size() == (m_size * m_size * m_size);
}


// 三线插值算法
RGB Lut3D::apply(float r, float g, float b) const {
    if (m_size == 0) return { r, g, b };

    //映射坐标：将 0.0-1.0 映射到 LUT 的索引空间 (0 到 size-1)
    float mapR = r * (m_size - 1);
    float mapG = g * (m_size - 1);
    float mapB = b * (m_size - 1);

    // 计算整数部分 (基准索引)
    // 必须限制在 [0, size-2] 范围内，防止数组越界
    int indexR = std::clamp((int)mapR, 0, m_size - 2);
    int indexG = std::clamp((int)mapG, 0, m_size - 2);
    int indexB = std::clamp((int)mapB, 0, m_size - 2);

    // 计算小数部分 (插值权重)
    float deltaR = mapR - indexR;
    float deltaG = mapG - indexG;
    float deltaB = mapB - indexB;

    // 4. 获取 8 个邻近顶点的颜色值
    // .cube 格式通常是 Red 变化最快，然后是 Green，最后是 Blue
    // Index = r + g*size + b*size*size
    auto getIndex = [&](int r, int g, int b) {
        return r + (g * m_size) + (b * m_size * m_size);
        };

    const RGB& c000 = m_table[getIndex(indexR, indexG, indexB)];
    const RGB& c100 = m_table[getIndex(indexR + 1, indexG, indexB)];
    const RGB& c010 = m_table[getIndex(indexR, indexG + 1, indexB)];
    const RGB& c110 = m_table[getIndex(indexR + 1, indexG + 1, indexB)];
    const RGB& c001 = m_table[getIndex(indexR, indexG, indexB + 1)];
    const RGB& c101 = m_table[getIndex(indexR + 1, indexG, indexB + 1)];
    const RGB& c011 = m_table[getIndex(indexR, indexG + 1, indexB + 1)];
    const RGB& c111 = m_table[getIndex(indexR + 1, indexG + 1, indexB + 1)];

    // 5. 开始插值
    // 第一步：沿着 R 轴，把 8 个点压缩成 4 个
    RGB c00 = lerpRGB(c000, c100, deltaR);
    RGB c10 = lerpRGB(c010, c110, deltaR);
    RGB c01 = lerpRGB(c001, c101, deltaR);
    RGB c11 = lerpRGB(c011, c111, deltaR);

    // 第二步：沿着 G 轴，把 4 个点压缩成 2 个
    RGB c0 = lerpRGB(c00, c10, deltaG);
    RGB c1 = lerpRGB(c01, c11, deltaG);

    // 第三步：沿着 B 轴，把 2 个点压缩成最终结果
    return lerpRGB(c0, c1, deltaB);
}