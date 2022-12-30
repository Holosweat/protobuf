using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Text;

namespace Google.Protobuf.Reflection
{
    internal sealed class SingleFieldValueAccessor : IFieldAccessor
    {
        // All the work here is actually done in the constructor - it creates the appropriate delegates.
        // There are various cases to consider, based on the property type (message, string/bytes, or "genuine" primitive)
        // and proto2 vs proto3 for non-message types, as proto3 doesn't support "full" presence detection or default
        // values.

        private readonly Func<IMessage, bool> hasDelegate;
        private readonly Func<IMessage, object> getDelegate;
        private FieldDescriptor descriptor;

        internal SingleFieldValueAccessor(
            [DynamicallyAccessedMembers(GeneratedClrTypeInfo.MessageAccessibility)]
            Type messageType, MethodInfo getter, MethodInfo hasValue, FieldDescriptor descriptor)
        {
            this.hasDelegate = hasValue != null ? ReflectionUtil.CreateFuncIMessageBool(hasValue) : (_) => true;
            this.getDelegate = ReflectionUtil.CreateFuncIMessageObject(getter);
            this.descriptor = descriptor;
        }

        public FieldDescriptor Descriptor => descriptor;

        //private static object GetDefaultValue(FieldDescriptor descriptor) =>
        //    descriptor.FieldType switch
        //    {
        //        FieldType.Bool => false,
        //        FieldType.Bytes => ByteString.Empty,
        //        FieldType.String => "",
        //        FieldType.Double => 0.0,
        //        FieldType.SInt32 or FieldType.Int32 or FieldType.SFixed32 or FieldType.Enum => 0,
        //        FieldType.Fixed32 or FieldType.UInt32 => (uint) 0,
        //        FieldType.Fixed64 or FieldType.UInt64 => 0UL,
        //        FieldType.SFixed64 or FieldType.Int64 or FieldType.SInt64 => 0L,
        //        FieldType.Float => 0f,
        //        FieldType.Message or FieldType.Group => /*descriptor.MessageType.GetOption<MessageOptions>()*/ null,
        //        _ => throw new ArgumentException("Invalid field type"),
        //    };

        public void Clear(IMessage message) {
            throw new NotImplementedException("This operation isn't supported for value types.");
        }

        public object GetValue(IMessage message)
        {
            return getDelegate(message);  
        }

        public  bool HasValue(IMessage message)
        {
            return hasDelegate(message);
        }
        public  void SetValue(IMessage message, object value)
        {
            throw new NotImplementedException("This operation isn't supported for value types.");
        }
    }
}