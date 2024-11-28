#include <memory>
#include <string>
#include <tuple>
#include <vector>
#include <functional>

#include <d2d1_3.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi.h>
#include <wincodec.h>
#include <wrl.h>

#include "D2DFont.hpp"
#include "D2DImage.hpp"
#include "D2DCommand.hpp"

template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;
class D2DPainter {
public:
    struct CachedGeometry {
        Command command{};
    };

    D2DPainter(ID3D11Device* device, IDXGISurface* surface);

    ComPtr<ID2D1Bitmap> cache_bitmap;
    void init_cache(std::vector<Command>&);
    void begin();
    void end();

    void set_color(unsigned int color);

    void text(std::shared_ptr<D2DFont>& font, const std::string& text, float x, float y, unsigned int color);
    void fill_rect(float x, float y, float w, float h, unsigned int color);
    void outline_rect(float x, float y, float w, float h, float thickness, unsigned int color);
    void rounded_rect(float x, float y, float w, float h, float radiusX, float radiusY, float thickness, unsigned int color);
    void fill_rounded_rect(float x, float y, float w, float h, float radiusX, float radiusY, unsigned int color);
    void quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float thickness, unsigned int color);
    void fill_quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, unsigned int color);
    void line(float x1, float y1, float x2, float y2, float thickness, unsigned int color);
    void image(std::shared_ptr<D2DImage>& image, float x, float y);
    void image(std::shared_ptr<D2DImage>& image, float x, float y, float w, float h);
    void fill_circle(float centerX, float centerY, float radius, unsigned int color);
    void fill_circle(float centerX, float centerY, float radiusX, float radiusY, unsigned int color);
    void circle(float centerX, float centerY, float radius, int thickness, unsigned int color);
    void circle(float centerX, float centerY, float radiusX, float radiusY, int thickness, unsigned int color);
    void pie(float centerX, float centerY, float radius, float startAngle, float sweepAngle, unsigned int color, bool clockwise);
    void ring(float centerX, float centerY, float outerRadius, float innerRadius, unsigned int color);
    void ring(float centerX, float centerY, float outerRadius, float innerRadius, float startAngle, float sweepAngle, unsigned int color,
        bool clockwise);

    auto surface_size() const { return std::make_tuple(m_rt_desc.Width, m_rt_desc.Height); }

    const auto& context() const { return m_context; }
    const auto& dwrite() const { return m_dwrite; }
    const auto& wic() const { return m_wic; }
    bool need_repaint{};
    int cache_index{};
    int cache_hit{};
    int cache_miss{};
    int no_cache{};

private:
    int cache_command_count{};
    std::vector<CachedGeometry> command_cache{};

    ComPtr<ID2D1Factory3> m_d2d1{};
    ComPtr<ID2D1Device> m_device{};
    ComPtr<ID2D1DeviceContext> m_context{};

    DXGI_SURFACE_DESC m_rt_desc{};
    ComPtr<ID2D1Bitmap1> m_rt{};
    ComPtr<ID2D1SolidColorBrush> m_brush{};

    ComPtr<IDWriteFactory> m_dwrite{};
    ComPtr<IWICImagingFactory> m_wic{};
};
