;;; murphi-mode.el

;; major mode for editing Murphi source files

;; Author: Matthew Fernandez <matthew.fernandez@gmail.com>
;; Homepage: https://github.com/smattr/rumur

(setq murphi-highlights
  '(
    ("--.*\n" . font-lock-comment-delimiter-face)
    ; FIXME: how do we match multiline comments?
    ("\\_<\\(alias\\|assert\\|begin\\|by\\|case\\|clear\\|const\\|do\\|else\\|elsif\\|end\\|endalias\\|endexists\\|endfor\\|endforall\\|endfunction\\|endif\\|endprocedure\\|endrecord\\|endrule\\|endruleset\\|endstartstate\\|endswitch\\|endwhile\\|error\\|exists\\|for\\|forall\\|if\\|in\\|isundefined\\|of\\|put\\|return\\|switch\\|then\\|to\\|type\\|undefine\\|var\\|while\\)\\_>" . font-lock-keyword-face)
    ("\\_<\\(false\\|true\\)\\_>" . font-lock-constant-face)
    ("\\_<-*[0-9]+\\_>" . font-lock-constant-face)
    ("\\_<\\(function\\|procedure\\|invariant\\|rule\\|ruleset\\|startstate\\)\\_>" . font-lock-function-name-face)
    ; FIXME: why does the following take precedence over a -- comment?
    ; TODO: support for \" as an escape within strings
    ("\"[^\"]*\"" . font-lock-string-face)
    ("\\_<\\(array\\|boolean\\|enum\\|record\\|scalarset\\|union\\)\\_>" . font-lock-type-face)
   ))

(define-derived-mode murphi-mode fundamental-mode "murphi"
  "major mode for editing Murphi source code"
  (setq font-lock-defaults '(murphi-highlights))
  (setq font-lock-keywords-case-fold-search t)
  )
