#include "api_validation.h"

#include <cstdio>

namespace {
int failures = 0;
void Check(bool condition, const char* message) {
    if (condition) return;
    std::fprintf(stderr, "FAIL: %s\n", message);
    ++failures;
}
} // namespace

int main() {
    const XrVersion supported = XR_MAKE_VERSION(1, 0, 34);
    Check(api_validation::ValidateApiVersion(XR_MAKE_VERSION(1, 0, 34), supported) == XR_SUCCESS,
          "the advertised API version is accepted");
    Check(api_validation::ValidateApiVersion(XR_MAKE_VERSION(1, 0, 35), supported) == XR_SUCCESS,
          "patch revisions remain fully compatible");
    Check(api_validation::ValidateApiVersion(XR_MAKE_VERSION(1, 1, 0), supported) ==
              XR_ERROR_API_VERSION_UNSUPPORTED,
          "a newer minor version is rejected");

    uint32_t count = 0;
    int values[2]{};
    Check(api_validation::ValidateEnumeration(0, &count, (int*)nullptr, 2) == XR_SUCCESS &&
              count == 2,
          "a zero-capacity query reports the required count");
    Check(api_validation::ValidateEnumeration(1, &count, values, 2) ==
              XR_ERROR_SIZE_INSUFFICIENT && count == 2,
          "an undersized enumeration reports size insufficient");
    Check(api_validation::ValidateEnumeration(2, &count, values, 2) == XR_SUCCESS,
          "an exact-capacity enumeration succeeds");
    Check(api_validation::ValidateEnumeration(2, (uint32_t*)nullptr, values, 2) ==
              XR_ERROR_VALIDATION_FAILURE,
          "an enumeration requires a count output");
    Check(api_validation::ValidateEnumeration(2, &count, (int*)nullptr, 2) ==
              XR_ERROR_VALIDATION_FAILURE,
          "a nonzero capacity requires output storage");

    XrViewConfigurationView view{XR_TYPE_VIEW_CONFIGURATION_VIEW};
    Check(api_validation::ValidateOutputStruct(&view, XR_TYPE_VIEW_CONFIGURATION_VIEW) == XR_SUCCESS,
          "an output struct with the expected type succeeds");
    view.type = XR_TYPE_UNKNOWN;
    Check(api_validation::ValidateOutputStruct(&view, XR_TYPE_VIEW_CONFIGURATION_VIEW) ==
              XR_ERROR_VALIDATION_FAILURE,
          "an output struct with the wrong type is rejected");

    if (failures) return 1;
    std::puts("API validation tests passed");
    return 0;
}
