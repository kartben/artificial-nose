#pragma once
#include"seeed_graphics_base.h"
#include"float.h"  

struct range {
    double max_value;
    double min_value;
    size_t max_count;

    template<class type>
    range(type & list) {
        max_value = 0;
        min_value = 0;
        max_count = 0;
        
        if (list.size() == 0) {
            return;
        }

        min_value = max_value = list[0].front();

        for (size_t i = 0; i < list.size(); i++) {
            if (max_count < list[i].size()) {
                max_count = list[i].size();
            }
            for (size_t j = 0; j < list[i].size(); j++) {
                if (max_value < list[i].front()) {
                    max_value = list[i].front();
                }
                else if (min_value > list[i].front()) {
                    min_value = list[i].front();
                }
                list[i].push(list[i].front());
                list[i].pop();
            }
        }
    }
};

struct match_tick {
    double abs_value;
    double top_value;
    double start_value;
    double step;
    pix_t  tick;

    match_tick(
        double max_value, 
        double min_value, 
        double based_on,
        pix_t max_tick, 
        pix_t min_tick) {
        constexpr bool found = true;
        constexpr bool not_found = false;
        double  exp;
        double  val;
        bool    is_neg = false;
        bool    need_find = true;
        int32_t max_val = 0;
        int32_t min_val = 0;
        auto    min_step = { 5, 4, 2, 1 };
        max_value -= based_on;
        min_value -= based_on;
        start_value = 0;
        abs_value = 0;
        top_value = 0;
        tick = 0;
        step = 0;

        auto invoke = [&](int min_step){
            min_val = int32_t(val) + 1;
            max_val = min_val * 15 / 100 * 10; //乘以1.5倍后并舍去个位，除以100再乘以10用于舍去个位
            while (min_tick <= max_tick) {
                while (max_val > min_val) {
                    if (max_val % min_tick == 0) {
                        tick = min_tick;
                        step = double(max_val / min_tick) * pow(10, -exp);
                        top_value = step * tick;
                        abs_value = top_value - start_value;
                        if (is_neg) {
                            start_value = -top_value;
                            top_value = 0;
                        }
                        min_tick = max_tick;
                        return found;
                    }
                    max_val -= 5;
                }
                min_tick++;
            }
            return not_found;
        };

        if (min_value >= 0) {
            if (max_value <= 1) {
                exp = round(-log10(max_value)) + 2;
            }
            else {
                exp = -round(log10(max_value)) + 2;
            }
            val = max_value * pow(10, exp);
            
        }
        else if (max_value <= 0){
            if (min_value >= -1) {
                exp = round(-log10(-min_value)) + 2;
            }
            else {
                exp = -round(log10(-min_value)) + 2;
            }
            is_neg = true;
            val = -min_value * pow(10, exp);
        }
        else {
            while(min_tick <= max_tick) {
                for (double i = 1.0, j = 1; i < 1.5; i += 0.1) {
                    auto tmp = match_tick((max_value - min_value) * i, 0, 0, max_tick, min_tick);
                    min_tick++;

                    while (j * tmp.step < -min_value) {
                        j++;
                    }
                    
                    auto start = j * tmp.step;
                    auto top = tmp.abs_value - start;

                    if (top >= max_value) {
                        abs_value = tmp.abs_value;
                        top_value = top;
                        start_value = -start;
                        step = tmp.step;
                        tick = tmp.tick;
                        min_tick = max_tick + 1;
                        break;
                    }
                }
            }
            need_find = false;
        }
        if (need_find){
            for (auto m : min_step){
                if (invoke(m) == found){
                    break;
                }
            }
        }
        top_value += based_on;
        start_value += based_on;
    }
};

struct line_chart {
    template<class list, class callback>
    void draw_tick(
        list & items, 
        size_t tick_count,
        pix_t total_pix,
        callback call) {
        auto tick_step = 1.0 * items.size() / tick_count;
        auto tick_pix_step = 1.0 * total_pix / tick_count;
        auto tick_sum = 0.0;
        auto tick_pix_sum = 0.0;

        for (size_t i = 0; i <= tick_count; i++, tick_sum += tick_step, tick_pix_sum += tick_pix_step) {
            auto index = size_t(round(tick_sum));
            auto x = pos_t(round(tick_pix_sum));
            call(i, x);
        }
    }
public:
    xpositionx(
        line_chart, 
        xlist(pix_t width, pix_t height), {
            _height = height;
            _width = width;
        }, {
            _x_max_tick_count = 10;
            _x_min_tick_count = 3;
            _y_max_tick_count = 8;
            _y_min_tick_count = 3;
            _x_skip_tick = 0;
            _tick = 8;
            _x_auxi_role = dash_line().color(gray);
            _x_role_color = pan_color;
            _x_tick_color = pan_color;
            _x_role_thickness = pan_thickness;
            _y_role_color = pan_color;
            _y_tick_color = pan_color;
            _y_role_thickness = pan_thickness;
            _format = "%g";
            _color = classic_colors;
            _show_circle = std::initializer_list<bool>{ true };
            _based_on = 0;
        });
    xpoint(line_chart);
    xprop(pix_t,     height);
    xprop(pix_t,     width) ;
    xprop(pix_t,     x_max_tick_count);
    xprop(pix_t,     x_min_tick_count);
    xprop(pix_t,     y_max_tick_count);
    xprop(pix_t,     y_min_tick_count);
    xprop(float,     x_skip_tick);
    xprop(pix_t,     tick);
    xprop(dash_line, x_auxi_role);
    xprop(color_t,   x_role_color);
    xprop(color_t,   x_tick_color);
    xprop(pix_t,     x_role_thickness);
    xprop(color_t,   y_role_color);
    xprop(color_t,   y_tick_color);
    xprop(pix_t,     y_role_thickness);
    xprop(const char *, format);
    xprop(double,    based_on);

private:
    std::vector<color_t> _color;
public:
    template<class ... arg>
    auto & color(color_t first, arg ... list){
        _color = std::vector<color_t>{ first, list... };
        return this[0];
    }
private:
    std::vector<bool> _show_circle;
public:
    template<class ... arg>
    auto & show_circle(bool first, arg ... list) {
        _show_circle = std::vector<bool>{ first, list... };
        return this[0];
    }

    xprop(std::vector<text_t>, note);
private:
    std::vector<doubles> _value;
public:
    auto & value(doubles const & list){
        _value.clear();
        _value.push_back(list);
        return this[0];
    }
    auto & value(std::vector<double> const & list){
        doubles que;
        for(auto i : list){
            que.push(i);
        }
        return value(que);
    }
    auto & value(std::vector<doubles> const & items){
        _value = items;
        return this[0];
    }

    void draw() {
        //·绘制y轴/x轴
        //·绘制x轴刻度
        auto y_tick_value_template = text().origin(right).vorigin(vcenter).color(_y_tick_color);
        auto x_tick_value_template = text().origin(center).vorigin(top).color(_x_tick_color);
        auto r = range(_value);
        auto m = match_tick(r.max_value, r.min_value, _based_on, _y_max_tick_count, _y_min_tick_count);
        auto w = pix_t(0);
        auto max_y_tick_pix_width = 0;
        char buf[256];
        char * p = buf;
        std::vector<text> y_tick_value;
        for (size_t i = 0; i <= m.tick; i++, p += strlen(p) + 1) {
            sprintf(p, _format, m.start_value + i * m.step);
            y_tick_value.push_back(
                y_tick_value_template.value(p).content_width(&w)
            );
            if (max_y_tick_pix_width < w) {
                max_y_tick_pix_width = w;
            }
        }

        //1.5 -> 0.5为y轴刻度值高度的一半 1.0为x轴刻度的高度
        auto x_extend_step = _value.size() != 0 ? 1.5 : 0.5;
        auto y_extend_height = _tick + x_extend_step * x_tick_value_template.font_height(); 
        auto y_extend_width  = _tick + max_y_tick_pix_width;
        auto x       = _x + y_extend_width;
        auto y       = _y + 0.5 * x_tick_value_template.font_height();
        auto width   = _width - y_extend_width;
        auto height  = _height - y_extend_height;
        auto origin  = point(x, y + height);
        auto y_start = origin(0, _tick);
        auto y_end   = origin(0, -(pos_t)height);
        auto x_start = origin(-_tick, 0);
        auto x_end   = origin(width, 0);
        

        if (_value.size()) draw_tick(_value, m.tick, height, [&](size_t i, pix_t y) {
            auto p0 = origin(0, -(pos_t)y);
            auto p1 = origin(-_tick, -(pos_t)y);

            //·绘制水平辅助线
            if (i != 0) {
                _x_auxi_role.xy(p0).length(width).draw();
            }

            //·绘制刻度线和刻度值
            line(p0, p1).color(_x_tick_color).draw();
            y_tick_value[i].xy(p1).draw();
        });

        //·绘制x轴刻度
        auto x_tick           = std::min(_note.size(), size_t(_x_max_tick_count));
        auto x_skip_half_tick = round(_x_skip_tick) > _x_skip_tick;
        auto x_skip_tick      = pix_t(_x_skip_tick);
        auto x_tick_addition  = x_skip_half_tick ? 0 : 1;
        auto x_step           = double(width) / (r.max_count + x_skip_tick - x_tick_addition);
        auto x_offset         = x_skip_half_tick ? x_step / 2 : 0;
        auto x_tick_width     = pix_t(x_step);
        
        if (_note.size()) draw_tick(_note, x_tick + x_skip_tick - x_tick_addition, width, [&](size_t i, pix_t x) {
            auto p0 = origin(x, 0);
            auto p1 = origin(x, _tick);
            line(p0, p1).color(_x_tick_color).draw();

            if (i < x_skip_tick) {
                return;
            }

            auto index = i - x_skip_tick;
            auto p2 = p1(pix_t(x_offset), 0);

            if (index >= _note.size()) {
                return;
            }

            text(p2, _note[index])
                .origin(center)
                .width(x_tick_width)
                .color(_x_tick_color)
                .draw();
        });

        //·绘制折线
        for (size_t i = 0; i < _value.size(); i++) {
            doubles cur = _value[i];
            auto    default_color = _color[std::min(i, _color.size() - 1)];
            auto    show_circle = _show_circle[std::min(i, _show_circle.size() - 1)];
            std::vector<point> value_point;

            for(int j = 0; j < cur.size() - 1; j++){
                cur.push(cur.front());
                auto a  = cur.front(); cur.pop();
                auto b  = cur.front();
                auto ha = pos_t(round((a - m.start_value) / m.abs_value * height));
                auto hb = pos_t(round((b - m.start_value) / m.abs_value * height));
                auto pa = origin(pos_t((j + x_skip_tick) * x_step + x_offset), -(pos_t)ha);
                auto pb = origin(pos_t((j + x_skip_tick + 1) * x_step + x_offset), -(pos_t)hb);
                line(pa, pb).color(default_color).draw();
                if (j == 0) {
                    value_point.push_back(pa);
                }
                value_point.push_back(pb);
            }
            if (show_circle) {
                for (auto p : value_point) {
                    ellipse(p, 5, 5).color(default_color).fill(white).origin(center).vorigin(vcenter).draw();
                }
            }
        }
        line(x_start, x_end).color(_x_role_color).thickness(_x_role_thickness).draw();
        line(y_start, y_end).color(_y_role_color).thickness(_y_role_thickness).draw();
    }

    operator can_drawable(){
        return can_drawable(this);
    }
};

// struct ring_chart {
//     xpositionx(
//         ring_chart,
//         xlist(pix_t r), {
//             _r = r;
//         }, {
//             _r = 0;
//             _angle_offset = 0;
//             _color = classic_colors;
//             _thickness.push_back(20);
//         });
    
//     xpoint(ring_chart);
//     xprop(pix_t,    r);
//     xprop(float,    angle_offset);
//     xvprop(color_t, color);
//     xvprop(pix_t,   thickness);
//     xvprop(double,  value);

//     void draw() {
//         if (_value.size() == 0) {
//             return;
//         }
        
//         auto color = _color.cbegin();
//         auto thickness = _thickness.cbegin();
//         auto sum = 0.0;
//         auto offset = _angle_offset;

//         for (auto v : _value) {
//             sum += v;
//         }
//         for (auto v : _value) {
//             auto angle = 360.0 * v / sum;
//             // ellipse(_x, _y, _r)
//             //     .start_angle(offset)
//             //     .end_angle(offset += angle)
//             //     .color(*color)
//             //     .thickness(*thickness)
//             //     .draw();

//             if (++color == _color.cend()) {
//                 color = _color.cbegin();
//             }
//             if (thickness + 1 != _thickness.cend()) {
//                 thickness++;
//             }
//         }
//     }
// };

// #endif
