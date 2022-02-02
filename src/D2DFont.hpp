#pragma once

#include <string>
#include <tuple>

#include <d2d1.h>
#include <dwrite.h>
#include <wrl.h>

#include "LruCache.hpp"

class D2DFont {
public:
    template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    D2DFont(ComPtr<IDWriteFactory> dwrite, const std::string& name, int size, bool bold, bool italic);

    ComPtr<IDWriteTextLayout> layout(const std::string& text);
    std::tuple<float, float> measure(const std::string& text);

private:
    ComPtr<IDWriteFactory> m_dwrite{};
    ComPtr<IDWriteTextFormat> m_format{};
    LruCache<std::string, ComPtr<IDWriteTextLayout>> m_layouts{100};
};