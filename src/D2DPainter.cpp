#include <stdexcept>

#include "utf8.h"

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

void D2DPainter::text(std::shared_ptr<D2DFont>& font, const std::string& text, float x, float y, unsigned int color) {
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

void D2DPainter::rounded_rect(float x, float y, float w, float h, float radiusX, float radiusY, float thickness, unsigned int color) {
    set_color(color);
    m_context->DrawRoundedRectangle({x, y, x + w, y + h, radiusX, radiusY}, m_brush.Get(), thickness);
}

void D2DPainter::fill_rounded_rect(float x, float y, float w, float h, float radiusX, float radiusY, unsigned int color) {
    set_color(color);
    m_context->FillRoundedRectangle({x, y, x + w, y + h, radiusX, radiusY}, m_brush.Get());
}

void D2DPainter::quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float thickness, unsigned int color) {
    ComPtr<ID2D1PathGeometry> pathGeometry;
    m_d2d1->CreatePathGeometry(&pathGeometry);

    ComPtr<ID2D1GeometrySink> sink;
    pathGeometry->Open(&sink);

    sink->BeginFigure(D2D1::Point2F(x1, y1), D2D1_FIGURE_BEGIN_FILLED);
    sink->AddLine(D2D1::Point2F(x2, y2));
    sink->AddLine(D2D1::Point2F(x3, y3));
    sink->AddLine(D2D1::Point2F(x4, y4));

    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink->Close();

    set_color(color);
    m_context->DrawGeometry(pathGeometry.Get(), m_brush.Get(), thickness);
}

void D2DPainter::fill_quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, unsigned int color) {
    ComPtr<ID2D1PathGeometry> pathGeometry;
    m_d2d1->CreatePathGeometry(&pathGeometry);

    ComPtr<ID2D1GeometrySink> sink;
    pathGeometry->Open(&sink);

    sink->BeginFigure(D2D1::Point2F(x1, y1), D2D1_FIGURE_BEGIN_FILLED);
    sink->AddLine(D2D1::Point2F(x2, y2));
    sink->AddLine(D2D1::Point2F(x3, y3));
    sink->AddLine(D2D1::Point2F(x4, y4));

    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink->Close();

    set_color(color);
    m_context->FillGeometry(pathGeometry.Get(), m_brush.Get());
}

void D2DPainter::line(float x1, float y1, float x2, float y2, float thickness, unsigned int color) {
    set_color(color);
    m_context->DrawLine({x1, y1}, {x2, y2}, m_brush.Get(), thickness);
}

void D2DPainter::image(std::shared_ptr<D2DImage>& image, float x, float y, float alpha) {
    auto [w, h] = image->size();
    m_context->DrawBitmap(image->bitmap().Get(), {x, y, x + w, y + h}, alpha);
}

void D2DPainter::image(std::shared_ptr<D2DImage>& image, float x, float y, float w, float h, float alpha) {
    m_context->DrawBitmap(image->bitmap().Get(), {x, y, x + w, y + h}, alpha);
}

void D2DPainter::fill_circle(float centerX, float centerY, float radius, unsigned int color) {
    set_color(color);
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(centerX, centerY), radius, radius);
    m_context->FillEllipse(ellipse, m_brush.Get());
}

void D2DPainter::fill_circle(float centerX, float centerY, float radiusX, float radiusY, unsigned int color) {
    set_color(color);
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(centerX, centerY), radiusX, radiusY);
    m_context->FillEllipse(ellipse, m_brush.Get());
}

void D2DPainter::circle(float centerX, float centerY, float radius, float thickness, unsigned int color) {
    set_color(color);
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(centerX, centerY), radius, radius);
    m_context->DrawEllipse(ellipse, m_brush.Get(), thickness);
}

void D2DPainter::circle(float centerX, float centerY, float radiusX, float radiusY, float thickness, unsigned int color) {
    set_color(color);
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(centerX, centerY), radiusX, radiusY);
    m_context->DrawEllipse(ellipse, m_brush.Get(), thickness);
}

void D2DPainter::pie(float centerX, float centerY, float radius, float startAngle, float sweepAngle, float thickness,
    unsigned int color, bool clockwise) {
    if (startAngle < 0) {
        startAngle += 360.0f;
    }
    startAngle = std::clamp(startAngle, 0.0f, 360.0f);
    sweepAngle = std::clamp(sweepAngle, 0.0f, 360.0f);
    if (sweepAngle == 0.0f) {
        return;
    }
    if (sweepAngle == 360.0f) {
        if (thickness == 0) {
            return this->fill_circle(centerX, centerY, radius, color);
        } else {
            return this->circle(centerX, centerY, radius, thickness, color);
        }
    }
    auto direction = clockwise ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;

    ComPtr<ID2D1PathGeometry> pathGeometry;
    m_d2d1->CreatePathGeometry(&pathGeometry);

    ComPtr<ID2D1GeometrySink> geometrySink;
    pathGeometry->Open(&geometrySink);

    const float startRadians = startAngle * (3.14159265f / 180.0f);
    const float sweepRadians = sweepAngle * (3.14159265f / 180.0f);
    const float endRadians = clockwise ? (startRadians + sweepRadians) : (startRadians - sweepRadians);

    D2D1_POINT_2F circleCenter = D2D1::Point2F(centerX, centerY);

    // circle center -> arc start
    D2D1_POINT_2F arcStart = D2D1::Point2F(centerX + radius * cosf(startRadians), centerY + radius * sinf(startRadians));
    geometrySink->BeginFigure(circleCenter, D2D1_FIGURE_BEGIN_FILLED);
    geometrySink->AddLine(arcStart);

    // arc start -> arc end
    D2D1_POINT_2F arcEnd = D2D1::Point2F(centerX + radius * cosf(endRadians), centerY + radius * sinf(endRadians));
    geometrySink->AddArc(D2D1::ArcSegment(arcEnd, D2D1::SizeF(radius, radius), 0.0f, direction,
        (sweepAngle > 180.0f) ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL));

    // arc end -> circle center
    geometrySink->AddLine(circleCenter);

    // end
    geometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
    geometrySink->Close();

    set_color(color);
    if (thickness == 0) {
        m_context->FillGeometry(pathGeometry.Get(), m_brush.Get());
    } else {
        m_context->DrawGeometry(pathGeometry.Get(), m_brush.Get(), thickness);
    }
}

void D2DPainter::ring(float centerX, float centerY, float outerRadius, float innerRadius, float thickness, unsigned int color) {
    ComPtr<ID2D1EllipseGeometry> outerCircle;
    m_d2d1->CreateEllipseGeometry(D2D1::Ellipse(D2D1::Point2F(centerX, centerY), outerRadius, outerRadius), &outerCircle);
    ComPtr<ID2D1EllipseGeometry> innerCircle;
    m_d2d1->CreateEllipseGeometry(D2D1::Ellipse(D2D1::Point2F(centerX, centerY), innerRadius, innerRadius), &innerCircle);

    ComPtr<ID2D1PathGeometry> pathGeometry;
    m_d2d1->CreatePathGeometry(&pathGeometry);
    ComPtr<ID2D1GeometrySink> sink;
    pathGeometry->Open(&sink);

    outerCircle->CombineWithGeometry(innerCircle.Get(), D2D1_COMBINE_MODE_EXCLUDE, NULL, sink.Get());
    sink->Close();

    set_color(color);
    if (thickness == 0) {
        m_context->FillGeometry(pathGeometry.Get(), m_brush.Get());
    } else {
        m_context->DrawGeometry(pathGeometry.Get(), m_brush.Get(), thickness);
    }
}

void D2DPainter::ring(float centerX, float centerY, float outerRadius, float innerRadius, float startAngle, float sweepAngle,
    float thickness, unsigned int color, bool clockwise) {
    if (startAngle < 0) {
        startAngle += 360.0f;
    }
    startAngle = std::clamp(startAngle, 0.0f, 360.0f);
    sweepAngle = std::clamp(sweepAngle, 0.0f, 360.0f);
    if (sweepAngle == 0.0f) {
        return;
    }
    if (sweepAngle == 360.0f) {
        return this->ring(centerX, centerY, outerRadius, innerRadius, thickness, color);
    }
    auto direction = clockwise ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
    auto counterDirection = !clockwise ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;

    set_color(color);

    ComPtr<ID2D1PathGeometry> pathGeometry;
    m_d2d1->CreatePathGeometry(&pathGeometry);

    ComPtr<ID2D1GeometrySink> sink;
    pathGeometry->Open(&sink);

    const float startRadians = startAngle * (3.14159265f / 180.0f);
    const float sweepRadians = sweepAngle * (3.14159265f / 180.0f);
    const float endRadians = clockwise ? (startRadians + sweepRadians) : (startRadians - sweepRadians);

    // outer arc start
    D2D1_POINT_2F outerStart = D2D1::Point2F(centerX + outerRadius * std::cos(startRadians), centerY + outerRadius * std::sin(startRadians));
    sink->BeginFigure(outerStart, D2D1_FIGURE_BEGIN_FILLED);

    // outer arc start -> outer arc end
    D2D1_POINT_2F outerEnd = D2D1::Point2F(centerX + outerRadius * std::cos(endRadians), centerY + outerRadius * std::sin(endRadians));
    sink->AddArc(D2D1::ArcSegment(outerEnd, D2D1::SizeF(outerRadius, outerRadius), 0.0f, direction,
            (sweepAngle > 180.0f) ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL));

    // outer arc end -> inner arc end
    D2D1_POINT_2F innerEnd = D2D1::Point2F(centerX + innerRadius * std::cos(endRadians), centerY + innerRadius * std::sin(endRadians));
    sink->AddLine(innerEnd);

    // inner arc end -> inner arc start
    D2D1_POINT_2F innerStart = D2D1::Point2F(centerX + innerRadius * std::cos(startRadians), centerY + innerRadius * std::sin(startRadians));
    sink->AddArc(D2D1::ArcSegment(innerStart, D2D1::SizeF(innerRadius, innerRadius), 0.0f, counterDirection,
            (sweepAngle > 180.0f) ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL));

    // inner arc start -> outer arc start
    sink->AddLine(outerStart);

    // end
    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink->Close();

    set_color(color);
    if (thickness == 0) {
        m_context->FillGeometry(pathGeometry.Get(), m_brush.Get());
    } else {
        m_context->DrawGeometry(pathGeometry.Get(), m_brush.Get(), thickness);
    }
}
