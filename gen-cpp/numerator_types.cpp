/**
 * Autogenerated by Thrift Compiler (0.9.0)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "numerator_types.h"

#include <algorithm>

namespace numerator {

int _kOperationValues[] = {
  Operation::STR2ID,
  Operation::ID2STR
};
const char* _kOperationNames[] = {
  "STR2ID",
  "ID2STR"
};
const std::map<int, const char*> _Operation_VALUES_TO_NAMES(::apache::thrift::TEnumIterator(2, _kOperationValues, _kOperationNames), ::apache::thrift::TEnumIterator(-1, NULL, NULL));

const char* Query::ascii_fingerprint = "D478EE81C1C8AB0A3FEC2FF77EFAFD1F";
const uint8_t Query::binary_fingerprint[16] = {0xD4,0x78,0xEE,0x81,0xC1,0xC8,0xAB,0x0A,0x3F,0xEC,0x2F,0xF7,0x7E,0xFA,0xFD,0x1F};

uint32_t Query::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;

  bool isset_op = false;

  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_I32) {
          int32_t ecast0;
          xfer += iprot->readI32(ecast0);
          this->op = (Operation::type)ecast0;
          isset_op = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->strings.clear();
            uint32_t _size1;
            ::apache::thrift::protocol::TType _etype4;
            xfer += iprot->readListBegin(_etype4, _size1);
            this->strings.resize(_size1);
            uint32_t _i5;
            for (_i5 = 0; _i5 < _size1; ++_i5)
            {
              xfer += iprot->readString(this->strings[_i5]);
            }
            xfer += iprot->readListEnd();
          }
          this->__isset.strings = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->ids.clear();
            uint32_t _size6;
            ::apache::thrift::protocol::TType _etype9;
            xfer += iprot->readListBegin(_etype9, _size6);
            this->ids.resize(_size6);
            uint32_t _i10;
            for (_i10 = 0; _i10 < _size6; ++_i10)
            {
              xfer += iprot->readI64(this->ids[_i10]);
            }
            xfer += iprot->readListEnd();
          }
          this->__isset.ids = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 4:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->failures.clear();
            uint32_t _size11;
            ::apache::thrift::protocol::TType _etype14;
            xfer += iprot->readListBegin(_etype14, _size11);
            this->failures.resize(_size11);
            uint32_t _i15;
            for (_i15 = 0; _i15 < _size11; ++_i15)
            {
              xfer += iprot->readI32(this->failures[_i15]);
            }
            xfer += iprot->readListEnd();
          }
          this->__isset.failures = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  if (!isset_op)
    throw TProtocolException(TProtocolException::INVALID_DATA);
  return xfer;
}

uint32_t Query::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  xfer += oprot->writeStructBegin("Query");

  xfer += oprot->writeFieldBegin("op", ::apache::thrift::protocol::T_I32, 1);
  xfer += oprot->writeI32((int32_t)this->op);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("strings", ::apache::thrift::protocol::T_LIST, 2);
  {
    xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRING, static_cast<uint32_t>(this->strings.size()));
    std::vector<std::string> ::const_iterator _iter16;
    for (_iter16 = this->strings.begin(); _iter16 != this->strings.end(); ++_iter16)
    {
      xfer += oprot->writeString((*_iter16));
    }
    xfer += oprot->writeListEnd();
  }
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("ids", ::apache::thrift::protocol::T_LIST, 3);
  {
    xfer += oprot->writeListBegin(::apache::thrift::protocol::T_I64, static_cast<uint32_t>(this->ids.size()));
    std::vector<NumID> ::const_iterator _iter17;
    for (_iter17 = this->ids.begin(); _iter17 != this->ids.end(); ++_iter17)
    {
      xfer += oprot->writeI64((*_iter17));
    }
    xfer += oprot->writeListEnd();
  }
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("failures", ::apache::thrift::protocol::T_LIST, 4);
  {
    xfer += oprot->writeListBegin(::apache::thrift::protocol::T_I32, static_cast<uint32_t>(this->failures.size()));
    std::vector<FailureIdx> ::const_iterator _iter18;
    for (_iter18 = this->failures.begin(); _iter18 != this->failures.end(); ++_iter18)
    {
      xfer += oprot->writeI32((*_iter18));
    }
    xfer += oprot->writeListEnd();
  }
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  return xfer;
}

void swap(Query &a, Query &b) {
  using ::std::swap;
  swap(a.op, b.op);
  swap(a.strings, b.strings);
  swap(a.ids, b.ids);
  swap(a.failures, b.failures);
  swap(a.__isset, b.__isset);
}

} // namespace
