#pragma once

#include <string>
#include <vector>

#include "D2DFont.hpp"
#include "D2DImage.hpp"

enum class CommandType {
    NONE,
    TEXT,
    FILL_RECT,
    OUTLINE_RECT,
    ROUNDED_RECT,
    FILL_ROUNDED_RECT,
    QUAD,
    FILL_QUAD,
    LINE,
    IMAGE,
    FILL_CIRCLE,
    CIRCLE,
    PIE,
    RING
};

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
            float outerRadius{};
            float innerRadius{};
            float startAngle{};
            float sweepAngle{};
            unsigned int color{};
            bool clockwise{};
        } ring;
    };
    std::string str{};
    std::shared_ptr<D2DFont> font_resource{};
    std::shared_ptr<D2DImage> image_resource{};
};


bool command_equals(const Command& lhs, const Command& rhs);
