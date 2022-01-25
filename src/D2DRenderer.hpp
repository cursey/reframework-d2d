#include <string>
#include <vector>

#include <d2d1_3.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwrite.h>
#include <wrl.h>

class D2DRenderer {
public:
    D2DRenderer(ID3D11Device* device, IDXGISurface* surface);

    void begin();
    void end();

    int create_font(std::string name, int size, bool bold = false, bool italic = false);

    void color(float r, float g, float b, float a);
    void text(int font, float x, float y, const std::string& text);
    void fill_rect(float x, float y, float w, float h);
    void outline_rect(float x, float y, float w, float h, float thickness);

private:
    template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

    ComPtr<ID2D1Factory3> m_d2d1{};
    ComPtr<ID2D1Device> m_device{};
    ComPtr<ID2D1DeviceContext> m_context{};

    ComPtr<ID2D1Bitmap1> m_rt{};
    ComPtr<ID2D1SolidColorBrush> m_brush{};

    ComPtr<IDWriteFactory> m_dwrite{};

    struct Font {
        std::string name;
        int size;
        bool bold;
        bool italic;
        ComPtr<IDWriteTextFormat> text_format;
    };

    std::vector<Font> m_fonts{};
};

