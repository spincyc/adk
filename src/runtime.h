#pragma once

#include "resource.h"

namespace adk {

    struct Runtime
    {
        ResourceRegistry& resources () noexcept;

      private:
        ResourceRegistry resources_;
    };
}
