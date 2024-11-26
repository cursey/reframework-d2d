#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "D2DFont.hpp"
#include "D2DImage.hpp"

class DrawList {
public:
    enum class CommandType { TEXT, FILL_RECT, OUTLINE_RECT, ROUNDED_RECT, FILL_ROUNDED_RECT, LINE, IMAGE, FILL_CIRCLE, CIRCLE, PIE, RING };

    struct Command {
        CommandType type;
        union {
            struct {
                float x{};
                float y{};
                unsigned int color{};
            } text;
            struct {
                float x{};
                float y{};
                float w{};
                float h{};
                unsigned int color{};
            } fill_rect;
            struct {
                float x{};
                float y{};
                float w{};
                float h{};
                float thickness{};
                unsigned int color{};
            } outline_rect;
            struct {
                float x{};
                float y{};
                float w{};
                float h{};
                float rX{};
                float rY{};
                float thickness{};
                unsigned int color{};
            } rounded_rect;
            struct {
                float x{};
                float y{};
                float w{};
                float h{};
                float rX{};
                float rY{};
                unsigned int color{};
            } fill_rounded_rect;
            struct {
                float x1{};
                float y1{};
                float x2{};
                float y2{};
                float thickness{};
                unsigned int color{};
            } line;
            struct {
                float x{};
                float y{};
                float w{};
                float h{};
            } image;
            struct {
                float x{};
                float y{};
                float radiusX{};
                float radiusY{};
                unsigned int color{};
            } fill_circle;
            struct {
                float x{};
                float y{};
                float radiusX{};
                float radiusY{};
                float thickness{};
                unsigned int color{};
            } circle;
            struct {
                float x{};
                float y{};
                float r{};
                float startAngle{};
                float sweepAngle{};
                unsigned int color{};
            } pie;
            struct {
                float x{};
                float y{};
                float outerRadius{};
                float innerRadius{};
                float startAngle{};
                float sweepAngle{};
                unsigned int color{};
            } ring;
        };
        std::string str{};
        std::shared_ptr<D2DFont> font_resource{};
        std::shared_ptr<D2DImage> image_resource{};
    };

    struct [[nodiscard]] CommandLock {
        std::vector<Command>& commands;
        std::scoped_lock<std::mutex> lock;

        void text(std::shared_ptr<D2DFont>& font, std::string text, float x, float y, unsigned int color);
        void fill_rect(float x, float y, float w, float h, unsigned int color);
        void outline_rect(float x, float y, float w, float h, float thickness, unsigned int color);
        void rounded_rect(float x, float y, float w, float h, float rX, float rY, float thickness, unsigned int color);
        void fill_rounded_rect(float x, float y, float w, float h, float rX, float rY, unsigned int color);
        void line(float x1, float y1, float x2, float y2, float thickness, unsigned int color);
        void image(std::shared_ptr<D2DImage>& image, float x, float y, float w, float h);
        void fill_circle(float x, float y, float radiusX, float radiusY, unsigned int color);
        void circle(float x, float y, float radiusX, float radiusY, float thickness, unsigned int color);
        void pie(float x, float y, float r, float startAngle, float sweepAngle, unsigned int color);
        void ring(float x, float y, float outerRadius, float innerRadius, float startAngle, float sweepAngle, unsigned int color);
    };

    auto acquire() { return CommandLock{m_commands, std::scoped_lock{m_commands_mux}}; }

private:
    std::vector<Command> m_commands{};
    std::mutex m_commands_mux{};
};
