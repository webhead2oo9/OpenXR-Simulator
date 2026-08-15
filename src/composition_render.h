#pragma once

#include <openxr/openxr.h>

#include <algorithm>
#include <cstdint>

namespace composition_render {

struct SubImageRect {
    uint32_t x{0};
    uint32_t y{0};
    uint32_t w{0};
    uint32_t h{0};
};

// Resolve an application-supplied image rectangle defensively. Validation normally
// guarantees that it lies inside the swapchain, but keeping the clamp here prevents a
// malformed rectangle from reaching a graphics API copy operation. A zero extent is a
// legal empty rectangle in OpenXR and must stay empty rather than exposing the full image.
inline SubImageRect ResolveSubImageRect(const XrRect2Di& rect, uint32_t textureWidth,
                                        uint32_t textureHeight, bool showFullRender) {
    if (rect.extent.width == 0 || rect.extent.height == 0) {
        return {
            static_cast<uint32_t>((std::max)(0, (std::min)(rect.offset.x,
                                                          static_cast<int32_t>(textureWidth)))),
            static_cast<uint32_t>((std::max)(0, (std::min)(rect.offset.y,
                                                          static_cast<int32_t>(textureHeight)))),
            0,
            0,
        };
    }

    const SubImageRect full{0, 0, textureWidth, textureHeight};
    if (showFullRender) return full;
    if (rect.extent.width < 0 || rect.extent.height < 0) return {};

    int64_t x = rect.offset.x;
    int64_t y = rect.offset.y;
    int64_t width = rect.extent.width;
    int64_t height = rect.extent.height;
    if (x < 0) {
        width += x;
        x = 0;
    }
    if (y < 0) {
        height += y;
        y = 0;
    }
    if (x >= textureWidth || y >= textureHeight) return {};
    width = (std::min)(width, static_cast<int64_t>(textureWidth) - x);
    height = (std::min)(height, static_cast<int64_t>(textureHeight) - y);
    if (width <= 0 || height <= 0) return {};
    return {static_cast<uint32_t>(x), static_cast<uint32_t>(y),
            static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

inline bool HasPixels(const XrRect2Di& rect) {
    return rect.extent.width > 0 && rect.extent.height > 0;
}

inline bool HasPixels(const XrCompositionLayerProjection& projection) {
    if (!projection.views) return false;
    for (uint32_t view = 0; view < projection.viewCount; ++view) {
        if (HasPixels(projection.views[view].subImage.imageRect)) return true;
    }
    return false;
}

inline bool HasPixels(const XrCompositionLayerQuad& quad) {
    return quad.size.width > 0.0f && quad.size.height > 0.0f &&
           HasPixels(quad.subImage.imageRect);
}

} // namespace composition_render
