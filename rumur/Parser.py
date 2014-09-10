'''
Parse a Murphi input into a programmatic AST.
'''

import os, plyplus

DEFAULT_GRAMMAR = os.path.join(
    os.path.dirname(
        os.path.realpath(__file__)), 'murphi.g')

class Parser(object):
    '''Murphi parser.'''
    def __init__(self, grammar=DEFAULT_GRAMMAR):
        with open(grammar, 'r') as f:
            self.grammar = plyplus.Grammar(f.read())

    def parse(self, s):
        return self.grammar.parse(s)

def parse(s):
    '''Simple parsing interface when you're only going to be parsing a single
    input or don't want to manually construct a parser.'''
    p = Parser()
    return p.parse(s)
