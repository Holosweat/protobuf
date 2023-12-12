#ifndef GOOGLE_PROTOBUF_DESCRIPTOR_UTILS_H__
#define GOOGLE_PROTOBUF_DESCRIPTOR_UTILS_H__
#include <string>

namespace google { 
namespace protobuf {
    class FieldDescriptor;
    class Descriptor;
    bool MessageIncludesUndefinedFields(const Descriptor &message);
    bool MessageIsReference(const Descriptor &message);
    bool FieldInsideReferenceContainer(const FieldDescriptor &field);
    bool FieldRequestedRefStructOptimization(const FieldDescriptor &field);
    std::string RepeatedFieldType(const FieldDescriptor &field);
    std::string FieldExplicitType(const FieldDescriptor &field);
    std::string RepeatedFieldTypeClassName(const FieldDescriptor &field);
}
}
#endif