#include "DrawList.hpp"

void DrawList::CommandLock::text(std::shared_ptr<D2DFont>& font, std::string text, float x, float y, unsigned int color) {
    Command cmd{};
    cmd.type = CommandType::TEXT;
    cmd.text.x = x;
    cmd.text.y = y;
    cmd.text.color = color;
    cmd.str = std::move(text);
    cmd.font_resource = font;
    commands.emplace_back(std::move(cmd));
}

void DrawList::CommandLock::fill_rect(float x, float y, float w, float h, unsigned int color) {
    Command cmd{};
    cmd.type = CommandType::FILL_RECT;
    cmd.fill_rect.x = x;
    cmd.fill_rect.y = y;
    cmd.fill_rect.w = w;
    cmd.fill_rect.h = h;
    cmd.fill_rect.color = color;
    commands.emplace_back(std::move(cmd));
}

void DrawList::CommandLock::outline_rect(float x, float y, float w, float h, float thickness, unsigned int color) {
    Command cmd{};
    cmd.type = CommandType::OUTLINE_RECT;
    cmd.outline_rect.x = x;
    cmd.outline_rect.y = y;
    cmd.outline_rect.w = w;
    cmd.outline_rect.h = h;
    cmd.outline_rect.thickness = thickness;
    cmd.outline_rect.color = color;
    commands.emplace_back(std::move(cmd));
}

void DrawList::CommandLock::rounded_rect(float x, float y, float w, float h, float rX, float rY, float thickness, unsigned int color) {
    Command cmd{};
    cmd.type = CommandType::ROUNDED_RECT;
    cmd.rounded_rect.x = x;
    cmd.rounded_rect.y = y;
    cmd.rounded_rect.w = w;
    cmd.rounded_rect.h = h;
    cmd.rounded_rect.rX = rX;
    cmd.rounded_rect.rY = rY;
    cmd.rounded_rect.thickness = thickness;
    cmd.rounded_rect.color = color;
    commands.emplace_back(std::move(cmd));
}

void DrawList::CommandLock::fill_rounded_rect(float x, float y, float w, float h, float rX, float rY, unsigned int color) {
    Command cmd{};
    cmd.type = CommandType::FILL_ROUNDED_RECT;
    cmd.rounded_rect.x = x;
    cmd.rounded_rect.y = y;
    cmd.rounded_rect.w = w;
    cmd.rounded_rect.h = h;
    cmd.rounded_rect.rX = rX;
    cmd.rounded_rect.rY = rY;
    cmd.rounded_rect.color = color;
    commands.emplace_back(std::move(cmd));
}

void DrawList::CommandLock::quad(
    float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float thickness, unsigned int color) {
    Command cmd{};
    cmd.type = CommandType::QUAD;
    cmd.quad.x1 = x1;
    cmd.quad.y1 = y1;
    cmd.quad.x2 = x2;
    cmd.quad.y2 = y2;
    cmd.quad.x3 = x3;
    cmd.quad.y3 = y3;
    cmd.quad.x4 = x4;
    cmd.quad.y4 = y4;
    cmd.quad.thickness = thickness;
    cmd.quad.color = color;
    commands.emplace_back(std::move(cmd));
}

void DrawList::CommandLock::fill_quad(
    float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, unsigned int color) {
    Command cmd{};
    cmd.type = CommandType::FILL_QUAD;
    cmd.fill_quad.x1 = x1;
    cmd.fill_quad.y1 = y1;
    cmd.fill_quad.x2 = x2;
    cmd.fill_quad.y2 = y2;
    cmd.fill_quad.x3 = x3;
    cmd.fill_quad.y3 = y3;
    cmd.fill_quad.x4 = x4;
    cmd.fill_quad.y4 = y4;
    cmd.fill_quad.color = color;
    commands.emplace_back(std::move(cmd));
}

void DrawList::CommandLock::line(float x1, float y1, float x2, float y2, float thickness, unsigned int color) {
    Command cmd{};
    cmd.type = CommandType::LINE;
    cmd.line.x1 = x1;
    cmd.line.y1 = y1;
    cmd.line.x2 = x2;
    cmd.line.y2 = y2;
    cmd.line.thickness = thickness;
    cmd.line.color = color;
    commands.emplace_back(std::move(cmd));
}

void DrawList::CommandLock::image(std::shared_ptr<D2DImage>& image, float x, float y, float w, float h) {
    Command cmd{};
    cmd.type = CommandType::IMAGE;
    cmd.image.x = x;
    cmd.image.y = y;
    cmd.image.w = w;
    cmd.image.h = h;
    cmd.image_resource = image;
    commands.emplace_back(std::move(cmd));
}

void DrawList::CommandLock::fill_circle(float x, float y, float radiusX, float radiusY, unsigned int color) {
    Command cmd{};
    cmd.type = CommandType::FILL_CIRCLE;
    cmd.fill_circle.x = x;
    cmd.fill_circle.y = y;
    cmd.fill_circle.radiusX = radiusX;
    cmd.fill_circle.radiusY = radiusY;
    cmd.fill_circle.color = color;
    commands.emplace_back(std::move(cmd));
}

void DrawList::CommandLock::circle(float x, float y, float radiusX, float radiusY, float thickness, unsigned int color) {
    Command cmd{};
    cmd.type = CommandType::CIRCLE;
    cmd.circle.x = x;
    cmd.circle.y = y;
    cmd.circle.radiusX = radiusX;
    cmd.circle.radiusY = radiusY;
    cmd.circle.thickness = thickness;
    cmd.circle.color = color;
    commands.emplace_back(std::move(cmd));
}

void DrawList::CommandLock::pie(float x, float y, float r, float startAngle, float sweepAngle, unsigned int color, bool clockwise) {
    Command cmd{};
    cmd.type = CommandType::PIE;
    cmd.pie.x = x;
    cmd.pie.y = y;
    cmd.pie.r = r;
    cmd.pie.startAngle = startAngle;
    cmd.pie.sweepAngle = sweepAngle;
    cmd.pie.color = color;
    cmd.pie.clockwise = clockwise;
    commands.emplace_back(std::move(cmd));
}

void DrawList::CommandLock::ring(float x, float y, float outerRadius, float innerRadius, float startAngle, float sweepAngle, unsigned int color,
    bool clockwise) {
    Command cmd{};
    cmd.type = CommandType::RING;
    cmd.ring.x = x;
    cmd.ring.y = y;
    cmd.ring.outerRadius = outerRadius;
    cmd.ring.innerRadius = innerRadius;
    cmd.ring.startAngle = startAngle;
    cmd.ring.sweepAngle = sweepAngle;
    cmd.ring.color = color;
    cmd.ring.clockwise = clockwise;
    commands.emplace_back(std::move(cmd));
}
