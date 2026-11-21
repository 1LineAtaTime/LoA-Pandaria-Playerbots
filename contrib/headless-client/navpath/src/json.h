// Minimal JSON reader/writer used by navpath's line protocol.
// Supports objects, arrays, numbers, strings (with basic escapes), true/false/null.
#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

namespace json
{
    struct Value
    {
        enum Type { Null, Bool, Number, String, Array, Object };
        Type type = Null;
        bool b = false;
        double n = 0.0;
        std::string s;
        std::vector<Value> a;
        std::vector<std::pair<std::string, Value>> o;

        Value const* get(char const* key) const
        {
            if (type != Object)
                return nullptr;
            for (auto const& kv : o)
                if (kv.first == key)
                    return &kv.second;
            return nullptr;
        }

        double num(char const* key, double def) const
        {
            Value const* v = get(key);
            if (!v)
                return def;
            if (v->type == Number)
                return v->n;
            if (v->type == Bool)
                return v->b ? 1.0 : 0.0;
            return def;
        }

        bool boolean(char const* key, bool def) const
        {
            Value const* v = get(key);
            if (!v)
                return def;
            if (v->type == Bool)
                return v->b;
            if (v->type == Number)
                return v->n != 0.0;
            return def;
        }

        std::string str(char const* key, std::string const& def) const
        {
            Value const* v = get(key);
            return (v && v->type == String) ? v->s : def;
        }
    };

    class Parser
    {
    public:
        explicit Parser(std::string const& text) : _p(text.c_str()), _e(text.c_str() + text.size()) { }

        bool parse(Value& out, std::string& err)
        {
            if (!value(out, 0))
            {
                err = _err.empty() ? "invalid json" : _err;
                return false;
            }
            ws();
            if (_p != _e)
            {
                err = "trailing characters";
                return false;
            }
            return true;
        }

    private:
        char const* _p;
        char const* _e;
        std::string _err;

        void ws()
        {
            while (_p < _e && (*_p == ' ' || *_p == '\t' || *_p == '\r' || *_p == '\n'))
                ++_p;
        }

        bool lit(char const* word)
        {
            size_t len = std::strlen(word);
            if (size_t(_e - _p) < len || std::strncmp(_p, word, len) != 0)
                return false;
            _p += len;
            return true;
        }

        bool str(std::string& out)
        {
            if (_p >= _e || *_p != '"')
                return false;
            ++_p;
            while (_p < _e && *_p != '"')
            {
                char c = *_p++;
                if (c == '\\' && _p < _e)
                {
                    char esc = *_p++;
                    switch (esc)
                    {
                        case 'n': out += '\n'; break;
                        case 't': out += '\t'; break;
                        case 'r': out += '\r'; break;
                        case 'b': out += '\b'; break;
                        case 'f': out += '\f'; break;
                        case 'u':
                        {
                            // Only ASCII code points are kept; others become '?'.
                            if (_e - _p < 4)
                                return false;
                            unsigned code = unsigned(std::strtoul(std::string(_p, 4).c_str(), nullptr, 16));
                            _p += 4;
                            out += code < 0x80 ? char(code) : '?';
                            break;
                        }
                        default: out += esc; break;
                    }
                }
                else
                    out += c;
            }
            if (_p >= _e)
                return false;
            ++_p;
            return true;
        }

        bool value(Value& v, int depth)
        {
            if (depth > 32)
            {
                _err = "json nested too deeply";
                return false;
            }
            ws();
            if (_p >= _e)
                return false;
            char c = *_p;
            if (c == '{')
            {
                ++_p;
                v.type = Value::Object;
                ws();
                if (_p < _e && *_p == '}')
                {
                    ++_p;
                    return true;
                }
                for (;;)
                {
                    ws();
                    std::string key;
                    if (!str(key))
                        return false;
                    ws();
                    if (_p >= _e || *_p != ':')
                        return false;
                    ++_p;
                    Value child;
                    if (!value(child, depth + 1))
                        return false;
                    v.o.emplace_back(std::move(key), std::move(child));
                    ws();
                    if (_p < _e && *_p == ',')
                    {
                        ++_p;
                        continue;
                    }
                    if (_p < _e && *_p == '}')
                    {
                        ++_p;
                        return true;
                    }
                    return false;
                }
            }
            if (c == '[')
            {
                ++_p;
                v.type = Value::Array;
                ws();
                if (_p < _e && *_p == ']')
                {
                    ++_p;
                    return true;
                }
                for (;;)
                {
                    Value child;
                    if (!value(child, depth + 1))
                        return false;
                    v.a.push_back(std::move(child));
                    ws();
                    if (_p < _e && *_p == ',')
                    {
                        ++_p;
                        continue;
                    }
                    if (_p < _e && *_p == ']')
                    {
                        ++_p;
                        return true;
                    }
                    return false;
                }
            }
            if (c == '"')
            {
                v.type = Value::String;
                return str(v.s);
            }
            if (lit("true"))
            {
                v.type = Value::Bool;
                v.b = true;
                return true;
            }
            if (lit("false"))
            {
                v.type = Value::Bool;
                v.b = false;
                return true;
            }
            if (lit("null"))
            {
                v.type = Value::Null;
                return true;
            }
            // number
            std::string buf;
            while (_p < _e && (std::strchr("+-0123456789.eE", *_p) != nullptr))
                buf += *_p++;
            if (buf.empty())
                return false;
            char* end = nullptr;
            v.type = Value::Number;
            v.n = std::strtod(buf.c_str(), &end);
            return end && *end == '\0';
        }
    };

    // Small append-only writer. The caller is responsible for commas via the helpers below.
    class Writer
    {
    public:
        std::string out;

        void raw(char const* s) { out += s; }
        void key(char const* k)
        {
            comma();
            out += '"';
            out += k;
            out += "\":";
            _needComma = false;
        }
        void beginObject() { comma(); out += '{'; _needComma = false; }
        void endObject() { out += '}'; _needComma = true; }
        void beginArray() { comma(); out += '['; _needComma = false; }
        void endArray() { out += ']'; _needComma = true; }
        void number(double v, int decimals = 3)
        {
            comma();
            char buf[64];
            std::snprintf(buf, sizeof(buf), "%.*f", decimals, v);
            out += buf;
            _needComma = true;
        }
        void integer(long long v)
        {
            comma();
            out += std::to_string(v);
            _needComma = true;
        }
        void boolean(bool v) { comma(); out += v ? "true" : "false"; _needComma = true; }
        void null() { comma(); out += "null"; _needComma = true; }
        void string(std::string const& s)
        {
            comma();
            out += '"';
            for (char c : s)
            {
                switch (c)
                {
                    case '"': out += "\\\""; break;
                    case '\\': out += "\\\\"; break;
                    case '\n': out += "\\n"; break;
                    case '\r': out += "\\r"; break;
                    case '\t': out += "\\t"; break;
                    default:
                        if (static_cast<unsigned char>(c) < 0x20)
                        {
                            char buf[8];
                            std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                            out += buf;
                        }
                        else
                            out += c;
                }
            }
            out += '"';
            _needComma = true;
        }
        // Writes a previously parsed value back out (used to echo request ids).
        void value(Value const& v)
        {
            switch (v.type)
            {
                case Value::Null: null(); break;
                case Value::Bool: boolean(v.b); break;
                case Value::Number:
                    if (v.n == double((long long)v.n))
                        integer((long long)v.n);
                    else
                        number(v.n, 6);
                    break;
                case Value::String: string(v.s); break;
                case Value::Array:
                    beginArray();
                    for (auto const& c : v.a)
                        value(c);
                    endArray();
                    break;
                case Value::Object:
                    beginObject();
                    for (auto const& kv : v.o)
                    {
                        key(kv.first.c_str());
                        value(kv.second);
                    }
                    endObject();
                    break;
            }
        }

    private:
        bool _needComma = false;
        void comma()
        {
            if (_needComma)
                out += ',';
        }
    };
}
