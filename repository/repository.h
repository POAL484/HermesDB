#pragma once

#include <models/operation.h>

namespace repository {

class Repository {
 public:
   virtual operation::OperationResult apply(const operation::Operation&) = 0;
   virtual bool contains(const std::string& key) const = 0;
   virtual types::Type type(const std::string& key) const = 0;
 protected:
    Repository() = default;
};

}
