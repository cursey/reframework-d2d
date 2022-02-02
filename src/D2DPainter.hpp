#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include <d2d1_3.h>
#include <d3d11.h>
#include <dwrite.h>
#include <dxgi.h>
#include <wincodec.h>
#include <wrl.h>

#include "D2DFont.hpp"
#include "D2DImage.hpp"

class D2DPainter {
public:
    template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    D2DPainter(ID3D11Device* device, IDXGISurface* surface);

    void begin();
    void end();

    void set_color(unsigned int color);

    void text(std::unique_ptr<D2DFont>& font, const std::string& text, float x, float y, unsigned int color);
    void fill_rect(float x, float y, float w, float h, unsigned int color);
    void outline_rect(float x, float y, float w, float h, float thickness, unsigned int color);
    void line(float x1, float y1, float x2, float y2, float thickness, unsigned int color);
    void image(std::unique_ptr<D2DImage>& image, float x, float y);
    void image(std::unique_ptr<D2DImage>& image, float x, float y, float w, float h);

    auto surface_size() const { return std::make_tuple(m_rt_desc.Width, m_rt_desc.Height); }

    const auto& context() const { return m_context; }
    const auto& dwrite() const { return m_dwrite; }
    const auto& wic() const { return m_wic; }

private:
    ComPtr<ID2D1Factory3> m_d2d1{};
    ComPtr<ID2D1Device> m_device{};
    ComPtr<ID2D1DeviceContext> m_context{};

    DXGI_SURFACE_DESC m_rt_desc{};
    ComPtr<ID2D1Bitmap1> m_rt{};
    ComPtr<ID2D1SolidColorBrush> m_brush{};

    ComPtr<IDWriteFactory> m_dwrite{};
    ComPtr<IWICImagingFactory> m_wic{};
};
