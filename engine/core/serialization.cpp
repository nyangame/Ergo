#include "serialization.hpp"
#include <sstream>
#include <iomanip>

namespace ergo {

namespace {

std::string indent_str(int level) {
    return std::string(level * 2, ' ');
}

std::string escape_string(const std::string& s) {
    std::string result;
    result.reserve(s.size() + 2);
    result += '"';
    for (char c : s) {
        switch (c) {
            case '"':  result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default:   result += c;
        }
    }
    result += '"';
    return result;
}

} // anonymous namespace

std::string JsonValue::to_string(int indent) const {
    switch (type) {
        case Null: return "null";
        case Bool: return bool_val ? "true" : "false";
        case Number: {
            std::ostringstream ss;
            if (number_val == static_cast<int>(number_val)) {
                ss << static_cast<int>(number_val);
            } else {
                ss << std::setprecision(6) << number_val;
            }
            return ss.str();
        }
        case String: return escape_string(string_val);
        case Array: {
            if (array_val.empty()) return "[]";
            std::string result = "[\n";
            for (size_t i = 0; i < array_val.size(); ++i) {
                result += indent_str(indent + 1) + array_val[i].to_string(indent + 1);
                if (i + 1 < array_val.size()) result += ",";
                result += "\n";
            }
            result += indent_str(indent) + "]";
            return result;
        }
        case Object: {
            if (object_val.empty()) return "{}";
            std::string result = "{\n";
            size_t i = 0;
            for (auto& [key, val] : object_val) {
                result += indent_str(indent + 1) + escape_string(key) + ": " + val.to_string(indent + 1);
                if (++i < object_val.size()) result += ",";
                result += "\n";
            }
            result += indent_str(indent) + "}";
            return result;
        }
    }
    return "null";
}

} // namespace ergo
