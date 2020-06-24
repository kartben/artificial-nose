#pragma once
#include"seeed_graphics_define.h"

#define xlist(...)          __VA_ARGS__
#define xprop(type,name,...)                              \
protected:                                                \
type _ ## name;                                           \
public:                                                   \
auto & name(type value) {                                 \
    _ ## name = value;                                    \
    return (__VA_ARGS__  this)[0];                        \
}                                                         \
auto & name(type * value){                                \
    value[0] = _ ## name;                                 \
    return (__VA_ARGS__  this)[0];                        \
}                                                         \
auto & name(){                                            \
    return _ ## name;                                     \
}

#define xvprop(type,name,...)                             \
protected:                                                \
std::vector<type> _ ## name;                              \
public:                                                   \
auto & name(type value) {                                 \
    _ ## name = std::vector<type>{ value };               \
    return (__VA_ARGS__  this)[0];                        \
}                                                         \
template<class ... arg>                                   \
auto & name(type value, arg ... list) {                   \
    _ ## name = std::vector<type>{ value, list... };      \
    return (__VA_ARGS__  this)[0];                        \
}                                                         \
auto & name(std::vector<type> * value){                   \
    value[0] = _ ## name;                                 \
    return (__VA_ARGS__  this)[0];                        \
}                                                         \
auto & name(){                                            \
    return _ ## name;                                     \
}


#define xpoint(type,...)                                  \
xprop(pos_t, x ## __VA_ARGS__);                           \
xprop(pos_t, y ## __VA_ARGS__);                           \
auto & xy ## __VA_ARGS__(                                 \
    pos_t x,                                              \
    pos_t y){                                             \
    this->x ## __VA_ARGS__(x);                            \
    this->y ## __VA_ARGS__(y);                            \
    return ((type *)this)[0];                             \
}                                                         \
auto & xy ## __VA_ARGS__(                                 \
    pos_t * x,                                            \
    pos_t * y){                                           \
    this->x ## __VA_ARGS__(x);                            \
    this->y ## __VA_ARGS__(y);                            \
    return ((type *)this)[0];                             \
}                                                         \
auto & xy ## __VA_ARGS__(point p){                        \
    return xy ## __VA_ARGS__(p.x, p.y);                   \
}                                                         \
auto xy ## __VA_ARGS__(){                                 \
    return point(x ## __VA_ARGS__(), y ## __VA_ARGS__()); \
}                                                         \
auto & xy ## __VA_ARGS__(point * p){                      \
    return xy ## __VA_ARGS__(& p->x, & p->y);             \
}

#define xpositionx(struct_type,list,set,default_set)      \
struct_type() {                                           \
    _x = 0;                                               \
    _y = 0;                                               \
    default_set;                                          \
}                                                         \
struct_type(pos_t x, pos_t y) :                           \
    struct_type(){                                        \
    _x = x;                                               \
    _y = y;                                               \
}                                                         \
struct_type(point p) :                                    \
    struct_type(p.x, p.y) {                               \
}                                                         \
struct_type(point p, list) :                              \
    struct_type(p.x, p.y) {                               \
    set;                                                  \
}                                                         \
struct_type(pos_t x, pos_t y, list) :                     \
    struct_type(x,y){                                     \
    set;                                                  \
}                                                         \

                                                          
#define xposition(struct_type,...)                        \
struct_type() {                                           \
    _x = 0;                                               \
    _y = 0;                                               \
    __VA_ARGS__;                                          \
}                                                         \
struct_type(pos_t x, pos_t y) :                           \
    struct_type(){                                        \
    _x = x;                                               \
    _y = y;                                               \
}                                                         \
struct_type(point p) :                                    \
    struct_type(p.x, p.y) {                               \
}                                                         \


namespace detail {
    template<class return_type>
    struct poly{
        poly(){
            _color.push_back(pan_color);
            _thickness.push_back(pan_thickness);
        }
        poly(std::initializer_list<point> const & value) : 
            poly(){
            _value = value;
        }
        xvprop(point,   value, (return_type *));
        xvprop(color_t, color, (return_type *));
        xvprop(pix_t,   thickness, (return_type*));
    };

    template<class return_type>
    struct aligner{
        aligner() :
            _x(0),
            _y(0),
            _width(0),
            _height(0),
            _origin(left),
            _vorigin(top){
        }
        xpoint(return_type);
        xprop(pix_t,       width, (return_type*));
        xprop(pix_t,       height, (return_type*));
        xprop(align_type,  origin, (return_type*));
        xprop(valign_type, vorigin, (return_type*));
    //protected:
        point adjust(pix_t width, pix_t height) {
            auto x = _x;
            auto y = _y;

            if (_width == 0) {
                _width = width;
            }
            if (_height == 0) {
                _height = height;
            }
            if (_origin == center) {
                x -= width / 2;
            }
            else if (_origin == right) {
                x -= width;
            }
            if (_vorigin == vcenter) {
                y -= height / 2;
            }
            else if (_vorigin == bottom) {
                y -= height;
            }
            return point(x, y);
        }
    };
}

struct dot{
    xposition(dot, {
        _color = pan_color;
    });
    xpoint(dot);
    xprop(color_t, color);
    void draw();
    operator can_drawable(){
        return can_drawable(this);
    }
};

struct line{
    line(){
        _x0 = 0;
        _y0 = 0;
        _x1 = 0;
        _y1 = 0;
        _thickness = 1;
        _color = black;
    }
    line(point p0, point p1) :
        line(p0.x, p0.y, p1.x, p1.y){
    }
    line(pos_t x0, pos_t y0, pos_t x1, pos_t y1):
        line() {
        _x0 = x0;
        _y0 = y0;
        _x1 = x1;
        _y1 = y1;
    }
    xpoint(line, 0);
    xpoint(line, 1);
    xprop(color_t,  color);
    xprop(pix_t,    thickness);
    void draw();
    operator can_drawable(){
        return can_drawable(this);
    }
};

struct rectangle;
struct rectangle : detail::aligner<rectangle>{
    xpositionx(
        rectangle, xlist(pix_t width, pix_t height), {
            _width = width;
            _height = height;
        }, {
            _fill = transparent;
            _color = pan_color;
            _thickness_left = pan_thickness;
            _thickness_top = pan_thickness;
            _thickness_right = pan_thickness;
            _thickness_bottom = pan_thickness;
        });
    xprop(color_t, fill);
    xprop(color_t, color);
    xprop(pix_t,   thickness_left);
    xprop(pix_t,   thickness_top);
    xprop(pix_t,   thickness_right);
    xprop(pix_t,   thickness_bottom);
    rectangle & thickness(pix_t all){
        return thickness(all, all, all, all);
    }
    rectangle & thickness(pix_t left, pix_t top, pix_t right, pix_t bottom){
        _thickness_left = left;
        _thickness_top = top;
        _thickness_right = right;
        _thickness_bottom = bottom;
        return this[0];
    }
    void draw();
    operator can_drawable() {
        return can_drawable(this);
    }
};

struct dash_line;
struct dash_line {
    xpositionx(dash_line, 
        pix_t length, {
            _length = length;
        }, {
            _length = 0;
            _solid = 2;
            _empty = 2;
            _color = pan_color;
            _thickness = pan_thickness;
            _orientation = horizon;
        });
    xpoint(dash_line);
    xprop(pix_t,         length);
    xprop(pix_t,         empty);
    xprop(pix_t,         solid);
    xprop(color_t,       color);
    xprop(pix_t,         thickness);
    xprop(::orientation, orientation);

    void draw();
    operator can_drawable(){
        return can_drawable(this);
    }
};

struct ellipse;
struct ellipse : detail::aligner<ellipse> {
    ellipse() : 
        detail::aligner<ellipse>(){
        _color = pan_color;
        _fill = transparent;
        _thickness = pan_thickness;
        _origin = center;
        _vorigin = vcenter;
    }
    ellipse(pos_t x, pos_t y):
        ellipse() {
        _x = x;
        _y = y;
    }
    ellipse(point p) :
        ellipse(p.x, p.y) {
    }
    ellipse(point p, pix_t r) :
        ellipse(p.x, p.y, r * 2, r * 2) {
    }
    ellipse(point p, pix_t width, pix_t height) :
        ellipse(p.x, p.y, width, height) {
    }
    ellipse(pos_t x, pos_t y, pix_t r) :
        ellipse(x, y, r * 2, r * 2) {
    }
    ellipse(pos_t x, pos_t y, pix_t width, pix_t height):
        ellipse(x, y) {
        _width = width;
        _height = height;
    }
    xprop(color_t,  color);
    xprop(color_t,  fill);
    xprop(pix_t,    thickness);

    auto & r(pix_t value) {
        _height = _width = value * 2;
        return this[0];
    }
    void draw();
    operator can_drawable() {
        return can_drawable(this);
    }
protected:
    point adjust(pix_t width, pix_t height) {
        auto x = _x;
        auto y = _y;

        if (_width == 0) {
            _width = width;
        }
        if (_height == 0) {
            _height = height;
        }
        if (_origin == left) {
            x += _width / 2;
        }
        else if (_origin == right) {
            x -= _width / 2;
        }
        if (_vorigin == top) {
            y -= _height / 2;
        }
        else if (_vorigin == bottom) {
            y += _height / 2;
        }
        return point(x, y);
    }
};

struct text;
struct text : detail::aligner<text>{
    xpositionx(text,
        text_t value, {
            _value = value;
        }, {
            _value = "";
            _font = pix;
            _font_size = 12;
            _color = pan_color;
            _align = left;
            _valign = top;
            _thickness = 1;
        });
private:
    font_t _font;
    pix_t  _thickness;
public:
    text & font(font_t value);
    font_t font(){
        return _font;
    }
    text & thickness(pix_t value);
    pix_t thickness(){
        return _thickness;
    }
    xprop(pix_t,       font_size);
    xprop(color_t,     color);
    xprop(text_t,      value);
    xprop(align_type,  align);
    xprop(valign_type, valign);
    text & font_height(pix_t * value);
    text & content_width(pix_t * value);
    pix_t font_height() {
        pix_t height;
        font_height(& height);
        return height;
    }
    pix_t content_width() {
        pix_t width;
        content_width(& width);
        return width;
    }

    void draw();
    operator can_drawable(){
        return can_drawable(this);
    }
};

struct polyline;
struct polyline : detail::poly<polyline>{
    polyline(){}
    polyline(std::initializer_list<point> data) : 
        detail::poly<polyline>(data){
    }
    void draw();
    operator can_drawable() {
        return can_drawable(this);
    }
};

struct polygen;
struct polygen : detail::poly<polygen>{
    polygen() {}
    polygen(std::initializer_list<point> data) :
        detail::poly<polygen>(data) {
    }
    void draw();
    operator can_drawable() {
        return can_drawable(this);
    }
};


