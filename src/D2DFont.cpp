#include <stdexcept>

#include "utf8.h"

#include "D2DFont.hpp"
#include "reframework/API.hpp"

D2DFont::D2DFont(ComPtr<IDWriteFactory5> dwrite, const std::string& family, int size, bool bold, bool italic)
    : m_dwrite{dwrite} {
    std::wstring wide_family{};
    utf8::utf8to16(family.begin(), family.end(), std::back_inserter(wide_family));

    if (FAILED(m_dwrite->CreateTextFormat(wide_family.c_str(), nullptr, bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
            italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size, L"en-us", &m_format))) {
        throw std::runtime_error{"Failed to create DWrite text format"};
    }
}

D2DFont::D2DFont(
    ComPtr<IDWriteFactory5> dwrite, std::filesystem::path filepath, const std::string& family, int size, bool bold, bool italic)
    : m_dwrite{dwrite} {
    ComPtr<IDWriteFontSetBuilder1> fontSetBuilder;
    if (FAILED(m_dwrite->CreateFontSetBuilder(&fontSetBuilder))) {
        throw std::runtime_error{"Failed to create DWrite font set builder"};
    }

    if (FAILED(m_dwrite->CreateFontFileReference(filepath.c_str(), nullptr, &m_fontFile))) {
        throw std::runtime_error{"Failed to create font file reference"};
    }

    /* BOOL supported;
    DWRITE_FONT_FILE_TYPE ftype;
    UINT32 facecount;
    m_fontFile->Analyze(&supported, &ftype, nullptr, &facecount);
    if (!supported) {
        throw std::runtime_error{"Unsupported font file"};
    }

    for (UINT32 i = 0; i < facecount; ++i) {
        ComPtr<IDWriteFontFaceReference> ffref;
        if (FAILED(m_dwrite->CreateFontFaceReference(m_fontFile.Get(), i, DWRITE_FONT_SIMULATIONS_NONE, &ffref))) {
            throw std::runtime_error{"Failed to create font face reference"};
        }
        if (FAILED(m_fontSetBuilder->AddFontFaceReference(ffref.Get()))) {
            throw std::runtime_error{"Failed to add font face reference"};
        }
    }*/

    fontSetBuilder->AddFontFile(m_fontFile.Get());

    ComPtr<IDWriteFontSet> fontSet;
    if (FAILED(fontSetBuilder->CreateFontSet(&fontSet))) {
        throw std::runtime_error{"Failed to create font set"};
    }

    if (FAILED(m_dwrite->CreateFontCollectionFromFontSet(fontSet.Get(), &m_fontCollection))) {
        throw std::runtime_error{"Failed to create font collection from font set"};
    }

    std::wstring wide_family;
    if (!family.empty()) {
        utf8::utf8to16(family.begin(), family.end(), std::back_inserter(wide_family));
    } else {
        for (auto i = 0; i < m_fontCollection->GetFontFamilyCount(); ++i) {
            ComPtr<IDWriteFontFamily1> fontFamily;
            if (FAILED(m_fontCollection->GetFontFamily(i, &fontFamily))) {
                throw std::runtime_error{"IDWriteFontCollection1::GetFontFamily failed"};
            }
            ComPtr<IDWriteLocalizedStrings> names;
            if (FAILED(fontFamily->GetFamilyNames(&names))) {
                throw std::runtime_error{"IDWriteFontFamily1::GetFamilyNames failed"};
            }
            UINT32 idx;
            BOOL exists;
            if (FAILED(names->FindLocaleName(L"en-us", &idx, &exists)) || !exists) {
                throw std::runtime_error{"Can't find en-us name for font family"};
            }
            UINT32 len;
            if (FAILED(names->GetStringLength(idx, &len))) {
                throw std::runtime_error{"IDWriteLocalizedStrings::GetStringLength failed"};
            }
            wide_family.resize(len + 1, L'\0');
            names->GetString(idx, wide_family.data(), wide_family.size());
        }
    }

    reframework::API::get()->log_info("[reframework-d2d] [D2DFont] Chose family %S for font %S", wide_family.c_str(), filepath.filename().c_str() );

    if (FAILED(m_dwrite->CreateTextFormat(wide_family.c_str(), m_fontCollection.Get(),
            bold ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,
            italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size, L"en-us", &m_format))) {
        throw std::runtime_error{"Failed to create DWrite text format"};
    }
}

D2DFont::ComPtr<IDWriteTextLayout> D2DFont::layout(const std::string& text) {
    if (auto l = m_layouts.get(text)) {
        return (*l).get();
    }

    ComPtr<IDWriteTextLayout> l{};
    std::wstring wide_text{};

    utf8::utf8to16(text.begin(), text.end(), std::back_inserter(wide_text));

    if (FAILED(m_dwrite->CreateTextLayout(wide_text.c_str(), wide_text.size(), m_format.Get(), 10000.0f, 10000.0f, &l))) {
        throw std::runtime_error{"Failed to create dwrite text layout"};
    }

    m_layouts.put(text, l);

    return l;
}

std::tuple<float, float> D2DFont::measure(const std::string& text) {
    DWRITE_TEXT_METRICS metrics{};

    layout(text)->GetMetrics(&metrics);

    return std::make_tuple(metrics.width, metrics.height);
}