#pragma once

#include <limits.h>
#include <stdint.h>

namespace adk {

    enum struct SignedAxis : uint8_t
    {
        PositiveX,
        NegativeX,
        PositiveY,
        NegativeY,
        PositiveZ,
        NegativeZ
    };

    struct SignedAxisMapping
    {
        SignedAxis x;
        SignedAxis y;
        SignedAxis z;
    };

    inline bool validSignedAxis (SignedAxis axis) noexcept
    {
        return axis >= SignedAxis::PositiveX && axis <= SignedAxis::NegativeZ;
    }

    inline uint8_t signedAxisIndex (SignedAxis axis) noexcept
    {
        return static_cast<uint8_t> (axis) / 2U;
    }

    inline int8_t signedAxisSign (SignedAxis axis) noexcept
    {
        return (static_cast<uint8_t> (axis) & 1U) == 0U ? 1 : -1;
    }

    inline bool validSignedAxisMapping (const SignedAxisMapping& mapping) noexcept
    {
        if (!validSignedAxis (mapping.x) || !validSignedAxis (mapping.y) ||
            !validSignedAxis (mapping.z))
        {
            return false;
        }

        const uint8_t xIndex = signedAxisIndex (mapping.x);
        const uint8_t yIndex = signedAxisIndex (mapping.y);
        const uint8_t zIndex = signedAxisIndex (mapping.z);
        if (xIndex == yIndex || xIndex == zIndex || yIndex == zIndex)
        {
            return false;
        }

        int8_t x[3] = {0, 0, 0};
        int8_t y[3] = {0, 0, 0};
        int8_t z[3] = {0, 0, 0};
        x[xIndex]   = signedAxisSign (mapping.x);
        y[yIndex]   = signedAxisSign (mapping.y);
        z[zIndex]   = signedAxisSign (mapping.z);

        const int8_t cross[3] = {static_cast<int8_t> (x[1] * y[2] - x[2] * y[1]),
                                 static_cast<int8_t> (x[2] * y[0] - x[0] * y[2]),
                                 static_cast<int8_t> (x[0] * y[1] - x[1] * y[0])};
        return cross[0] == z[0] && cross[1] == z[1] && cross[2] == z[2];
    }

    inline bool mapSignedAxes (const SignedAxisMapping& mapping, int32_t inputX,
                               int32_t inputY, int32_t inputZ, int32_t& outputX,
                               int32_t& outputY, int32_t& outputZ) noexcept
    {
        if (!validSignedAxisMapping (mapping))
        {
            return false;
        }

        const int32_t    input[3]      = {inputX, inputY, inputZ};
        const SignedAxis outputAxes[3] = {mapping.x, mapping.y, mapping.z};
        int32_t          candidate[3];
        for (uint8_t index = 0; index < 3; ++index)
        {
            const int32_t value =
                input[signedAxisIndex (outputAxes[index])];
            if (signedAxisSign (
                    outputAxes[index]) < 0)
            {
                if (value == INT32_MIN)
                {
                    return false;
                }
                candidate[index] = -value;
            }
            else
            {
                candidate[index] = value;
            }
        }

        outputX = candidate[0];
        outputY = candidate[1];
        outputZ = candidate[2];
        return true;
    }
} // namespace adk
