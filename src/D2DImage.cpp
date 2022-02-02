#include <stdexcept>

#include "D2DImage.hpp"

D2DImage::D2DImage(ComPtr<IWICImagingFactory> wic, ComPtr<ID2D1DeviceContext> context, std::filesystem::path filepath) {
    ComPtr<IWICBitmapDecoder> decoder{};

    if (FAILED(wic->CreateDecoderFromFilename(filepath.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder))) {
        throw std::runtime_error{"Failed to create WIC decoder"};
    }

    ComPtr<IWICBitmapFrameDecode> frame{};
    
    if (FAILED(decoder->GetFrame(0, &frame))) {
        throw std::runtime_error{"Failed to get frame from WIC decoder"};
    }

    ComPtr<IWICFormatConverter> converter{};

    if (FAILED(wic->CreateFormatConverter(&converter))) {
        throw std::runtime_error{"Failed to create WIC format converter"};
    }

    if (FAILED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut))) {
        throw std::runtime_error{"Failed to initialize WIC format converter"};
    }

    if (FAILED(context->CreateBitmapFromWicBitmap(converter.Get(), nullptr, &m_bitmap))) {
        throw std::runtime_error{"Failed to create D2D bitmap from WIC bitmap"};
    }

    m_size = m_bitmap->GetPixelSize();
}
