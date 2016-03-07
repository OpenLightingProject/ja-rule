#!/usr/bin/python
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# verify_cpplint_coverge.py
# Copyright (C) 2016 Peter Newman

from __future__ import print_function

import fnmatch
import os
import re
import sys
import textwrap

# Lintable files
LINTABLE_PATTERNS = [
  '*.c',
  '*.cpp',
  '*.h'
]

# File & directory patterns that differ between what's in the git repo and
# what's checked by cpplint.py
IGNORE_PATTERNS = [
  'Bootloader/firmware/src/system_config/*',
  'config.h',
  'firmware/src/system_config/*',
  'gtest-read-only/*',
  'src/system_config/*',
  'tests/src/system_config/*',
  'tests/boot_src/system_config/*',
]


def Usage(arg0):
  print (textwrap.dedent("""\
  Usage: %s <tree> <cpplintLog>

  Check for files that exist in tree but aren't checked by cpplint. This can be used to
  ensure we don't forget to lint files.""" % arg0))


def ShouldLint(path):
  for pattern in LINTABLE_PATTERNS:
    if fnmatch.fnmatch(path, pattern):
      return True
  return False


def ShouldIgnore(path):
  for pattern in IGNORE_PATTERNS:
    if fnmatch.fnmatch(path, pattern):
      return True
  return False


def BuildTree(root):
  tree = set()
  for directory, sub_dirs, files in os.walk(root):
    rel_dir = os.path.relpath(directory, root)
    if rel_dir == '.':
      rel_dir = ''
    if ShouldIgnore(rel_dir):
      continue
    for f in files:
      path = os.path.join(rel_dir, f)
      if ShouldLint(path) and not ShouldIgnore(path):
        tree.add(path)

  return tree


def main():
  if len(sys.argv) != 3:
    Usage(sys.argv[0])
    sys.exit(1)

  tree_root = sys.argv[1]
  cpplint_log = sys.argv[2]
  if not os.path.isdir(tree_root):
    print('Not a directory `%s`' % tree_root, file=sys.stderr)
    sys.exit(2)

  tree = BuildTree(tree_root)
  cpplint = set()

  f = open(cpplint_log)
  lines = f.readlines()
  f.close()

  for line in lines:
    m = re.match(r'^Done processing (.+)\n$', line)
    if (m is not None):
      cpplint.add(m.group(1))

  difference = tree.difference(cpplint)

  if difference:
    for file_path in difference:
      print('Not being linted %s' % file_path, file=sys.stderr)

    print('\n---------------------------------------', file=sys.stderr)
    print('Either add the missing files to the cpplint run or update\n'
          'IGNORE_PATTERNS in scripts/verify_cpplint_coverge.py',
          file=sys.stderr)
    print('---------------------------------------', file=sys.stderr)
    sys.exit(1)

  sys.exit()

if __name__ == '__main__':
  main()
