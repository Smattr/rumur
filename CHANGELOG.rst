Change log
==========

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
