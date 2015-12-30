import abc, operator, six

class Node(six.with_metaclass(abc.ABCMeta, object)):

    _fields = ()

    def __init__(self, node):
        self.node = node
    
    def postorder(self, visitor):
        for field in self._fields:
            assert hasattr(self, field)
            item = getattr(self, field).preorder(visitor)
            if item is not getattr(self, field):
                setattr(self, field, item)
        return visitor(self)

    def preorder(self, visitor):
        me = visitor(self)
        for field in me._fields:
            assert hasattr(me, field)
            item = getattr(me, field).postorder(visitor)
            if item is not getattr(me, field):
                setattr(me, field, item)
        return me

    def visit(self, visitor):
        return visitor(self)

class ConstDecl(Node):

    _fields = ('expr',)

    def __init__(self, name, expr, node):
        super(ConstDecl, self).__init__(node)
        self.name = name
        self.expr = expr

class Stmt(six.with_metaclass(abc.ABCMeta, Node)):
    pass

class Assignment(Stmt):
    def __init__(self, designator, expr, node):
        super(Assignment, self).__init__(node)
        self.designator = designator
        self.expr = expr

class Branch(Node):
    def __init__(self, cond, stmts):
        self.cond = cond
        self.stmts = stmts

class IfStmt(Stmt):
    def __init__(self, branches):
        self.branches = branches

class Case(Node):
    def __init__(self, expr, stmts):
        self.expr = expr
        self.stmts = stmts

class SwitchStmt(Stmt):
    def __init__(self, expr, cases):
        self.expr = expr
        self.cases = cases

class ForStmt(Stmt):
    def __init__(self, quantifier, stmts):
        self.quantifier = quantifier
        self.stmts = stmts

class Quantifier(Node):
    def __init__(self, symbol, typeexpr=None, lower=None, upper=None, step=None):
        self.typeexpr = typeexpr
        self.lower = lower
        self.upper = upper
        self.step = step

class WhileStmt(Stmt):
    def __init__(self, expr, stmts):
        self.expr = expr
        self.stmts = stmts

class Alias(Node):
    def __init__(self, symbol, expr):
        self.symbol = symbol
        self.expr = expr

class AliasStmt(Stmt):
    def __init__(self, aliases, stmts):
        self.aliases = aliases
        self.stmts = stmts

class ProcCall(Stmt):
    def __init__(self, symbol, exprs):
        self.symbol = symbol
        self.exprs = exprs

class ClearStmt(Stmt):
    def __init__(self, designator):
        self.designator = designator

class ErrorStmt(Stmt):
    def __init__(self, designator):
        self.designator = designator

class AssertStmt(Stmt):
    def __init__(self, expr, string):
        self.expr = expr
        self.string = string

class PutStmt(Stmt):
    def __init__(self, arg):
        self.arg = arg

class ReturnStmt(Stmt):
    def __init__(self, value):
        self.value = value

class Invariant(Node):
    def __init__(self, string, expr):
        self.string = string
        self.expr = expr

class StartState(Node):
    def __init__(self, string, decls, stmts):
        self.string = string
        self.decls = decls
        self.stmts = stmts

class SimpleRule(Node):
    def __init__(self, string, expr, decls, stmts):
        self.string = string
        self.expr = expr
        self.decls = decls
        self.stmts = stmts

class RuleSet(Node):
    def __init__(self, quantifiers, rules):
        self.quantifiers = quantifiers
        self.rules = rules

class AliasRule(Node):
    def __init__(self, aliases, rules):
        self.aliases = aliases
        self.rules = rules

class Designator(Node):
    def __init__(self, *atoms):
        self.atoms = atoms

class Method(six.with_metaclass(abc.ABCMeta, Node)):
    def __init__(self, name, parameters, decls, stmts):
        self.name = name
        self.parameters = parameters
        self.decls = decls
        self.stmts = stmts

class Function(Method):
    def __init__(self, name, parameters, returntype, decls, stmts):
        super(Function, self).__init__(name, parameters, decls, stmts)
        self.returntype = returntype

class Procedure(Method):
    pass

class TypeExpr(six.with_metaclass(abc.ABCMeta, Node)):
    pass

class TypeRef(TypeExpr):
    def __init__(self, symbol):
        self.symbol = symbol

class TypeRange(TypeExpr):
    def __init__(self, lower, upper, node):
        super(TypeRange, self).__init__(node)
        self.lower = lower
        self.upper = upper

class TypeEnum(TypeExpr):
    def __init__(self, members, node):
        super(TypeEnum, self).__init__(node)
        self.members = members

class TypeRecord(TypeExpr):
    def __init__(self, *vardecls):
        self.vardecls = vardecls

class TypeArray(TypeExpr):
    def __init__(self, index_type, member_type, node):
        super(TypeArray, self).__init__(node)
        self.index_type = index_type
        self.member_type = member_type

class Formal(Node):
    def __init__(self, var, name, typeexpr):
        self.var = var
        self.name = name
        self.typeexpr = typeexpr

class Expr(six.with_metaclass(abc.ABCMeta, Node)):

    result_type = None

class BinOp(six.with_metaclass(abc.ABCMeta, Expr)):

    _fields = ('left', 'right')

    def __init__(self, left, right, node):
        super(BinOp, self).__init__(node)
        self.left = left
        self.right = right

    @abc.abstractproperty
    def op(self):
        raise NotImplementedError

class Add(BinOp):

    result_type = int

    @property
    def op(self):
        return operator.add

class Sub(BinOp):

    result_type = int

    @property
    def op(self):
        return operator.sub

class Mul(BinOp):

    result_type = int

    @property
    def op(self):
        return operator.mul

class Div(BinOp):

    result_type = int

    @property
    def op(self):
        return operator.floordiv

class Mod(BinOp):

    result_type = int

    @property
    def op(self):
        return operator.mod

class Or(BinOp):

    result_type = bool

    @property
    def op(self):
        return (lambda a, b: int(a or b))

class And(BinOp):

    result_type = bool

    @property
    def op(self):
        return (lambda a, b: int(a and b))

class Imp(BinOp):

    result_type = bool

    @property
    def op(self):
        return (lambda a, b: int((not a) or b))

class LT(BinOp):

    result_type = bool

    @property
    def op(self):
        return operator.lt

class LTE(BinOp):

    result_type = bool

    @property
    def op(self):
        return operator.lte

class GT(BinOp):

    result_type = bool

    @property
    def op(self):
        return operator.gt

class GTE(BinOp):

    result_type = bool

    @property
    def op(self):
        return operator.gte

class Eq(BinOp):

    result_type = bool

    @property
    def op(self):
        return operator.eq

class NEq(BinOp):

    result_type = bool

    @property
    def op(self):
        return operator.ne

class UnaryOp(six.with_metaclass(abc.ABCMeta, Expr)):

    _fields = ('operand',)

    def __init__(self, operand):
        self.operand = operand

    @abc.abstractproperty
    def op(self):
        raise NotImplementedError

class Not(UnaryOp):

    result_type = bool

    @property
    def op(self):
        return (lambda x: int(not x))

class Lit(Expr):

    def __init__(self, value, node):
        super(Lit, self).__init__(node)
        self.value = value
        
    @property
    def result_type(self):
        if isinstance(self.value, bool):
            return bool
        return int

class TriCond(Expr):

    _fields = ('cond', 'casetrue', 'casefalse')

    def __init__(self, cond, casetrue, casefalse):
        self.cond = cond
        self.casetrue = casetrue
        self.casefalse = casefalse

    @property
    def result_type(self):
        return self.casetrue.result_type

class Program(Node):
    def __init__(self):
        self.decls = []
        self.procs = []
        self.rules = []

class TypeDecl(Node):
    def __init__(self, name, typeexpr):
        self.name = name
        self.typeexpr = typeexpr

class VarDecl(Node):
    def __init__(self, name, typeexpr):
        self.name = name
        self.typeexpr = typeexpr
