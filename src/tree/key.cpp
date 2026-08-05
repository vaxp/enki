/// @file key.cpp
/// @brief Key implementation — toString and non-inline methods.

#include "enki/tree/key.hpp"
#include <sstream>

namespace enki {

std::string Key::toString() const {
    switch (type_) {
        case Type::None:
            return "[Key::None]";
        case Type::Value: {
            std::ostringstream oss;
            oss << "[Key::Value(" << std::get<int64_t>(data_) << ")]";
            return oss.str();
        }
        case Type::String: {
            std::ostringstream oss;
            oss << "[Key::String(\"" << std::get<std::string>(data_) << "\")]";
            return oss.str();
        }
        case Type::Unique: {
            std::ostringstream oss;
            oss << "[Key::Unique(#" << std::get<uint64_t>(data_) << ")]";
            return oss.str();
        }
    }
    return "[Key::Unknown]";
}

}  // namespace enki
