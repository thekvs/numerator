/**
 * Autogenerated by Thrift Compiler (0.9.3)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef numerator_TYPES_H
#define numerator_TYPES_H

#include <iosfwd>

#include <thrift/Thrift.h>
#include <thrift/TApplicationException.h>
#include <thrift/protocol/TProtocol.h>
#include <thrift/transport/TTransport.h>

#include <thrift/cxxfunctional.h>


namespace numerator {

struct Operation {
  enum type {
    STR2ID = 1,
    ID2STR = 2
  };
};

extern const std::map<int, const char*> _Operation_VALUES_TO_NAMES;

struct ErrorCode {
  enum type {
    STR2ID_QUERIES_DISABLED = 1
  };
};

extern const std::map<int, const char*> _ErrorCode_VALUES_TO_NAMES;

typedef int64_t NumID;

typedef int32_t FailureIdx;

class Query;

class NumeratorException;

typedef struct _Query__isset {
  _Query__isset() : strings(false), ids(false), failures(false) {}
  bool strings :1;
  bool ids :1;
  bool failures :1;
} _Query__isset;

class Query {
 public:

  Query(const Query&);
  Query& operator=(const Query&);
  Query() : op((Operation::type)0) {
  }

  virtual ~Query() throw();
  Operation::type op;
  std::vector<std::string>  strings;
  std::vector<NumID>  ids;
  std::vector<FailureIdx>  failures;

  _Query__isset __isset;

  void __set_op(const Operation::type val);

  void __set_strings(const std::vector<std::string> & val);

  void __set_ids(const std::vector<NumID> & val);

  void __set_failures(const std::vector<FailureIdx> & val);

  bool operator == (const Query & rhs) const
  {
    if (!(op == rhs.op))
      return false;
    if (!(strings == rhs.strings))
      return false;
    if (!(ids == rhs.ids))
      return false;
    if (!(failures == rhs.failures))
      return false;
    return true;
  }
  bool operator != (const Query &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Query & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
};

void swap(Query &a, Query &b);

inline std::ostream& operator<<(std::ostream& out, const Query& obj)
{
  obj.printTo(out);
  return out;
}

typedef struct _NumeratorException__isset {
  _NumeratorException__isset() : message(false) {}
  bool message :1;
} _NumeratorException__isset;

class NumeratorException : public ::apache::thrift::TException {
 public:

  NumeratorException(const NumeratorException&);
  NumeratorException& operator=(const NumeratorException&);
  NumeratorException() : code((ErrorCode::type)0), message() {
  }

  virtual ~NumeratorException() throw();
  ErrorCode::type code;
  std::string message;

  _NumeratorException__isset __isset;

  void __set_code(const ErrorCode::type val);

  void __set_message(const std::string& val);

  bool operator == (const NumeratorException & rhs) const
  {
    if (!(code == rhs.code))
      return false;
    if (!(message == rhs.message))
      return false;
    return true;
  }
  bool operator != (const NumeratorException &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const NumeratorException & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  virtual void printTo(std::ostream& out) const;
  mutable std::string thriftTExceptionMessageHolder_;
  const char* what() const throw();
};

void swap(NumeratorException &a, NumeratorException &b);

inline std::ostream& operator<<(std::ostream& out, const NumeratorException& obj)
{
  obj.printTo(out);
  return out;
}

} // namespace

#endif
