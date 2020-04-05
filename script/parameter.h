#ifndef PARAMETER_H
#define PARAMETER_H

#include <string>
#include <vector>

#include "types.h"

namespace tw
{

enum BasicParameterType : int32_t
{
    Empty,
    String,
    Int,
    Float,
    Boolean,
    Point,
    Rect,
    DateTime,
    Object
};

typedef int32_t ObjectReference;

struct ParameterType
{
public:
    inline ParameterType() : basic_type(Empty), obj_ref(-1) {}
    inline ParameterType(BasicParameterType basic_type) : basic_type(basic_type), obj_ref(-1) {}
    inline ParameterType(ObjectReference obj_ref) : basic_type(Object), obj_ref(obj_ref) {}

    BasicParameterType basic_type;
    ObjectReference    obj_ref;
};

inline bool operator==(const ParameterType &pt1, const ParameterType &pt2)
{ return pt1.basic_type == pt2.basic_type && pt1.obj_ref == pt2.obj_ref; }

inline bool operator!=(const ParameterType &pt1, const ParameterType &pt2)
{ return pt1.basic_type != pt2.basic_type || pt1.obj_ref != pt2.obj_ref; }

class ParameterObject
{
public:
    virtual ~ParameterObject();
    virtual void copyTo(void *&) const {}
    virtual void moveTo(void *&) {}

    virtual ObjectReference objRef() const = 0;
};

template<class T>
class ParameterObjectBase : public ParameterObject
{
public:
    typedef T ObjectType;

    template<class... _Args>
    inline ParameterObjectBase(_Args... __args) : obj(__args...) {}

    T obj;

    static ObjectReference ref;
    inline ObjectReference objRef() const override { return ref; }
};

class Parameter
{
public:
    typedef BasicParameterType Type;

    inline Parameter() : value(nullptr), _type(Empty), is_reference(false) {}
    inline Parameter(const Parameter &src) : Parameter() { operator=(src); }
    inline Parameter(Parameter &&src) : Parameter() { operator=(std::move(src)); }
    inline ~Parameter() { clear(); }

    void clear();
    inline bool empty() const { return value == nullptr; }

    inline Type type() const { return _type; }

    void assign(const std::string &);
    void assign(int32_t);
    void assign(double);
    void assign(bool);
    void assign(const _Point &);
    void assign(const _Rect &);
    void assign(const _DateTime &);
    void assign(const ParameterObject &);
    void assign(ParameterObject &&);

    template<class T, class... _Args>
    inline T &createObject(_Args... __args)
    { clear(); value = new ParameterObjectBase<T>(__args...); _type = Object; return static_cast<ParameterObjectBase<T>*>(value)->obj; }

    inline const std::string &asString() const   { return *static_cast<std::string*>(value); }
           int32_t            asInt() const;
           double             asFloat() const;
    inline bool               asBoolean() const  { return *static_cast<bool*>(value); }
    inline const _Point      &asPoint() const    { return *static_cast<_Point*>(value); }
    inline const _Rect       &asRect() const     { return *static_cast<_Rect*>(value); }
    inline const _DateTime   &asDateTime() const { return *static_cast<_DateTime*>(value); }

    template<class T>
    inline const T           &asObject() const   { return static_cast<ParameterObjectBase<T>*>(value)->obj; }
    inline ObjectReference    objectRef() const  { return static_cast<ParameterObject*>(value)->objRef(); }

    void copyReference(Parameter &dest) const;

    Parameter &operator=(const Parameter &);
    Parameter &operator=(Parameter &&);

private:
    void *value;
    Type _type;
    bool is_reference;
};

std::ostream &operator <<(std::ostream &os, const Parameter &param);

inline Parameter referenceTo(const Parameter &src)
{ Parameter param; src.copyReference(param); return param; }

typedef std::vector<Parameter> ParameterList;

} // namespace tw

#endif // PARAMETER_H
