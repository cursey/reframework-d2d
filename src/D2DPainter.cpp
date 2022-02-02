#include <stdexcept>

#include <utf8.h>

#include "D2DPainter.hpp"

D2DPainter::D2DPainter(ID3D11Device* device, IDXGISurface* surface) {
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2d1.GetAddressOf()))) {
        throw std::runtime_error{"Failed to create D2D factory"};
    }

    ComPtr<IDXGIDevice> dxgi_device{};

    if (FAILED(device->QueryInterface(dxgi_device.GetAddressOf()))) {
        throw std::runtime_error{"Failed to query DXGI device"};
    }

    if (FAILED(m_d2d1->CreateDevice(dxgi_device.Get(), m_device.GetAddressOf()))) {
        throw std::runtime_error{"Failed to create D2D device"};
    }

    if (FAILED(m_device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, m_context.GetAddressOf()))) {
        throw std::runtime_error{"Failed to create D2D device context"};
    }

    if (FAILED(surface->GetDesc(&m_rt_desc))) {
        throw std::runtime_error{"Failed to get DXGI surface description"};
    }

    if (FAILED(m_context->CreateBitmapFromDxgiSurface(surface, nullptr, &m_rt))) {
        throw std::runtime_error{"Failed to create D2D render target from dxgi surface"};
    }

    if (FAILED(m_context->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_brush))) {
        throw std::runtime_error{"Failed to create D2D brush"};
    }

    if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dwrite))) {
        throw std::runtime_error{"Failed to create DWrite factory"};
    }

    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_wic)))) {
        throw std::runtime_error{"Failed to create WIC factory"};
    }
}

void D2DPainter::begin() {
    m_context->SetTarget(m_rt.Get());
    m_context->BeginDraw();
    m_context->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
}

void D2DPainter::end() {
    m_context->EndDraw();
}

void D2DPainter::set_color(unsigned int color) {
    float r = ((color & 0xFF'0000) >> 16) / 255.0f;
    float g = ((color & 0xFF00) >> 8) / 255.0f;
    float b = ((color & 0xFF) >> 0) / 255.0f;
    float a = ((color & 0xFF00'0000) >> 24) / 255.0f;
    m_brush->SetColor({r, g, b, a});
}

void D2DPainter::text(std::unique_ptr<D2DFont>& font, const std::string& text, float x, float y, unsigned int color) {
    set_color(color);
    m_context->DrawTextLayout({x, y}, font->layout(text).Get(), m_brush.Get());
}


void D2DPainter::fill_rect(float x, float y, float w, float h, unsigned int color) {
    set_color(color);
    m_context->FillRectangle({x, y, x + w, y + h}, m_brush.Get());
}

void D2DPainter::outline_rect(float x, float y, float w, float h, float thickness, unsigned int color) {
    set_color(color);
    m_context->DrawRectangle({x, y, x + w, y + h}, m_brush.Get(), thickness);
}

void D2DPainter::line(float x1, float y1, float x2, float y2, float thickness, unsigned int color) {
    set_color(color);
    m_context->DrawLine({x1, y1}, {x2, y2}, m_brush.Get(), thickness);
}

void D2DPainter::image(std::unique_ptr<D2DImage>& image, float x, float y) {
    auto [w, h] = image->size();
    m_context->DrawBitmap(image->bitmap().Get(), {x, y, x + w, y + h});
}

void D2DPainter::image(std::unique_ptr<D2DImage>& image, float x, float y, float w, float h) {
    m_context->DrawBitmap(image->bitmap().Get(), {x, y, x + w, y + h});
}
