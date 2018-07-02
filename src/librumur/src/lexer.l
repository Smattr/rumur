%{

#include <rumur/except.h>
#include <rumur/scanner.h>

/* Override the declaration of yylex with the one from our derived scanner. */
#ifdef YY_DECL
  #undef YY_DECL
#endif
#define YY_DECL int rumur::scanner::yylex(\
  rumur::parser::semantic_type *const lval, rumur::parser::location_type *loc)

%}

  /* Track line numbers while scanning. These are retrievable via
   * FlexLexer::lineno().
   */
%option yylineno

  /* When the end of the input is encountered, do not expect another file. */
%option noyywrap

  /* Generate a C++ scanner, not C. */
%option c++

  /* Use our own scanner class, rather than the default yyFlexLexer. */
%option yyclass="rumur::scanner"

  /* Switch to case-insensitive. */
%option caseless

  /* States for handling comments. This is not the most efficient way of
   * dealing with these, but meh.
   */
%x SC_COMMENT SC_MULTILINE_COMMENT

%%

(0x[[:xdigit:]]+|[[:digit:]]+) {
  lval->build<std::string>(YYText());
  return rumur::parser::token::NUMBER;
}

array         { return rumur::parser::token::ARRAY;         }
assert        { return rumur::parser::token::ASSERT;        }
begin         { return rumur::parser::token::BEGIN_TOK;     }
by            { return rumur::parser::token::BY;            }
const         { return rumur::parser::token::CONST;         }
do            { return rumur::parser::token::DO;            }
else          { return rumur::parser::token::ELSE;          }
elsif         { return rumur::parser::token::ELSIF;         }
end           { return rumur::parser::token::END;           }
endexists     { return rumur::parser::token::ENDEXISTS;     }
endfor        { return rumur::parser::token::ENDFOR;        }
endforall     { return rumur::parser::token::ENDFORALL;     }
endif         { return rumur::parser::token::ENDIF;         }
endrecord     { return rumur::parser::token::ENDRECORD;     }
endrule       { return rumur::parser::token::ENDRULE;       }
endruleset    { return rumur::parser::token::ENDRULESET;    }
endstartstate { return rumur::parser::token::ENDSTARTSTATE; }
enum          { return rumur::parser::token::ENUM;          }
error         { return rumur::parser::token::ERROR;         }
exists        { return rumur::parser::token::EXISTS;        }
for           { return rumur::parser::token::FOR;           }
forall        { return rumur::parser::token::FORALL;        }
if            { return rumur::parser::token::IF;            }
invariant     { return rumur::parser::token::INVARIANT;     }
of            { return rumur::parser::token::OF;            }
record        { return rumur::parser::token::RECORD;        }
rule          { return rumur::parser::token::RULE;          }
ruleset       { return rumur::parser::token::RULESET;       }
scalarset     { return rumur::parser::token::SCALARSET;     }
startstate    { return rumur::parser::token::STARTSTATE;    }
then          { return rumur::parser::token::THEN;          }
to            { return rumur::parser::token::TO;            }
type          { return rumur::parser::token::TYPE;          }
undefine      { return rumur::parser::token::UNDEFINE;      }
var           { return rumur::parser::token::VAR;           }

"∀"           { return rumur::parser::token::FORALL;        }
"∃"           { return rumur::parser::token::EXISTS;        }

[[:alpha:]][_[:alnum:]]* {
  lval->build<std::string>(YYText());
  return rumur::parser::token::ID;
}

"--" {
  BEGIN(SC_COMMENT);
}

"/*" {
  BEGIN(SC_MULTILINE_COMMENT);
}

":=" { return rumur::parser::token::COLON_EQ;  }
"≔"  { return rumur::parser::token::COLON_EQ;  }
".." { return rumur::parser::token::DOTDOT;    }
">=" { return rumur::parser::token::GEQ;       }
"≥"  { return rumur::parser::token::GEQ;       }
"->" { return rumur::parser::token::IMPLIES;   }
"→"  { return rumur::parser::token::IMPLIES;   }
"<=" { return rumur::parser::token::LEQ;       }
"≤"  { return rumur::parser::token::LEQ;       }
"!=" { return rumur::parser::token::NEQ;       }
"≠"  { return rumur::parser::token::NEQ;       }
"==>" { return rumur::parser::token::ARROW;    }
"⇒"  { return rumur::parser::token::ARROW;    }
"==" { return rumur::parser::token::DEQ;       }

[&!|:=><\-\%\+;{},\[\]\.()/] { return YYText()[0]; }

\"[^\n\r\f\"]*\" {
  std::string s(YYText());
  lval->build<std::string>(s.substr(1, s.length() - 2).c_str());
  return rumur::parser::token::STRING;
}

    /* Ensure we keep a correct line count for error reporting. */
"\n" {
  loc->lines();
}

  /* Ignore white space. */
[[:space:]] {}

. {
  throw rumur::Error(std::string("unexpected character: '") + YYText() + "'", *loc);
}

  /* Comment handling. */
<SC_COMMENT>. {
  /* do nothing */
}

<SC_COMMENT>\n {
  loc->lines();
  BEGIN(INITIAL);
}

<SC_MULTILINE_COMMENT>[^*\n]* {
  /* do nothing */
}

<SC_MULTILINE_COMMENT>"*"+[^*/\n]* {
  /* do nothing */
}

<SC_MULTILINE_COMMENT>\n {
  loc->lines();
}

<SC_MULTILINE_COMMENT>"*"+"/" {
  BEGIN(INITIAL);
}
