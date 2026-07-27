#include "runtime.h"

namespace adk {

    ResourceRegistry& Runtime::resources () noexcept
    {
        return resources_;
    }
}
