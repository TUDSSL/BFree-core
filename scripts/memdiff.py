#!/usr/bin/env python3

import os

def group_consecutives(vals, step=1):
    """Return list of consecutive lists of numbers from vals (number list)."""
    run = []
    result = [run]
    expect = None
    for v in vals:
        if (v == expect) or (expect is None):
            run.append(v)
        else:
            run = [v]
            result.append(run)
        expect = v + step
    return result

class MemDiff:
    def __init__(self, file_a, file_b, offset_addr=0):
        self.file_a = file_a
        self.file_b = file_b
        self.offset_addr = offset_addr
        self.bytes_file_a = None
        self.bytes_file_b = None

        self.read_bin_files()

    def ok(self):
        if self.bytes_file_a == None or self.bytes_file_b == None:
            return False

        if len(self.bytes_file_a) != len(self.bytes_file_b):
            # Files not of equal length, failure
            return False

        return True

    def read_bin_files(self):
        with open(self.file_a, 'rb') as fa_bin:
            self.bytes_file_a = bytearray(fa_bin.read())
        with open(self.file_b, 'rb') as fb_bin:
            self.bytes_file_b = bytearray(fb_bin.read())

    def diff(self):
        if not self.ok():
            return

        # Find the differences
        diff_idx = []
        for idx in range(0, len(self.bytes_file_a)):
            if not (self.bytes_file_a[idx] == self.bytes_file_b[idx]):
                diff_idx.append(idx)

        self.correct_addr(diff_idx)
        return diff_idx

    def diff_range(self, diff=None):
        if diff == None:
            diff = self.diff()

        diff_range = []

        def diff_range_add():
            if range_start == range_end:
                diff_range.append(range_start)
            else:
                diff_range.append((range_start, range_end))

        len_diff = len(diff)
        for i in range(0, len_diff):
            if i == 0:
                # first one
                range_start = diff[i]
            if i+1 == len_diff:
                # last one
                range_end = diff[i]
                diff_range_add()
                break

            if diff[i+1] - diff[i] > 1:
                # range was broken
                range_end = diff[i]
                diff_range_add()
                range_start = diff[i+1]

        return diff_range

    def correct_addr(self, diff):
        for i in range(len(diff)):
            diff[i] = diff[i] + self.offset_addr

    def get_val_a(self, idx):
        return self.bytes_file_a[idx-self.offset_addr]

    def get_val_b(self, idx):
        return self.bytes_file_b[idx-self.offset_addr]

    def get_filename_a(self):
        return self.file_a

    def get_filename_b(self):
        return self.file_b

def main(file_a, file_b, offset):
    try:
        md = MemDiff(file_a, file_b, offset_addr=offset)
    except IOError as e:
        print('Could not open file (%s)' % e)
        return

    diff = md.diff()
    #diff_range = md.diff_range(diff)
    #print(diff)
    #print(diff_range)

    if (len(diff) == 0):
        print('Binary files \'%s\' and \'%s\' are identical!' % \
            (os.path.basename(md.get_filename_a()), os.path.basename( md.get_filename_b())))
        return

    print('Binary difference between \'%s\' and \'%s\'' % \
            (os.path.basename(md.get_filename_a()), os.path.basename( md.get_filename_b())))
    print('There are ' + str(len(diff)) + ' differences')
    for diff_idx in diff:
        print('idx ' + hex(diff_idx) + '\t[' \
                + hex(int(md.get_val_a(diff_idx))) + ' -> ' \
                + hex(int(md.get_val_b(diff_idx))) + ']')
    return diff


if __name__ == '__main__':
    import argparse
    import sys

    parser = argparse.ArgumentParser()
    parser.add_argument("-a", "--address_offset", help="offset the address")
    parser.add_argument("file_a", help="file a")
    parser.add_argument("file_b", help="file b")
    args = parser.parse_args()

    if args.address_offset is not None:
        try:
            offset = int(args.address_offset)
        except ValueError:
            try:
                offset = int(args.address_offset, 16)
            except ValueError:
                print('Error parsing offset: ' + args.address_offset)
                sys.exit()
    else:
        offset = 0

    print('File a: ' + args.file_a)
    print('File b: ' + args.file_b)
    print('Address offset: ' + str(offset))

    main(args.file_a, args.file_b, offset)
