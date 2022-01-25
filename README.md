# REFramework Direct 2D
This is an [REFramework](https://github.com/praydog/REFramework) plugin that adds a Direct2D scripting API. It is under heavy development and currently in a pre-alpha state.

## Building
Currently suggest building in RelWithDebInfo so that when issues arise you can debug and fix them.
```
git clone https://github.com/cursey/reframework-d2d.git
cd reframework-d2d
cmake -B build
cmake --build build --config RelWithDebInfo
```

## Example
```lua
local font = nil
local x = 0
local y = 0

d2d.register(function()
    font = d2d.create_font("Tahoma", 50)
end,
function()
    d2d.text(font, "Hello World!", 0, 0, 0xFFFFFFFF)
    d2d.text(font, "你好世界！", 0, 50, 0xFFFF0000) -- chinese
    d2d.text(font, "こんにちは世界！", 0, 100, 0xFF00FF00) -- japanese
    d2d.text(font, "안녕하세요, 세계입니다!", 0, 150, 0xFF0000FF) -- korean
    d2d.text(font, "Привет мир!", 0, 200, 0xFFFF00FF) -- russian
    d2d.text(font, "😁💕😒😘🤣😂😊🤔🥴👈👉🤦‍♀️", 0, 250, 0xFFFFFF00) -- emoji

    local str = "This is only a test"
    local w, h = d2d.measure_text(font, str)

    d2d.fill_rect(500, 100, w, h, 0xFFFFFFFF)
    d2d.text(font, str, 500, 100, 0xFF000000)
    d2d.outline_rect(500, 100, w, h, 5, 0xFF00FFFF)

    d2d.fill_rect(x, y, 50, 50, 0xFFFF0000)

    x = x + 10
    y = y + 10
    w, h = d2d.surface_size()

    if x > w then
        x = 0
    end
    if y > h then
        y = 0
    end
end)
```

## API

### `d2d.register(init_fn, draw_fn)`
Registers your script with d2d allowing you to create d2d resources and draw using them.

#### Params
* `init_fn` a function that gets called when your script should create d2d resources (such as fonts via `d2d.create_font`)
* `draw_fn` a function that gets called when your script should draw using d2d and the d2d resources you've created in your `init_fn`

---

### `d2d.create_font(name, size, [bold], [italic])`
Creates a font resource.

#### Params
* `name` the font family name
* `size` the size of the created font
* `bold` an optional boolean value to make the font bold
* `italic` and optional boolean value to make the font italic

#### Notes
You must call this function from the `init_fn` passed to `d2d.register`. That's the only valid place to call it.

---

### `d2d.text(font, text, x, y, color)`
Draws text on the screen at the position you supply using a font resource you've created.

#### Params
* `font` the font resource you've created in your `init_fn` via `d2d.create_font`
* `text` the text to draw
* `x` the horizontal position on the screen
* `y` the vertical position on the screen
* `color` the ARGB color of the text

---

### `d2d.measure_text(font, text)`
Returns the width and height of the rendered text

#### Params
* `font` the font resource you've created in your `init_fn` via `d2d.create_font`
* `text` the text to measure

---

### `d2d.fill_rect(x, y, w, h, color)`
Draws a filled in rectangle

#### Params
* `x` the horizontal position on the screen
* `y` the vertical position on the screen
* `w` the width of the rectangle
* `h` the height of the rectangle
* `color` the ARGB color of the rectangle

---

### `d2d.outline_rect(x, y, w, h, thickness, color)`
Draws the outline of a rectangle

#### Params
* `x` the horizontal position on the screen
* `y` the vertical position on the screen
* `w` the width of the rectangle
* `h` the height of the rectangle
* `thickness` the thickness of the outline
* `color` the ARGB color of the rectangle

---

### `d2d.line(x1, y1, x2, y2, thickness, color)`
Draws a line between two points

#### Params
* `x1` the first horizontal position on the screen
* `y1` the first vertical position on the screen
* `x2` the second horizontal position on the screen
* `y2` the second vertical position on the screen
* `thickness` the thickness of the line
* `color` the ARGB color of the rectangle

---

### `d2d.surface_size()`
Returns the width and height of the drawable surface. This is essentially the screen or window size of the game.
