#ifndef GOOGLE_PROTOBUF_DESCRIPTOR_UTILS_H__
#define GOOGLE_PROTOBUF_DESCRIPTOR_UTILS_H__

namespace google { 
namespace protobuf {
    class FieldDescriptor;
    class Descriptor;
    bool MessageIncludesUndefinedFields(const Descriptor &message);
    bool MessageIsReference(const Descriptor &message);
    bool FieldInsideReferenceContainer(const FieldDescriptor &field);
    bool FieldRequestedRefStructOptimization(const FieldDescriptor &field);
} 
}
#endif