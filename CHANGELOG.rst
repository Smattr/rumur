Change log
==========

v2024.07.14
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: ``murphi2uclid`` no longer ignores the long form option
  ``--numeric-type`` (commit 285453789b7394aab8e182935aca7c91ed504832).
* Bug fix: ``murphi2uclid`` no longer ignores the short form options ``-n`` and
  others (commit ad1ddb7ed750d82460e97dd952814ad6b6156edd).
* Bug fix: irrelevant file descriptors are no longer leaked into an SMT solver
  configured via ``--smt-path`` (commits
  90132cf838c7bcfa2a1081786f7a0b511c5e281a,
  cb9d47fa6037187d995995bbc8730b6976d8c804).
* Bug fix: ``--help`` works even when ``$TMPDIR`` contains spaces (commit
  9db30b901b0598b99d300c7988e24ddb28be344c).
* A negation that results in the greatest negative number of the value type
  (controllable with ``--value-type``) is no longer considered a runtime error
  (commit fd1627d092275e96f475a0ac28de7927ad65d69b).
* Modulo of the greatest negative number of the value type by ``-1`` is no
  longer considered a runtime error (commit
  f794358e5c08bcafe57c6a3e1d74a11f8d478ebd).
* A new ``murphi2smv`` binary has been added that translates Murphi models or
  fragments to the input language for the NuSMV model checker (commit
  391cb95ad30026a0579fe04e1c8591413433ae2b).
* ``murphi2uclid`` supports translation of Murphi fragments in addition to full
  models (commit 2de59f8599bd64c3d1b7c673a6fde1a711187a32).
* Missing author and licence information from manual pages has been filled in
  (commit a037954531bdbd88072d2d779b95a4ca20b0cd3c).

Internal changes
~~~~~~~~~~~~~~~~
* New parsing functions are available for parsing a fragment of Murphi rather
  than a fully formed model (commits 782efd73320c232c484538e2ac3ec108e6f350e6,
  3f1b6e765794bdfdfa0403b94693b8c82a68c080,
  603a2e4bd4a8ef3a99907f408a04e361dbe9f3bb,
  628b3fb66c0825e0937369bdde912d5e922049b0).
* The build system sets rpath such that Rumur installed to a custom path should
  more reliably be able to find its librumur.so (commit
  d932fe53ef57afcdcc24bea53582e21f2d076a67).

v2024.05.07
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: the generated verifier no longer mixes ``__atomic_*`` and
  ``__sync_*`` operations on the same variable. Previously this could have
  caused incorrect results on some hardware platforms (commit
  e6e8572cb5422ba2c272c49760bc16e404fe53a0).
* Bug fix: the generated verifier no longer assumes that the size of a
  ``struct`` containing two machine words is exactly 8 or 16 bytes (commit
  ebbd5fdc5bd2c3c86b5786c039956b10c910793d).
* On ARM platforms supporting Large System Extensions (≥ armv8.1-a), the
  generated verifier no longer needs to be linked against libatomic. It instead
  uses lock-free operations, which should result in a performance improvement
  (commits 28eb088ce8fdd5c039c19d39a4ef6cd85d4ea70f,
  5afc797f7f8f0869e33e7c5c45846c8b70f66b59,
  adba81cde626901077a1c946dc57446660db47e3).

Internal changes
~~~~~~~~~~~~~~~~
* The ``final`` keyword has been removed from some member functions, allowing
  third-party child classes to inherit and override these functions (commit
  ed68536883b6ba27c37af12fdc2e7adcf8b7bf6b).

v2023.11.27
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Various UTF-8 equivalents of arithmetic operators (``÷``, ``−``, ``∕``,
  ``×``) are supported (commits d2aaea8993e469b933e2b790a4c3e077174afaee,
  a9de733aba646e1b0612269bbba6c8e21f485584,
  6071da6b6189b8f24a07960dfccb540c6693ce0f,
  5dac72950d59457aba636cb58487e54120338c7f,
  eba3529b10c35576c7f057be96eee86184443184,
  10acb27984e0580e30e0d995cf89d2b50fba10bb,
  9ebd4f24b13685ca97f05dc3e493063d3431e14e,
  cc345ea2b6beebe63951549b9d6616f533dd1d2e).
* Man pages use more precise dashes (commit
  5e7993681962346a02aac7bcb9de952da851b0bf).

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2023.05.21
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: headers that use ``SIZE_MAX`` now all ``#include <cstdint>`` (commit
  46e830d74473743598d1481aed8a845b8dc3b2f2).

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2022.08.20
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* The ``getrandom`` Linux system call is now allowed in ``--sandbox on`` mode
  (commit 1d3a61eb5fbff15af089a7f33ee90d0c98455124).

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2022.03.05
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* The ``statx`` Linux system call is now allowed in ``--sandbox on`` mode
  (commit 7d33181707dbeab8ec0ae2c949ac15215308f521).

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2021.12.27
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* The ``newfstatat`` syscall is now allowed in ``--sandbox on`` mode (commit
  53fab5e0cc431652c57439de3b15181cd07173ab).

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2021.09.29
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Nothing relevant.

Internal changes
~~~~~~~~~~~~~~~~
* Bug fix: build errors when compiling with shared libraries due to symbol
  visibility have been fixed (commit c73e8a28870d37a44bf16a1bd3701edfe82a2521)

v2021.08.28
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: code generation of loop overflow checks was incorrect in a number of
  cases. This has now been fixed (commits
  c1e46b1267bd52cbb445466b2fa1aa7ccaca9a26,
  a2a45efb689968de5440a1b419f070077f84d306,
  9c898780ea30652f4beeb1e5694c4077ea972ba8,
  27a9f27f35ca024ea0de1ae2038a4e66535043b0,
  d69c6b51209c3e78eee4808c15501b0cbc325e5f,
  2feee3e0382a561bdbdee1f0186de0db8e277e8e).
* A new experimental binary, ``murphi2uclid``, is provided for translating
  Murphi models to the `Uclid5 prover`_.
* Syntax errors now refer to operators (e.g. ``>>``) rather than their internal
  names (e.g. ``RSH``) (commit 7cf378f9711bbd19b4c4bdc648357b0043a57fc7).
* GCC 4.7 and GCC 4.8 are no longer supported. If using GCC, the minimum
  baseline is now GCC 4.9. An attempt will be made to not break compilation with
  GCC 4.7 and 4.8, but no guarantee is provided going forwards (commits
  201a7cab8d90b78696197c365d42d70998f16bd8,
  517c2151e7a1923d6e625e7256e5da78d063ad6a).
* The minimum required version of Python has been *decreased* from Python 3.6 to
  Python 3.4 (commits 25cc3303434d048066c75e1242e8ef556c09c9e4,
  0b68a69a702f1c7b518f425061c6657437e0c004,
  b3288d12bd5d67e2ed5c23d30f0412ca0bcdda8c,
  a27329276aa00c5e15ba9b1831c26c950412e68b,
  162355d42b9dda960624548b4c95c0b9f8008af8).
* The previously deprecated ``--smt-logic`` command line option has been removed
  (commit bf5b076bbba95c1423e9a7c5a2b64b8ced55fb73).

Internal changes
~~~~~~~~~~~~~~~~
* Librumur now includes API functions for sanitising rule names to be
  symbol-like. This can be useful for code generators that want to emit symbols
  corresponding to rule names (commit 5d57a5a13a6f0c93e31bbddc4f22a066edb40658).
* Copy constructors and assignment operators are now protected on AST node
  classes (commit 45b27dbac7082e2b2dee85ee4ff7bb2f0c54d063).
* Visibility of librumur symbols is now set via attributes. The ``RUMUR_API``
  macro may be used to override this if necessary (commits
  53344ec86e3a71725039de282d4307ca9af25fc0,
  c0cdc06024adb23f94409449266fe220f3094de9).
* A previously deprecated ``Model`` constructor has been removed (commit
  902e4389fca623a68c9c6fbfe298e5561a6aef04).

.. _`Uclid5 prover`: https://github.com/uclid-org/uclid

v2020.12.20
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: a number of issues in ``murphi2c`` translation of ``put`` statements
  have been fixed (commits 60c804a6cc5d04191788fc1756d1c9046cb09091,
  0df3eb85f3932eb91c92c43d94d02e11a5846629,
  4b7b5dfd3534aabd266fb3614b55d047526bae26,
  652b50b7fc9f811ed206aa50398a5656acae0eef).
* Bug fix: ``murphi2c`` no longer produces malformed code when referencing a
  record field within an alias statement where an alias has the same name as the
  field (commit 43f0dec2e112ff0adc05ab6267b8752e683d591d).
* Bug fix: ``murphi2c`` no longer produces malformed code when referencing a
  record field within a function with a var parameter with the same name as the
  field (commit 074946937005c908cb7ace5ef65f3779d12420d2).
* ``murphi2c`` makes a best effort to preserve comments during translation,
  emitting these as C comments.
* Translated ``put`` statements by ``murphi2c`` no longer print a trailing
  newline (commit 338fa7b38a2e3224b76c095b54ef6dd869a6dd32).

Internal changes
~~~~~~~~~~~~~~~~
* A new API ``parse_comments()`` has been introduced for extracting the source
  comments from a Murphi model.

v2020.09.06
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: when using XML output (``--output-format machine-readable``) error
  messages no longer have their last character truncated (commit
  6133e71b65c53cec050cdf5d40f735f2b9b3b525).
* Bug fix: ``clock_gettim64()`` is now allowed within a sandboxed checker,
  fixing sandboxing on armel, armhf, and mipsel (commit
  68683c4742b380421936a703c4b9262dac1e68dc).
* Bug fix: ``clock_gettime()`` is now allowed within a sandboxed checker, fixing
  sandboxing on mips64el (commit af121b2a9bb7f7dcdd63fbc2716b314c408abf2c).
* Declarations, functions/procedures, and rules no longer need to appear in this
  precise order within a model. E.g. a constant declaration can now appear in
  the middle of your model, after the definition of some functions. Expressions
  can still only refer to entities that have been defined prior in the source
  file (commits a77dc6d63e5f8bed9c39aa37209ec0b430aff67d,
  c99b0a604d45bedc9a1d8680912371d0a76783b6,
  d984fb9ad4bbedc8e99ded8aa3081bf4eba9a0ef,
  40114146920f6bef44b84532f5ff6ed1f24dd454,
  0a1dd7c6f476df1d7dcdd760722bff5343762609,
  501e02d288532c32c236875977e65b99bdb3ebb1,
  aa2a9a8774af651fe46410aee2405385c23c1a28,
  2d712b5838c638b6e90e0e0e34529d62b16319db,
  92ca08a13ba5c40fe459733d10ae1819fc9f0796,
  67a01344ad7a197887bc59ad3726847a2f2f530b).
* The Murphi AST XML format emitted by ``murphi2xml`` now allows declarations,
  functions, procedures, and rules as direct children of ``model``. This new
  alternative hierarchy is what ``murphi2xml`` now emits (commits
  d984fb9ad4bbedc8e99ded8aa3081bf4eba9a0ef,
  40114146920f6bef44b84532f5ff6ed1f24dd454).
* ``rumur`` gained a new command line flag, ``--pointer-bits``, for indicating
  how many low bits of a pointer on the target platform are meaningful. You can
  use this to get extra memory optimisations through pointer compression. See
  the manpage for more information (commit
  bc90b807687ac20af9fd025a46493832977ec9aa).
* On x86-64 Linux, ``rumur-run`` now auto-detects when your CPU does not support
  5-level paging and enables pointer compression optimisations (commit
  b34bf4f2843c60c916bdafb9a95ad901f2aad5de).
* In debug output during checking, the initial printing of state variables and
  their offsets now lists them in the order they appeared in the source file,
  regardless of whether they were rearranged for efficiency (commit
  a77dc6d63e5f8bed9c39aa37209ec0b430aff67d).
* There are some minor white space changes to the C code emitted by ``murphi2c``
  (commit 40114146920f6bef44b84532f5ff6ed1f24dd454).

Internal changes
~~~~~~~~~~~~~~~~
* The ``Model`` constructor, ``Model::Model`` that takes four arguments has been
  deprecated in favour of a new constructor that takes two arguments (commits
  501e02d288532c32c236875977e65b99bdb3ebb1,
  f375d67d929e789d22f9df882c23d774f4e60518).
* The AST node members of ``Model`` have been removed in favour of a new unified
  collection, ``Model::children`` (commits
  c99b0a604d45bedc9a1d8680912371d0a76783b6,
  0a1dd7c6f476df1d7dcdd760722bff5343762609).

v2020.07.28
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* The permutations applied when shuffling scalarsets are now tracked and later
  used to reconstruct symbolic scalarset values for counterexample traces and
  print statements. The effect is that counterexample traces now make more
  intuitive sense because symmetry reduction does not interfere with
  interpreting scalarset values. This behaviour is controllable via the
  ``--scalarset-schedules`` command line option. See the manpage for more
  information.

Internal changes
~~~~~~~~~~~~~~~~
* ``Symtab::is_global_scope()`` which was previously deprecated has now been
  removed (commit 7959973ce9345d16718a16b741d754c5e64bbc9e).

v2020.07.11
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: using ``&`` or ``|`` within a ``return`` statement would erroneously
  cause the error “cannot retrieve the type of an unresolved '&' expression.”
  This has now been corrected (commit 54c79e090a8bd5eb3939f15742e0c45d0c09187e).
* Bug fix: similar to the above, this error would also occur when using ``&`` or
  ``|`` within a right shift, ``>>``. This has now been corrected (commit
  65f4d0d85ab1a1de530c9751a8a4af4b2da4b6b5).
* Bug fix: similar to the above two items, this error would also occur when
  using ``&`` or ``|`` within range bounds. This has now been corrected (commit
  72d2ef5b7c12803af2d1102a11321cc19a77dd55).
* Bug fix: defining an alias within an ``aliasrule`` whose target was another
  alias previously defined in the same rule would result in generated code that
  would not compile. This has now been corrected (commit
  30408bde597f774330748309633e547f98041e0e).
* Bug fix: During verification, certain shift operations would erroneously
  return 0 on some platforms. These now return the correct value (commit
  e065dcdda6d5d263b95a101ab2e353aed9e49c9f).
* Printing an array within a model (using a ``put`` statement) results in more
  efficient generated code (commit b2edcd1ae8408da6c647b7fa7698c2d37c2b8b73).

Internal changes
~~~~~~~~~~~~~~~~
* ``Node::operator==`` which was previously deprecated has now been removed
  (commit df26837f4fea6a7da7fa24858ce3383367e33e82).

v2020.06.20
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: State variable offsets are now updated after reordering. Previously
  this could cause erroneous reads or writes during checking (commit
  3d8e551bf1c4873d570dc0a8adac7f52c0b9ea25).
* Bug fix: fields within records are now reordered universally. Previously
  inconsistencies could occur resulting in a record’s fields appearing in
  differing orders across references to the same type (commits
  cf03554574c7fd2fa78d461fbae95b97624b6f78,
  8b74668d28cdc73718c7e5b8234c9a138456d3ce).
* In light of bugs like the above two, there is a new command line argument,
  ``--reorder-fields`` to control whether field reordering is enabled. This can
  be used to turn it off in case further bugs are encountered. See the ``rumur``
  manpage for details (commit 9a33888f2303a3d1bf0e9339a2fddc4570945b02).
* ``rumur-run`` now preferences the ``rumur`` binary in the same directory as
  itself, ahead of any ``rumur`` binary in your ``$PATH`` (commit
  1f03555f89090e7de3e07dc5677380017a3762e9).

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2020.05.27
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: rumur-run’s check for whether libatomic is required is now more
  accurate. Previously this would incorrectly detect that libatomic was not
  required on some platforms (e.g. Linux ARM64), resulting in a link failure
  (commit 620e514c1d322e05a9e67bb09cd0dc68cb810d38).

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2020.05.18
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Various bitwise operators are now supported in the input syntax. See
  `doc/bitwise-operators.rst`_ for details.
* Some more verbose messages are now printed when passing ``--debug`` to rumur
  (commit 7f52532280054e32b1be72f44d0f4180d1a2dc86).
* Progress output lines during verification are now skipped when there is heavy
  contention on access to stdout. This reduces runtime bottlenecks in highly
  multithreaded verifications (commit 4d47d9a9abf8882935011d20950c50fe75460657).

Internal changes
~~~~~~~~~~~~~~~~
* The variant of ``parse()`` that accepted a stream pointer was previously
  deprecated and has now been removed. Clients should call the variant that
  accepts a reference instead (commit dcabb240eeb7d505f673879c2ba68fbbb5d3fd96).

.. _`doc/bitwise-operators.rst`: ./doc/bitwise-operators.rst

v2020.04.26
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: value type calculation (which C type to use for scalar values during
  checking) now correctly assesses ``:= ... to ...`` for ranges. Previously the
  presence of one of these expressions in the input model would pessimise the
  calculation into selecting ``int64_t`` even if a narrower type would be
  acceptable (commit 371fbc37047088c7f964dfdeedea2420cae46b1c).
* Record field ordering and model variable ordering is now optimised for runtime
  performance during checking (commit 2cb30e7c675d08837c26e0e204fa9f8457c40053).

Internal changes
~~~~~~~~~~~~~~~~
* ``IsUndefined`` now inherits from ``UnaryExpr`` (commit
  523a021e059382e6fa76afab7bfa011638332360).

v2020.04.05
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: murphi2c should no longer confuse multiple enum types within a model
  when generating its C output (commit
  34b66de87e17909538ff25e6c090791d1738f1f6).
* Bug fix: murphi2c now reports its name correctly in its ``--version`` output
  instead of referring to Rumur (commit
  8cf120cff76e1d58425be553b2a333c8c23482d9).
* A new binary, murphi2murphi, has been added that serves as a preprocessor or
  source-to-source translator for Murphi models. See its man page or ``--help``
  for more information.

Internal changes
~~~~~~~~~~~~~~~~
* A new API function, ``Expr::is_pure()`` has been added for determining whether
  an expression is side-effect free (commit
  499151975b8f6b25829e1bf2605943ab5e1832e0).
* ``TypeExpr::equatable_with()`` that was previously deprecated, has been
  removed. Clients should call ``TypeExpr::coerces_to()`` instead (commit
  f7fc46cb7de8ead4ea840d249ae7ff0689e35abe).

v2020.03.12
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: ``time()`` and ``gettimeofday()`` are now permitted when generating a
  sandboxed verifier on Linux. These were supposed to be allowed previously but
  there was a typo when initially adding this. This is only relevant for Linux
  platforms that do not implement these system calls in vDSO_ (commit
  6cce8fe23796e459bb98021ccc172ba139745f46).
* A new binary, murphi2c, has been added that translates a Murphi model into C
  code suitable for integration into a C/C++ simulator. See its man page or
  ``--help`` for more information.
* A minor typo was corrected in the murphi2xml man page (commit
  75dcef20a57ff939bf789bc98f6f2bd037fd1629).

Internal changes
~~~~~~~~~~~~~~~~
* ``VarDecl::state_variable`` that was previously deprecated, has been removed.
  Clients should call ``VarDecl::is_in_state()`` as a replacement (commit
  1776a4c6968e3c98861665af398bd042e435c096).
* XCode < 8.3.3 is no longer supported as a development environment under macOS.
  Users are recommended to upgrade to a newer version of XCode/macOS.

v2020.02.17
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: several latent bugs in the verifier’s state writing code have been
  fixed. These only affected large scalar types (> 49-bit) which were not known
  to be used in any existing real world models (commits
  2d27f8b97aa2d24caf217a97a6df7de11e70b1b4,
  7bbf8498c42ca8f19a059acc8169be2559b81427,
  fa87b0a361b1f7dd9fc436c063ffa5a1d4529ee6,
  5b4d7154902d8474f6d0233e5af9f3bd85b0a628,
  410fdbe533c3597bc2029f63e0426f56250c52bf).
* The ``rumur-ast-dump`` utility has been renamed to ``murphi2xml`` to more
  obviously indicate its purpose (commit
  d5cb6a6f88498e9d8c999540f66cc838ffe1707a).
* When generating a sandboxed verifier (``--sandbox on``), some further
  time-related system calls are now allowed. This allows the verifier to run
  correctly on platforms that do not have these system calls implemented in
  vDSO_ (commits 3ee7d3d3c2f4f35d86b59b6de7139feae8763b4c,
  498853681c25272e23cf480c6c8d7269f23a974c).
* The verifier’s state reading and writing functions now anticipate that the
  host platform may be big endian. Full big endian support will require further
  changes, but this is a first step (commit
  8f7bb60c1bc82638dd4ed5f2248c44cd47436461).

.. _vDSO: https://en.wikipedia.org/wiki/VDSO

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2020.01.27
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: quantified expressions no longer result in malformed SMT problems in
  the SMT simplification bridge. This previously prevented some optimisation
  that could have otherwise occurred (commit
  2a1b724d25817b1bf9f95932ed8a4f9bb65a2af9).
* Bug fix: pointer compression is no longer incorrectly enabled when targeting
  the x32 ABI on Linux. This would cause assertion failures or invalid memory
  references on this platform (commit 37cfa28ad640757eb42d4e394974ad2630987089).
* ``forall`` and ``exists`` expressions are now supported by the SMT bridge. The
  only remaining unsupported expressions are function calls and ``isundefined``
  (commits 49a0d0df8d5ea67b1c26b549929f6eea361b879e,
  5bb6144f684a905df44aa5955a8d04b37739e65c,
  5b4e5e52e4bba0fb7ea03cb63d210701c5f3bc65,
  5d4038c3933592b060203bda3e94b259a9ba9f43).
* ``rumur-run`` now automatically detects whether your C compiler supports the
  ``-mcx16`` flag and whether the checker needs to link against libatomic
  (commits 6547e8b5022522732421ff337ab5113a19afb44a,
  f7958a3fdad6a280360903108de5f05837fa1e5f).
* Some compiler warnings on Linux on ARMEL have been suppressed (commit
  b56cd94c6af0153dbdb983b8fd4177fc041526c8).

Internal changes
~~~~~~~~~~~~~~~~
* ``Model::assumption_count()`` which was previously deprecated has been removed
  (commit ce2fe9d30db11dbce337355924986af48ee8878d).
* ``Symtab::is_global_scope()`` has been deprecated and will be removed in a
  future release (commit 7943b55ab80e0ecf3563158a2ff7b8100d60ca78).

v2020.01.11
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: negative literals no longer cause malformed problems to be sent to
  the SMT bridge (commit 47f0207dcaee6909d59ddc5577f92b3bf97571b2).

Internal changes
~~~~~~~~~~~~~~~~
* Bug fix: a missing header is now shipped (commit
  8cf196c3548962b15488abe293b4891740da4da0).

v2020.01.07
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: compile errors in the generated verifier on Linux on ARM and RISC-V
  due to references to missing syscalls in the sandboxing code have been fixed
  (commit f1af745c54346f74ec650b192e708234de603b58).
* Bug fix: the syscalls ``fstat64()`` and ``mmap2()`` are allowed within the
  verifier’s sandbox on Linux on i386, removing a spurious runtime error (commit
  047f23b32e2510af15dd4021a3a63941a909d13f).
* The state data structure in the generated verifier is now more aggressively
  packed, leading to reduced memory usage during checking. The runtime speed of
  the checker may be slightly degraded. However, see the next item (commits
  c17f056efcb5d3ef0cbd2160df3762a29ee90530,
  db0e25f04d9140242643f7c5ebf8b8e9fbc62d82,
  3c8ba379e44085e772ada03c8607aac95be2ef30,
  ae6d776609de0462601f9beb75a8c93ce718f658,
  50cff5aef32fa02f096bb7fc161a93f10b829124,
  299be2fab2588b3367e8dd3406c8c9c0f591ebc6,
  46d495f31c202298aef9f9dcd6638295df3f3e88,
  c423db32f4c34db11d671d4e9078a4211a237630,
  c6a040344ef4415e1983bd67dec6bb146b020d5a,
  f6df17322a787d268c5ba8e587070649533b82c5,
  a30665fb0b71040c99a19201e37ff9946b77a628,
  77b97767661d17bff8b70d42b03ac63ba28c1da6,
  654156b1bde6cc8d9dd613053d20de70587827cc,
  77c8a12a6d6293de89670d0cbc6c4dc05c6ca9f3,
  1b3383e3d2064826f67d211890011d651bfae88d,
  cff8c6c938cf9b491f136dcb31072d1fe8dcc00c).
* ``rumur`` has a new command line option, ``--pack-state``, for controlling
  the trade off between memory reduction and runtime speed in the generated
  verifier. See the manpage for more information (commit
  aca06ba25db9a6a8e6311c8eaec015750371b772).
* ``rumur-run`` no longer uses the compiler flags ``-march=native`` and
  ``-mtune=native`` if they are not supported. This is primarily relevant to
  non-x86 platforms whose toolchains do not all have these options (commit
  1dd341e29dd7033b1d7598af8af899c322880a50).
* ``rumur-run`` passes toolchain flags to link against libatomic on
  architectures that do not have a double-word compare-exchange instruction
  (MIPS, PowerPC, s390, RISC-V). This causes queue operations that are lock-free
  on other architectures to take a global mutex, but it seems not easily
  avoidable (commit 4cd3ffef193e2a87d1dd58a642ebaf93541b70ab).
* ``rumur-run`` now uses `Link-Time Optimisation`_ if it is supported (commit
  0adcb633ec56b476505e22fa47126437f9665671).
* Various minor performance improvements were made to the generated verifier
  (commits 5af91bf0dfe0d8bef9f7045f5ae5692a179e9ca3,
  dee407613c0b1fd0c7ab851c6f84cbcb184dbea4,
  b517be6b83b5c17f97ab82bda448e62ecded9688,
  fe49bea9af67f71763227e95009441438433522a,
  fd04cb9c1b3f432cb35f66d6cfe0b0726ad84068).

Internal changes
~~~~~~~~~~~~~~~~
* ``validate_model()`` which was previously deprecated has been removed (commit
  ba3a70ce8902c9baecdc94505f7c71d7dba6dca3).
* ``Node::operator==`` and ``Node::operator!=`` have been deprecated and should
  no longer be called. There is not a consistent notion of AST node equality and
  these functions only implemented an approximation. They will be removed in a
  future release (commits 019dbe9c4b2fdf24f8cf16028e73e6105e3336fe,
  489947c7e3a01ae256d467565688eded2564f34e).
* New functions, ``Expr::is_literal_true()`` and ``Expr::is_literal_false()``
  have been introduced for determining if an expression is the literal value
  ``true`` or ``false``, respectively (commit
  bd084b982b6f209ec2356bb56f69dc0622b9345b).
* A new function, ``TypeExpr::is_boolean()`` has been introduced for determining
  if a type is the built-in ``Boolean`` (commit
  f4ad5d02161da0b6f2d5264b6a9db482c392e77e).
* Some documentation on the use of C atomic APIs has been added in
  doc/internals-atomics.rst (commits 85602619752cb8b173a0821bb7afe2a8c301f0e1,
  7fb1f0266beafd58e7bf7f859204b0ce61f35b28).
* Liveness is now documented as something beyond what is supported by CMurphi in
  doc/vs-cmurphi.rst (commit 5c82890e2a11ccb5da5e155faba8c7b9c26544d5).

.. _`Link-Time Optimisation`: https://en.wikipedia.org/wiki/Interprocedural_optimization#WPO_and_LTO

v2019.12.22
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: ``rumur-run`` no longer crashes during UTF-8 decoding in generated C
  code (commit 7bbd50f6a7241475826e8d380b6a60bb3c6dfd18).
* Support for Python 2 in ``rumur-run`` has been dropped. To use this script you
  will now need at least Python 3.6 (commits
  0c4d5f05ebcc937921edd924465827e50d345842,
  ded15a4d8f23f1f1584566bd6e251679ba8f915c).
* The final check of liveness properties now prints regular progress updates
  (commits ce162be56035e726e1077bb6b6ecc89999e8607e,
  2635dae9a4f27962f4ed951a54b3d6c54b9d62c6,
  44e80dc6142205904dca188d2a0277b49ed0fb7f,
  048a4b54fa7a1c2a7f48fdb8a7e470d396529200,
  eef60ad5cf61d1a8cac2d1dbcf63581da2590e24).

Internal changes
~~~~~~~~~~~~~~~~
* Support for Python 2 in all scripts has been dropped. These now require at
  least Python 3.6 (commits 5ad77dc6de53de9a78639faba5b65668e43c3ad8,
  729a7f8a096369115bde345890bc14e03c5bd428,
  6e0d248eae25a8a68b04bb5e99a3172e1e2ab453,
  244b41225d36309f9e5985dbe594957782bef7fb).

v2019.11.24
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* ``rumur`` has a new command line option ``--smt-bitvectors`` for controlling
  whether bitvectors are used in preference to integers when interacting with
  SMT solvers. See the man page for more information (commits
  37c84bbe255d3a7aa6d234a8334379edbb24ec3c,
  9821bedfa4cdadda8cf1b9f065c07813854ea7d1).
* ``rumur`` has a new command line option ``--smt-prelude`` for prepending text
  to problems sent to SMT solvers. The ``--smt-logic`` command line option is
  now deprecated and ``--smt-prelude`` should be used to set the logic instead.
  See the man page for more information (commit
  ad022eb0767250734562ec1ec932ef4d99ec1f5d).
* The ``rumur`` option ``--smt-simplification`` is now automatically enabled if
  you pass any of the other SMT related command line options (commit
  39482d62009232477f18c7e5e295c633004e7b82).
* A new tracing feature for memory usage in the generated checker has been
  added, ``--trace memory_usage``. See the man page for how to use this (commit
  4f9195707ae261ed4f6f94d1411579751deff618).
* ``rumur-ast-dump`` now has a ``--version`` option to print out its version
  (commit 76716edc76fbe608a013b0178b6e4d2d72614d08).
* Some warnings when compiling generated code with recent versions of Clang have
  been suppressed (commit 3e9efb2855be52c20023ef3cd03e02b183e22ff5).

Internal changes
~~~~~~~~~~~~~~~~
* A new ``version()`` function has been added to librumur for retrieving its
  version as a string (commits 77ee1c40884627e5418e3c25f902c6d7d73f5f4f,
  7f95b7491859548b27ec7d9226d7c28cdec380c0).

v2019.11.09
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: returning an expression of range type within a function with a return
  type of a differing range is now accepted. This pattern was previously
  rejected by ``rumur`` claiming the types were incompatible (commit
  2279e30e74983c8288d097979f31ffecd25b9b4f).
* Bug fix: the filename in the AST dump produced by ``rumur-ast-dump`` is now
  XML-escaped. Previously characters like ``<`` were incorrectly printed as-is
  (commit cec7f83ac781554a99e9018cef6a0285f67c8955).
* ``rumur-ast-dump`` now shows source content in its output even when the input
  model was supplied on stdin. Previously source content was only included if
  the input came from an on-disk file (commits
  ff36e8fec7750a921d4bdc57c509ca7d12fde8cb,
  6fbc34e9a6cbee0e8c9f09c9b8dc5796fd3d2aaa,
  8fc052d0c3d034ed057ec69aa3ebab95b60234b7).
* ``rumur-ast-dump`` now gives the filename in its output as “<stdin>” when the
  input model is supplied on stdin instead of omitting it. The ``filename``
  attribute of the head ``unit`` tag in the dump has now become mandatory
  (commit f20463f3e00f5ae2de9871b6b24f83f7799ff4d2).

Internal changes
~~~~~~~~~~~~~~~~
* ``rumur::parse()`` now takes its argument as a reference instead of a pointer.
  The old implementation remains for backwards compatibility but it is
  deprecated (commit 947ae70c647a955ea6e24b651a6feead64bac787).

v2019.10.27
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: several problems with code generation related to statements of the
  form ``for x := i to j by k ...`` have been fixed. Rumur now supports
  arbitrary expressions for any of ``i``, ``j``, and ``k``, including reverse
  (down-counting) loops (commits
  1186e622868c124b21637f7ddb5f35f818b18f3b,
  8b73384edfceb8c6f55dffdb1ae8d9952b5c8adb,
  245887647ac4bfbf08685f97c99c0c84b581e8f8,
  b7078e9b17fb572ff7126aa42930d3dd50a4577b,
  df4264e5f72d7e4528211e74444512d58dd32048).
* Bug fix: quantified variables are taken into account when calculating range
  limits for values of simple type (commits
  e4746dc130d3f69bf623bed503b88b0ba109b176,
  3e0ac51a379a2b5612b6d72e3e286955f143e525).
* Bug fix: overriding the automatically chosen value type (using
  ``--value-type ...``) can no longer cause an assertion failure in the
  generated checker. Forcing a value type that is too small previously violated
  an assumption in the generated code. This now causes a runtime error (commit
  77729447d3cfbb523e3a4a79654eb0a1b5fbd8e8).
* Bug fix: the initial pool size of the arena allocator in the generated code
  was being miscalculated and has now been corrected to approximate 8MB (commit
  381f08975e2a0a70cd0a2210a9af12b374580075).
* Bug fix: the SMT bridge now correctly detects a failure to start the child
  process. The check for this was previously incorrect and it would look as if
  the SMT solver malfunctioned (commit
  d1cbfd41d3051d548186acf1f17acd85df7f96d8).
* Blank (``""``) and unknown logics are now supported by the SMT bridge. Solvers
  such as Z3 function best when given no ``set-logic`` command (commit
  6c92a15f33da3804aaaba628ecc8450ac2fde13d).
* The default SMT logic is now ``AUFLIA`` (commit
  03ab27d04eccc18c142db7364f7000bf67c12a7f).
* Some GCC warnings when compiling generated code have been suppressed (commit
  bae9b849a781f97e690c8e52196512150aeae4ab).

Internal changes
~~~~~~~~~~~~~~~~
* Bug fix: Unresolved ``TypeExprIDs`` with differing names are now considered
  unequal (commit 7fe656c7db5f2578db826ea1a39a200ece93f57f).
* ``TypeExpr::equatable_with`` is deprecated, and replaced by
  ``TypeExpr::coerces_to`` (commits aa1557bf044e62c8f3adaaca591fe272b30ca19a,
  e45f214cd2097bbe710a2a3eed9ed196e9feace8,
  befe6bb4a9b9c342ad3a7a8b96a8bff94c47319d).
* ``Quantifier`` has a new member, ``decl``, that is a ``VarDecl`` for the
  variable it represents (commits c079a460749b1b8e7ea9dd627d369fe3395aa204,
  4aba73cb86885531a56228a145ad2529cf5fe2a0).
* Quantifier expressions — the bounds of the quantifier — are now validated in
  ``Quantifier::validate()`` (commit 1b7cd5aad63c8b3e55a266facb8100752946a59d).
* The type of a ``TypeExprID`` that refers to a quantified variable is now a
  persistent, valid ``VarDecl``. Previously it was a synthetic declaration with
  an invalid ``unique_id`` (commit c567645c4778cbb33d9f696450e9c9c13f12896b).

v2019.09.15
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: an alias of a constant is now correctly recognised as constant
  itself. This makes it possible to, for example, use such an alias as the lower
  or upper bound of an integer range (commit
  e4d139880498cfe140ae3298985c615d44f3930d).
* The SMT bridge supports variable and type shadowing. For example, if your
  model has a rule with a local variable with the same name as something in the
  global state. Such models would previously cause malformed SMT problems to be
  passed to the solver (commits
  b2d5c1566530fa009c06b1c2710617b71f7c8c57,
  4f5611986b12cbafa9663f1dd7b31f33d3211d25,
  7b1718259185ff3e5ceabbb34fca41028da12010).
* Smart quotes (“ and ”) can now be used as string delimiters in models (commit
  82db1716e7b18259b00ea1941163c4808513793c).
* Using an SMT logic without array support (for example, ``--smt-logic QF_LIA``)
  suppresses SMT simplification in models with arrays. Previously this would
  cause a malformed problem to be passed to the solver (commit
  1100fae5b5c629b2d3e1f7dc386906ae16d7bd5a).

Internal changes
~~~~~~~~~~~~~~~~
* Breaking change: ``TypeExprID::referent`` is now a ``TypeDecl`` instead of a
  ``TypeExpr``. The ``TypeExpr`` that would previously be stored here is
  available via ``referent->value`` (commit
  117ae412d6aa863f54d25fa87106265cced7f680).
* A new method ``Function::is_pure`` is available for determining whether a
  function is side effect free (commits
  455acdc883a7080ad764524a7d22e8bf056c9e09,
  ef5eb689d81bf96c183ad6f74a754eab47229095).

v2019.09.07
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* The SMT bridge now supports record types. This makes SMT simplification
  (``--smt-simplification on``) realistically usable on real world models.
  Simplification will still give up on some unsupported expressions (commits
  787f074328874a470d595576ae9e8b16837582f4,
  33d120df8fc7bedf1361a59f328930d311478376,
  308a8239eee6dc42684c3bed21210ea95d0dd66e,
  b9dd7f185d6f22c31d98dfbeb2af4418fb661b79,
  13092b8d8c5e62da0178b71825328cc7e75bea5b).
* Recursive functions and procedures are now supported. These are supported by
  CMurphi, Rumur's precursor, but seemingly rarely used in real world models so
  their absence in Rumur went unnoticed until recently. Mutual recursion is
  still unsupported (commits e61b8a787ab46bde3c0ce14da885cd3005cc54c9,
  a9bd211028e591d90e28e2410f5988700bc5efcd).
* ``rumur-ast-dump --help`` now shows its manpage instead of abbreviated help
  text (commits 4198edc67ed37c3dfa91031f90fdfb9e8a5190aa,
  8cf86df9ef718d1e22d1ba47a63c9f1a6ba1ad78,
  295b565f88660ecf4264ad1ace4e6f88423fab69,
  8c612b898e9d42a17847cca3a9435fc575c58135,
  577ae2862a45a1d89fe995c1a9bd7bb11fc7e34d,
  38a61d670d748d7072162e506c873afa13e757ec).
* Function or procedure parameters that shadow a return type are now supported.
  Previously Rumur would reject such models (commit
  ff5bbb8cd7a016fbe210757dd1c4b90093c44b4d). E.g.:

.. code-block:: murphi

  type t: 0 .. 1;

  function foo(t: boolean): t; begin
  ...

* It is now valid to name two rules identically in a model. This can lead to
  confusing counterexample traces, but sometimes it is natural to name multiple
  rules the same so supporting this seemed reasonable (commit
  a1d419c4d70f99d0945164e708ddd90379ddc858).

Internal changes
~~~~~~~~~~~~~~~~
* A new interface, ``Function::is_recursive()``, is available for querying
  whether a function calls itself (commit
  de4cd48cc2ff64b8ba8eb41163ea45fd1676658c).

v2019.08.18
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: Boolean literals (``false`` and ``true``) are now supported by the
  SMT bridge. These previously led to a malformed SMT problem (commit
  0c9917b87523db07b604c566e2f8e3481872857b).
* Array types are now supported by the SMT bridge. The bridge is still of
  limited use as there are many constructs it cannot handle, but it improves
  incrementally (commits 424467a264b923c53a1b1738604630a05457315c,
  5d4f1939ddc5d5d9336f0ce35e953c51e8b5aeca,
  5e07b5527a910d12be558d665110a7809838360c).
* The default logic for the SMT bridge has been changed to QF_ALIA. As before,
  this is controllable via the ``--smt-logic`` command line option (commit
  dc81631881a16764d55dea834ae39d8715b13e83).
* Some compiler warnings in the generated verifier have been suppressed (commits
  e60db38a76b2cd1ce169ad17b442b5285ee83b4c,
  ef5dd68576dc37d109e2370c653f1a6286042f78,
  a657bb19ae4ce589e64b217823b0e2c49b8b282e).

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2019.07.21
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: quantified ranges that span 0 (e.g. ``-1 .. 1``) now iterate
  correctly. Previously such loops would become no-ops which could cause the
  verifier to incorrectly not explore some states. This bug was introduced in
  v2019.04.28 (commit 2329056db14d87301bba9c56115cdd4539bed1af).
* Bug fix: models that contain assume statements but no top level assumptions no
  longer segfault. This bug was introduced in v2019.05.11 (commits
  eab626a859982d55b2ebfae8ca216ce79aec25ee,
  d4ae6d2c88cf0ca5a4e2a4f1f94b375d1405b2a5,
  ad79600751bb017ff8f85ef34e2747924c0e6eca,
  0fd8636f2eca1ed6d90545ab3ee91f4ebae1da85).
* Bug fix: the file descriptors used to communicate with the SMT bridge were
  being configured incorrectly. This caused inconsistent behaviour across
  different Libc implementations. This bug was introduced in v2019.06.30. Thanks
  to @wangqr for reporting this (commit
  53f20cc00398eefd81a7a1d015517d3051b23548).
* The dialect used to communicate with SMT solvers was backported from SMTLIB
  2.5 to SMTLIB 2.0. This enables support for more diverse solvers (commit
  e0e9c5d46c8c2192d6c70987de2a1d50889dc3fd).
* There is a new option for specifying the logic in which to encode SMT problems
  for the external solver, ``--smt-logic``. See the manpage for more information
  (commits e6b76b518439c0667de0b4b575ec18e5e6994705,
  6ba664c341f5796a99a7b4623f424ad4f33c9852,
  07ff7f7df1f4e8473f4e5f63dc0654009abb18db).
* The SMT bridge learned to understand type-declared ranges/scalarsets, integer
  constants and enum types. It is still of limited use
  because it does not understand records or arrays, but support for these will
  arrive in future (commits c38a0f1188924622e716abbc4dcee924cb10ce52,
  33ce2be1adf8c0922ea6fa7594ad9c783df35e20,
  7d0146ead2cf30b15ed515beb3c56dd1da8464a8,
  ca07c576bb272193c1177790c359b5984f636180).
* The SMT bridge has increased support for division when using CVC4 (commit
  e55c4c1b274dfd8797f71f49209d2e0e5eb799d7).
* Some inconsistency in the XML output when using
  ``--output-format machine-readable`` was corrected (commit
  22a0c59054563116f6210a886dd538bdfd7cd90a).
* Some ``-Wsign-compare`` warnings when compiling the verifier have been
  suppressed (commits d2949e3516c613f6183ce3219d403e4b3e96add9,
  1a7342956115a691118b315bf8ea1cb551f718f9).

Internal changes
~~~~~~~~~~~~~~~~
* ``Model::assumption_count()`` has been deprecated (commit
  99529844092fcbe1bbbfb3170c7b9a8364a6d055).
* ``VarDecl::state_variable`` has been deprecated (commits
  39bf6a2661bb6a296fbd73d9f466f052c4865477,
  175193b6e0a920f016545008796a99ec3a588bfa,
  6a4f9ac363b8c90beac7d5b5ddacc152f5e329d4).
* A RelaxNG schema is now included for the XML output of the verifier (commit
  123e2507ddf6694ddb7d2bb1baf654e467f28e23).
* The validation API has been extended and now also descends into referents. The
  function ``validate_model()`` has been deprecated (commits
  860f71d1db91e71bcab60a8fc8097ad37d3895a0,
  499857ec7ab25886be5c4a76802889cb1fc034f8,
  5d2449ac780c39cb72f21a03b498c766607fabb7,
  45f095c97174b96df5612d0c762283f7187ba0f7).
* The data members of referents (e.g. ``ExprID::value``) now have accurate
  values. This avoids confusion as users can now access these and rely on
  getting the same, e.g., ``offset`` as the target (commits
  7268f636cd9187c30f6bc990abef8e4b493b0534,
  c3d23559c40b1504bb1a284f76303891fafae23f).

v2019.06.30
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: duplicated semi-colons are now ignored. For example, an empty
  statement no longer causes a syntax error (commit
  7e0a3eeff15707e6a67515acd499dce9e598d9ee).
* Rumur gained some rudimentary ability to interact with an SMT solver. See the
  manpage or ``rumur --help`` for information on how to use this functionality.
  This simplification performed via SMT will incrementally improve in future
  releases. (commits 45f56b3d06759bd9a0e6343334b5fa2bf2161f2a,
  1c75eefb8c9c1b3e1e543cefd992b91066929081,
  0f8c1aa01f5ec517d4186ab8f65b81872dcc4374,
  9aa75f12adc38efd7a107c90f659ca4d98e8d925,
  dce3565a8d059e480efd34ff35c5d43134eed607,
  4a0b72a25318e642a4648dbcb1082068f7c20354,
  4bf443d4a1eb4f069998109f8f4e9380ad35ef6c,
  c66061ffa216e291a325e3a33cb55fd6d911960b,
  c32ed61d1b51439e760558712c5c3de5e8cc2a4c).

Internal changes
~~~~~~~~~~~~~~~~
* A new member of ``VarDecl`` has been added for determining whether a variable
  declaration is part of the global state or not (commit
  80e6154c748b3cbd36c3b9fb9e1164447e85246f).
* ``True`` and ``False`` constants are available to use for comparison or
  cloning when working with the librumur AST (commit
  dcb3559fbe03014bdf353649f390fc368b7e813c).

v2019.06.12
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: an unlikely edge case was possible wherein the results of checking
  could be reported inaccurately if one thread was exiting while other threads
  decided to expand the seen state set. This was never seen in the wild, but has
  been corrected in this release anyway (commit
  8cf9d785c925554e6ec4b2a8a55e619f3ecc66f2).
* The generated verifier no longer requires linking against libatomic on i386
  platforms. This change means FreeBSD on i386 is now supported (commit
  0da98254af604a4812201b8f06dc885dcebb9787).

Internal changes
~~~~~~~~~~~~~~~~
* Rumur now compiles correctly on platforms where ``size_t`` is not
  ``unsigned long``. Thanks to Yuri Victorovich for reporting this (commit
  38489a811f0abc4aaaf6f6425dd6321325f959a0).

v2019.06.05
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: when generating XML output from the verifier
  (``--output-format machine-readable``) some text within error messages was not
  correctly escaped, leading to invalid XML. This has now been corrected
  (commit ca97a1eb90ac667f3e5f32b41ccbb59940804516).
* Bug fix: FreeBSD compatibility which had been accidentally broken was
  restored. Thanks to Yuri Victorovich for reporting this (commit
  43054e83417e028c48b18739f6ac7916cfcbac47).

Internal changes
~~~~~~~~~~~~~~~~
* Bug fix: the test suite should now run successfully in a non-UTF-8 locale. As
  for the above entry, thanks to Yuri Victorovich for reporting this (commits
  a88c8d2faf2b003e2b65af26cc42b2bcdd82e819,
  a9e327cd43f94ea22129244f514261ea3880eedb).

v2019.06.01
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: the output message for a syntax error on a line containing a tab
  character previously indicated the wrong column offset with the underlining
  caret. This has now been corrected (commit
  323fda58984e1768b659298afddc5c022160c428).
* ``rumur-run`` now exits cleanly and cleans up temporary directories when you
  terminate it with Ctrl-C (commit 9acb49fd46d8eeddd59104d48621aa1a3c71cd34).
* The default load factor of the seen state set has been changed from 65% to
  75%. On most models, this decreases the runtime of the verifier. As before, it
  is still possible to change this value with the ``--set-expand-threshold``
  command line option (commit 8ac5bf762d744fc68d8e64918fc7af120b4fc3c7).

Internal changes
~~~~~~~~~~~~~~~~
* The documentation available under doc/ has been extended (commits
  63e0db1b8d67529e3f042e1b1ed7ffd65ca78cab,
  49e8c6a857ba8f9b46d3cf36bb702268d7e822da,
  f39447766ba43ccf2f218370d6a644024a3e1215,
  ba0521cfcd2b30d19a125b319ade63775505c73f).

v2019.05.11
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: Counterexample traces using "diff" mode (the default) now correctly
  only show the value of a variable if it has changed compared to the previous
  state. Previously variables whose values did not change were sometimes
  repeated (commit 94ef1dec8a82d643dba459d97af3870c9e325528).
* Bug fix: Running with counterexample traces disabled
  (``--counterexample-trace off``) is repaired. Previously this would result in
  generated code that did not compile (commits
  f78335f5d72c3fa5b4565103697c678ef62379cf,
  58b7ac310caa008d57af71039080095c801956a2).
* Bug fix: negative literals are taken into account when determining a type to
  represent scalars. Previously Rumur would fail to notice that something like
  ``-1`` in your model implied that values could be negative, and it might have
  inferred that an unsigned type like ``uint8_t`` was suitable to store this in
  (commit 2b27e22f00354080589815416b7796d06b37fb6c).
* Bug fix: Using ``--max-errors`` with a value greater than ``1`` produces safe
  code. Previously this would emit a call to ``sigsetjmp`` with live
  non-volatile local variables. The result could lead to memory corruption or
  an inaccurate fired rules count, but neither of these were observed in the
  wild (commit 7dda120345da13f739427915fde630d71bae9ff5).
* Bug fix: some spurious ``-Wtype-limits`` and ``-Wtautological-compare``
  warnings when building the generated verifier have been suppressed (commit
  d82f251210560df694f03a6d8b6c5c2cbbe04886).
* The concept of disabled properties has been removed. This feature was never
  documented and had no use yet, so its removal is unlikely to affect any users
  (commit 4e30098aee291414b5108936548218657fb47900).

Internal changes
~~~~~~~~~~~~~~~~
* Some spurious ``-Wsign-compare`` warnings when using older GCC versions have
  been suppressed (commit 25847dca93e45a3b0616c9f2bd254eae1738f7a1).
* The documentation available under doc/ has been extended (commits
  5a56d259bf2b9e039ed18a4b48861b48083e730e,
  7ab3e74ae2a63809ee657ea981cb2d9ae0da3fb4,
  b6e8ed7c4c4818aa13d7ec24cc3f7fb40f1d9842,
  d76467f065585a2cbc5f4f237ea20fb367140c26)

v2019.04.28
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: enum types that are printed in error messages now correctly have
  their members separated by a comma and a space (commit
  1107d95909bdd9df019f55f1208c857de5db7239).
* Bug fix: one case where the size of the seen set was incorrectly read
  non-atomically has been fixed. This would only have affected platforms where
  naturally aligned reads are not already atomic (e.g. not x86). The result
  would have been a rare chance of a miscalculation of when to expand the seen
  set. (commit 02d2803ecb6a459a1a41f7d1c630d1b84d6d75ff).
* Syntax error messages now provide more information about what token the lexer
  was expecting to see (commit 06dfee962cb3541fcedf2f319ca4504f90ee0514).
* Instead of unconditionally using ``int64_t`` to represent scalar values in the
  generated verifier, the fastest type that can contain all scalar values in
  your model is used. You can override automatic selection with the new
  ``--value-type`` command line argument. This change has no immediate benefit
  but it opens the way to optimisations using Single Instruction Multiple Data
  (SIMD) or even SIMD Within A Register (SWAR). (commits
  0a5129fb89358ea67ecc32fb07b1d768f655223e,
  0933edbb4831c5fc9e483e865b202a6609090b54,
  f5c8cc54a8a02338a62985aaf2190d7f5fc79ca0,
  2fde1dbf0fff5c3776fb77e7468a2e83693a444b,
  6d20e571685f18cdb2d9bf6dd77c615ce1ab5385,
  e98a3d0041d64dd331a16e45897e9c3a789e0235,
  f9a29ea64cccbc41155b689d80ea6eb3be9189e9,
  c95df7007b48a89df981eec037679dd3cb87dab5,
  5b33f977a55a4bd370aefea205548b1b0bf887d9,
  5e5945535ff60ed01501d2b10282220b96b009bc,
  5e1ee6dbe6e784516a1171996bb442e9936e426b,
  79579fd5ee7cc3c120439b5d3187a09ffd5dcd6e).

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2019.04.13
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: malicious models can no longer cause bad printf format strings to
  be generated (commits 6b30d43f6672278db0c0d7c8dfd5dbe83785fad5,
  a27c2391ede24c0833b045d0d4a138ecb829434b,
  322d1e26b343cdc308efb50ce2d952bb26ad9ad8).
* Bug fix: characters in text like rule names are no longer dropped when using
  XML output (commit f119f745218ed9404f6922e95aa6076bc0bdf291).
* Bug fix: unnamed invariants are now correctly numbered when they are referred
  to (commit 450a2e7b9ced7f670eaf568e9ba484ea43a2dcb4).
* Bug fix: with deadlock detection enabled (default) and multiple errors
  (``--max-errors ...`` with a value > 1), deadlock counterexamples are no
  longer duplicated in the error output (commit
  17ebb307b68cb323ad0840903b96070ea1b6ca0a).
* New syntax has been added for writing liveness properties. See
  doc/properties.rst for how to use this (commits
  e99fa1104ff578106075f6dc19c35b4ef2f7d986,
  ee1aecd172edb9fa5be775548841e38c4aa547b0,
  36fae15066562eedee594fa1fd77e60af19e13bd,
  4c6ee24bc922955f419c05391fa1ddc49cbc122e,
  53f80d8565af4217bfe11ac2bfe549d9b2ada0af,
  b094269cfe516bad7bd3ab0993288ff7f3a8285a,
  6ed296f61b7b942323974a7d40c2b20f7003ff26,
  ac54ed1cef5326260128d189a3705679a3ba02aa,
  85cbc94ac9b734572874d3564d9a4240f10614f9).
* Support for macOS has been extended back to XCode 7.3 (commit
  35e1803b370f8a47df84812eab19bbb01dcf4e41).

Internal changes
~~~~~~~~~~~~~~~~
* The test case tweak snippets (``-- ...`` Python comments at the beginning of
  test cases) can now refer to whether XML output is in use or not (commit
  af393a106773c98b79f283f02e250ec9ca9a73a5).
* Using the ``-- checker_output: ...`` test case tweak no longer limits a test
  case to running when XML output is not in use (commit
  af393a106773c98b79f283f02e250ec9ca9a73a5).
* There is a new API function for counting the liveness properties in a model
  (commit ee1aecd172edb9fa5be775548841e38c4aa547b0).
* The build dependency on ``xxd`` (bundled with Vim) has been removed (commits
  a8575179f9a5c956be5bb50c182bbb89f1d8d057,
  6b907684c4d7696acf6f9ea2a2ca566e5175da18,
  43759055bf873814ec18cb692ee9a6d9d6889d1a).

v2019.03.30
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: an error when compiling the generated verifier on non-x86-64 was
  addressed (commit 7e59f1c25a71fd6c3444fc11adc6f932b32ce926).
* Bug fix: the Vim syntax extensions were missing the ``property`` keyword which
  has now been added (commit 9e70f6114899ca04556c3cdeb198928a65ab19fc).
* Errors when generating the verifier are now printed showing the relevant
  source line from the model. They are also colourised Clang-style when possible
  (commit e7f2b615cb432bf8fab55d3a00225f3b26e8d8d9).
* Support for sandboxing the generated verifier has been extended from Linux and
  macOS to include FreeBSD (using Capsicum) and OpenBSD (using ``pledge``)
  (commits b73b180dd7fedb2795f19e8a065eefe429f1177e,
  cb53074aaa1c898c6c0a3d6e962597b9c77c3785).
* Expansion of the set of seen states has been optimised, resulting in a ~4%
  decrease in the runtime of the verifier. This change reduced contention, so
  likely leads to greater speed ups on large, multicore platforms (commits
  022c3708b24b828a96f3a50c0f11c7cc1476a439,
  5f4bb2cd96660a48518680f992fee041566ac722,
  2e84387ec6f56c42f41ea21e17ba99eef501ab65,
  5b29f2c4cb96989ba862a19acfcae0912a19f86c,
  9287f5af063a430e83c8957d9f7282d1af33d6ba).

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2019.03.21
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* A new bounded model checking mode is available using the command line option
  ``--bound``. See the manpage or ``--help`` for more detailed information
  (commit e60697531ab636d374946d547ae65cd380b2ce0b).
* The names of quantifier variables are now included in the XML produced by
  ``rumur-ast-dump`` (commit 78539fa086bbdaf06c5a079e5e482637cf6f2e11).
* Some optimisation has been done to state handles, resulting in a ~9% decrease
  in the runtime of the generated verifier (commits
  d783655eae837b805b69185d1d198ea142825973,
  96268246ad3c9635998647fb31faf73e6721c83b).
* Support for GCC on Linux has been extended from 4.8 back to GCC 4.7. It is
  unlikely Rumur will ever support a lesser GCC version than this (commit
  76a97b5354cc10cbd5fd188c385eeb457b3fd2ab).
* All major BSD flavours (DragonFly, FreeBSD, NetBSD, OpenBSD) are supported.
  Rumur now runs on all major desktop operating systems except Windows (commits
  6524f1eaedc6724fb26462ec901c241ded7861e1,
  026c9a476ba5efea5dd4fd7a5a8bcec7588381e8,
  7e9addb34df01abe7449823c33772985e9f6172b).

Internal changes
~~~~~~~~~~~~~~~~
* Bug fix: a memory leak on passing invalid command line options has been
  removed. This is under "Internal changes" because the leak occurred
  immediately prior to program exit, so would only have affected users debugging
  or embedding Rumur (commit 4f89903e244c7c188577d082c204bdb344ed1af8).
* New options for scoping the range of tests that the test suite runs. This is
  mainly for use by the continuous integration setup (commit
  ba2377a3b7240774d6bfb6745bb3c424c67b9277).

v2019.03.11
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: enums and booleans that were used as ruleset parameters would
  previously have their values printed numerically in counterexample traces. For
  example, ``false`` would be printed as ``0``. Both are now printed as their
  textual names (commit 40c281d80342e684401425769e8e91ec78e3b019).
* Support for "cover" properties has been introduced. These are described in
  doc/properties.rst (commits 22a865897d23e2281541fe43276277b4b980a14d,
  29ac671ca93a0eef79b4f2b85a43da624d10938f,
  f9fe9614a4beb930f54db50250e4004ad773cee5,
  b4c5ead18eb3d99d2434aad6732cfce305c629c2).
* State allocation has been optimised, resulting in a speed up of ~46% and
  peak memory usage reduction of ~9% in the generated verifier (commit
  7ddf00bbce10a5f0cdd994658ac4545b186826ac).
* When using GCC, the minimum required version has been reduced from 4.9 to 4.8
  (commits c84bad26079f49a40b4c9cbdcd50b508292a8689,
  657eea8b8b84d269916207268edab85d71aba532,
  ff5a32521e4f937bd4d81b3ac7ae7204c8f913ec,
  227f340a059ce704ac1dff9cff75d721b987e147,
  7ba30edd5657c94fe5fe8c559fbde179817c795b,
  554d37e47cc9f878f65161d3ae51f6fbb9345bd8,
  3c827ae7b0f20d3f3f10118f61adcf73e58ee701,
  e929000525239eb357ad780c95aa54008633c678,
  a1ece0ad453ef95decd6256dac69b2af99ced2ff,
  b18e0430c8cd1cb5f67827e8ca2a6b0ab4117147,
  4e04bb5a6333df60444710f949486ea34739acc0).
* A Vim extension is included in misc/murphi.vim to add support for syntax
  highlighting Rumur's Murphi extensions (commit
  6dbcd208025a4a07b94d818110613a69efc05e4a).

Internal changes
~~~~~~~~~~~~~~~~
* Bug fix: the test suite no longer attempts to output decoded UTF-8 data on
  stdout/stderr (commit 551d18398189cb11ba6274d708d3ff293af034c7).

v2019.03.02
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: enum types with duplicate members are now rejected. Previously,
  members would silently shadow earlier duplicate members (commit
  b476ffbdb7f5afb245c933a89d8f3cf9ecc8a884).
* Bug fix: models that redeclare symbols are now rejected. Previously,
  definitions would silently shadow earlier same-named definitions (commit
  96b8acab16310f4e80008b92827f804ba6e3ae66).
* The generated verifier produces more context information in error messages
  (commits 45a63a9f26f531587d0c461da74467e2cc008c38,
  7238dcacbf676c2649cfe82c98df25dbe96af93d,
  9384c756477cbf164ea7f41227b053fca4c67fc5,
  063e92bd53a5dbbb642e1d5c302a9240afff5fbc,
  668c1d6ab02e9c55cfd8119e5a403c5595cd5b45,
  39d35f4344633c2e1280fc0d5b28e2356140229b,
  434fbf2f50d69b7824a224280bd5f7f3bcc2275d,
  6822bba8a280b70d53d6dbb470f631143df0b5c4).
* The implementation of the queue of pending states has been further optimised,
  resulting in a ~25% reduction in the runtime of the generated verifier
  (commits 8f0329c33343cfcf16675a110ed3211b9abc95e3,
  2153f1f9e0ac7e2d015aff58cd0d8007901de808).
* The warning emitted by Rumur when your model is missing a start state is now
  suppressed when you pass ``--quiet`` (commit
  55514d39e40b2c018379e15d2f706e0a1c56ed18).

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2019.02.14
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: calls of procedures (a.k.a. functions with no return type) are now
  rejected when appearing within an expression (commit
  72d9196308a8b0d3b43929566beb571029b7e006).
* Bug fix: unary negation that never worked correctly has been repaired (commit
  48228f32c43423cd956f988fb0567fca080b9b28).
* Between v2019.02.01 and v2019.02.04, there was an unintended performance
  regression in the runtime of the generated verifier (commit
  f5589751de2f860c3cca7d681f9710160d3c20a8). This has been addressed and the
  verifier runs faster than even v2019.02.01 (commit
  ccf410672326e04230331576a1c76003ad2ab1a3).
* Returning a range-typed expression within a function that returns a
  *different* range type is now supported (commit
  e196ed43199d6d47d36eb9f225017c2123e294c3).

Internal changes
~~~~~~~~~~~~~~~~
* ``Expr::type()`` returns a smart pointer that is never null (commits
  d89de1376abe5bbbef61d68b02c45a35c4f9a12f,
  beeffb42ad6514448e463e8a2d73d3a1d8b35898,
  e196ed43199d6d47d36eb9f225017c2123e294c3,
  5dcf10f2821ffb8a2080b297fc664485884747be).

v2019.02.04
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: using a non-scalar (record or array) result of a function call as an
  input parameter to another function or procedure would previously cause an
  assertion failure during code generation. This has been addressed and correct
  code is now generated (commit 73dcbf237f747d8958528127f6a05442bd3bf2c0).
* Bug fix: the convenience wrapper ``rumur-run`` now correctly exits when one of
  its steps fails and also returns the correct exit status (commits
  9eae5c5a22a87507713a2ebc5b57120de00e6f10,
  46cc017ee8c6337453601c245e6e764254687f48,
  235fbc552addefc1f34e8840a9d80845b423d30e,
  80825dfb406eb6f39aaa01c9011eadd7b6ad9b05).
* Bug fix: column offset information in the XML produced by ``rumur-ast-dump``
  was sometimes off by one. This is now corrected (commit
  7d8dc868d9e1c31243b15e3de116e4f0740a38b3).
* GCC 4.9 is now supported. Previously the oldest version of GCC we supported
  was 5 (commits 83ce80ad8bba3f48d4316dba29b4795c13facd03,
  0ed86df81586b5808be82c924ad964b25cb38447).
* The error message when a model assertion fails has been made more informative
  (commit 608fe69abfd7aa7ab724a42b1327bb055f7fb3ac).

Internal changes
~~~~~~~~~~~~~~~~
* Nothing relevant.

v2019.02.01
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* The values of ruleset parameters are reported in counter-example traces
  (commits 37f742797d8c76523607f90e80a5d1cc0ff16226,
  f7a8b012bfce555f156d1682cfd1073e8ccfe462,
  ee2d85200708cc70c2df056409d3da1283da2218).
* The name of a failing invariant is given in the failure message (commit
  60e864ccd8abefd617f21af4e1a78c53d1a3a66e).
* Comparison of complex types using ``=`` or ``!=`` is supported in models
  (commits 107f6c4ac88ce4e2c6745507aa332aa17dfd3264,
  bbd3beebb6ce0a51475a241eff45d7c2a223bcbb).
* ``rumur-run`` passes ``-march=native -mtune=native`` to the C compiler (commit
  ad9e26bfafb1cdf3877f46dd31b4072e1efffb5d).
* Rulesets with non-constant parameters are rejected (commit
  90810e214e7fa200d683f4ee4b79ef489d9e3d34).

Internal changes
~~~~~~~~~~~~~~~~
* Various new interfaces were added to types and quantifiers (commits
  6ea740ec2f6518733a626805af6b0f7275fc9b86,
  41e01629c30293dc91dd460d0286b74763eba387,
  aea30d24234777a0b0698c1ce6f28f8267b15d9f,
  154885bac4950b70c80620566e37d5a2890d317e).

v2019.01.12
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: an issue that could have led to the corruption of reference-counted
  pointers in the checker was addressed (commit
  04fede03a59624f3c08ee7b80d8f928dfc1e45be).
* The licence has been changed from the vague "public domain" to the Unlicense.
  This is just a clarification and does not indicate a change in the licensing
  intent (commit 592e0c62ff9b1b7bf1bada4e41fa058d2d669ab8).
* All Python components now work with Python 2 in addition to Python 3 (commits
  f04b1442af0b30581b17fc517aeecce99bd8f1ef,
  de4fcd64ed20b128e7dceb44dd57b757e15096c5).
* ``rumur-run`` and ``rumur-ast-dump`` now have accompanying manpages (commits
  fe484a28ac3f77766b7de30569c85350b499ffbd,
  3c2ba659f36e6b4cbedb8fd35b7f5c0f0af3be65).
* A Zsh completion script was added (commit
  aac9e7718f3849b66932e375d673ea6b80547ff8).
* Missing documentation for the ``--output`` option was added (commit
  3047fb45f4a1aee9c5064ee9bb260df25bf72c8e).

Internal changes
~~~~~~~~~~~~~~~~
* A RelaxNG schema was added for the format produced by the AST dumper (commit
  36d26f6c327dbbd541537ad12d07636aba55f502).
* Rumur should now be compilable with ``-pedantic`` in most environments
  (commits b4ef8c0e8bcc1af2a1afd00204e2df735928488f,
  526afa1fb9e00bb159caf8ce49f83e40c571f747).

v2018.12.20
-----------

User-facing changes
~~~~~~~~~~~~~~~~~~~
* Bug fix: boolean constants are now usable in boolean expressions, rather than
  being considered ranges (commits 3f8e25eed1b2cd88b04aec973b84efea3737f16b,
  6ee751955a0781becae7dcc0e34a7477e668e462).
* Bug fix: indexing a non-array expression is now reported as an
  error, rather than causing an assertion failure (commits
  606657b7fc656fd4c304523b98c5e2828a896271,
  a31c9973f63a719b676be97e7a893dd21d451511,
  5222f6ddce51ea66ceda6ecb0e016a94308e835b).
* Bug fix: calling a function with incorrect arguments is now reported as an
  error, rather than causing an assertion failure or uncaught exception (commits
  705793e6b0f3646d30dcab247d27cdd3ac94430c,
  2427b74c4d6fb40115943dc01bbd66cc4ada7d17,
  fe9344f5b723608cd8916bd16c2688f9494ca92a).
* Bug fix: trying to access the field of a non-record expression is now reported
  as an error, rather than causing an assertion failure or uncaught exception
  (commits f72373b30e8031baa8c8e0e953c05e47874ae854,
  76d09b6bf77414b51af2bf1da0ecd099c25ad2e1,
  27b61a5f6b0be2e838a39c02e567c87b4ce80d76,
  b917ece31a209ba9586c7c44577ba34b19a2c0a7).
* Bug fix: The boolean literals ``true`` and ``false`` are now accepted in all
  possible casings (commits 121d724c00e2afc1d1fa6c525dad958646936fb1,
  68e9164ae8a5a17c6e6346266051b24780bbf203).
* The ``isundefined`` operator is now implemented (commits
  d12841246e207a5691159f8ed46faf08cb596dd5,
  8e3563a0309d57dc19dbd7f0d1c50a8f30878559).
* Range-typed expressions can now be passed in to functions as non-var
  parameters of a differing range type, where previously this scenario would
  only accept rvalues or identical range types (commits
  343e97eeeb8ccd4c59bf150c42c0b74f1b00ec6a,
  09cfec88a1e648eaa240404c2b215ed4cefec926,
  2324e3efc370a09a289a4998c677cf1bfb31a245,
  90a95c31d5c04a6083f753bc15f566658abcdf9d).
* If the generated verifier is multithreaded, it now prints a thread identifier
  in each progress line (commit b222b3bc5fad2ff6e8371d3b46ad28809daa2451).
* Some spurious compiler warnings when building the generated verifier have been
  suppressed (commits 8a05ab0d209c0b8cbfa7048d5775505c1f70f283,
  4f447fdcc44f694f8bc1d948bbc17d690ca3d59f,
  7885b611ef9d9e6d18629b1eb696def0183eed16).

Internal changes
~~~~~~~~~~~~~~~~
* The use of ``static_assert`` has been replaced with ``_Static_assert`` (commit
  ad26fe525f7ba99dfbf3d5c6bc248ef41602d9a5).
* ``Expr`` and ``Decl`` gained a new ``is_readonly`` method (commit
  47c27f217b035fa9881fe32576354c08669b0899) and the distinction between the
  concepts of "lvalue" and "writable" is now more accurate.
* The test suite has been backported so it also runs on Python 2 in addition to
  Python 3 (commit 7fe028271d376188d8b5d6353e0bca720d12e6b9).

v2018.12.08
-----------
* Hello world!
