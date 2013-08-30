#!/usr/bin/env python

import sys
import argparse
import time

sys.path.append('gen-py')

from numerator import Numerator
from numerator.ttypes import *

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--data", type=str, metavar="arg", required=True,
                        help="file with data (one string per line)")
    parser.add_argument("--batch-size", type=int, metavar="arg", required=True,
                        help="number of items in one request")
    parser.add_argument("--type", type=str, choices=['s2i', 'i2s'], required=True,
                        help="type of the query")
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

    try:
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
        query_data = []

        data = open(args.data)

        for line in data:
            line = line.strip()
            if args.type == "i2s":
                query_data.append(int(line))
            else:
                query_data.append(line)
            cnt += 1
            if cnt >= args.batch_size:
                do_query(args, client, query_data)
                cnt = 0
                query_data = []

        if query_data:
            do_query(args, client, query_data)

        # Close!
        transport.close()

    except Thrift.TException as tx:
        print >>sys.stderr, "Error: %s" % tx.message
    except Exception as exc:
        print >>sys.stderr, "Error: %s" % exc


if __name__ == "__main__":
    main()
