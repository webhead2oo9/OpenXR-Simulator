#pragma once

#include <openxr/openxr.h>

#include <cstdint>

namespace api_validation {

inline XrResult ValidateApiVersion(XrVersion requested, XrVersion supported) {
    // Patch revisions are fully compatible in OpenXR. Compare the interface-bearing
    // major/minor pair and deliberately ignore patch.
    if (requested == 0 || XR_VERSION_MAJOR(requested) != XR_VERSION_MAJOR(supported) ||
        XR_VERSION_MINOR(requested) > XR_VERSION_MINOR(supported)) {
        return XR_ERROR_API_VERSION_UNSUPPORTED;
    }
    return XR_SUCCESS;
}

template <typename T>
inline XrResult ValidateEnumeration(uint32_t capacityInput, uint32_t* countOutput,
                                    T* values, uint32_t requiredCount) {
    if (!countOutput) return XR_ERROR_VALIDATION_FAILURE;
    *countOutput = requiredCount;
    if (capacityInput == 0) return XR_SUCCESS;
    if (!values) return XR_ERROR_VALIDATION_FAILURE;
    if (capacityInput < requiredCount) return XR_ERROR_SIZE_INSUFFICIENT;
    return XR_SUCCESS;
}

template <typename T>
inline XrResult ValidateOutputStruct(const T* value, XrStructureType expectedType) {
    if (!value || value->type != expectedType) return XR_ERROR_VALIDATION_FAILURE;
    return XR_SUCCESS;
}

} // namespace api_validation
