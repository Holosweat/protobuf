#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor_utils.h"
#include "google/protobuf/descriptor.pb.h"

namespace google { 
namespace protobuf {
    namespace {
        bool BoolOptionsFieldValue(const Descriptor &message, int field_number) {
            // Access the message options
            const MessageOptions &options = message.options();
            const google::protobuf::UnknownFieldSet& unknownFields = options.unknown_fields();

            for (int i = 0; i < unknownFields.field_count(); ++i) {
                const google::protobuf::UnknownField& field = unknownFields.field(i);

                if (field.number() == field_number) {
                    if (field.type() == google::protobuf::UnknownField::TYPE_VARINT) {
                        return field.varint() != 0;
                    }
                }
            }

            return false; // Default value if the field is not found
      }
      bool BoolOptionsFieldValue(const FieldDescriptor &field, int field_number) {
            // Access the message options
            const FieldOptions &options = field.options();
            const google::protobuf::UnknownFieldSet& unknownFields = options.unknown_fields();

            for (int i = 0; i < unknownFields.field_count(); ++i) {
                const google::protobuf::UnknownField& field = unknownFields.field(i);

                if (field.number() == field_number) {
                    if (field.type() == google::protobuf::UnknownField::TYPE_VARINT) {
                        return field.varint() != 0;
                    }
                }
            }

            return false; // Default value if the field is not found
      }
    }

    bool MessageIncludesUndefinedFields(const Descriptor &message) {
      return !BoolOptionsFieldValue(message, /*field_number=*/50002); // 50002 is ignore_undefined_fields
    }
    bool MessageIsReference(const Descriptor &message) {
      return !BoolOptionsFieldValue(message, /*field_number=*/50001); // 50001 is use_struct
      }
    bool FieldInsideReferenceContainer(const FieldDescriptor &field) {
        if (field.containing_type() == nullptr) {
                ABSL_LOG(FATAL) << "Field isn't in container";
        }
        return MessageIsReference(*field.containing_type());
    }
    bool FieldRequestedRefStructOptimization(const FieldDescriptor &field) {
        return BoolOptionsFieldValue(field, /*field_number=*/50003); // 50003 is use_ref
    }
}
}