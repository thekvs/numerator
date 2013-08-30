#!/usr/bin/env python

import argparse
import random
import string


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("--count", type=int, metavar="arg", required=True,
                        help="number of random strings to generate")
    parser.add_argument("--length", type=int, metavar="arg", default=32,
                        help="length of random string")
    args = parser.parse_args()

    return args


def main():
    args = parse_args()
    characters = string.ascii_uppercase + string.ascii_lowercase + string.digits

    for i in xrange(args.count):
        print ''.join(random.choice(characters) for x in range(args.length))


if __name__ == "__main__":
    main()
