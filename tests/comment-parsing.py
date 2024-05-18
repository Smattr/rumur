#!/usr/bin/env python3

"""
test comment retrieval API
"""

import shutil
import subprocess
import sys

if not shutil.which("murphi-comment-ls"):
  print("murphi-comment-ls not found")
  sys.exit(125)


# a model without any comments
NONE = """
var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end;
"""

# a model with a single line comment
SINGLE = """
var
  x: boolean;

startstate begin
  x := -- hello world
  true;
end;

rule begin
  x := !x;
end;
"""

# a model with a multiline comment
MULTILINE = """
var
  x: boolean;

startstate begin
  x := /* hello world */ true;
end;

rule begin
  x := !x;
end;
"""

# a model with a single line comment terminated by EOF, not \n
SINGLE_EOF = """
var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end; -- hello world"""

# a model with a multiline comment terminated by EOF, not */
MULTILINE_EOF = """
var
  x: boolean;

startstate begin
  x := true;
end;

rule begin
  x := !x;
end; /* hello world """

# comments inside strings that should be ignored
STRING = """
var
  x: boolean;

startstate "hello -- world" begin
  x := true;
end;

rule "hello /* world */" begin
  x := !x;
end;
"""

# a model with a mixture
MIX = """
var
  x: boolean;

startstate begin
  x := -- hello world
  /* hello world */true;
end; -- hello /* hello world */
/* hello -- hello */
rule begin
  x := !x;
end;
"""

def process(input):
  proc = subprocess.Popen(["murphi-comment-ls"], stdin=subprocess.PIPE,
    stdout=subprocess.PIPE, universal_newlines=True)
  stdout, _ = proc.communicate(input)
  assert proc.returncode == 0
  return stdout

def main():

  assert process(NONE) == ""

  assert process(SINGLE) == "6.8-21:  hello world\n"
  assert process(MULTILINE) == "6.8-24:  hello world \n"

  assert process(SINGLE_EOF) == "11.6-19:  hello world\n"
  assert process(MULTILINE_EOF) == "11.6-20:  hello world \n"

  assert process(STRING) == ""

  assert process(MIX) == "6.8-21:  hello world\n" \
                         "7.3-19:  hello world \n" \
                         "8.6-31:  hello /* hello world */\n" \
                         "9.1-20:  hello -- hello \n"

  return 0

if __name__ == "__main__":
  sys.exit(main())
