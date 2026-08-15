#pragma once

#include <openxr/openxr.h>

#include <cstdint>

namespace api_validation {

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
