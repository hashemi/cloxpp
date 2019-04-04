#!/usr/bin/env python3

from os.path import dirname, realpath
import sys
from test import C_SUITES, INTERPRETERS, run_suites, run_suite, filter_path
import test

def c_to_cpp_interpreter(interpreter):
    if interpreter.language == 'c':
        interpreter.args = ['build/Release/cloxpp']
    return interpreter

test.REPO_DIR = dirname(realpath(__file__))

INTERPRETERS = {name: c_to_cpp_interpreter(interpreter) for (name, interpreter) in INTERPRETERS.items()}

def main():
  global filter_path

  if len(sys.argv) < 2 or len(sys.argv) > 3:
    print('Usage: test.py <interpreter> [filter]')
    sys.exit(1)

  if len(sys.argv) == 3:
    filter_path = sys.argv[2]

  if sys.argv[1] == 'cpp':
    run_suites(C_SUITES)
  elif sys.argv[1] not in INTERPRETERS:
    print('Unknown interpreter "{}"'.format(sys.argv[1]))
    sys.exit(1)

  else:
    if not run_suite(sys.argv[1]):
      sys.exit(1)

if __name__ == '__main__':
  main()