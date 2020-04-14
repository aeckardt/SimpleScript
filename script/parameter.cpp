#include <ostream>

#include "lexer.h"
#include "parameter.h"

using namespace tw;

// The destructor is here to have a non-pure virtual function in the class
// It is the key function needed for the vtable,
// which will otherwise be emitted in each translation unit
ParameterObject::~ParameterObject()
{
}

Parameter::Type getParameterType(const lx::TokenId &token_id)
{
    switch (token_id) {
    case lx::Integer:
        return Int;
    case lx::Float:
        return Float;
    case lx::String:
        return String;
    default:
        return Empty;
    }
}

void Parameter::clear()
{
    if (empty())
        return;
    else if (is_reference) {
        value = nullptr;
        _type = Empty;
        is_reference = false;
        return;
    }

    switch (_type) {
    case Empty:
        // do nothing...
        break;
    case String:
        delete static_cast<std::string*>(value);
        break;
    case Int:
        delete static_cast<int32_t*>(value);
        break;
    case Float:
        delete static_cast<double*>(value);
        break;
    case Boolean:
        delete static_cast<bool*>(value);
        break;
    case Point:
        delete static_cast<_Point*>(value);
        break;
    case Rect:
        delete static_cast<_Rect*>(value);
        break;
    case DateTime:
        delete static_cast<_DateTime*>(value);
        break;
    case Object:
        delete static_cast<ParameterObject*>(value);
        break;
    }
    _type = Empty;
    value = nullptr;
}

void Parameter::assign(const std::string &str)
{
    clear();
    value = new std::string(str);
    _type = String;
}

void Parameter::assign(int32_t i)
{
    clear();
    value = new int32_t(i);
    _type = Int;
}

void Parameter::assign(double f)
{
    clear();
    value = new double(f);
    _type = Float;
}

void Parameter::assign(bool b)
{
    clear();
    value = new bool(b);
    _type = Boolean;
}

void Parameter::assign(const _Point &pt)
{
    clear();
    value = new _Point(pt);
    _type = Point;
}

void Parameter::assign(const _Rect &rect)
{
    clear();
    value = new _Rect(rect);
    _type = Rect;
}

void Parameter::assign(const _DateTime &dt)
{
    clear();
    value = new _DateTime(dt);
    _type = DateTime;
}

void Parameter::assign(const ParameterObject &o)
{
    clear();
    o.copyTo(value);
    _type = Object;
}

double Parameter::asFloat() const
{
    if (_type == Float)
        return *static_cast<double*>(value);
    else if (_type == Int) {
        int32_t val = *static_cast<int32_t*>(value);
        return static_cast<double>(val);
    }
    return 0;
}

int32_t Parameter::asInt() const
{
    if (_type == Int)
        return *static_cast<int32_t*>(value);
    else if (_type == Float) {
        double val = *static_cast<double*>(value);
        return static_cast<int32_t>(val);
    }
    return 0;
}

void Parameter::copyReference(Parameter &dest) const
{
    dest.clear();
    dest.value = value;
    dest._type = _type;
    dest.is_reference = true;
}

Parameter &Parameter::operator=(const Parameter &src)
{
    clear();
    if (src.is_reference) {
        value = src.value;
        _type = src._type;
        is_reference = true;
    } else {
        switch (src.type()) {
        case Empty:
            _type = Empty;
            break;
        case String:
            assign(src.asString());
            break;
        case Int:
            assign(src.asInt());
            break;
        case Float:
            assign(src.asFloat());
            break;
        case Boolean:
            assign(src.asBoolean());
            break;
        case Point:
            assign(src.asPoint());
            break;
        case Rect:
            assign(src.asRect());
            break;
        case DateTime:
            assign(src.asDateTime());
            break;
        case Object:
            static_cast<ParameterObject*>(src.value)->copyTo(value);
            _type = Object;
            break;
        }
    }
    return *this;
}

Parameter &Parameter::operator=(Parameter &&src)
{
    clear();
    value = src.value;
    _type = src._type;
    is_reference = src.is_reference;
    src.value = nullptr;
    src._type = Empty;
    src.is_reference = false;
    return *this;
}

std::ostream &tw::operator<<(std::ostream &os, const Parameter &param)
{
    switch (param.type()) {
    case Empty:
        return os;
    case String:
        return os << param.asString();
    case Int:
        return os << param.asInt();
    case Float:
        return os << param.asFloat();
    case Boolean:
        return os << (param.asBoolean() ? "true" : "false");
    case Point: {
        const _Point &pt = param.asPoint();
        return os << "(" << pt.x() << ", " << pt.y() << ")";
    }
    case Rect: {
        const _Rect &rect = param.asRect();
        return os << "(" << rect.x() << ", " << rect.y() << ", " << rect.width() << ", " << rect.height() << ")";
    }
    case DateTime: {
        const _DateTime &dt = param.asDateTime();
        return os << dt.toString("yyyy-MM-dd HH:mm:ss").toStdString();
    }
    default:
        return os;
    }
}
