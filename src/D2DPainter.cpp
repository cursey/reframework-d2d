#include <stdexcept>

#include "utf8.h"

#include "D2DPainter.hpp"

#include "reframework/API.hpp"
using API = reframework::API;

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

void D2DPainter::init_cache(std::vector<Command>& commands) {
    cache_index = 0;
    cache_hit = 0;
    cache_miss = 0;
    no_cache = 0;
    need_repaint = false;

    if (command_cache.size() != commands.size()) {
        need_repaint = true;
    }
    if (command_cache.size() < commands.size()) {
        command_cache.resize(commands.size());
    }
    
    for (auto&& cmd : commands) {
        const CachedGeometry& cache = command_cache[cache_index];
        if (cache.command.type == cmd.type && command_equals(cmd, cache.command)) {
            cache_hit++;
        } else {
            cache_miss++;
            need_repaint = true;
            command_cache[cache_index].command = cmd;
        }
        cache_index++;
    }

    cache_index = 0;
    cache_hit = 0;
    cache_miss = 0;
    no_cache = 0;
}

void D2DPainter::begin() {
    m_context->SetTarget(m_rt.Get());
    m_context->BeginDraw();
    if (need_repaint) {
        m_context->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));
    }
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

void D2DPainter::text(const Command& cmd) {
    const CachedGeometry& cache = command_cache[cache_index];
    if (cache.command.type == CommandType::TEXT && command_equals(cmd, cache.command)) {
        cache_hit++;
        m_context->DrawBitmap(cache.bitmap.Get());
    } else {
        command_cache[cache_index].command = cmd;
        cache_miss++;
    }
    set_color(cmd.text.color);
    m_context->DrawTextLayout({cmd.text.x, cmd.text.y}, cmd.font_resource->layout(cmd.str).Get(), m_brush.Get());
}

ComPtr<ID2D1RectangleGeometry> D2DPainter::rect_geometry(float x, float y, float w, float h) {
    D2D1_RECT_F rect = {x, y, x + w, y + h};
    ComPtr<ID2D1RectangleGeometry> geometry;
    m_d2d1->CreateRectangleGeometry(rect, &geometry);
    return geometry;
}

void D2DPainter::fill_rect(const Command& cmd) {
    ComPtr<ID2D1Geometry> geometry;

    const CachedGeometry& cache = command_cache[cache_index];
    if (cache.command.type == CommandType::FILL_RECT && command_equals(cmd, cache.command)) {
        geometry = cache.geometry;
        cache_hit++;
        m_context->DrawBitmap(cache.bitmap.Get());
    } else {
        geometry = rect_geometry(cmd.fill_rect.x, cmd.fill_rect.y, cmd.fill_rect.w, cmd.fill_rect.h);
        command_cache[cache_index].command = cmd;
        command_cache[cache_index].geometry = geometry;
        cache_miss++;
    }

    set_color(cmd.fill_rect.color);
    m_context->FillGeometry(geometry.Get(), m_brush.Get());
}

void D2DPainter::outline_rect(const Command& cmd) {
    ComPtr<ID2D1Geometry> geometry;

    const CachedGeometry& cache = command_cache[cache_index];
    if (cache.command.type == CommandType::OUTLINE_RECT && command_equals(cmd, cache.command)) {
        geometry = cache.geometry;
        cache_hit++;
        m_context->DrawBitmap(cache.bitmap.Get());
    } else {
        geometry = rect_geometry(cmd.outline_rect.x, cmd.outline_rect.y, cmd.outline_rect.w, cmd.outline_rect.h);

        command_cache[cache_index].command = cmd;
        command_cache[cache_index].geometry = geometry;
        cache_miss++;
    }

    set_color(cmd.outline_rect.color);
    m_context->DrawGeometry(geometry.Get(), m_brush.Get(), cmd.outline_rect.thickness);
}

ComPtr<ID2D1RoundedRectangleGeometry> D2DPainter::rounded_rect_geometry(float x, float y, float w, float h, float radiusX, float radiusY) {
    D2D1_ROUNDED_RECT rect = {x, y, x + w, y + h, radiusX, radiusY};
    ComPtr<ID2D1RoundedRectangleGeometry> geometry;
    m_d2d1->CreateRoundedRectangleGeometry(rect, &geometry);
    return geometry;
}

void D2DPainter::rounded_rect(const Command& cmd) {
    ComPtr<ID2D1Geometry> geometry;

    const CachedGeometry& cache = command_cache[cache_index];
    if (cache.command.type == CommandType::ROUNDED_RECT && command_equals(cmd, cache.command)) {
        geometry = cache.geometry;
        cache_hit++;
        m_context->DrawBitmap(cache.bitmap.Get());
    } else {
        geometry = rounded_rect_geometry(cmd.rounded_rect.x, cmd.rounded_rect.y, cmd.rounded_rect.w, cmd.rounded_rect.h, 
            cmd.rounded_rect.rX, cmd.rounded_rect.rY);

        command_cache[cache_index].command = cmd;
        command_cache[cache_index].geometry = geometry;
        cache_miss++;
    }

    set_color(cmd.rounded_rect.color);
    m_context->DrawGeometry(geometry.Get(), m_brush.Get(), cmd.rounded_rect.thickness);
}

void D2DPainter::fill_rounded_rect(const Command& cmd) {
    ComPtr<ID2D1Geometry> geometry;

    const CachedGeometry& cache = command_cache[cache_index];
    if (cache.command.type == CommandType::FILL_ROUNDED_RECT && command_equals(cmd, cache.command)) {
        geometry = cache.geometry;
        cache_hit++;
        m_context->DrawBitmap(cache.bitmap.Get());
    } else {
        geometry = rounded_rect_geometry(cmd.fill_rounded_rect.x, cmd.fill_rounded_rect.y, cmd.fill_rounded_rect.w,
            cmd.fill_rounded_rect.h, cmd.fill_rounded_rect.rX, cmd.fill_rounded_rect.rY);

        command_cache[cache_index].command = cmd;
        command_cache[cache_index].geometry = geometry;
        cache_miss++;
    }

    set_color(cmd.fill_rounded_rect.color);
    m_context->FillGeometry(geometry.Get(), m_brush.Get());
}

ComPtr<ID2D1PathGeometry> D2DPainter::quad_geometry(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
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

    return pathGeometry;
}

void D2DPainter::quad(const Command& cmd) {
    ComPtr<ID2D1Geometry> geometry;

    const CachedGeometry& cache = command_cache[cache_index];
    if (cache.command.type == CommandType::QUAD && command_equals(cmd, cache.command)) {
        geometry = cache.geometry;
        cache_hit++;
        m_context->DrawBitmap(cache.bitmap.Get());
    } else {
        geometry = quad_geometry(cmd.quad.x1, cmd.quad.y1, cmd.quad.x2, cmd.quad.y2, cmd.quad.x3, cmd.quad.y3, cmd.quad.x4, cmd.quad.y4);

        command_cache[cache_index].command = cmd;
        command_cache[cache_index].geometry = geometry;
        cache_miss++;
    }

    set_color(cmd.quad.color);
    m_context->DrawGeometry(geometry.Get(), m_brush.Get(), cmd.quad.thickness);
}

void D2DPainter::fill_quad(const Command& cmd) {
    ComPtr<ID2D1Geometry> geometry;

    const CachedGeometry& cache = command_cache[cache_index];
    if (cache.command.type == CommandType::FILL_QUAD && command_equals(cmd, cache.command)) {
        geometry = cache.geometry;
        cache_hit++;
        m_context->DrawBitmap(cache.bitmap.Get());
    } else {
        geometry = quad_geometry(cmd.fill_quad.x1, cmd.fill_quad.y1, cmd.fill_quad.x2, cmd.fill_quad.y2, 
            cmd.fill_quad.x3, cmd.fill_quad.y3, cmd.fill_quad.x4, cmd.fill_quad.y4);

        command_cache[cache_index].command = cmd;
        command_cache[cache_index].geometry = geometry;
        cache_miss++;
    }

    set_color(cmd.fill_quad.color);
    m_context->FillGeometry(geometry.Get(), m_brush.Get());
}

void D2DPainter::line(float x1, float y1, float x2, float y2, float thickness, unsigned int color) {
    set_color(color);
    m_context->DrawLine({x1, y1}, {x2, y2}, m_brush.Get(), thickness);
    no_cache++;
}

void D2DPainter::image(std::shared_ptr<D2DImage>& image, float x, float y) {
    auto [w, h] = image->size();
    m_context->DrawBitmap(image->bitmap().Get(), {x, y, x + w, y + h});
    no_cache++;
}

void D2DPainter::image(std::shared_ptr<D2DImage>& image, float x, float y, float w, float h) {
    m_context->DrawBitmap(image->bitmap().Get(), {x, y, x + w, y + h});
    no_cache++;
}

ComPtr<ID2D1EllipseGeometry> D2DPainter::circle_geometry(float centerX, float centerY, float radiusX, float radiusY) {
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(centerX, centerY), radiusX, radiusY);
    ComPtr<ID2D1EllipseGeometry> ellipseGeometry;
    m_d2d1->CreateEllipseGeometry(ellipse, &ellipseGeometry);
    return ellipseGeometry;
}

void D2DPainter::fill_circle(const Command& cmd) {
    ComPtr<ID2D1Geometry> geometry;

    const CachedGeometry& cache = command_cache[cache_index];
    if (cache.command.type == CommandType::FILL_CIRCLE && command_equals(cmd, cache.command)) {
        geometry = cache.geometry;
        cache_hit++;
        m_context->DrawBitmap(cache.bitmap.Get());
    } else {
        geometry = circle_geometry(cmd.fill_circle.x, cmd.fill_circle.y, cmd.fill_circle.radiusX, cmd.fill_circle.radiusY);

        command_cache[cache_index].command = cmd;
        command_cache[cache_index].geometry = geometry;
        cache_miss++;
    }

    set_color(cmd.fill_circle.color);
    m_context->FillGeometry(geometry.Get(), m_brush.Get());
}

void D2DPainter::circle(const Command& cmd) {
    ComPtr<ID2D1Geometry> geometry;

    const CachedGeometry& cache = command_cache[cache_index];
    if (cache.command.type == CommandType::CIRCLE && command_equals(cmd, cache.command)) {
        geometry = cache.geometry;
        cache_hit++;
        m_context->DrawBitmap(cache.bitmap.Get());
    } else {
        geometry = circle_geometry(cmd.circle.x, cmd.circle.y, cmd.circle.radiusX, cmd.circle.radiusY);

        command_cache[cache_index].command = cmd;
        command_cache[cache_index].geometry = geometry;
        cache_miss++;
    }

    set_color(cmd.circle.color);
    m_context->DrawGeometry(geometry.Get(), m_brush.Get(), cmd.circle.thickness);
}

ComPtr<ID2D1PathGeometry> D2DPainter::pie_geometry(float centerX, float centerY, float radius, float startAngle, float sweepAngle, bool clockwise) {
    if (startAngle < 0) {
        startAngle += 360.0f;
    }
    startAngle = std::clamp(startAngle, 0.0f, 360.0f);

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
    geometrySink->AddArc(D2D1::ArcSegment(
        arcEnd, D2D1::SizeF(radius, radius), 0.0f, direction, (sweepAngle > 180.0f) ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL));

    // arc end -> circle center
    geometrySink->AddLine(circleCenter);

    // end
    geometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
    geometrySink->Close();

    return pathGeometry;
}

void D2DPainter::pie(const Command& cmd) {
    auto sweepAngle = std::clamp(cmd.pie.sweepAngle, 0.0f, 360.0f);
    if (sweepAngle == 0.0f) {
        return;
    }

    ComPtr<ID2D1Geometry> geometry;

    const CachedGeometry& cache = command_cache[cache_index];
    if (cache.command.type == CommandType::PIE && command_equals(cmd, cache.command)) {
        geometry = cache.geometry;
        cache_hit++;
        m_context->DrawBitmap(cache.bitmap.Get());
    } else {
        if (sweepAngle == 360.0f) {
            geometry = circle_geometry(cmd.pie.x, cmd.pie.y, cmd.pie.r, cmd.pie.r);
        } else {
            geometry = pie_geometry(cmd.pie.x, cmd.pie.y, cmd.pie.r, cmd.pie.startAngle, sweepAngle, cmd.pie.clockwise);
        }

        command_cache[cache_index].command = cmd;
        command_cache[cache_index].geometry = geometry;
        cache_miss++;
    }

    set_color(cmd.pie.color);
    m_context->FillGeometry(geometry.Get(), m_brush.Get());
}

ComPtr<ID2D1PathGeometry> D2DPainter::ring_geometry(float centerX, float centerY, float outerRadius, float innerRadius) {
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

    return pathGeometry;
}

ComPtr<ID2D1PathGeometry> D2DPainter::ring_geometry(float centerX, float centerY, float outerRadius, float innerRadius, 
    float startAngle, float sweepAngle, bool clockwise) {
    if (startAngle < 0) {
        startAngle += 360.0f;
    }
    startAngle = std::clamp(startAngle, 0.0f, 360.0f);

    auto direction = clockwise ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
    auto counterDirection = !clockwise ? D2D1_SWEEP_DIRECTION_CLOCKWISE : D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;

    ComPtr<ID2D1PathGeometry> pathGeometry;
    m_d2d1->CreatePathGeometry(&pathGeometry);

    ComPtr<ID2D1GeometrySink> sink;
    pathGeometry->Open(&sink);

    const float startRadians = startAngle * (3.14159265f / 180.0f);
    const float sweepRadians = sweepAngle * (3.14159265f / 180.0f);
    const float endRadians = clockwise ? (startRadians + sweepRadians) : (startRadians - sweepRadians);

    // outer arc start
    D2D1_POINT_2F outerStart =
        D2D1::Point2F(centerX + outerRadius * std::cos(startRadians), centerY + outerRadius * std::sin(startRadians));
    sink->BeginFigure(outerStart, D2D1_FIGURE_BEGIN_FILLED);

    // outer arc start -> outer arc end
    D2D1_POINT_2F outerEnd = D2D1::Point2F(centerX + outerRadius * std::cos(endRadians), centerY + outerRadius * std::sin(endRadians));
    sink->AddArc(D2D1::ArcSegment(outerEnd, D2D1::SizeF(outerRadius, outerRadius), 0.0f, direction,
        (sweepAngle > 180.0f) ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL));

    // outer arc end -> inner arc end
    D2D1_POINT_2F innerEnd = D2D1::Point2F(centerX + innerRadius * std::cos(endRadians), centerY + innerRadius * std::sin(endRadians));
    sink->AddLine(innerEnd);

    // inner arc end -> inner arc start
    D2D1_POINT_2F innerStart =
        D2D1::Point2F(centerX + innerRadius * std::cos(startRadians), centerY + innerRadius * std::sin(startRadians));
    sink->AddArc(D2D1::ArcSegment(innerStart, D2D1::SizeF(innerRadius, innerRadius), 0.0f, counterDirection,
        (sweepAngle > 180.0f) ? D2D1_ARC_SIZE_LARGE : D2D1_ARC_SIZE_SMALL));

    // inner arc start -> outer arc start
    sink->AddLine(outerStart);

    // end
    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink->Close();

    return pathGeometry;
}

void D2DPainter::ring(const Command& cmd) {
    auto sweepAngle = std::clamp(cmd.ring.sweepAngle, 0.0f, 360.0f);
    if (sweepAngle == 0.0f) {
        return;
    }

    ComPtr<ID2D1Geometry> geometry;

    const CachedGeometry& cache = command_cache[cache_index];
    if (cache.command.type == CommandType::RING && command_equals(cmd, cache.command)) {
        geometry = cache.geometry;
        cache_hit++;
        m_context->DrawBitmap(cache.bitmap.Get());
    } else {
        if (sweepAngle == 360.0f) {
            geometry = ring_geometry(cmd.ring.x, cmd.ring.y, cmd.ring.outerRadius, cmd.ring.innerRadius);
        }else {
            geometry = ring_geometry(cmd.ring.x, cmd.ring.y, cmd.ring.outerRadius, cmd.ring.innerRadius, cmd.ring.startAngle, sweepAngle, cmd.ring.clockwise);
        }

        command_cache[cache_index].command = cmd;
        command_cache[cache_index].geometry = geometry;
        cache_miss++;
    }

    set_color(cmd.ring.color);
    m_context->FillGeometry(geometry.Get(), m_brush.Get());
}
