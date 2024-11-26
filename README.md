# REFramework Direct 2D
This is an [REFramework](https://github.com/praydog/REFramework) plugin that adds a Direct2D scripting API.

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
local image = nil

d2d.register(function()
    font = d2d.Font.new("Tahoma", 50)
    image = d2d.Image.new("test.png") -- Loads <gamedir>/reframework/images/test.png
end,
function()
    d2d.text(font, "Hello World!", 0, 0, 0xFFFFFFFF)
    d2d.text(font, "你好世界！", 0, 50, 0xFFFF0000) -- chinese
    d2d.text(font, "こんにちは世界！", 0, 100, 0xFF00FF00) -- japanese
    d2d.text(font, "안녕하세요, 세계입니다!", 0, 150, 0xFF0000FF) -- korean
    d2d.text(font, "Привет мир!", 0, 200, 0xFFFF00FF) -- russian
    d2d.text(font, "😁💕😒😘🤣😂😊🤔🥴👈👉🤦‍♀️", 0, 250, 0xFFFFFF00) -- emoji

    local str = "This is only a test"
    local w, h = font:measure(str)

    d2d.fill_rect(500, 100, w, h, 0xFFFFFFFF)
    d2d.text(font, str, 500, 100, 0xFF000000)
    d2d.outline_rect(500, 100, w, h, 5, 0xFF00FFFF)

    local screen_w, screen_h = d2d.surface_size()
    local img_w, img_h = image:size()

    -- Draw image at the bottom right corner of the screen in its default size.
    d2d.image(image, screen_w - img_w, screen_h - img_h) 

    -- Draw image at the bottom left corner of the screen but scaled to 50x50.
    d2d.image(image, 0, screen_h - 50, 50, 50)
    
    -- x, y, width, height, corner round x, corner round y, thickness, color
    d2d.rounded_rect(400, 500, 80, 40, 5, 15, 5, 0xFF00FFFF)
    -- x, y, width, height, corner round x, corner round y, color
    d2d.fill_rounded_rect(400, 500, 80, 40, 5, 15, 0xFF00FFFF)

    -- x, y, radius, color
    d2d.fill_circle(600, 500, 50, 0xFF00FFFF)
    -- x, y, radius x, radius y, color
    d2d.fill_oval(700, 500, 50, 80, 0xFF00FFFF)

    -- x, y, radius, thickness, color
    d2d.circle(800, 500, 50, 5, 0xFF00FFFF)
    -- x, y, radius x, radius y, thickness, color
    d2d.oval(900, 500, 50, 80, 5, 0xFF00FFFF)

    -- x, y, radius, start angle, sweep angle, color
    d2d.pie(1000, 500, 50, 0, 240, 0xFF00FFFF)
    d2d.pie(1100, 500, 50, 60, 240, 0xFF00FFFF)
    -- negative start angle equals +360 degree
    d2d.pie(1200, 100, 50, -90, 240, 0xFF00FFFF)
    d2d.pie(1200, 200, 50, 270, 240, 0xFF00FFFF)
    -- with clockwise=false
    d2d.pie(1300, 100, 50, -90, 240, 0xFF00FFFF, false)

    -- x, y, outer radius, inner radius, start angle, sweep angle, color
    d2d.ring(1200, 500, 50, 30, 0, 240, 0xFF00FFFF)
    d2d.ring(1300, 500, 50, 30, 60, 240, 0xFF00FFFF)
    -- negative start angle equals +360 degree
    d2d.ring(1600, 100, 50, 30, -90, 240, 0xFF00FFFF)
    d2d.ring(1600, 200, 50, 30, 270, 240, 0xFF00FFFF)
    -- with clockwise=false
    d2d.ring(1700, 100, 50, 30, -90, 240, 0xFF00FFFF, false)
end)
```

## API

### `d2d.register(init_fn, draw_fn)`
Registers your script with d2d allowing you to create d2d resources and draw using them.

#### Params
* `init_fn` a function that gets called when your script should create d2d resources (such as fonts via `d2d.create_font`)
* `draw_fn` a function that gets called when your script should draw using d2d and the d2d resources you've created in your `init_fn`

---

### `d2d.create_font(name, size, [bold], [italic])` **DEPRECATED**
**This function has been deprecated in favor of `d2d.Font.new(...)`**

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
* `font` the font resource you've created in your `init_fn` via `d2d.Font.new(...)`
* `text` the text to draw
* `x` the horizontal position on the screen
* `y` the vertical position on the screen
* `color` the ARGB color of the text

---

### `d2d.measure_text(font, text)` **DEPRECATED**
**This function has been deprecated in favor of `d2d.Font:measure(...)`**

Returns the width and height of the rendered text

#### Params
* `font` the font resource you've created in your `init_fn` via `d2d.Font.new(...)`
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

### `d2d.image(image, x, y, [w], [h])`
Draws an image at the specified position, optionally scaled.

#### Params
* `image` the image resource loaded in your `init_fn` via `d2d.Image.new(...)`
* `x` the horizontal position on the screen
* `y` the vertical position on the screen
* `w` the optional width to scale the image by
* `h` the optional height to scale the image by

#### Notes
If the `w` and `h` parameters are omitted, the image will be drawn at its natural size.

---

### `d2d.surface_size()`
Returns the width and height of the drawable surface. This is essentially the screen or window size of the game.

---

## Type: `d2d.Font`
Represents a d2d font resource.

---

### `d2d.Font.new(name, size, [bold], [italic])`
Creates a font resource.

#### Params
* `name` the font family name
* `size` the size of the created font
* `bold` an optional boolean value to make the font bold
* `italic` and optional boolean value to make the font italic

#### Notes
You must call this function from the `init_fn` passed to `d2d.register`. That's the only valid place to call it.

---

### `d2d.Font:measure(text)`
Returns the width and height of the rendered text.

#### Params
* `text` the text to measure

---

## Type: `d2d.Image`
Represents a d2d image resource.

---

### `d2d.Image.new(filepath)`
Loads an image resource from `<gamedir>\reframework\images\<filepath>`.

#### Params
* `filepath` A file path for the image to load

---

### `d2d.Image:size()`
Returns the width and height of the image in pixels.
