#pragma once
#define null16          32767
#include<TFT_eSPI.h>
#pragma push(max)
#pragma push(min)
#undef max
#undef min
#include<initializer_list>
#include<list>
#include<vector>
#include<queue>
#include<stdint.h>
#pragma pop(min)
#pragma pop(max)
#include<string>

typedef const char *        text_t;
typedef int16_t             pos_t;
typedef uint16_t            pix_t;
typedef uint32_t            color_t;
typedef std::queue<double>  doubles;

enum class font_t      : uint16_t {};
enum class align_type  : uint8_t {};
enum class valign_type : uint8_t {};
enum class orientation : uint8_t {};

constexpr orientation horizon  = orientation(0);
constexpr orientation vertical = orientation(1);

constexpr align_type  left    = align_type(0);
constexpr align_type  center  = align_type(1);
constexpr align_type  right   = align_type(2);
constexpr valign_type top     = valign_type(0);
constexpr valign_type vcenter = valign_type(1);
constexpr valign_type bottom  = valign_type(2);


constexpr color_t black             = 0x0000;      /*   0,   0,   0 */
constexpr color_t navy              = 0x000F;      /*   0,   0, 128 */
constexpr color_t drakgreen         = 0x03E0;      /*   0, 128,   0 */
constexpr color_t darkcyan          = 0x03EF;      /*   0, 128, 128 */
constexpr color_t maroon            = 0x7800;      /* 128,   0,   0 */
constexpr color_t purple            = 0x780F;      /* 128,   0, 128 */
constexpr color_t olive             = 0x7BE0;      /* 128, 128,   0 */
constexpr color_t lightgray         = 0xC618;      /* 192, 192, 192 */
constexpr color_t darkgray          = 0x7BEF;      /* 128, 128, 128 */
constexpr color_t gray              = 0xCE79;
constexpr color_t blue              = 0x001F;      /*   0,   0, 255 */
constexpr color_t green             = 0x07E0;      /*   0, 255,   0 */
constexpr color_t cyan              = 0x07FF;      /*   0, 255, 255 */
constexpr color_t red               = 0xF800;      /* 255,   0,   0 */
constexpr color_t magenta           = 0xF81F;      /* 255,   0, 255 */
constexpr color_t yellow            = 0xFFE0;      /* 255, 255,   0 */
constexpr color_t white             = 0xFFFF;      /* 255, 255, 255 */
constexpr color_t orange            = 0xFDA0;      /* 255, 180,   0 */
constexpr color_t greenyellow       = 0xB7E0;      /* 180, 255,   0 */
constexpr color_t pink              = 0xFC9F;
constexpr color_t transparent       = 0xFF000000;
constexpr color_t pan_color         = color_t(black);
constexpr font_t  pix               = font_t(1);
constexpr pix_t   pan_thickness     = 1;

struct can_drawable {
    typedef void (can_drawable::*invoke_t)();
    typedef can_drawable * can_drawable_p;
    template<class type>
    can_drawable(type * obj){
        invoke = invoke_t(& type::draw);
        object = can_drawable_p(obj);
    }
    void draw(){
        (object->*invoke)();
    }
private:
    void empty(){}
    invoke_t       invoke;
    can_drawable * object;
};

extern std::vector<color_t> classic_colors;

struct point{
    pos_t x;
    pos_t y;
    point():x(0), y(0){}
    point(pos_t x, pos_t y): 
        x(x), y(y){
    }
    point operator()(const point & p){
        return point{ this->x + p.x, this->y + p.y };
    }
    point operator()(pos_t x, pos_t y) {
        return point{ this->x + x, this->y + y };
    }
    point operator + (point value) {
        return point{ this->x + value.x, this->y + value.y };
    }
    point operator - (point value) {
        return point{ this->x - value.x, this->y - value.y };
    }
};

