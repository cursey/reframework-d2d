#pragma once

#include <filesystem>
#include <string>
#include <tuple>

#include <d2d1.h>
#include <dwrite.h>
#include <dwrite_3.h>
#include <wrl.h>

#include "LruCache.hpp"

class D2DFont {
public:
    template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    D2DFont(ComPtr<IDWriteFactory5> dwrite, const std::string& family, int size, bool bold, bool italic);
    D2DFont(ComPtr<IDWriteFactory5> dwrite, std::filesystem::path filepath, const std::string& family, int size, bool bold, bool italic);

    ComPtr<IDWriteTextLayout> layout(const std::string& text);
    std::tuple<float, float> measure(const std::string& text);

private:
    ComPtr<IDWriteFactory5> m_dwrite{};
    ComPtr<IDWriteFontFile> m_fontFile{};
    ComPtr<IDWriteFontCollection1> m_fontCollection{};
    ComPtr<IDWriteTextFormat> m_format{};
    LruCache<std::string, ComPtr<IDWriteTextLayout>> m_layouts{100};
};