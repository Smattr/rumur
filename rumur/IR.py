import abc, six

class Node(six.with_metaclass(abc.ABCMeta, object)):

    def __init__(self, node):
        self.node = node
    
    def postorder(self, visitor):
        for field, value in self.__dict__.items():
            if isinstance(value, Node):
                item = value.postorder(visitor)
                if item is not value:
                    setattr(self, field, item)
            elif isinstance(value, (list, tuple)):
                for i in range(len(value)):
                    if isinstance(value[i], Node):
                        item = value[i].postorder(visitor)
                        if item is not value[i]:
                            value[i] = item
        return visitor(self)

    def preorder(self, visitor):
        me = visitor(self)
        for field, value in me.__dict__.items():
            if isinstance(value, Node):
                item = value.preorder(visitor)
                if item is not value:
                    setattr(me, field, item)
            elif isinstance(value, (list, tuple)):
                for i in range(len(value)):
                    if isinstance(value[i], Node):
                        item = value[i].preorder(visitor)
                        if item is not value[i]:
                            value[i] = item
        return me

    def visit(self, visitor):
        return visitor(self)

class Stmt(six.with_metaclass(abc.ABCMeta, Node)):
    pass

class Assignment(Stmt):
    def __init__(self, designator, expr, node):
        super(Assignment, self).__init__(node)
        self.designator = designator
        self.expr = expr

class Branch(Node):
    def __init__(self, cond, stmts, node):
        super(Branch, self).__init__(node)
        self.cond = cond
        self.stmts = stmts

class IfStmt(Stmt):
    def __init__(self, branches, node):
        super(IfStmt, self).__init__(node)
        self.branches = branches

class Case(Node):
    def __init__(self, expr, stmts, node):
        super(Case, self).__init__(node)
        self.expr = expr
        self.stmts = stmts

class SwitchStmt(Stmt):
    def __init__(self, expr, cases, node):
        super(SwitchStmt, self).__init__(node)
        self.expr = expr
        self.cases = cases

class ForStmt(Stmt):
    def __init__(self, quantifier, stmts, node):
        super(ForStmt, self).__init__(node)
        self.quantifier = quantifier
        self.stmts = stmts

class Quantifier(Node):
    def __init__(self, symbol, typeexpr, lower, upper, step, node):
        super(Quantifier, self).__init__(node)
        self.symbol = symbol
        self.typeexpr = typeexpr
        self.lower = lower
        self.upper = upper
        self.step = step

class WhileStmt(Stmt):
    def __init__(self, expr, stmts, node):
        super(WhileStmt, self).__init__(node)
        self.expr = expr
        self.stmts = stmts

class Alias(Node):
    def __init__(self, symbol, expr, node):
        super(Alias, self).__init__(node)
        self.symbol = symbol
        self.expr = expr

class AliasStmt(Stmt):
    def __init__(self, aliases, stmts, node):
        super(AliasStmt, self).__init__(node)
        self.aliases = aliases
        self.stmts = stmts

class ProcCall(Stmt):
    def __init__(self, symbol, exprs, node):
        super(ProcCall, self).__init__(node)
        self.symbol = symbol
        self.exprs = exprs

class ClearStmt(Stmt):
    def __init__(self, designator, node):
        super(ClearStmt, self).__init__(node)
        self.designator = designator

class ErrorStmt(Stmt):
    def __init__(self, string, node):
        super(ErrorStmt, self).__init__(node)
        self.string = string

class PutStmt(Stmt):
    def __init__(self, arg, node):
        super(PutStmt, self).__init__(node)
        self.arg = arg

class ReturnStmt(Stmt):
    def __init__(self, value, node):
        super(ReturnStmt, self).__init__(node)
        self.value = value

class Invariant(Node):
    def __init__(self, string, expr, node):
        super(Invariant, self).__init__(node)
        self.string = string
        self.expr = expr

class StartState(Node):
    def __init__(self, string, decls, stmts, node):
        super(StartState, self).__init__(node)
        self.string = string
        self.decls = decls
        self.stmts = stmts

class SimpleRule(Node):
    def __init__(self, string, expr, decls, stmts, node):
        super(SimpleRule, self).__init__(node)
        self.string = string
        self.expr = expr
        self.decls = decls
        self.stmts = stmts

class RuleSet(Node):
    def __init__(self, quantifiers, rules, node):
        super(RuleSet, self).__init__(node)
        self.quantifiers = quantifiers
        self.rules = rules

class AliasRule(Node):
    def __init__(self, aliases, rules, node):
        super(AliasRule, self).__init__(node)
        self.aliases = aliases
        self.rules = rules

class Method(six.with_metaclass(abc.ABCMeta, Node)):
    def __init__(self, name, parameters, decls, stmts, node):
        super(Method, self).__init__(node)
        self.name = name
        self.parameters = parameters
        self.decls = decls
        self.stmts = stmts

class Function(Method):
    def __init__(self, name, parameters, returntype, decls, stmts, node):
        super(Function, self).__init__(name, parameters, decls, stmts, node)
        self.returntype = returntype

class Procedure(Method):
    pass

class TypeExpr(six.with_metaclass(abc.ABCMeta, Node)):

    @abc.abstractmethod
    def cardinality(self):
        raise NotImplementedError

class TypeRange(TypeExpr):
    def __init__(self, lower, upper, node):
        super(TypeRange, self).__init__(node)
        self.lower = lower
        self.upper = upper

    def cardinality(self):
        return self.upper - self.lower + 2

class TypeEnum(TypeExpr):
    def __init__(self, members, node):
        super(TypeEnum, self).__init__(node)
        self.members = members

    def cardinality(self):
        return len(self.members) + 1

Bool = TypeEnum(('false', 'true'), None)

class TypeRecord(TypeExpr):
    def __init__(self, vardecls, node):
        super(TypeRecord, self).__init__(node)
        self.vardecls = vardecls

    def cardinality(self):
        card = 1
        for v in self.vardecls.values():
            member_card = v.typeexpr.cardinality()
            card *= member_card
        return card

    def lookup_var(self, key):
        return self.vardecls.lookup_var(key)

class TypeArray(TypeExpr):
    def __init__(self, index_type, member_type, node):
        super(TypeArray, self).__init__(node)
        self.index_type = index_type
        self.member_type = member_type

    def cardinality(self):
        index_card = self.index_type.cardinality();
        member_card = self.member_type.cardinality();
        return (index_card - 1) * member_card

class TypeConstant(TypeExpr):
    '''
    Artificial type of a literal.
    '''

    def __init__(self, value):
        self.value = value

    def cardinality(self):
        return 1

class Formal(Node):
    def __init__(self, var, name, typeexpr, node):
        super(Formal, self).__init__(node)
        self.var = var
        self.name = name
        self.typeexpr = typeexpr

class Expr(six.with_metaclass(abc.ABCMeta, Node)):

    result_type = None

class BinOp(six.with_metaclass(abc.ABCMeta, Expr)):

    def __init__(self, left, right, node):
        super(BinOp, self).__init__(node)
        self.left = left
        self.right = right

class Add(BinOp):

    result_type = int

class Sub(BinOp):

    result_type = int

class Mul(BinOp):

    result_type = int

class Div(BinOp):

    result_type = int

class Mod(BinOp):

    result_type = int

class Or(BinOp):

    result_type = bool

class And(BinOp):

    result_type = bool

class Imp(BinOp):

    result_type = bool

class LT(BinOp):

    result_type = bool

class LTE(BinOp):

    result_type = bool

class GT(BinOp):

    result_type = bool

class GTE(BinOp):

    result_type = bool

class BoolEq(BinOp):

    result_type = bool

class IntEq(BinOp):

    result_type = bool

class BoolNEq(BinOp):

    result_type = bool

class IntNEq(BinOp):

    result_type = bool

class UnaryOp(six.with_metaclass(abc.ABCMeta, Expr)):

    def __init__(self, operand, node):
        super(UnaryOp, self).__init__(node)
        self.operand = operand

class Not(UnaryOp):

    result_type = bool

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

    def __init__(self, cond, casetrue, casefalse, node):
        super(TriCond, self).__init__(node)
        self.cond = cond
        self.casetrue = casetrue
        self.casefalse = casefalse

    @property
    def result_type(self):
        return self.casetrue.result_type

class HOLExpr(Expr):

    def __init__(self, quantifier, expr, node):
        super(HOLExpr, self).__init__(node)
        self.quantifier = quantifier
        self.expr = expr

    result_type = bool

class Forall(HOLExpr):
    pass

class Exists(HOLExpr):
    pass

class Guard(Expr):

    def __init__(self, child, lower, upper, node):
        super(Guard, self).__init__(node)
        self.child = child
        self.lower = lower
        self.upper = upper

    result_type = int

class Program(Node):
    def __init__(self, node):
        super(Program, self).__init__(node)
        self.decls = []
        self.procs = []
        self.rules = []

class VarDecl(Node):
    def __init__(self, name, typeexpr):
        self.name = name
        self.typeexpr = typeexpr

class VarRead(Expr):
    def __init__(self, root, offset, result_type, node):
        super(VarRead, self).__init__(node)
        self.root = root
        self.offset = offset
        if isinstance(result_type, TypeRange):
            self.result_type = int
        else:
            self.result_type = result_type

class StateRead(VarRead):
    pass

class VarWrite(Node):
    def __init__(self, root, offset, result_type, node):
        super(VarWrite, self).__init__(node)
        self.root = root
        self.offset = offset
        self.result_type = result_type

class StateWrite(VarWrite):
    pass
