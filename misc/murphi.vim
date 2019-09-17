" Vim support for Rumur extensions to the Murphi syntax
" Language: murphi
" Maintainer: Matthew Fernandez <matthew.fernandez@gmail.com>
" Lincese: The Unlicense

" Vim already knows how to highlight Murphi files, but doesn't know about the
" extra things that Rumur supports. To use this file, copy it into
" ~/.vim/after/syntax/. The Rumur extensions to Murphi should now be correctly
" highlighted when editing models.

" extra keywords that Rumur supports
syntax case ignore
syn keyword murphiKeyword assume
syn keyword murphiKeyword cover
syn keyword murphiKeyword liveness
syntax case match

" support for hex numbers in addition to decimal
syn match murphiNumber "\<\(0[xX]\x\+\|\d\+\)\>"

" override the base syntax's highlighting of `==` as an error
syn match murphiOperator "==[^>]"he=e-1

" UTF-8 operators that Rumur recognises
syn match murphiOperator "[∀∃≔≥→≤≠⇒¬∧∨]"

" recognise escape sequences in strings
syn region murphiString start=+"\|“+ skip=+\\\\\|\\"\|\\”+ end=+"\|”+
