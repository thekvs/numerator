#!/usr/bin/env python

import argparse
import random
import string


def parse_args():

    def positive_int(string):
        value = int(string)
        if value < 1:
            raise argparse.ArgumentTypeError("value must be a positive integer")
        return value

    parser = argparse.ArgumentParser()
    parser.add_argument("--count", type=int, metavar="arg", required=True,
                        help="number of random strings to generate")
    parser.add_argument("--length", type=positive_int, metavar="arg", nargs=2, default=[32, 32],
                        help="lower and upper limits of random string's length")
    args = parser.parse_args()

    return args


def main():
    args = parse_args()
    characters = string.ascii_uppercase + string.ascii_lowercase + string.digits

    for i in xrange(args.count):
        length = random.randint(args.length[0], args.length[1])
        print ''.join(random.choice(characters) for x in range(length))


if __name__ == "__main__":
    main()
