#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "D2DFont.hpp"
#include "D2DImage.hpp"

class DrawList {
public:
    enum class CommandType { TEXT, FILL_RECT, OUTLINE_RECT, ROUNDED_RECT, FILL_ROUNDED_RECT, QUAD, FILL_QUAD, LINE, IMAGE, FILL_CIRCLE, CIRCLE, PIE, OUTLINE_PIE, RING, OUTLINE_RING };

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
                float x3{};
                float y3{};
                float x4{};
                float y4{};
                float thickness{};
                unsigned int color{};
            } quad;
            struct {
                float x1{};
                float y1{};
                float x2{};
                float y2{};
                float x3{};
                float y3{};
                float x4{};
                float y4{};
                unsigned int color{};
            } fill_quad;
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
				float alpha{1.0f};
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
                bool clockwise{};
            } pie;
            struct {
                float x{};
                float y{};
                float r{};
                float startAngle{};
                float sweepAngle{};
                float thickness{};
                unsigned int color{};
                bool clockwise{};
            } outline_pie;
            struct {
                float x{};
                float y{};
                float outerRadius{};
                float innerRadius{};
                float startAngle{};
                float sweepAngle{};
                unsigned int color{};
                bool clockwise{};
            } ring;
            struct {
                float x{};
                float y{};
                float outerRadius{};
                float innerRadius{};
                float startAngle{};
                float sweepAngle{};
                float thickness{};
                unsigned int color{};
                bool clockwise{};
            } outline_ring;
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
        void quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float thickness, unsigned int color);
        void fill_quad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, unsigned int color);
        void line(float x1, float y1, float x2, float y2, float thickness, unsigned int color);
        void image(std::shared_ptr<D2DImage>& image, float x, float y, float w, float h, float alpha = 1.0f);
        void fill_circle(float x, float y, float radiusX, float radiusY, unsigned int color);
        void circle(float x, float y, float radiusX, float radiusY, float thickness, unsigned int color);
        void pie(float x, float y, float r, float startAngle, float sweepAngle, unsigned int color, bool clockwise);
        void outline_pie(float x, float y, float r, float startAngle, float sweepAngle, float thickness, unsigned int color, bool clockwise);
        void ring(float x, float y, float outerRadius, float innerRadius, float startAngle, float sweepAngle, unsigned int color, bool clockwise);
        void outline_ring(float x, float y, float outerRadius, float innerRadius, float startAngle, float sweepAngle, float thickness,
            unsigned int color, bool clockwise);
    };

    auto acquire() { return CommandLock{m_commands, std::scoped_lock{m_commands_mux}}; }

private:
    std::vector<Command> m_commands{};
    std::mutex m_commands_mux{};
};
