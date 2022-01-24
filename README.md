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

d2d.register(function()
    font = d2d.create_font("Tahoma", 50)
end,
function()
    d2d.color(1, 1, 1, 1)
    d2d.text(font, 0, 0, "Hello World!")
    d2d.color(1, 0, 0, 1)
    d2d.text(font, 0, 50, "你好世界！") -- chinese
    d2d.color(0, 1, 0, 1)
    d2d.text(font, 0, 100, "こんにちは世界！") -- japanese
    d2d.color(0, 0, 1, 1)
    d2d.text(font, 0, 150, "안녕하세요, 세계입니다!") -- korean
    d2d.color(1, 0, 1, 1)
    d2d.text(font, 0, 200, "Привет мир!")
    d2d.color(1, 1, 0, 1)
    d2d.text(font, 0, 250, "😁💕😒😘🤣😂😊🤔🥴👈👉🤦‍♀️") -- emoji
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

### `d2d.color(r, g, b, a)`
Sets the current draw color for the next draw commands (sets it until it is changed).

#### Params
* `r` the red component 
* `g` the green component
* `b` the blue component
* `a` the alpha component

#### Notes
All components are floating point values between 0 and 1.

---

### `d2d.text(font, x, y, text)`
Draws text on the screen at the position you supply using a font resource you've created.

#### Params
* `font` the font resource you've created in your `init_fn` via `d2d.create_font`
* `x` the horizontal position on the screen
* `y` the vertical position on the screen
* `text` the text to draw
