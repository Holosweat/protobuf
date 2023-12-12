// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
// https://developers.google.com/protocol-buffers/
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "google/protobuf/compiler/csharp/csharp_repeated_enum_field.h"

#include <sstream>

#include "google/protobuf/compiler/code_generator.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/wire_format.h"
#include "google/protobuf/compiler/csharp/csharp_doc_comment.h"
#include "google/protobuf/compiler/csharp/csharp_helpers.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/io/printer.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/descriptor_utils.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace csharp {

RepeatedEnumFieldGenerator::RepeatedEnumFieldGenerator(
    const FieldDescriptor* descriptor, int presenceIndex, const Options *options)
    : FieldGeneratorBase(descriptor, presenceIndex, options) {
  variables_["setter"] = FieldInsideReferenceContainer(*descriptor_) ? "init" : "set";
  variables_["repeated_type"] = RepeatedFieldType(*descriptor_);
  variables_["repeated_type_classname"] = RepeatedFieldTypeClassName(*descriptor_);
}

RepeatedEnumFieldGenerator::~RepeatedEnumFieldGenerator() {

}

void RepeatedEnumFieldGenerator::GenerateMembers(io::Printer* printer) {
  printer->Print(
    variables_,
    "private static readonly pb::FieldCodec<$type_name$> _repeated_$name$_codec\n"
    "    = pb::FieldCodec.ForEnum($tag$, x => (int) x, x => ($type_name$) x);\n");
  if (FieldInsideReferenceContainer(*descriptor_)) {
    printer->Print(
      variables_,
      "private $repeated_type$<$type_name$> $name$_ = $repeated_type$<$type_name$>.Empty;\n");
  } else {
    printer->Print(
      variables_,
      "private $repeated_type$<$type_name$>? $name$_;\n");
    printer->Print(
        variables_,
        "public static $repeated_type$<$type_name$> __$property_name$($extended_type$ message) { return message.$property_name$; }\n");
  }
  WritePropertyDocComment(printer, descriptor_);
  AddPublicMemberAttributes(printer);
  printer->Print(
    variables_,
    "$access_level$ $repeated_type$<$type_name$> $property_name$ {\n"
    "  get { return $name$_; }\n"
    "  $setter$ { $name$_ = value; }\n"
    "}\n"
    "private pbc::RepeatedField<$type_name$> $name$_ForSerialization { get { return pbc::RepeatedField<$type_name$>.From($name$_); } }\n"
    );
}

void RepeatedEnumFieldGenerator::GenerateMergingCode(io::Printer* printer) {
  printer->Print(
    variables_,
    "if (!other.$property_name$.IsEmpty) { $name$_ = $property_name$.AddRange(other.$property_name$); }\n");
}

void RepeatedEnumFieldGenerator::GenerateParsingCode(io::Printer* printer) {
  GenerateParsingCode(printer, true);
}

void RepeatedEnumFieldGenerator::GenerateParsingCode(io::Printer* printer, bool use_parse_context) {
  printer->Print(
      variables_,
      use_parse_context
          ? "var $name$_mutation = new pbc::RepeatedField<$type_name$>(); $name$_mutation.AddEntriesFrom(ref input, _repeated_$name$_codec); $name$_ = $name$_mutation.To$repeated_type_classname$();\n"
          : "var $name$_mutation = new pbc::RepeatedField<$type_name$>(); $name$_mutation.AddEntriesFrom(input, _repeated_$name$_codec); $name$_ = $name$_mutation.To$repeated_type_classname$();\n");
}

void RepeatedEnumFieldGenerator::GenerateSerializationCode(io::Printer* printer) {
  GenerateSerializationCode(printer, true);
}

void RepeatedEnumFieldGenerator::GenerateSerializationCode(io::Printer* printer, bool use_write_context) {
  printer->Print(
    variables_,
    use_write_context
    ? "$name$_ForSerialization.WriteTo(ref output, _repeated_$name$_codec);\n"
    : "$name$_ForSerialization.WriteTo(output, _repeated_$name$_codec);\n");
}

void RepeatedEnumFieldGenerator::GenerateSerializedSizeCode(io::Printer* printer) {
  printer->Print(
    variables_,
    "size += pbc::RepeatedField<$type_name$>.RepeatedFieldCalculateSize(_repeated_$name$_codec, $property_name$);\n");
}

void RepeatedEnumFieldGenerator::WriteHash(io::Printer* printer) {
  printer->Print(
    variables_,
    "hash ^= pbc::RepeatedField<$type_name$>.GetRepeatedFieldHashCode($property_name$);\n");
}

void RepeatedEnumFieldGenerator::WriteEquals(io::Printer* printer) {
  printer->Print(
    variables_,
    "if(!pbc::RepeatedField<$type_name$>.RepeatedFieldEquals($property_name$, other.$property_name$)) return false;\n");
}

void RepeatedEnumFieldGenerator::WriteToString(io::Printer* printer) {
  printer->Print(variables_,
    "PrintField(\"$descriptor_name$\", $name$_, writer);\n");
}

void RepeatedEnumFieldGenerator::GenerateCloningCode(io::Printer* printer) {
  printer->Print(variables_,
    "if (deep) { $name$_ = other.$name$_ForSerialization.Clone().To$repeated_type_classname$(); } else { $name$_ = other.$name$_; }\n");
}

void RepeatedEnumFieldGenerator::GenerateExtensionCode(io::Printer* printer) {
  WritePropertyDocComment(printer, descriptor_);
  AddDeprecatedFlag(printer);
  printer->Print(
    variables_,
    "$access_level$ static readonly pb::RepeatedExtension<$extended_type$, $type_name$> $property_name$ =\n"
    "  new pb::RepeatedExtension<$extended_type$, $type_name$>($number$, "
    "pb::FieldCodec.ForEnum($tag$, x => (int) x, x => ($type_name$) x));\n");
}

void RepeatedEnumFieldGenerator::GenerateFreezingCode(io::Printer* printer) {
}

void RepeatedEnumFieldGenerator::GenerateStructConstructorCode(io::Printer *printer) {
  printer->Print(variables_,
                 "$name$_ = $repeated_type$<T>.Empty;\n");
}

}  // namespace csharp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
