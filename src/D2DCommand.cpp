#include "D2DCommand.hpp"

const float epsilon = 0.0001f;
auto float_equal = [](float a, float b) { return std::fabs(a - b) < epsilon; };

bool command_equals(const Command& lhs, const Command& rhs) {
    if (lhs.type != rhs.type)
        return false;

    switch (lhs.type) {
    case CommandType::TEXT:
        return float_equal(lhs.text.x, rhs.text.x) && float_equal(lhs.text.y, rhs.text.y) && lhs.text.color == rhs.text.color &&
               lhs.str == rhs.str && lhs.font_resource == rhs.font_resource;

    case CommandType::FILL_RECT:
        return float_equal(lhs.fill_rect.x, rhs.fill_rect.x) && float_equal(lhs.fill_rect.y, rhs.fill_rect.y) &&
               float_equal(lhs.fill_rect.w, rhs.fill_rect.w) && float_equal(lhs.fill_rect.h, rhs.fill_rect.h) &&
               lhs.fill_rect.color == rhs.fill_rect.color;

    case CommandType::OUTLINE_RECT:
        return float_equal(lhs.outline_rect.x, rhs.outline_rect.x) && float_equal(lhs.outline_rect.y, rhs.outline_rect.y) &&
               float_equal(lhs.outline_rect.w, rhs.outline_rect.w) && float_equal(lhs.outline_rect.h, rhs.outline_rect.h) &&
               float_equal(lhs.outline_rect.thickness, rhs.outline_rect.thickness) && lhs.outline_rect.color == rhs.outline_rect.color;

    case CommandType::ROUNDED_RECT:
        return float_equal(lhs.rounded_rect.x, rhs.rounded_rect.x) && float_equal(lhs.rounded_rect.y, rhs.rounded_rect.y) &&
               float_equal(lhs.rounded_rect.w, rhs.rounded_rect.w) && float_equal(lhs.rounded_rect.h, rhs.rounded_rect.h) &&
               float_equal(lhs.rounded_rect.rX, rhs.rounded_rect.rX) && float_equal(lhs.rounded_rect.rY, rhs.rounded_rect.rY) &&
               float_equal(lhs.rounded_rect.thickness, rhs.rounded_rect.thickness) && lhs.rounded_rect.color == rhs.rounded_rect.color;

    case CommandType::FILL_ROUNDED_RECT:
        return float_equal(lhs.fill_rounded_rect.x, rhs.fill_rounded_rect.x) &&
               float_equal(lhs.fill_rounded_rect.y, rhs.fill_rounded_rect.y) &&
               float_equal(lhs.fill_rounded_rect.w, rhs.fill_rounded_rect.w) &&
               float_equal(lhs.fill_rounded_rect.h, rhs.fill_rounded_rect.h) &&
               float_equal(lhs.fill_rounded_rect.rX, rhs.fill_rounded_rect.rX) &&
               float_equal(lhs.fill_rounded_rect.rY, rhs.fill_rounded_rect.rY) &&
               lhs.fill_rounded_rect.color == rhs.fill_rounded_rect.color;

    case CommandType::QUAD:
        return float_equal(lhs.quad.x1, rhs.quad.x1) && float_equal(lhs.quad.y1, rhs.quad.y1) && float_equal(lhs.quad.x2, rhs.quad.x2) &&
               float_equal(lhs.quad.y2, rhs.quad.y2) && float_equal(lhs.quad.x3, rhs.quad.x3) && float_equal(lhs.quad.y3, rhs.quad.y3) &&
               float_equal(lhs.quad.x4, rhs.quad.x4) && float_equal(lhs.quad.y4, rhs.quad.y4) &&
               float_equal(lhs.quad.thickness, rhs.quad.thickness) && lhs.quad.color == rhs.quad.color;

    case CommandType::FILL_QUAD:
        return float_equal(lhs.fill_quad.x1, rhs.fill_quad.x1) && float_equal(lhs.fill_quad.y1, rhs.fill_quad.y1) &&
               float_equal(lhs.fill_quad.x2, rhs.fill_quad.x2) && float_equal(lhs.fill_quad.y2, rhs.fill_quad.y2) &&
               float_equal(lhs.fill_quad.x3, rhs.fill_quad.x3) && float_equal(lhs.fill_quad.y3, rhs.fill_quad.y3) &&
               float_equal(lhs.fill_quad.x4, rhs.fill_quad.x4) && float_equal(lhs.fill_quad.y4, rhs.fill_quad.y4) &&
               lhs.fill_quad.color == rhs.fill_quad.color;

    case CommandType::LINE:
        return float_equal(lhs.line.x1, rhs.line.x1) && float_equal(lhs.line.y1, rhs.line.y1) && float_equal(lhs.line.x2, rhs.line.x2) &&
               float_equal(lhs.line.y2, rhs.line.y2) && float_equal(lhs.line.thickness, rhs.line.thickness) &&
               lhs.line.color == rhs.line.color;

    case CommandType::IMAGE:
        return float_equal(lhs.image.x, rhs.image.x) && float_equal(lhs.image.y, rhs.image.y) && float_equal(lhs.image.w, rhs.image.w) &&
               float_equal(lhs.image.h, rhs.image.h) && lhs.image_resource == rhs.image_resource;

    case CommandType::FILL_CIRCLE:
        return float_equal(lhs.fill_circle.x, rhs.fill_circle.x) && float_equal(lhs.fill_circle.y, rhs.fill_circle.y) &&
               float_equal(lhs.fill_circle.radiusX, rhs.fill_circle.radiusX) &&
               float_equal(lhs.fill_circle.radiusY, rhs.fill_circle.radiusY) && lhs.fill_circle.color == rhs.fill_circle.color;

    case CommandType::CIRCLE:
        return float_equal(lhs.circle.x, rhs.circle.x) && float_equal(lhs.circle.y, rhs.circle.y) &&
               float_equal(lhs.circle.radiusX, rhs.circle.radiusX) && float_equal(lhs.circle.radiusY, rhs.circle.radiusY) &&
               float_equal(lhs.circle.thickness, rhs.circle.thickness) && lhs.circle.color == rhs.circle.color;

    case CommandType::PIE:
        return float_equal(lhs.pie.x, rhs.pie.x) && float_equal(lhs.pie.y, rhs.pie.y) && float_equal(lhs.pie.r, rhs.pie.r) &&
               float_equal(lhs.pie.startAngle, rhs.pie.startAngle) && float_equal(lhs.pie.sweepAngle, rhs.pie.sweepAngle) &&
               lhs.pie.color == rhs.pie.color && lhs.pie.clockwise == rhs.pie.clockwise;

    case CommandType::RING:
        return float_equal(lhs.ring.x, rhs.ring.x) && float_equal(lhs.ring.y, rhs.ring.y) &&
               float_equal(lhs.ring.outerRadius, rhs.ring.outerRadius) && float_equal(lhs.ring.innerRadius, rhs.ring.innerRadius) &&
               float_equal(lhs.ring.startAngle, rhs.ring.startAngle) && float_equal(lhs.ring.sweepAngle, rhs.ring.sweepAngle) &&
               lhs.ring.color == rhs.ring.color && lhs.ring.clockwise == rhs.ring.clockwise;

    default:
        return false;
    }
}
