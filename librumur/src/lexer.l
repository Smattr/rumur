%{

#include <cstddef>
#include "parser.yy.hh"
#include <rumur/except.h>
#include <rumur/scanner.h>

/* Override the declaration of yylex with the one from our derived scanner. */
#ifdef YY_DECL
  #undef YY_DECL
#endif
#define YY_DECL int rumur::scanner::yylex(\
  rumur::parser::semantic_type *const lval, rumur::parser::location_type *loc, int &start_token)

/* Each time yylex() is called, update the begin position with the last end
 * position. This ensures we get correct starting column numbers.
 */
#define YY_USER_ACTION  loc->step(); loc->columns(yyleng);

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

%{
  if (start_token) {
    const int token = start_token;
    start_token = 0;
    return token;
  }
%}

(0x[[:xdigit:]]+|[[:digit:]]+) {
  lval->build<std::string>(YYText());
  return rumur::parser::token::NUMBER;
}

([[:digit:]]?\.[[:digit:]]+) {
  throw rumur::Error("real types are not supported", *loc);
}

alias         { return rumur::parser::token::ALIAS;         }
array         { return rumur::parser::token::ARRAY;         }
assert        { return rumur::parser::token::ASSERT;        }
assume        { return rumur::parser::token::ASSUME;        }
begin         { return rumur::parser::token::BEGIN_TOK;     }
boolean       { return rumur::parser::token::BOOLEAN;       }
by            { return rumur::parser::token::BY;            }
case          { return rumur::parser::token::CASE;          }
clear         { return rumur::parser::token::CLEAR;         }
const         { return rumur::parser::token::CONST;         }
cover         { return rumur::parser::token::COVER;         }
do            { return rumur::parser::token::DO;            }
else          { return rumur::parser::token::ELSE;          }
elsif         { return rumur::parser::token::ELSIF;         }
end           { return rumur::parser::token::END;           }
endalias      { return rumur::parser::token::ENDALIAS;      }
endexists     { return rumur::parser::token::ENDEXISTS;     }
endfor        { return rumur::parser::token::ENDFOR;        }
endforall     { return rumur::parser::token::ENDFORALL;     }
endfunction   { return rumur::parser::token::ENDFUNCTION;   }
endif         { return rumur::parser::token::ENDIF;         }
endprocedure  { return rumur::parser::token::ENDPROCEDURE;  }
endrecord     { return rumur::parser::token::ENDRECORD;     }
endrule       { return rumur::parser::token::ENDRULE;       }
endruleset    { return rumur::parser::token::ENDRULESET;    }
endstartstate { return rumur::parser::token::ENDSTARTSTATE; }
endswitch     { return rumur::parser::token::ENDSWITCH;     }
endwhile      { return rumur::parser::token::ENDWHILE;      }
enum          { return rumur::parser::token::ENUM;          }
error         { return rumur::parser::token::ERROR;         }
exists        { return rumur::parser::token::EXISTS;        }
for           { return rumur::parser::token::FOR;           }
forall        { return rumur::parser::token::FORALL;        }
function      { return rumur::parser::token::FUNCTION;      }
if            { return rumur::parser::token::IF;            }
invariant     { return rumur::parser::token::INVARIANT;     }
isundefined   { return rumur::parser::token::ISUNDEFINED;   }
liveness      { return rumur::parser::token::LIVENESS;      }
of            { return rumur::parser::token::OF;            }
procedure     { return rumur::parser::token::PROCEDURE;     }
put           { return rumur::parser::token::PUT;           }
real          { throw rumur::Error("real types are not supported", *loc); }
record        { return rumur::parser::token::RECORD;        }
return        { return rumur::parser::token::RETURN;        }
rule          { return rumur::parser::token::RULE;          }
ruleset       { return rumur::parser::token::RULESET;       }
scalarset     { return rumur::parser::token::SCALARSET;     }
startstate    { return rumur::parser::token::STARTSTATE;    }
switch        { return rumur::parser::token::SWITCH;        }
then          { return rumur::parser::token::THEN;          }
to            { return rumur::parser::token::TO;            }
type          { return rumur::parser::token::TYPE;          }
undefine      { return rumur::parser::token::UNDEFINE;      }
union         { throw rumur::Error("union types are not supported", *loc); }
var           { return rumur::parser::token::VAR;           }
while         { return rumur::parser::token::WHILE;         }

"∀"           { return rumur::parser::token::FORALL;        }
"∃"           { return rumur::parser::token::EXISTS;        }

  /* Recognise true and false explicitly rather than as generic IDs (below). The
   * purpose of this is so that we match them case-insensitively.
   */
false {
  lval->build<std::string>("false");
  return rumur::parser::token::ID;
}
true {
  lval->build<std::string>("true");
  return rumur::parser::token::ID;
}

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
">>" { return rumur::parser::token::RSH;       }
"->" { return rumur::parser::token::IMPLIES;   }
"→"  { return rumur::parser::token::IMPLIES;   }
"<=" { return rumur::parser::token::LEQ;       }
"≤"  { return rumur::parser::token::LEQ;       }
"<<" { return rumur::parser::token::LSH;       }
"!=" { return rumur::parser::token::NEQ;       }
"≠"  { return rumur::parser::token::NEQ;       }
"==>" { return rumur::parser::token::ARROW;    }
"⇒"  { return rumur::parser::token::ARROW;    }
"==" { return rumur::parser::token::DEQ;       }
"¬"  { return '!';                             }
"∧"  { return rumur::parser::token::LAND;      }
"∨"  { return rumur::parser::token::LOR;       }
"&&" { return rumur::parser::token::AMPAMP;    }
"||" { return rumur::parser::token::PIPEPIPE;  }
"÷"  { return '/';                             }
"−"  { return '-';                             }
"∕"  { return '/';                             }
"×"  { return '*';                             }

[\^~&!|:=><\-\%\+;{},\[\]\.()/\*\?] {
  return YYText()[0];
}

(\"|“)(\\.|[^\\\"]|\\”)*(\"|”) {
  std::string s(YYText());

  /* figure out which quote character this string starts with */
  const std::string opener = s.find("“") == 0 ? "“" : "\"";
  assert(s.find(opener) == 0 && "logic bug in string lexing rule");

  /* figure out which quote character this string ends with */
  static const std::string squote = "”";
  size_t offset = s.size() - squote.size();
  const std::string closer
    = s.size() > squote.size() && s.find(squote, offset) == offset
    ? squote
    : "\"";

  /* build a string, stripping the quote delimiters */
  lval->build<std::string>(s.substr(opener.size(),
    s.size() - opener.size() - closer.size()).c_str());

  return rumur::parser::token::STRING;
}

    /* Ensure we keep a correct line count for error reporting. */
"\n" {
  loc->lines(yyleng);
  loc->step(); /* maintain correct column count */
}

  /* Ignore white space. */
[[:space:]] {
  loc->step(); /* maintain correct column count */
}

. {
  throw rumur::Error(std::string("unexpected character: '") + YYText() + "'", *loc);
}

  /* Comment handling. */
<SC_COMMENT>. {
  /* do nothing */
}

<SC_COMMENT>\n {
  loc->lines(yyleng);
  loc->step(); /* maintain correct column count */
  BEGIN(INITIAL);
}

<SC_MULTILINE_COMMENT>[^*\n]* {
  /* do nothing */
}

<SC_MULTILINE_COMMENT>"*"+[^*/\n]* {
  /* do nothing */
}

<SC_MULTILINE_COMMENT>\n {
  loc->lines(yyleng);
}

<SC_MULTILINE_COMMENT>"*"+"/" {
  loc->step(); /* maintain correct column count */
  BEGIN(INITIAL);
}
