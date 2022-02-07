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
