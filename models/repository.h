#pragma once

#include <models/operation.h>

namespace repository {

class Repository {
 public:
   virtual operation::OperationResult apply(const operation::Operation&) = 0;
 protected:
    Repository() = default;
};

}

