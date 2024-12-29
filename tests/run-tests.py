#!/usr/bin/env python3

"""
integration test suite

Despite using Python’s unittest, this is not a set of unit tests. The Python
module is simply a nice low-overhead testing framework.
"""

import codecs
import multiprocessing
import os
from pathlib import Path
import re
import subprocess as sp
import sys
import tempfile
import unittest

CPUS = multiprocessing.cpu_count()

VERIFIER_RNG = Path(__file__).resolve().parent / "../misc/verifier.rng"
MURPHI2XML_RNG = Path(__file__).resolve().parent / "../misc/murphi2xml.rng"

# test configuration variables, set during main
CONFIG = {}

def enc(s): return s.encode("utf-8", "replace")
def dec(s): return s.decode("utf-8", "replace")

def run(args, stdin = None):
  """
  run a command and return its result
  """
  if stdin is not None:
    stdin = enc(stdin)
  env = {k: v for k, v in os.environ.items()}
  env.update({k: str(v) for k, v in CONFIG.items()})
  p = sp.Popen([str(a) for a in args], stdout=sp.PIPE, stderr=sp.PIPE,
               stdin=sp.PIPE, env=env)
  stdout, stderr = p.communicate(stdin)
  return p.returncode, dec(stdout), dec(stderr)

def parse_test_options(src, debug = False, multithreaded = False, xml = False):
  """
  extract test tweaks and directives from leading comments in a test input
  """
  with open(str(src), "rt", encoding="utf-8") as f:
    for line in f:
      # recognise “-- rumur_flags: …” etc lines
      m = re.match(r"\s*--\s*(?P<key>[a-zA-Z_]\w*)\s*:(?P<value>.*)$", line)
      if m is None:
        break
      yield m.group("key"), eval(m.group("value").strip())

class executable(unittest.TestCase):
  """
  test cases involving running a custom executable file
  """

  def _run(self, testcase):

    assert os.access(str(testcase), os.X_OK), "non-executable test case " \
      "{} attached to executable class".format(testcase)

    ret, stdout, stderr = run([testcase])
    output = "{}{}".format(stdout, stderr)
    if ret == 125:
      self.skipTest(output.strip())
    elif ret != 0:
      self.fail(output)

class murphi2c(unittest.TestCase):
  """
  test cases for murphi2c
  """

  def _run(self, testcase):

    tweaks = {k: v for k, v in parse_test_options(testcase)}

    # there is no C equivalent of isundefined, because an implicit assumption in
    # the C representation is that you do not rely on undefined values
    with open(str(testcase), "rt", encoding="utf-8") as f:
      should_fail = re.search(r"\bisundefined\b", f.read()) is not None

    args = ["murphi2c", testcase]
    if CONFIG["HAS_VALGRIND"]:
      args = ["valgrind", "--leak-check=full", "--show-leak-kinds=all",
        "--error-exitcode=42"] + args
    ret, stdout, stderr = run(args)
    if CONFIG["HAS_VALGRIND"]:
      if ret == 42:
        self.fail("Memory leak:\n{}{}".format(stdout, stderr))

    # if rumur was expected to reject this model, we allow murphi2c to fail
    if tweaks.get("rumur_exit_code", 0) == 0 and not should_fail and ret != 0:
      self.fail("Unexpected murphi2c exit status {}:\n{}{}"
                .format(ret, stdout, stderr))

    if should_fail and ret == 0:
      self.fail("Unexpected murphi2c exit status {}:\n{}{}"
                .format(ret, stdout, stderr))

    if ret != 0:
      return

    # omit -Werror=maybe-uninitialized which identifies legitimate problems in
    # input models
    cflags = [
      f for f in CONFIG["C_FLAGS"] if f != "-Werror=maybe-uninitialized"
    ]

    # ask the C compiler if this is valid
    args = [CONFIG["CC"]] + cflags + ["-c", "-o", os.devnull, "-"]
    ret, out, err = run(args, stdout)
    if ret != 0:
      self.fail("C compilation failed:\n{}{}\nProgram:\n{}"
                .format(out, err, stdout))

class murphi2cHeader(unittest.TestCase):
  """
  test cases for murphi2c --header
  """

  def _run(self, testcase):

    tweaks = {k: v for k, v in parse_test_options(testcase)}

    # there is no C equivalent of isundefined, because an implicit assumption in
    # the C representation is that you do not rely on undefined values
    with open(str(testcase), "rt", encoding="utf-8") as f:
      should_fail = re.search(r"\bisundefined\b", f.read()) is not None

    args = ["murphi2c", "--header", testcase]
    if CONFIG["HAS_VALGRIND"]:
      args = ["valgrind", "--leak-check=full", "--show-leak-kinds=all",
        "--error-exitcode=42"] + args
    ret, stdout, stderr = run(args)
    if CONFIG["HAS_VALGRIND"]:
      if ret == 42:
        self.fail("Memory leak:\n{}{}".format(stdout, stderr))

    # if rumur was expected to reject this model, we allow murphi2c to fail
    if tweaks.get("rumur_exit_code", 0) == 0 and not should_fail and ret != 0:
      self.fail("Unexpected murphi2c exit status {}:\n{}{}"
                .format(ret, stdout, stderr))

    if should_fail and ret == 0:
      self.fail("Unexpected murphi2c exit status {}:\n{}{}"
                .format(ret, stdout, stderr))

    if ret != 0:
      return

    with tempfile.TemporaryDirectory() as tmp:

      # write the header to a temporary file
      header = Path(tmp) / "header.h"
      with open(str(header), "wt", encoding="utf-8") as f:
        f.write(stdout)

      # ask the C compiler if the header is valid
      main_c = '#include "{}"\nint main(void) {{ return 0; }}\n'.format(header)
      args = [CONFIG["CC"]] + CONFIG["C_FLAGS"] + ["-o", os.devnull, "-"]
      ret, stdout, stderr = run(args, main_c)
      if ret != 0:
        self.fail("C compilation failed:\n{}{}".format(stdout, stderr))

      # ask the C++ compiler if it is valid there too
      ret, stdout, stderr = run([CONFIG["CXX"], "-std=c++11", "-o", os.devnull,
        "-x", "c++", "-", "-Werror=format", "-Werror=sign-compare",
        "-Werror=type-limits"], main_c)
      if ret != 0:
        self.fail("C++ compilation failed:\n{}{}".format(stdout, stderr))

class murphi2uclid(unittest.TestCase):
  """
  test cases for murphi2uclid
  """

  def _run(self, testcase):

    tweaks = {k: v for k, v in parse_test_options(testcase)}

    # test cases for which murphi2uclid is expected to fail
    MURPHI2UCLID_FAIL = (
      # contains `<<` or `>>`
      "const-folding5.m",
      "const-folding6.m",
      "lsh-basic.m",
      "rsh-and.m",
      "rsh-basic.m",
      "smt-bv-lsh.m",
      "smt-bv-rsh.m",

      # contains `/`
      "const-folding3.m",
      "division.m",
      "smt-bv-div.m",
      "smt-bv-div2.m",
      "smt-div.m",
      "unicode-div.m",
      "unicode-div2.m",

      # contains `%`
      "const-folding4.m",
      "mod-neg-1.m",
      "mod-neg-1_2.m",
      "put-string-injection.m",
      "smt-bv-mod.m",
      "smt-bv-mod2.m",
      "smt-mod.m",

      # contains alias statements
      "alias-and-field.m",
      "alias-in-bound.m",
      "alias-in-bound2.m",
      "alias-literal.m",
      "alias-of-alias-rule.m",
      "alias-of-alias-rule2.m",
      "alias-of-alias-stmt.m",
      "basic-aliasrule.m",
      "mixed-aliases.m",

      # `clear` of a complex type
      "clear-complex.m",

      # contains `cover`
      "cover-basic.m",
      "cover-basic2.m",
      "cover-miss.m",
      "cover-multiple.m",
      "cover-stmt.m",
      "cover-stmt-miss.m",
      "cover-trivial.m",
      "string-injection.m",

      # contains `isundefined`
      "diff-trace-arrays.m",
      "isundefined-basic.m",
      "isundefined-decl.m",
      "isundefined-element.m",
      "isundefined-function.m",
      "for-variants.m",
      "scalarset-cex.m",
      "scalarset-schedules-off.m",
      "scalarset-schedules-off-2.m",

      # contains `put`
      "for-step-0-dynamic.m",
      "put-stmt.m",
      "put-stmt2.m",
      "put-stmt3.m",
      "put-stmt4.m",
      "scalarset-put.m",

      # contains early return from a function/procedure/rule
      "return-from-rule.m",
      "return-from-ruleset.m",
      "return-from-startstate.m",

      # `exists` or `forall` with non-1 step
      "smt-bv-exists4.m",
      "smt-bv-forall4.m",
      "smt-exists4.m",
      "smt-forall4.m",

      # `liveness` inside a `ruleset`
      "liveness-in-ruleset.m",
      "liveness-in-ruleset2.m",
    )

    # test cases fo which Uclid5 is expected to fail
    UCLID_FAIL = (
      # contains a record field with the same name as a variable
      # https://github.com/uclid-org/uclid/issues/99
      "compare-record.m",
      "smt-array-of-record.m",
      "smt-record-bool-field.m",
      "smt-record-bool-field2.m",
      "smt-record-enum-field.m",
      "smt-record-enum-field2.m",
      "smt-record-of-array.m",
      "smt-record-range-field.m",
      "smt-record-range-field2.m",

      # recursive function calls
      "recursion2.m",
      "recursion4.m",

      # reference to a field of an array element
      "193.m",

      # function calls within expressions
      "differing-type-return.m",
      "differing-type-return3.m",
      "function-call-in-if.m",
      "function-in-guard.m",
      "multiple-parameters.m",
      "multiple-parameters2.m",
      "non-const-parameters.m",
      "recursion1.m",
      "recursion5.m",
      "reference-function-parameter.m",
      "reference-function-parameter2.m",
      "section-order4.m",
      "section-order5.m",
      "section-order10.m",
      "type-shadowing2.m",

      # modifies a mutable parameter within a function, which is not valid
      # within a Uclid5 procedure
      "reference-function-parameter3.m",
    )

    args = ["murphi2uclid", testcase]
    if CONFIG["HAS_VALGRIND"]:
      args = ["valgrind", "--leak-check=full", "--show-leak-kinds=all",
        "--error-exitcode=42"] + args
    ret, stdout, stderr = run(args)
    if CONFIG["HAS_VALGRIND"]:
      if ret == 42:
        self.fail("Memory leak:\n{}{}".format(stdout, stderr))

    # if rumur was expected to reject this model, we allow murphi2uclid to fail
    should_fail = testcase.name in MURPHI2UCLID_FAIL
    could_fail = tweaks.get("rumur_exit_code", 0) != 0 or should_fail

    if not could_fail and ret != 0:
      self.fail("Unexpected murphi2uclid exit status {}:\n{}{}"
                .format(ret, stdout, stderr))

    if should_fail and ret == 0:
      self.fail("Unexpected murphi2uclid exit status {}:\n{}{}"
                .format(ret, stdout, stderr))

    if ret != 0:
      return

    # if we do not have Uclid5 available, skip the remainder of the test
    if not CONFIG["HAS_UCLID"]:
      self.skipTest("uclid not available for validation")

    with tempfile.TemporaryDirectory() as tmp:

      # write the Uclid5 source to a temporary file
      src = Path(tmp) / "source.ucl"
      with open(str(src), "wt", encoding="utf-8") as f:
        f.write(stdout)

      # ask Uclid if the source is valid
      ret, stdout, stderr = run(["uclid", src])
      if testcase.name in UCLID_FAIL and ret == 0:
        self.fail("uclid unexpectedly succeeded:\n{}{}".format(stdout, stderr))
      if testcase.name not in UCLID_FAIL and ret != 0:
        self.fail("uclid failed:\n{}{}".format(stdout, stderr))

class murphi2xml(unittest.TestCase):
  """
  test cases for murphi2xml
  """

  def _run(self, testcase):

    tweaks = {k: v for k, v in parse_test_options(testcase)}

    args = ["murphi2xml", testcase]
    if CONFIG["HAS_VALGRIND"]:
      args = ["valgrind", "--leak-check=full", "--show-leak-kinds=all",
        "--error-exitcode=42"] + args
    ret, stdout, stderr = run(args)
    if CONFIG["HAS_VALGRIND"]:
      if ret == 42:
        self.fail("Memory leak:\n{}{}".format(stdout, stderr))

    # if rumur was expected to reject this model, we allow murphi2xml to fail
    if tweaks.get("rumur_exit_code", 0) == 0 and ret != 0:
      self.fail("Unexpected murphi2xml exit status {}:\n{}{}"
                .format(ret, stdout, stderr))

    if ret != 0:
      return

    # murphi2xml will have written XML to its stdout
    xmlcontent = stdout

    # See if we have xmllint
    if not CONFIG["HAS_XMLLINT"]:
      self.skipTest("xmllint not available for validation")

    # Validate the XML
    ret, stdout, stderr = run(["xmllint", "--relaxng", MURPHI2XML_RNG,
      "--noout", "-"], xmlcontent)
    if ret != 0:
      self.fail("Failed to validate:\n{}{}".format(stdout, stderr))

class rumur(unittest.TestCase):
  """
  test cases involving generating a checker and running it
  """

  def _run_param(self, testcase, debug, optimised, multithreaded, xml):

    tweaks = {k: v for k, v in parse_test_options(testcase, debug,
      multithreaded, xml)}

    if tweaks.get("skip_reason") is not None:
      self.skipTest(tweaks["skip_reason"])

    # build up arguments to call rumur
    args = ["rumur", "--output", "/dev/stdout", testcase]
    if debug: args += ["--debug"]
    if xml: args += ["--output-format", "machine-readable"]
    if multithreaded and CPUS == 1: args +=["--threads", "2"]
    elif not multithreaded: args += ["--threads", "1"]
    args += tweaks.get("rumur_flags", [])

    if CONFIG["HAS_VALGRIND"]:
      args = ["valgrind", "--leak-check=full", "--show-leak-kinds=all",
        "--error-exitcode=42"] + args

    # call rumur
    ret, stdout, stderr = run(args)
    if CONFIG["HAS_VALGRIND"]:
      if ret == 42:
        self.fail("Memory leak:\n{}{}".format(stdout, stderr))
    if ret != tweaks.get("rumur_exit_code", 0):
      self.fail("Rumur failed:\n{}{}".format(stdout, stderr))

    # if we expected to fail, we are done
    if ret != 0: return

    model_c = stdout

    with tempfile.TemporaryDirectory() as tmp:

      # build up arguments to call the C compiler
      model_bin = Path(tmp) / "model.exe"
      args = [CONFIG["CC"]] + CONFIG["C_FLAGS"]
      if optimised: args += ["-O3"]
      args += ["-o", model_bin, "-", "-lpthread"]

      if CONFIG["NEEDS_LIBATOMIC"]:
        args += ["-latomic"]

      # call the C compiler
      ret, stdout, stderr = run(args, model_c)
      if ret != 0:
        self.fail("C compilation failed:\n{}{}".format(stdout, stderr))

      # now run the model itself
      ret, stdout, stderr = run([model_bin])
      if ret != tweaks.get("checker_exit_code", 0):
        self.fail("Unexpected checker exit status {}:\n{}{}"
                  .format(ret, stdout, stderr))

    # if the test has a stdout expectation, check that now
    if tweaks.get("checker_output") is not None:
      if tweaks["checker_output"].search(stdout) is None:
        self.fail("Checker output did not match expectation regex:\n{}{}"
                  .format(stdout, stderr))

    # coarse grained check for whether the model contains a `put` statement that
    # could screw up XML validation
    with open(str(testcase), "rt", encoding="utf-8") as f:
      has_put = re.search(r"\bput\b", f.read()) is not None

    if xml and not has_put:

      model_xml = stdout

      if not CONFIG["HAS_XMLLINT"]: self.skipTest("xmllint not available")

      # validate the XML
      args = ["xmllint", "--relaxng", VERIFIER_RNG, "--noout", "-"]
      ret, stdout, stderr = run(args, model_xml)
      if ret != 0:
        self.fail("Failed to XML-validate machine reachable output:\n{}{}"
                  .format(stdout, stderr))

class rumurSingleThreaded(rumur):
  def _run(self, testcase):
    self._run_param(testcase, False, False, False, False)

class rumurDebugSingleThreaded(rumur):
  def _run(self, testcase):
    self._run_param(testcase, True, False, False, False)

class rumurOptimisedSingleThreaded(rumur):
  def _run(self, testcase):
    self._run_param(testcase, False, True, False, False)

class rumurDebugOptimisedSingleThreaded(rumur):
  def _run(self, testcase):
    self._run_param(testcase, True, True, False, False)

class rumurMultithreaded(rumur):
  def _run(self, testcase):
    self._run_param(testcase, False, False, True, False)

class rumurDebugMultithreaded(rumur):
  def _run(self, testcase):
    self._run_param(testcase, True, False, True, False)

class rumurOptimisedMultithreaded(rumur):
  def _run(self, testcase):
    self._run_param(testcase, False, True, True, False)

class rumurDebugOptimisedMultithreaded(rumur):
  def _run(self, testcase):
    self._run_param(testcase, True, True, True, False)

class rumurSingleThreadedXML(rumur):
  def _run(self, testcase):
    self._run_param(testcase, False, False, False, True)

class rumurOptimisedSingleThreadedXML(rumur):
  def _run(self, testcase):
    self._run_param(testcase, False, True, False, True)

class rumurMultithreadedXML(rumur):
  def _run(self, testcase):
    self._run_param(testcase, False, False, True, True)

class rumurOptimisedMultithreadedXML(rumur):
  def _run(self, testcase):
    self._run_param(testcase, False, True, True, True)


class MurphiFormat(unittest.TestCase):
    """
    test cases for murphi-format
    """

    def test_colon(self):
        """colon spacing in definitions as well as ternary expressions"""

        # sample Murphi that uses colons in both situations, that should be reflowed
        model = "const y: 0?1:2; var x : boolean;"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn("y: 0", stdout, "incorrect spacing for const definition")
        self.assertIn("0 ? 1 : 2", stdout, "incorrect spacing for ternary expression")
        self.assertIn("x: boolean", stdout, "incorrect spacing for var definition")

    def test_arrow_begin1(self):
        """`==> begin` should not be reflowed with a newline"""

        # sample Murphi that uses something that should be stable
        model = 'rule "foo" ==> begin end'

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn("==> begin", stdout, "incorrect spacing for `==> begin`")

    def test_arrow_begin2(self):
        """`==> begin` should not be reflowed with a newline"""

        # sample Murphi that uses something that should be reflowed
        model = 'rule "foo" ==>begin end'

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn("==> begin", stdout, "incorrect spacing for `==> begin`")

    def test_newline_on_end(self):
        """`end` should force a newline"""

        # sample Murphi that uses something that should be reflowed
        model = 'rule "foo" begin end rule "bar" begin end'

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertRegex(stdout, "\\bbegin\n+end\\b", "no newline before end")
        self.assertRegex(stdout, "\\bend\n+rule\\b", "no newline after end")

    def test_unary_in_for(self):
        """a unary operator following a keyword should be spaced correctly"""

        model = "rule begin for x := 0 to - 2 by - 1 do end; end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn("to -2", stdout, "incorrect unary minus spacing")
        self.assertIn("by -1", stdout, "incorrect unary minus spacing")

    def test_begin_indentation(self):
        """`begin` should not be erroneously indented by decl blocks"""

        model = "rule var x: boolean; begin end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertRegex(
            stdout,
            re.compile(r"^begin\b", flags=re.MULTILINE),
            "incorrect begin indentation"
        )
        self.assertNotRegex(
            stdout,
            re.compile(r"^\s+begin\b", flags=re.MULTILINE),
            "incorrect begin indentation"
        )

    def test_multiline_comment(self):
        """multiline comments should be recognised"""

        model = "rule /* hello\nworld */ begin end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn("/* hello", stdout, "multiline comments mishandled")
        self.assertIn("world */", stdout, "multiline comments mishandled")

    def test_then_indentation(self):
        """`then` should incur an indent"""

        model = "rule begin if 0 = 0 then\nx := 2; end; end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn("0 = 0 then", stdout, "incorrect `then` indentation")
        self.assertRegex(
            stdout,
            re.compile(r"^    x := 2", flags=re.MULTILINE),
            "incorrect `then` indentation",
        )

    def test_newline_comment(self):
        """is newline followed by a comment preserved?"""

        model = "const N:\n-- a comment\n0;"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertRegex(
            stdout,
            re.compile(r"^\s*-- a comment$", flags=re.MULTILINE),
            "incorrect newline,comment handling"
        )

    def test_brace_ender(self):
        """'}' should trigger a newline"""

        model = "type x: enum {A, B} y: boolean;"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn("}\n", stdout, "`}` did not trigger newline")

    def test_double_paren(self):
        """does multi-dimensional indexing get spaced correctly?"""

        model = "rule begin x [ 1 ] [ 2 ] := 3; end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn("x[1][2]", stdout, "spaced incorrectly")

    def test_procedure_var(self):
        """does `var` within a function/procedure parameter list cause problems?"""

        model = "procedure foo( var x : boolean); begin end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn("(var x: boolean)", stdout, "var parameters spaced incorrectly")

    def test_trailing_space(self):
        """
        does something that would normally be followed by a space also incur a space
        when landing at the end-of-file?
        """

        model = "invariant x"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertRegex(
            stdout,
            re.compile(r"\binvariant x$", flags=re.MULTILINE),
            "incorrect spacing around end-of-file"
        )

    def test_bad_operator(self):
        """
        non-operators like `>-` should not be recognised
        """

        model = "rule begin if x >- 1 then end; end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertNotIn(" >- ", stdout, "`>-` incorrectly considered an operator")
        self.assertNotIn(">-", stdout, "`>-` incorrectly considered an operator")
        self.assertIn("> -", stdout, "incorrect spacing around `>-`")

    def test_case(self):
        """reformatting should be caseless"""

        model = "cOnSt N: 0;"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn("cOnSt\n  N: 0;", stdout, "incorrect spacing around erratic casing")

    def test_no_start_newline(self):
        """a newline should not be inserted before all content"""

        model = "rule begin end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertTrue(stdout.startswith("rule"), "incorrect preceding space inserted")

    def test_switch(self):
        """formatting of switch statements"""

        model = "rule begin switch x\nend; end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertEqual(
            "rule begin\n  switch x\n  end;\nend\n",
            stdout,
            "incorrect switch formatting"
        )

    def test_multiple_inplace(self):
        """formatting multiple files in-place should work"""

        # a model with a small amount of text
        short = "const\n  x: 10;\n"

        # a model with a longer amount of text
        long = "var\n  x: 0..10;\n  y: 0..10;\n  z: 0..10;\n"

        with tempfile.TemporaryDirectory() as tmp:
            short_path = Path(tmp) / "short.m"
            long_path = Path(tmp) / "long.m"

            with open(short_path, "wt", encoding="utf-8") as f:
                f.write(short)
            with open(long_path, "wt", encoding="utf-8") as f:
                f.write(long)

            ret, stdout, stderr = run(
                ["murphi-format", "--in-place", long_path, short_path]
            )

            self.assertEqual(ret, 0, "failed to reflow Murphi snippets")
            self.assertEqual(
                stdout, "", "murphi-format produced output when asked for in-place"
            )
            self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

            with open(short_path, "rb") as f:
                content = f.read()
            self.assertEqual(
                content,
                short.encode("utf-8"),
                "model was reflowed incorrectly",
            )

            with open(long_path, "rb") as f:
                content = f.read()
            self.assertEqual(
                content,
                long.encode("utf-8"),
                "model was reflowed incorrectly",
            )

    def test_trailing_indentation(self):
        """we should not incur a trailing indentation at end of file"""

        model = "const N: 10;"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertEqual(
            "const\n  N: 10;\n", stdout, "incorrect const block formatting"
        )

    def test_startstate_no_begin(self):
        """unnamed startstate should be followed by correct indentation"""

        model = "startstate x := y; end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertEqual(
            "startstate\n  x := y;\nend\n", stdout, "incorrect startstate formatting"
        )

    def test_startstate_begin(self):
        """unnamed startstate with `begin` should be followed by correct indentation"""

        model = "startstate begin x := y; end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertEqual(
            "startstate begin\n  x := y;\nend\n",
            stdout,
            "incorrect startstate formatting",
        )

    def test_else(self):
        """indentation of `else` should be correct"""

        model = "rule begin if x = x then y := z; else y := w; end; end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn(
            "\n  if x = x then\n    y := z;\n  else\n    y := w;\n  end;\n",
            stdout,
            "incorrect else formatting",
        )

    def test_elsif(self):
        """indentation of `elsif` should be correct"""

        model = "rule begin if x = x then y := z; elsif y = y then y := w; end; end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn(
            "\n  if x = x then\n    y := z;\n  elsif y = y then\n    y := w;\n  end;\n",
            stdout,
            "incorrect elsif formatting",
        )

    def test_while_paren(self):
        """`while` followed by parenthesised expression should be spaced"""

        model = "rule begin while (x = x) do y := z; end; end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn(
            "\n  while (x = x) do\n    y := z;\n  end;\n",
            stdout,
            "incorrect while formatting",
        )

    def test_no_format(self):
        """formatting disabling comments should be respected"""

        model = "rule begin -- murphi-format: off\n while (x = x) do y := z; end; -- murphi-format: on\n end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn(
            "-- murphi-format: off\n while (x = x) do y := z; end; -- murphi-format: on\n",
            stdout,
            "format-disabling comments did not work",
        )

    def test_isundefined(self):
        """`isundefined` should be spaced correctly"""

        model = "rule begin x := isundefined (x); end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn(
            ":= isundefined(x)",
            stdout,
            "`isundefined` spaced incorrectly",
        )

    def test_line_comment(self):
        """line comments should not be bumped onto the next line"""

        model = (
            "rule begin x := y; -- line comment\nz := w;\n-- comment on newline\n end"
        )

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertEqual(
            "rule begin\n  x := y; -- line comment\n  z := w;\n  -- comment on newline\nend\n",
            stdout,
            "comments broken incorrectly",
        )

    def test_procedure_params(self):
        """parameter lists in procedures should appear correctly"""

        model = "procedure foo(a: b; c: d) begin end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn(
            "a: b; c: d",
            stdout,
            "procedure parameters formatted incorrectly",
        )

    def test_comment_const_interleaved(self):
        """comments attached to consts should stay where they are"""

        model = "const x: 42;\n\n-- hello world\ny: 42;"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertEqual(
            "const\n  x: 42;\n\n  -- hello world\n  y: 42;\n",
            stdout,
            "const comments formatted incorrectly",
        )

    def test_scalarset(self):
        """`scalarset` should be spaced correctly"""

        model = "type x : scalarset (4);"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn(": scalarset(4)", stdout, "`scalarset` spaced incorrectly")

    def test_array(self):
        """`array` should be spaced intuitively"""

        model = "type x : array [ boolean ] of boolean;"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn(
            ": array[boolean] of boolean", stdout, "`array` spaced incorrectly"
        )

    def test_not_unicode(self):
        """`¬` spacing should be correct"""

        model = "rule begin x := ¬ y; end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn(":= ¬y", stdout, "`¬` spaced incorrectly")

    def test_smart_quotes(self):
        """smart quotes (“”) should be handled correctly"""

        model = "rule begin assert “foo bar” x; end"

        ret, stdout, stderr = run(["murphi-format"], model)

        self.assertEqual(ret, 0, "failed to reflow Murphi snippet")
        self.assertEqual(stderr, "", "murphi-format printed errors/warnings")

        self.assertIn("assert “foo bar” x", stdout, "smart quotes spaced incorrectly")


def make_name(t):
  """
  name mangle a path into a valid test case name
  """
  safe_name = re.sub(r"[^a-zA-Z0-9]", "_", t.name)
  return "test_{}".format(safe_name)

def main():

  # setup stdout to make encoding errors non-fatal
  sys.stdout = codecs.getwriter("utf-8")(sys.stdout.buffer, "replace")

  # parse configuration
  global CONFIG
  for p in sorted((Path(__file__).parent / "config").iterdir()):

    # skip subdirectories
    if p.is_dir(): continue

    # skip non-executable files
    if not os.access(str(p), os.X_OK): continue

    CONFIG[p.name] = eval(sp.check_output([str(p)]))
    sys.stderr.write(f"configuration variable {p.name} = {CONFIG[p.name]}\n")

  # find files in our directory
  root = Path(__file__).parent
  for p in sorted(root.iterdir()):

    # skip directories
    if p.is_dir(): continue

    # skip ourselves
    if os.path.samefile(str(p), __file__): continue

    name = make_name(p)

    # if this is executable, treat it as a test case
    if os.access(str(p), os.X_OK):
      assert not hasattr(executable, name), \
        "name collision involving executable.{}".format(name)
      setattr(executable, name, lambda self, p=p: self._run(p))

    # if this is not a model, skip the remaining generic logic
    if p.suffix != ".m": continue

    for c in (rumurSingleThreaded,
              rumurDebugSingleThreaded,
              rumurOptimisedSingleThreaded,
              rumurDebugOptimisedSingleThreaded,
              rumurMultithreaded,
              rumurDebugMultithreaded,
              rumurOptimisedMultithreaded,
              rumurDebugOptimisedMultithreaded,
              rumurSingleThreadedXML,
              rumurOptimisedSingleThreadedXML,
              rumurMultithreadedXML,
              rumurOptimisedMultithreadedXML,
             ):
      assert not hasattr(c, name), \
        "name collision involving rumur.{}".format(name)
      setattr(c, name, lambda self, p=p: self._run(p))

    assert not hasattr(murphi2c, name), \
      "name collision involving murphi2c.{}".format(name)
    setattr(murphi2c, name, lambda self, p=p: self._run(p))

    assert not hasattr(murphi2cHeader, name), \
      "name collision involving murphi2cHeader.{}".format(name)
    setattr(murphi2cHeader, name, lambda self, p=p: self._run(p))

    assert not hasattr(murphi2uclid, name), \
      "name collision involving murphi2uclid.{}".format(name)
    setattr(murphi2uclid, name, lambda self, p=p: self._run(p))

    assert not hasattr(murphi2xml, name), \
      "name collision involving murphi2xml.{}".format(name)
    setattr(murphi2xml, name, lambda self, p=p: self._run(p))

  unittest.main()

if __name__ == "__main__":
  main()
