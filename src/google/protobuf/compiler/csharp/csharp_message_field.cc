// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/csharp/csharp_message_field.h"

#include <sstream>

#include "google/protobuf/compiler/code_generator.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/compiler/csharp/csharp_doc_comment.h"
#include "google/protobuf/compiler/csharp/csharp_helpers.h"
#include "google/protobuf/compiler/csharp/csharp_options.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/io/printer.h"
#include "google/protobuf/descriptor_utils.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace csharp {

MessageFieldGenerator::MessageFieldGenerator(const FieldDescriptor* descriptor,
                                             int presenceIndex,
                                             const Options *options)
    : FieldGeneratorBase(descriptor, presenceIndex, options) {
  std::string overwritten_member = variables_["name"] + "_";
  if (!EmbedBarePublicField(*descriptor) && !EmbedReadOnlyRefField(*descriptor_)) {
    variables_["writing_member"] = overwritten_member;
  }
}

MessageFieldGenerator::~MessageFieldGenerator() {

}

void MessageFieldGenerator::GenerateMembers(io::Printer* printer) {
  // Bare either reference or struct type.
  if (EmbedBarePublicField(*descriptor_)) {
    WritePropertyDocComment(printer, options(), descriptor_);
    printer->Print(
      variables_,
      "public $type_at_rest$ $property_name$;\n");

    printer->Print(
        variables_,
        "public static $type_at_rest$ __$property_name$($extended_type$ message) { return message.$property_name$; }\n");
    if (IsNullable(descriptor_) && SupportsPresenceApi(descriptor_)) {
            printer->Print(
        variables_,
        "public static bool __$property_name$_has_value($extended_type$ message) { return message.$property_name$ != null; }\n");
    }
  } else if (EmbedReadOnlyRefField(*descriptor_)) {
    printer->Print(
      variables_,
      "private $type_at_rest$ $name$_;\n");
    WritePropertyDocComment(printer, options(), descriptor_);
    AddPublicMemberAttributes(printer);
    printer->Print(
      variables_,
      "$access_level$ ref readonly $type_at_rest$ $property_name$ {\n"
      "  get { return ref $name$_; }\n"
      "}\n");
    printer->Print(
      variables_,
      "$access_level$ $type_at_rest$ $property_name$Setter {\n"
      "  init { $name$_ = value; }\n"
      "}\n");

    printer->Print(
        variables_,
        "public static $type_at_rest$ __$property_name$($extended_type$ message) { return message.$property_name$; }\n");
  } else {
    printer->Print(
      variables_,
      "private $type_at_rest$ $name$_;\n");
    WritePropertyDocComment(printer, options(), descriptor_);
    AddPublicMemberAttributes(printer);
    printer->Print(
        variables_,
        "$access_level$ $type_at_rest$ $property_name$ {\n"
        "  get { return $name$_; }\n");
      if (FieldInsideReferenceContainer(*descriptor_)) {
        printer->Print(
            variables_,
            "  init {\n");
      } else {
        printer->Print(
            variables_,
            "  set {\n");
      }
      printer->Print(
        variables_,
      "    $name$_ = value;\n"
      "  }\n"
      "}\n");
    if (SupportsPresenceApi(descriptor_)) {
      printer->Print(
        variables_,
        "/// <summary>Gets whether the $descriptor_name$ field is set</summary>\n");
      AddPublicMemberAttributes(printer);
      printer->Print(
        variables_,
        "$access_level$ bool Has$property_name$ {\n"
        "  get { return $has_property_check_internal$; }\n"
        "}\n");
      printer->Print(
        variables_,
        "/// <summary>Clears the value of the $descriptor_name$ field</summary>\n");
      AddPublicMemberAttributes(printer);
      printer->Print(
        variables_,
        "private void Clear$property_name$() {\n"
        "  $name$_ = default;\n"
        "}\n");
    }
    if (!FieldInsideReferenceContainer(*descriptor_)) {
      printer->Print(
        variables_,
        "public static $type_at_rest$ __$property_name$($extended_type$ message) { return message.$property_name$; }\n");
    }
  }
}

void MessageFieldGenerator::GenerateMergingCode(io::Printer* printer) {
  if (!IsNullable(descriptor_)) {
    printer->Print(
        variables_,
        "$writing_member$.MergeFrom(other.$writing_member$);\n");
  } else {
    printer->Print(
      variables_,
      "if ($other_has_property_check$) {\n"
      "  if (!($has_property_check$)) {\n"
      "    $property_name_existing$ = new $type_name$();\n"
      "  } else { $property_name_existing$ = new $type_name$($property_name_existing$); }\n"
      "  $property_name_existing$.MergeFrom($other_property_name_existing$);\n"
      "  $writing_member$ = $property_name_existing$;\n"
      "}\n");
  }
}

void MessageFieldGenerator::GenerateParsingCode(io::Printer* printer) {
  if (IsNullable(descriptor_)) {
    printer->Print(
      variables_,
      "if (!($has_property_check$)) {\n"
      "  $property_name_existing$ = new $type_name$();\n"
      "}\n");
  }
  if (MessageFieldIsValueType(*descriptor_)) {
    if (descriptor_->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
      printer->Print(variables_, "pb::IBufferMessage bufferMessage = $property_name_existing$; input.ReadMessage(bufferMessage);\n");
    } else {
      printer->Print(variables_, "pb::IBufferMessage bufferMessage = $property_name_existing$; input.ReadGroup(bufferMessage);\n");
    }
    printer->Print(variables_, "  $writing_member$ = ($type_at_rest$)bufferMessage;\n");
  } else {
    if (descriptor_->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
      printer->Print(variables_, "input.ReadMessage($property_name_existing$);\n");
    } else {
      printer->Print(variables_, "input.ReadGroup($property_name_existing$);\n");
    }
    printer->Print(variables_, "  $writing_member$ = $property_name_existing$;\n");
  }
}

void MessageFieldGenerator::GenerateSerializationCode(io::Printer* printer) {
    if (descriptor_->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
    printer->Print(
      variables_,
      "if ($has_property_check$) {\n"
      "  output.WriteRawTag($tag_bytes$);\n"
      "  output.WriteMessage($property_name_existing$);\n"
      "}\n");
  } else {
    printer->Print(
      variables_,
      "if ($has_property_check$) {\n"
      "  output.WriteRawTag($tag_bytes$);\n"
      "  output.WriteGroup($property_name_existing$);\n"
      "  output.WriteRawTag($end_tag_bytes$);\n"
      "}\n");
    }
}

void MessageFieldGenerator::GenerateSerializedSizeCode(io::Printer* printer) {
  if (descriptor_->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
    printer->Print(
      variables_,
      "if ($has_property_check$) {\n"
      "  size += $tag_size$ + pb::CodedOutputStream.ComputeMessageSize($property_name_existing$);\n"
      "}\n");
  } else {
    printer->Print(
      variables_,
      "if ($has_property_check$) {\n"
      "  size += $tag_size$ + pb::CodedOutputStream.ComputeGroupSize($property_name_existing$);\n"
      "}\n");
    }
}

void MessageFieldGenerator::WriteHash(io::Printer* printer) {
  if (IsNullable(descriptor_)) {
    printer->Print(
      variables_,
      "if ($has_property_check$) hash ^= $property_name_existing$.GetHashCode();\n");
  } else {
    printer->Print(
      variables_,
      "hash ^= $reading_member$.GetHashCode();\n");
  }
}
void MessageFieldGenerator::WriteEquals(io::Printer* printer) {
  if (MessageFieldIsValueType(*descriptor_) && !IsNullable(descriptor_)) {
    printer->Print(
      variables_,
      "if (!$reading_member$.Equals(other.$reading_member$)) return false;\n");
  } else {
    printer->Print(
      variables_,
      "if (!scg::EqualityComparer<$type_at_rest$>.Default.Equals($reading_member$, other.$reading_member$)) return false;\n");
  }
}
void MessageFieldGenerator::WriteToString(io::Printer* printer) {
  variables_["field_name"] = GetFieldName(descriptor_);
  printer->Print(
    variables_,
    "PrintField(\"$field_name$\", has$property_name$, $name$_, writer);\n");
}
void MessageFieldGenerator::GenerateExtensionCode(io::Printer* printer) {
  WritePropertyDocComment(printer, options(), descriptor_);
  AddDeprecatedFlag(printer);
  printer->Print(
    variables_,
    "$access_level$ static readonly pb::Extension<$extended_type$, $type_name$> $property_name$ =\n"
    "  new pb::Extension<$extended_type$, $type_name$>($number$, ");
  GenerateCodecCode(printer);
  printer->Print(");\n");
}
void MessageFieldGenerator::GenerateCloningCode(io::Printer* printer) {
  if (EmbedBarePublicField(*descriptor_) && !IsNullable(descriptor_)) {
    printer->Print(variables_,
      "$property_name$ = deep ? other.$property_name$.DeepClone() : other.$property_name$;\n");
  } else {
    if (IsNullable(descriptor_)) {
      printer->Print(variables_,
        "$writing_member$ = other.$has_property_check$ ? (deep ? $property_name_existing$.DeepClone() : $property_name_existing$) : null;\n");
    } else {
      printer->Print(variables_,
        "$writing_member$ = deep ? other.$reading_member$.DeepClone() : other.$reading_member$;\n");
    }
  }
}

void MessageFieldGenerator::GenerateFreezingCode(io::Printer* printer) {
}

void MessageFieldGenerator::GenerateCodecCode(io::Printer* printer) {
  if (descriptor_->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
    if (MessageFieldIsValueType(*descriptor_)) {
      printer->Print(
        variables_,
        "pb::FieldCodec.ForStructMessage($tag$, $type_name$.Parser)");
    } else {
      printer->Print(
        variables_,
        "pb::FieldCodec.ForMessage($tag$, $type_name$.Parser)");
    }
  } else {
    printer->Print(
      variables_,
      "pb::FieldCodec.ForGroup($tag$, $end_tag$, $type_name$.Parser)");
  }
}

void MessageFieldGenerator::GenerateStructConstructorCode(io::Printer *printer) {
  printer->Print(variables_, "$writing_member$ = default;\n");
}

MessageOneofFieldGenerator::MessageOneofFieldGenerator(
    const FieldDescriptor* descriptor,
	  int presenceIndex,
    const Options *options)
    : MessageFieldGenerator(descriptor, presenceIndex, options) {
  SetCommonOneofFieldVariables(&variables_);
}

MessageOneofFieldGenerator::~MessageOneofFieldGenerator() {

}

void MessageOneofFieldGenerator::GenerateMembers(io::Printer* printer) {
  WritePropertyDocComment(printer, options(), descriptor_);
  AddPublicMemberAttributes(printer);
  printer->Print(
      variables_,
      "$access_level$ $type_name$? $property_name$ {\n"
      "  get { return $property_name$_Internal; }\n");
  if (FieldInsideReferenceContainer(*descriptor_)) {
    printer->Print(
        variables_,
        "  init {\n");
  } else {
    printer->Print(
        variables_,
        "  set {\n");
  }
  printer->Print(
    variables_,
    "    $property_name$_Internal = value;\n"
    "  }\n"
    "}\n");

  printer->Print(
    variables_,
    "private $type_name$? $property_name$_Internal {\n"
    "  get { return $has_property_check_internal$ && $oneof_name$_ is $type_name$ value ? value : null; }\n"
    "  set {\n"
    "    $oneof_name$_ = value;\n"
    "    $oneof_name$Case_ = value == null ? $oneof_property_name$OneofCase.None : $oneof_property_name$OneofCase.$oneof_case_name$;\n"
    "  }\n"
    "}\n");
  if (SupportsPresenceApi(descriptor_)) {
    printer->Print(
      variables_,
      "/// <summary>Gets whether the \"$descriptor_name$\" field is set</summary>\n");
    AddPublicMemberAttributes(printer);
    printer->Print(
      variables_,
      "$access_level$ bool Has$property_name$ {\n"
      "  get { return $oneof_name$Case_ == $oneof_property_name$OneofCase.$oneof_case_name$; }\n"
      "}\n");
    printer->Print(
      variables_,
      "/// <summary> Clears the value of the oneof if it's currently set to \"$descriptor_name$\" </summary>\n");
    AddPublicMemberAttributes(printer);
    printer->Print(
      variables_,
      "private void Clear$property_name$() {\n"
      "  if ($has_property_check$) {\n"
      "    Clear$oneof_property_name$();\n"
      "  }\n"
      "}\n");
  }
}

void MessageOneofFieldGenerator::GenerateMergingCode(io::Printer* printer) {
  printer->Print(variables_,
    "if ($property_name$ == null) {\n"
    "  $property_name$_Internal = new $type_name$();\n"
    "} else { $property_name$_Internal = new $type_name$($property_name$); }\n"
    "$property_name$?.MergeFrom(other.$property_name$);\n");
}

void MessageOneofFieldGenerator::GenerateParsingCode(io::Printer* printer) {
  // TODO: We may be able to do better than this
  printer->Print(
    variables_,
    "$type_name$ subBuilder = new $type_name$();\n"
    "if ($has_property_check$) {\n"
    "  subBuilder.MergeFrom($property_name$);\n"
    "}\n");
  if (descriptor_->type() == FieldDescriptor::Type::TYPE_MESSAGE) {
    printer->Print("input.ReadMessage(subBuilder);\n");
  } else {
    printer->Print("input.ReadGroup(subBuilder);\n");
  }
  printer->Print(variables_, "$property_name$_Internal = subBuilder;\n");
}

void MessageOneofFieldGenerator::WriteToString(io::Printer* printer) {
  printer->Print(
    variables_,
    "PrintField(\"$descriptor_name$\", $has_property_check$, $oneof_name$_, writer);\n");
}

void MessageOneofFieldGenerator::GenerateCloningCode(io::Printer* printer) {
  printer->Print(variables_,
    "$property_name$_Internal = deep ? other.$property_name$?.DeepClone() : other.$property_name$;\n");
}

void MessageOneofFieldGenerator::GenerateStructConstructorCode(io::Printer *printer) {
}

}  // namespace csharp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
