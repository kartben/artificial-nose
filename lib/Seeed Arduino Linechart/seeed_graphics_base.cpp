#include<math.h>
#include"seeed_graphics_base.h"

std::vector<color_t> classic_colors { 
    yellow,
    red,
    green,
    black, 
    blue,
    darkcyan,
    magenta,
};

constexpr double pi = 3.14159265358979323846;
extern TFT_eSprite spr;

namespace detail {
    void draw(void * poly, bool gen){
        detail::poly<polygen> & p = *(detail::poly<polygen> *)poly;
        auto default_color = black;
        auto default_thickness = pan_thickness;
        auto & value = p.value();
        auto & color = p.color();
        auto & thickness = p.thickness();

        if (value.size() <= 1) {
            return;
        }
        for (size_t i = 1; i < value.size() + gen; i++){
            auto ai = (i - 1) % value.size();
            auto bi = (i) % value.size();
            auto a  = value[ai];
            auto b  = value[bi];
            if (i - 1 < color.size()){
                default_color = color[i - 1];
            }
            if (i - 1 < thickness.size()){
                default_thickness = thickness[i - 1];
            }
            line(a, b)
                .color(default_color)
                .thickness(default_thickness)
                .draw();
        }
    }
}

void dot::draw() {
    spr.drawPixel(_x, _y, _color);
}
void line::draw() {
    spr.drawLine(_x0, _y0, _x1, _y1, _color);
}
void dash_line::draw() {
    point origin(_x, _y);
    point solid_offset;
    point empty_offset;
    point end;
    if (_orientation == horizon) {
        solid_offset = point(_solid, 0);
        empty_offset = point(_empty, 0);
        end = origin(_length - 1, 0);
    }
    else {
        solid_offset = point(0, _solid);
        empty_offset = point(0, _empty);
        end = origin(0, _length - 1);
    }
    for (bool loop = true; loop;) {
        auto next = origin(solid_offset);
        if (next.x > end.x) {
            next.x = end.x;
            loop = false;
        }
        if (next.y > end.y) {
            next.y = end.y;
            loop = false;
        }
        line(origin, next)
            .color(_color)
            .thickness(_thickness)
            .draw();
        origin = next(empty_offset);
    }
}

void rectangle::draw() {
    auto origin = adjust(_width, _height);
    auto left_top = origin;
    auto left_bottom = origin(0, -(pos_t)_height + 1);
    auto right_top = left_top(_width - 1, 0);
    auto right_bottom = left_bottom(_width - 1, 0);
    spr.drawRect(left_top.x, left_top.y, _width, _height, _color);
    
    if (_fill != transparent) {
        spr.fillRect(left_top.x + 1, left_top.y + 1, _width - 2, _height - 2, _fill);
    }
}

void ellipse::draw() {
    auto p = adjust(_width, _height);
    if (_fill){
        spr.fillEllipse(_x, _y, _width / 2, _height / 2, _fill);
    }
    spr.drawEllipse(_x, _y, _width / 2, _height / 2, _color);
}
text & text::font(font_t value){
    _font = value;
    spr.setTextFont(uint8_t(value));
    return this[0];
}
text & text::thickness(pix_t value){
    _thickness = value;
    spr.setTextSize(uint8_t(value));
    return this[0];
}
text & text::font_height(pix_t * value) {
    font(font());
    thickness(thickness());
    value[0] = spr.fontHeight();
    return this[0];
}
text & text::content_width(pix_t * value) {
    font(font());
    thickness(thickness());
    value[0] = spr.textWidth(_value);
    return this[0];
}

void text::draw(){
    pix_t   height = font_height();
    pix_t   width = content_width();
    point   p = adjust(width, height);
    if (_align == center){
        p.x += (_width - width) / 2;
    }
    else if (_align == right){
        p.x += _width - width;
    }
    if (_valign == vcenter){
        p.y += (_height - height) / 2;
    }
    else if (_valign == bottom){
        p.y += _height - height;
    }
    spr.setTextColor(_color);
    spr.drawString(_value, p.x, p.y);
}
void polyline::draw() {
    detail::draw(this, false);
}
void polygen::draw() {
    detail::draw(this, true);
}

