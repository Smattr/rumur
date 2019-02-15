Change log
==========

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
