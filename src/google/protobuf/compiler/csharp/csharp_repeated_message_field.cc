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

#include "google/protobuf/compiler/csharp/csharp_repeated_message_field.h"

#include <sstream>

#include "google/protobuf/compiler/code_generator.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/compiler/csharp/csharp_doc_comment.h"
#include "google/protobuf/compiler/csharp/csharp_helpers.h"
#include "google/protobuf/compiler/csharp/csharp_message_field.h"
#include "google/protobuf/compiler/csharp/csharp_wrapper_field.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/io/printer.h"
#include "google/protobuf/io/zero_copy_stream.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace csharp {

RepeatedMessageFieldGenerator::RepeatedMessageFieldGenerator(
    const FieldDescriptor* descriptor, int presenceIndex, const Options *options)
    : FieldGeneratorBase(descriptor, presenceIndex, options) {
}

RepeatedMessageFieldGenerator::~RepeatedMessageFieldGenerator() {

}

void RepeatedMessageFieldGenerator::GenerateMembers(io::Printer* printer) {
  printer->Print(
    variables_,
    "private static readonly pb::FieldCodec<$type_name$> _repeated_$name$_codec\n"
    "    = ");
  // Don't want to duplicate the codec code here... maybe we should have a
  // "create single field generator for this repeated field"
  // function, but it doesn't seem worth it for just this.
  if (IsWrapperType(descriptor_)) {
    std::unique_ptr<FieldGeneratorBase> single_generator(
      new WrapperFieldGenerator(descriptor_, presenceIndex_, this->options()));
    single_generator->GenerateCodecCode(printer);
  } else {
    std::unique_ptr<FieldGeneratorBase> single_generator(
      new MessageFieldGenerator(descriptor_, presenceIndex_, this->options()));
    single_generator->GenerateCodecCode(printer);
  }
  printer->Print(";\n");
  printer->Print(
    variables_,
    "private pbc::RepeatedField<$type_name$> $name$_pb = new pbc::RepeatedField<$type_name$>();\n"
    "private scg.IEnumerable<$type_name$>? $name$_imm = null;\n");
  WritePropertyDocComment(printer, descriptor_);
  AddPublicMemberAttributes(printer);
  printer->Print(
    variables_,
    "$access_level$ scg::IEnumerable<$type_name$> $property_name$ {\n"
    "  get { return $name$_imm ?? $name$_pb; }\n"
    "  init { if (value is sci.ImmutableList<$type_name$>) { $name$_imm = value; } else { $name$_imm = null; $name$_pb = new pbc::RepeatedField<$type_name$>(value); } }\n"
    "}\n"
    "private pbc::RepeatedField<$type_name$> $name$_ForSerialization { get { return $name$_imm != null ? new pbc::RepeatedField<$type_name$>($name$_imm) : $name$_pb; } }\n"
    "private pbc::RepeatedField<$type_name$> $name$_ForMutation { get {  if ($name$_imm != null) { $name$_pb = new pbc::RepeatedField<$type_name$>($name$_imm); $name$_imm = null; } return $name$_pb; } }\n"
    );
}

void RepeatedMessageFieldGenerator::GenerateMergingCode(io::Printer* printer) {
  printer->Print(
    variables_,
    "if (other.$property_name$.Any()) { var $name$_new = new pbc::RepeatedField<$type_name$>($property_name$); $name$_new.Add(other.$property_name$); $name$_imm = null; $name$_pb = $name$_new; }\n");
}

void RepeatedMessageFieldGenerator::GenerateParsingCode(io::Printer* printer) {
  GenerateParsingCode(printer, true);
}

void RepeatedMessageFieldGenerator::GenerateParsingCode(io::Printer* printer, bool use_parse_context) {
  printer->Print(
    variables_,
    use_parse_context
    ? "$name$_ForMutation.AddEntriesFrom(ref input, _repeated_$name$_codec);\n"
    : "$name$_ForMutation.AddEntriesFrom(input, _repeated_$name$_codec);\n");
}

void RepeatedMessageFieldGenerator::GenerateSerializationCode(io::Printer* printer) {
  GenerateSerializationCode(printer, true);
}

void RepeatedMessageFieldGenerator::GenerateSerializationCode(io::Printer* printer, bool use_write_context) {
  printer->Print(
    variables_,
    use_write_context
    ? "$name$_ForSerialization.WriteTo(ref output, _repeated_$name$_codec);\n"
    : "$name$_ForSerialization.WriteTo(output, _repeated_$name$_codec);\n");
}

void RepeatedMessageFieldGenerator::GenerateSerializedSizeCode(io::Printer* printer) {
  printer->Print(
    variables_,
    "size += pbc::RepeatedField<$type_name$>.RepeatedFieldCalculateSize(_repeated_$name$_codec, $property_name$);\n");
}

void RepeatedMessageFieldGenerator::WriteHash(io::Printer* printer) {
  printer->Print(
    variables_,
    "hash ^= pbc::RepeatedField<$type_name$>.GetRepeatedFieldHashCode($property_name$);\n");
}

void RepeatedMessageFieldGenerator::WriteEquals(io::Printer* printer) {
  printer->Print(
    variables_,
    "if(!pbc::RepeatedField<$type_name$>.RepeatedFieldEquals($property_name$, other.$property_name$)) return false;\n");
}

void RepeatedMessageFieldGenerator::WriteToString(io::Printer* printer) {
  variables_["field_name"] = GetFieldName(descriptor_);
  printer->Print(
    variables_,
    "PrintField(\"$field_name$\", $name$_ForSerialization, writer);\n");
}

void RepeatedMessageFieldGenerator::GenerateCloningCode(io::Printer* printer) {
  printer->Print(variables_,
    "if (deep) { $name$_imm = null; $name$_pb = other.$name$_ForSerialization.Clone(); } else { $name$_imm = other.$name$_imm; $name$_pb = other.$name$_pb; }\n");
}

void RepeatedMessageFieldGenerator::GenerateFreezingCode(io::Printer* printer) {
}

void RepeatedMessageFieldGenerator::GenerateExtensionCode(io::Printer* printer) {
  WritePropertyDocComment(printer, descriptor_);
  AddDeprecatedFlag(printer);
  printer->Print(
    variables_,
    "$access_level$ static readonly pb::RepeatedExtension<$extended_type$, $type_name$> $property_name$ =\n"
    "  new pb::RepeatedExtension<$extended_type$, $type_name$>($number$, ");
  if (IsWrapperType(descriptor_)) {
    std::unique_ptr<FieldGeneratorBase> single_generator(
      new WrapperFieldGenerator(descriptor_, -1, this->options()));
    single_generator->GenerateCodecCode(printer);
  } else {
    std::unique_ptr<FieldGeneratorBase> single_generator(
      new MessageFieldGenerator(descriptor_, -1, this->options()));
    single_generator->GenerateCodecCode(printer);
  }
  printer->Print(");\n");
}

}  // namespace csharp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
