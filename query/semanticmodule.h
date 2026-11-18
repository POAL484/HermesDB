#pragma once

#include <models/operation.h>
#include <models/types.h>

#include <stdexcept>
#include <string>
#include <unordered_map>

namespace semantic {

class SemanticError : public std::runtime_error {
 public:
    explicit SemanticError(const std::string& message) : std::runtime_error(message) {}
};

class SemanticModule {
 public:
    using KeyTypes = std::unordered_map<std::string, types::Type>;

    static void validate(const operation::Operation& operation,
                         const KeyTypes& key_types);

    static types::Type typeFor(const operation::Operation& operation,
                               const KeyTypes& key_types);
};

} // namespace semantic
