#include <stdexcept>

#include <utf8.h>

#include "D2DFont.hpp"

D2DFont::D2DFont(ComPtr<IDWriteFactory> dwrite, std::string name, int size, bool bold, bool italic)
    : m_dwrite{dwrite},
    m_name{std::move(name)},
    m_size{size},
    m_bold{bold},
    m_italic{italic}
{
    std::wstring wide_name{};
    utf8::utf8to16(m_name.begin(), m_name.end(), std::back_inserter(wide_name));

    if (FAILED(m_dwrite->CreateTextFormat(wide_name.c_str(), nullptr, m_bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
            m_italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size, L"en-us", &m_text_format))) {
        throw std::runtime_error{"Failed to create DWrite text format"};
    }

}

D2DFont::ComPtr<IDWriteTextLayout> D2DFont::layout(const std::string& text) {
    if (auto l = m_text_layouts.get(text)) {
        return (*l).get();
    }

    ComPtr<IDWriteTextLayout> l{};
    std::wstring wide_text{};

    utf8::utf8to16(text.begin(), text.end(), std::back_inserter(wide_text));

    if (FAILED(m_dwrite->CreateTextLayout(wide_text.c_str(), wide_text.size(), m_text_format.Get(), 10000.0f, 10000.0f, &l))) {
        throw std::runtime_error{"Failed to create dwrite text layout"};
    }

    m_text_layouts.put(text, l);

    return l;
}

std::tuple<float, float> D2DFont::measure(const std::string& text) {
    DWRITE_TEXT_METRICS metrics{};

    layout(text)->GetMetrics(&metrics);

    return std::make_tuple(metrics.width, metrics.height);
}