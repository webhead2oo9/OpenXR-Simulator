#include "composition_render.h"

#include <cstdio>

namespace {

int failures = 0;

void Check(bool condition, const char* message) {
    if (condition) return;
    std::fprintf(stderr, "FAIL: %s\n", message);
    ++failures;
}

XrRect2Di Rect(int32_t x, int32_t y, int32_t width, int32_t height) {
    return {{x, y}, {width, height}};
}

} // namespace

int main() {
    using composition_render::ResolveSubImageRect;

    const auto full = ResolveSubImageRect(Rect(0, 0, 1024, 512), 1024, 512, false);
    Check(full.x == 0 && full.y == 0 && full.w == 1024 && full.h == 512,
          "a full-image rectangle remains unchanged");

    const auto cropped = ResolveSubImageRect(Rect(10, 20, 300, 200), 1024, 512, false);
    Check(cropped.x == 10 && cropped.y == 20 && cropped.w == 300 && cropped.h == 200,
          "a valid crop remains unchanged");

    const auto empty = ResolveSubImageRect(Rect(1024, 512, 0, 0), 1024, 512, false);
    Check(empty.x == 1024 && empty.y == 512 && empty.w == 0 && empty.h == 0,
          "a zero-area boundary rectangle stays empty");

    const auto diagnosticEmpty = ResolveSubImageRect(Rect(20, 30, 0, 100), 1024, 512, true);
    Check(diagnosticEmpty.w == 0 && diagnosticEmpty.h == 0,
          "the full-render diagnostic does not expose an empty subimage");

    const auto diagnosticFull = ResolveSubImageRect(Rect(10, 20, 300, 200), 1024, 512, true);
    Check(diagnosticFull.x == 0 && diagnosticFull.y == 0 &&
              diagnosticFull.w == 1024 && diagnosticFull.h == 512,
          "the full-render diagnostic still expands a non-empty crop");

    const auto clipped = ResolveSubImageRect(Rect(-10, -20, 100, 80), 1024, 512, false);
    Check(clipped.x == 0 && clipped.y == 0 && clipped.w == 90 && clipped.h == 60,
          "defensive resolution clips malformed rectangles safely");

    XrCompositionLayerProjectionView views[2]{};
    views[0].subImage.imageRect = Rect(0, 0, 0, 512);
    views[1].subImage.imageRect = Rect(0, 0, 1024, 512);
    XrCompositionLayerProjection projection{};
    projection.viewCount = 2;
    projection.views = views;
    Check(composition_render::HasPixels(projection),
          "a projection contributes when either submitted view has pixels");
    views[1].subImage.imageRect.extent.width = 0;
    Check(!composition_render::HasPixels(projection),
          "a projection with two empty views contributes no pixels");

    XrCompositionLayerQuad quad{};
    quad.subImage.imageRect = Rect(0, 0, 256, 256);
    quad.size = {1.0f, 1.0f};
    Check(composition_render::HasPixels(quad), "a positive-size quad contributes pixels");
    quad.size.width = 0.0f;
    Check(!composition_render::HasPixels(quad), "a zero-width quad contributes no pixels");

    if (failures) return 1;
    std::puts("composition render tests passed");
    return 0;
}
