#ifndef SCRIPTENGINE_H
#define SCRIPTENGINE_H

#include <string>
#include <map>
#include <vector>

#include <QPoint>
#include <QRect>
#include <QDateTime>
#include <QBrush>

#include "lexer.h"
#include "parser.h"

namespace tw
{

using tn::Lexer;
using tn::Token;
using tn::TokenId;

using ps::Parser;
using ps::Node;

using std::map;
using std::move;
using std::ostream;
using std::string;
using std::vector;

enum BasicParameterType : int16_t
{
    Empty,
    String,
    Int,
    Float,
    Point,
    Rect,
    Boolean,
    DateTime,
    Object
};

typedef int16_t ObjectReference;

class ParameterType
{
public:
    ParameterType() : basic_type(Empty), obj_ref(-1) {}
    ParameterType(BasicParameterType basic_type) : basic_type(basic_type), obj_ref(-1) {}
    ParameterType(ObjectReference obj_ref) : basic_type(Object), obj_ref(obj_ref) {}

    BasicParameterType basic_type;
    ObjectReference    obj_ref;
};

inline bool operator==(const ParameterType &pt1, const ParameterType &pt2) { return pt1.basic_type == pt2.basic_type && pt1.obj_ref == pt2.obj_ref; }
inline bool operator!=(const ParameterType &pt1, const ParameterType &pt2) { return pt1.basic_type != pt2.basic_type || pt1.obj_ref != pt2.obj_ref; }

class ParameterObject
{
public:
    virtual ~ParameterObject() {}
    virtual void copyTo(void *&) const = 0;
    virtual void moveTo(void *&) = 0;

    virtual ObjectReference objRef() const = 0;
};

template<class T>
class ParameterObjectBase : public ParameterObject
{
public:
    typedef T ObjectType;

    inline ParameterObjectBase() {}
    inline ParameterObjectBase(const ParameterObjectBase &src) { obj = src.obj; }
    inline ParameterObjectBase(ParameterObjectBase &&src) { obj = move(src.obj); }
    inline ParameterObjectBase(const T &src) { obj = src; }
    inline ParameterObjectBase(T &&src) { obj = move(src); }
    virtual inline ~ParameterObjectBase() override {}

    inline ParameterObjectBase &operator=(const ParameterObjectBase &src) { obj = src.obj; return *this; }
    inline ParameterObjectBase &operator=(ParameterObjectBase &&src) { obj = move(src.obj); return *this; }
    inline ParameterObjectBase &operator=(const T &src) { obj = src; return *this; }
    inline ParameterObjectBase &operator=(T &&src) { obj = move(src); return *this; }

    inline void copyTo(void *&ptr) const override { ptr = new ParameterObjectBase(*this); }
    inline void moveTo(void *&ptr) override { ptr = new ParameterObjectBase(move(*this)); }

    ObjectType obj;

    static ObjectReference ref;
    inline ObjectReference objRef() const override { return ref; }
};

class Parameter
{
public:
    typedef BasicParameterType Type;

    inline Parameter() : type_(Empty), value(nullptr), isReference(false) {}
    inline Parameter(const Parameter &src) : Parameter() { operator=(src); }
    inline Parameter(Parameter &&src) : Parameter() { operator=(move(src)); }
    inline ~Parameter() { clear(); }

    void clear();
    inline bool empty() const { return value == nullptr; }

    inline Type type() const { return type_; }

    void assign(const string &);
    void assign(int32_t);
    void assign(double);
    void assign(bool);
    void assign(const QPoint &);
    void assign(const QRect &);
    void assign(const QDateTime &);
    void assign(const ParameterObject &);
    void assign(ParameterObject &&);

    template<class T, class... _Args>
    inline typename ParameterObjectBase<T>::ObjectType &createObject(_Args... __args)
    { assign(ParameterObjectBase<T>((__args)...)); return static_cast<ParameterObjectBase<T>*>(value)->obj; }

    const string      &asString() const   { return *static_cast<string*>(value); }
    int32_t            asInt() const;
    double             asFloat() const;
    const QPoint      &asPoint() const    { return *static_cast<QPoint*>(value); }
    const QRect       &asRect() const     { return *static_cast<QRect*>(value); }
    bool               asBoolean() const  { return *static_cast<bool*>(value); }
    const QDateTime   &asDateTime() const { return *static_cast<QDateTime*>(value); }

    template<class T>
    const T           &asObject() const
    { return static_cast<ParameterObjectBase<T>*>(value)->obj; }

    ObjectReference   objectRef() const
    { return static_cast<ParameterObject*>(value)->objRef(); }

    void copyReference(Parameter &dest) const;

    Parameter &operator=(const Parameter &);
    Parameter &operator=(Parameter &&);

private:
    Type type_;
    void *value;
    bool isReference;
};

ostream &operator <<(ostream &os, const Parameter &param);

inline Parameter referenceTo(const Parameter &src)
{ Parameter param; src.copyReference(param); return param; }

typedef vector<Parameter> ParameterList;

typedef void(* OutputFnc)(const Parameter &, const QBrush &);
typedef bool(* CommandFnc)(const ParameterList &, Parameter &);

struct Command
{
    CommandFnc callback_fnc;
    vector<vector<ParameterType>> param_types;
    ParameterType return_type;
};

class ASTWalker
{
public:
    ASTWalker() : output_fnc(nullptr) {}

    bool run(const string &str);

    inline void registerCommand(const string &name, const CommandFnc &callback_fnc,
        const vector<vector<ParameterType>> &param_types, const ParameterType &return_type)
    { commands[name] = {callback_fnc, param_types, return_type}; }

    template<class T>
    inline void registerObject(const string &name)
    { obj_names[ParameterObjectBase<T>::ref] = name; }

    inline void setErrorOutput(const OutputFnc &fnc)
    { output_fnc = fnc; }

private:
    OutputFnc output_fnc;
    void errorMsg(const char *) const;

    map<string, Command> commands;
    map<ObjectReference, string> obj_names;

    map<string, Parameter> vars;
    Parameter return_value;

    Parameter getConstValue(const Node &node);
    bool traverse(const Node &node);
    bool traverseAssignment(const Node &node);
    bool traverseExpr(const Node &node);
    bool traverseFunction(const Node &node);
    bool traverseIfStatement(const Node &node);
    bool traverseOperation(const tn::TokenId &op, const Parameter &p1, const Parameter &p2);

    // the following types, variables and functions are exclusively used to validate the syntax
    typedef vector<ParameterType> ParameterTypeList;

    map<string, ParameterType> var_types;
    ParameterType return_value_type;

    Parameter::Type getParamType(const Node &node);
    bool validate(const Node &node);
    bool validateAssignment(const Node &node);
    bool validateCommand(const string &command, const ParameterTypeList &param_types);
    bool validateExpr(const Node &node);
    bool validateFunction(const Node &node);
    bool validateIfStatement(const Node &node);
    bool validateOperation(const tn::TokenId &op, const Parameter::Type &pt1, const Parameter::Type &pt2);
    bool validateParamType(const Node &node, ParameterTypeList *param_types = nullptr);
};

} // namespace tw

#endif // SCRIPTENGINE_H