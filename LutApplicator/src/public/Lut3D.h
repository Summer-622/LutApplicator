#pragma once
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

struct RGB {
    float r, g, b;
};

class Lut3D {
public:
    Lut3D() : m_size(0) {}

    bool load(const std::string& filePath);

    RGB apply(float r, float g, float b) const;

    bool isValid() const { return m_size > 0 && !m_table.empty(); }

private:
    int m_size; // LUT 的尺寸
    std::vector<RGB> m_table; // 存储所有的颜色点
};