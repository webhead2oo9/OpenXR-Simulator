#pragma once

#include <openxr/openxr.h>

namespace pose_math {

inline XrQuaternionf Multiply(const XrQuaternionf& a, const XrQuaternionf& b) {
    return {
        a.w*b.x + a.x*b.w + a.y*b.z - a.z*b.y,
        a.w*b.y - a.x*b.z + a.y*b.w + a.z*b.x,
        a.w*b.z + a.x*b.y - a.y*b.x + a.z*b.w,
        a.w*b.w - a.x*b.x - a.y*b.y - a.z*b.z,
    };
}

inline XrQuaternionf Inverse(const XrQuaternionf& q) {
    return {-q.x, -q.y, -q.z, q.w};
}

inline XrVector3f Rotate(const XrQuaternionf& q, const XrVector3f& value) {
    const XrQuaternionf vector{value.x, value.y, value.z, 0.0f};
    const XrQuaternionf result = Multiply(Multiply(q, vector), Inverse(q));
    return {result.x, result.y, result.z};
}

inline XrPosef Compose(const XrPosef& parent, const XrPosef& child) {
    const XrVector3f translated = Rotate(parent.orientation, child.position);
    return {
        Multiply(parent.orientation, child.orientation),
        {parent.position.x + translated.x,
         parent.position.y + translated.y,
         parent.position.z + translated.z},
    };
}

inline XrPosef Inverse(const XrPosef& pose) {
    const XrQuaternionf orientation = Inverse(pose.orientation);
    const XrVector3f position = Rotate(orientation,
        {-pose.position.x, -pose.position.y, -pose.position.z});
    return {orientation, position};
}

// Returns spaceFromOrigin expressed in the coordinate system of baseFromOrigin.
inline XrPosef Relative(const XrPosef& spaceFromOrigin, const XrPosef& baseFromOrigin) {
    return Compose(Inverse(baseFromOrigin), spaceFromOrigin);
}

inline XrPosef Identity() {
    return {{0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}};
}

} // namespace pose_math
