import sys
import argparse
import time

sys.path.append('../gen-py')

from numerator import Numerator
from numerator.ttypes import *

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--numerator", type=str, metavar="arg", default="localhost:9090",
                        help="numerator's address (location:port)")
    parser.add_argument("--verbose", action="store_true",
                        help="increase output verbosity")

    args = parser.parse_args()

    return args


def get_query_type(args):
    if args.type == "s2i":
        return Operation.STR2ID
    else:
        return Operation.ID2STR


def do_query(args, client, data):
    if args.type == "s2i":
        request = Query(get_query_type(args), strings=data)
        result = client.query(request)
        for idx, num in enumerate(result.ids):
            if args.verbose:
                print "%s\t%i" % (data[idx], num)
            else:
                print "%i" % num
    else:
        request = Query(get_query_type(args), ids=data)
        result = client.query(request)
        failures = set(result.failures)
        s = None
        for idx, id_value in enumerate(request.ids):
            if idx in failures:
                s = "!!! NOT FOUND !!!"
            else:
                s = result.strings[idx]
            if args.verbose:
                print "%i\t%s" % (id_value, s)
            else:
                print "%s" % s


def main():
    args = parse_args()

    host, port = args.numerator.split(":")
    # Make socket
    transport = TSocket.TSocket(host, int(port))
    # Buffering is critical. Raw sockets are very slow
    transport = TTransport.TBufferedTransport(transport)
    # Wrap in a protocol
    protocol = TBinaryProtocol.TBinaryProtocol(transport)
    # Create a client to use the protocol encoder
    client = Numerator.Client(protocol)
    # Connect!
    transport.open()

    client.ping()

    cnt = 0
    query_data = ["AAAAAA", "BBBBBB", "CCCCCC", "DDDDDD"]

    request = Query(Operation.STR2ID, strings=query_data)
    result = client.query(request)

    ids = result.ids[:]

    for idx, num in enumerate(result.ids):
        print "%s\t%i" % (query_data[idx], num)

    client.disable_s2i()
    print "string -> id lookups disabled"

    try:
        request = Query(Operation.STR2ID, strings=query_data)
        result = client.query(request)
    except Thrift.TException as exc:
        print "Received exception, that's good!"
        print exc
    else:
        print >>sys.stderr, "We should have received exception here"
        sys.exit(-1)

    request = Query(Operation.ID2STR, ids=ids)
    result = client.query(request)

    if result.strings != query_data:
        print >>sys.stderr, "unexpected result while performing id -> string request"
        sys.exit(-1)

    for idx, s in enumerate(result.strings):
        print "%s\t%s" % (ids[idx], s)

    client.enable_s2i()
    print "string -> id lookups enabled"

    request = Query(Operation.STR2ID, strings=query_data)
    result = client.query(request)

    if result.ids != ids:
        print >>sys.stderr, "unexpected result while performing string -> id request"
        sys.exit(-1)

    print "Test passed!"

    # Close!
    transport.close()


if __name__ == "__main__":
    main()
