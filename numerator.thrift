namespace cpp numerator

typedef i64 NumID
typedef i32 FailureIdx

enum Operation {
    STR2ID = 1,
    ID2STR = 2
}

enum ErrorCode {
    STR2ID_QUERIES_DISABLED = 1
}

struct Query {
    1: required Operation   op,
    2: list<string>         strings,
    3: list<NumID>          ids,
    4: list<FailureIdx>     failures    // indexes of ID2STR type queries for which no values were found
}

exception NumeratorException {
    1: required ErrorCode code,
    2: string             message
}

service Numerator {
    void ping(),
    Query query(1: Query request) throws (1:NumeratorException exc),
    void disable_s2i(),
    void enable_s2i()
}
