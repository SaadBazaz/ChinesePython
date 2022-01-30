import imp
import os
import marshal
import stat
import string
import struct
import sys
import types
from cStringIO import StringIO

from compiler import ast, parse, walk
from compiler import pyassem, misc, future, symbols
from compiler.consts import SC_LOCAL, SC_GLOBAL, SC_FREE, SC_CELL
from compiler.consts import CO_VARARGS, CO_VARKEYWORDS, CO_NEWLOCALS,\
     CO_NESTED
from compiler.pyassem import TupleArg

# Do we have Python 1.x or Python 2.x?
try:
    VERSION = sys.version_info[0]
except AttributeError:
    VERSION = 1

callfunc_opcode_info = {
    # (Have *args, Have **args) : opcode
    (0,0) : "CALL_FUNCTION",
    (1,0) : "CALL_FUNCTION_VAR",
    (0,1) : "CALL_FUNCTION_KW",
    (1,1) : "CALL_FUNCTION_VAR_KW",
}

LOOP = 1
EXCEPT = 2
TRY_FINALLY = 3
END_FINALLY = 4

def compile(filename, display=0):
    f = open(filename)
    buf = f.read()
    f.close()
    mod = Module(buf, filename)
    try:
        mod.compile(display)
    except SyntaxError:
        raise
    else:
        f = open(filename + "c", "wb")
        mod.dump(f)
        f.close()

class Module:
    def __init__(self, source, filename):
        self.filename = filename
        self.source = source
        self.code = None

    def compile(self, display=0):
        tree = parse(self.source)
        root, filename = os.path.split(self.filename)
        if "nested_scopes" in future.find_futures(tree):
            gen = NestedScopeModuleCodeGenerator(filename)
        else:
            gen = ModuleCodeGenerator(filename)
        walk(tree, gen, 1)
        if display:
            import pprint
            print pprint.pprint(tree)
        self.code = gen.getCode()

    def dump(self, f):
        f.write(self.getPycHeader())
        marshal.dump(self.code, f)

    MAGIC = imp.get_magic()

    def getPycHeader(self):
        # compile.c uses marshal to write a long directly, with
        # calling the interface that would also generate a 1-byte code
        # to indicate the type of the value.  simplest way to get the
        # same effect is to call marshal and then skip the code.
        mtime = os.stat(self.filename)[stat.ST_MTIME]
        mtime = struct.pack('i', mtime)
        return self.MAGIC + mtime

class LocalNameFinder:
    """Find local names in scope"""
    def __init__(self, names=()):
        self.names = misc.Set()
        self.globals = misc.Set()
        for name in names:
            self.names.add(name)

    # XXX list comprehensions and for loops

    def getLocals(self):
        for elt in self.globals.elements():
            if self.names.has_elt(elt):
                self.names.remove(elt)
        return self.names

    def visitDict(self, node):
        pass

    def visitGlobal(self, node):
        for name in node.names:
            self.globals.add(name)

    def visitFunction(self, node):
        self.names.add(node.name)

    def visitLambda(self, node):
        pass

    def visitImport(self, node):
        for name, alias in node.names:
            self.names.add(alias or name)

    def visitFrom(self, node):
        for name, alias in node.names:
            self.names.add(alias or name)

    def visitClass(self, node):
        self.names.add(node.name)

    def visitAssName(self, node):
        self.names.add(node.name)

def is_constant_false(node):
    if isinstance(node, ast.Const):
        if not node.value:
            return 1
    return 0

class CodeGenerator:
    """Defines basic code generator for Python bytecode

    This class is an abstract base class.  Concrete subclasses must
    define an __init__() that defines self.graph and then calls the
    __init__() defined in this class.

    The concrete class must also define the class attributes
    NameFinder, FunctionGen, and ClassGen.  These attributes can be
    defined in the initClass() method, which is a hook for
    initializing these methods after all the classes have been
    defined. 
    """

    optimized = 0 # is namespace access optimized?
    __initialized = None
    class_name = None # provide default for instance variable

    def __init__(self, filename):
        if self.__initialized is None:
            self.initClass()
            self.__class__.__initialized = 1
        self.checkClass()
        self.filename = filename
        self.locals = misc.Stack()
        self.setups = misc.Stack()
        self.curStack = 0
        self.maxStack = 0
        self.last_lineno = None
        self._setupGraphDelegation()

    def initClass(self):
        """This method is called once for each class"""

    def checkClass(self):
        """Verify that class is constructed correctly"""
        try:
            assert hasattr(self, 'graph')
            assert getattr(self, 'NameFinder')
            assert getattr(self, 'FunctionGen')
            assert getattr(self, 'ClassGen')
        except AssertionError, msg:
            intro = "Bad class construction for %s" % self.__class__.__name__
            raise AssertionError, intro

    def _setupGraphDelegation(self):
        self.emit = self.graph.emit
        self.newBlock = self.graph.newBlock
        self.startBlock = self.graph.startBlock
        self.nextBlock = self.graph.nextBlock
        self.setDocstring = self.graph.setDocstring

    def getCode(self):
        """Return a code object"""
        return self.graph.getCode()

    def mangle(self, name):
        if self.class_name is not None:
            return misc.mangle(name, self.class_name)
        else:
            return name

    def parseSymbols(self, tree):
        s = symbols.SymbolVisitor()
        walk(tree, s)
        return s.scopes

    # Next five methods handle name access

    def isLocalName(self, name):
        return self.locals.top().has_elt(name)

    def storeName(self, name):
        self._nameOp('STORE', name)

    def loadName(self, name):
        self._nameOp('LOAD', name)

    def delName(self, name):
        self._nameOp('DELETE', name)

    def _nameOp(self, prefix, name):
        name = self.mangle(name)
        scope = self.scope.check_name(name)
        if scope == SC_LOCAL:
            if not self.optimized:
                self.emit(prefix + '_NAME', name)
            else:
                self.emit(prefix + '_FAST', name)
        elif scope == SC_GLOBAL:
            if not self.optimized:
                self.emit(prefix + '_NAME', name)
            else:
                self.emit(prefix + '_GLOBAL', name)
        elif scope == SC_FREE or scope == SC_CELL:
            self.emit(prefix + '_DEREF', name)
        else:
            raise RuntimeError, "unsupported scope for var %s: %d" % \
                  (name, scope)

    def _implicitNameOp(self, prefix, name):
        """Emit name ops for names generated implicitly by for loops

        The interpreter generates names that start with a period or
        dollar sign.  The symbol table ignores these names because
        they aren't present in the program text.
        """
        if self.optimized:
            self.emit(prefix + '_FAST', name)
        else:
            self.emit(prefix + '_NAME', name)

    def set_lineno(self, node, force=0):
        """Emit SET_LINENO if node has lineno attribute and it is 
        different than the last lineno emitted.

        Returns true if SET_LINENO was emitted.

        There are no rules for when an AST node should have a lineno
        attribute.  The transformer and AST code need to be reviewed
        and a consistent policy implemented and documented.  Until
        then, this method works around missing line numbers.
        """
        lineno = getattr(node, 'lineno', None)
        if lineno is not None and (lineno != self.last_lineno
                                   or force):
            self.emit('SET_LINENO', lineno)
            self.last_lineno = lineno
            return 1
        return 0

    # The first few visitor methods handle nodes that generator new
    # code objects.  They use class attributes to determine what
    # specialized code generators to use.

    NameFinder = LocalNameFinder
    FunctionGen = None
    ClassGen = None

    def visitModule(self, node):
        self.scopes = self.parseSymbols(node)
        self.scope = self.scopes[node]
        self.emit('SET_LINENO', 0)
        if node.doc:
            self.emit('LOAD_CONST', node.doc)
            self.storeName('__doc__')
        lnf = walk(node.node, self.NameFinder(), verbose=0)
        self.locals.push(lnf.getLocals())
        self.visit(node.node)
        self.emit('LOAD_CONST', None)
        self.emit('RETURN_VALUE')

    def visitFunction(self, node):
        self._visitFuncOrLambda(node, isLambda=0)
        if node.doc:
            self.setDocstring(node.doc)
        self.storeName(node.name)

    def visitLambda(self, node):
        self._visitFuncOrLambda(node, isLambda=1)

    def _visitFuncOrLambda(self, node, isLambda=0):
        gen = self.FunctionGen(node, self.filename, self.scopes, isLambda,
                               self.class_name)
        walk(node.code, gen)
        gen.finish()
        self.set_lineno(node)
        for default in node.defaults:
            self.visit(default)
        self.emit('LOAD_CONST', gen)
        self.emit('MAKE_FUNCTION', len(node.defaults))

    def visitClass(self, node):
        gen = self.ClassGen(node, self.scopes, self.filename)
        walk(node.code, gen)
        gen.finish()
        self.set_lineno(node)
        self.emit('LOAD_CONST', node.name)
        for base in node.bases:
            self.visit(base)
        self.emit('BUILD_TUPLE', len(node.bases))
        self.emit('LOAD_CONST', gen)
        self.emit('MAKE_FUNCTION', 0)
        self.emit('CALL_FUNCTION', 0)
        self.emit('BUILD_CLASS')
        self.storeName(node.name)

    # The rest are standard visitor methods

    # The next few implement control-flow statements

    def visitIf(self, node):
        end = self.newBlock()
        numtests = len(node.tests)
        for i in range(numtests):
            test, suite = node.tests[i]
            if is_constant_false(test):
                continue
            self.set_lineno(test)
            self.visit(test)
            nextTest = self.newBlock()
            self.emit('JUMP_IF_FALSE', nextTest)
            self.nextBlock()
            self.emit('POP_TOP')
            self.visit(suite)
            self.emit('JUMP_FORWARD', end)
            self.startBlock(nextTest)
            self.emit('POP_TOP')
        if node.else_:
            self.visit(node.else_)
        self.nextBlock(end)

    def visitWhile(self, node):
        self.set_lineno(node)

        loop = self.newBlock()
        else_ = self.newBlock()

        after = self.newBlock()
        self.emit('SETUP_LOOP', after)

        self.nextBlock(loop)
        self.setups.push((LOOP, loop))

        self.set_lineno(node, force=1)
        self.visit(node.test)
        self.emit('JUMP_IF_FALSE', else_ or after)

        self.nextBlock()
        self.emit('POP_TOP')
        self.visit(node.body)
        self.emit('JUMP_ABSOLUTE', loop)

        self.startBlock(else_) # or just the POPs if not else clause
        self.emit('POP_TOP')
        self.emit('POP_BLOCK')
        self.setups.pop()
        if node.else_:
            self.visit(node.else_)
        self.nextBlock(after)

    def visitFor(self, node):
        start = self.newBlock()
        anchor = self.newBlock()
        after = self.newBlock()
        self.setups.push((LOOP, start))

        self.set_lineno(node)
        self.emit('SETUP_LOOP', after)
        self.visit(node.list)
        self.visit(ast.Const(0))
        self.nextBlock(start)
        self.set_lineno(node, force=1)
        self.emit('FOR_LOOP', anchor)
##        self.nextBlock()
        self.visit(node.assign)
        self.visit(node.body)
        self.emit('JUMP_ABSOLUTE', start)
##        self.startBlock(anchor)
        self.nextBlock(anchor)
        self.emit('POP_BLOCK')
        self.setups.pop()
        if node.else_:
            self.visit(node.else_)
        self.nextBlock(after)

    def visitBreak(self, node):
        if not self.setups:
            raise SyntaxError, "'break' outside loop (%s, %d)" % \
                  (self.filename, node.lineno)
        self.set_lineno(node)
        self.emit('BREAK_LOOP')

    def visitContinue(self, node):
        # XXX test_grammar.py, line 351
        if not self.setups:
            raise SyntaxError, "'continue' outside loop (%s, %d)" % \
                  (self.filename, node.lineno)
        kind, block = self.setups.top()
        if kind == LOOP:
            self.set_lineno(node)
            self.emit('JUMP_ABSOLUTE', block)
            self.nextBlock()
        elif kind == EXCEPT or kind == TRY_FINALLY:
            self.set_lineno(node)
            # find the block that starts the loop
            top = len(self.setups)
            while top > 0:
                top = top - 1
                kind, loop_block = self.setups[top]
                if kind == LOOP:
                    break
            if kind != LOOP:
                raise SyntaxError, "'continue' outside loop (%s, %d)" % \
                      (self.filename, node.lineno)
            self.emit('CONTINUE_LOOP', loop_block)
            self.nextBlock()
        elif kind == END_FINALLY:
            msg = "'continue' not allowed inside 'finally' clause (%s, %d)"
            raise SyntaxError, msg % (self.filename, node.lineno)

    def visitTest(self, node, jump):
        end = self.newBlock()
        for child in node.nodes[:-1]:
            self.visit(child)
            self.emit(jump, end)
            self.nextBlock()
            self.emit('POP_TOP')
        self.visit(node.nodes[-1])
        self.nextBlock(end)

    def visitAnd(self, node):
        self.visitTest(node, 'JUMP_IF_FALSE')

    def visitOr(self, node):
        self.visitTest(node, 'JUMP_IF_TRUE')

    def visitCompare(self, node):
        self.visit(node.expr)
        cleanup = self.newBlock()
        for op, code in node.ops[:-1]:
            self.visit(code)
            self.emit('DUP_TOP')
            self.emit('ROT_THREE')
            self.emit('COMPARE_OP', op)
            self.emit('JUMP_IF_FALSE', cleanup)
            self.nextBlock()
            self.emit('POP_TOP')
        # now do the last comparison
        if node.ops:
            op, code = node.ops[-1]
            self.visit(code)
            self.emit('COMPARE_OP', op)
        if len(node.ops) > 1:
            end = self.newBlock()
            self.emit('JUMP_FORWARD', end)
            self.startBlock(cleanup)
            self.emit('ROT_TWO')
            self.emit('POP_TOP')
            self.nextBlock(end)

    # list comprehensions
    __list_count = 0
    
    def visitListComp(self, node):
        self.set_lineno(node)
        # setup list
        append = "$append%d" % self.__list_count
        self.__list_count = self.__list_count + 1
        self.emit('BUILD_LIST', 0)
        self.emit('DUP_TOP')
        self.emit('LOAD_ATTR', 'append')
        self._implicitNameOp('STORE', append)
        
        stack = []
        for i, for_ in zip(range(len(node.quals)), node.quals):
            start, anchor = self.visit(for_)
            cont = None
            for if_ in for_.ifs:
                if cont is None:
                    cont = self.newBlock()
                self.visit(if_, cont)
            stack.insert(0, (start, cont, anchor))

        self._implicitNameOp('LOAD', append)
        self.visit(node.expr)
        self.emit('CALL_FUNCTION', 1)
        self.emit('POP_TOP')
        
        for start, cont, anchor in stack:
            if cont:
                skip_one = self.newBlock()
                self.emit('JUMP_FORWARD', skip_one)
                self.startBlock(cont)
                self.emit('POP_TOP')
                self.nextBlock(skip_one)
            self.emit('JUMP_ABSOLUTE', start)
            self.startBlock(anchor)
        self._implicitNameOp('DELETE', append)
        
        self.__list_count = self.__list_count - 1

    def visitListCompFor(self, node):
        start = self.newBlock()
        anchor = self.newBlock()

        self.visit(node.list)
        self.visit(ast.Const(0))
        self.nextBlock(start)
        self.emit('SET_LINENO', node.lineno)
        self.emit('FOR_LOOP', anchor)
        self.nextBlock()
        self.visit(node.assign)
        return start, anchor

    def visitListCompIf(self, node, branch):
        self.set_lineno(node, force=1)
        self.visit(node.test)
        self.emit('JUMP_IF_FALSE', branch)
        self.newBlock()
        self.emit('POP_TOP')

    # exception related

    def visitAssert(self, node):
        # XXX would be interesting to implement this via a
        # transformation of the AST before this stage
        end = self.newBlock()
        self.set_lineno(node)
        # XXX __debug__ and AssertionError appear to be special cases
        # -- they are always loaded as globals even if there are local
        # names.  I guess this is a sort of renaming op.
        self.emit('LOAD_GLOBAL', '__debug__')
        self.emit('JUMP_IF_FALSE', end)
        self.nextBlock()
        self.emit('POP_TOP')
        self.visit(node.test)
        self.emit('JUMP_IF_TRUE', end)
        self.nextBlock()
        self.emit('POP_TOP')
        self.emit('LOAD_GLOBAL', 'AssertionError')
        if node.fail:
            self.visit(node.fail)
            self.emit('RAISE_VARARGS', 2)
        else:
            self.emit('RAISE_VARARGS', 1)
        self.nextBlock(end)
        self.emit('POP_TOP')

    def visitRaise(self, node):
        self.set_lineno(node)
        n = 0
        if node.expr1:
            self.visit(node.expr1)
            n = n + 1
        if node.expr2:
            self.visit(node.expr2)
            n = n + 1
        if node.expr3:
            self.visit(node.expr3)
            n = n + 1
        self.emit('RAISE_VARARGS', n)

    def visitTryExcept(self, node):
        body = self.newBlock()
        handlers = self.newBlock()
        end = self.newBlock()
        if node.else_:
            lElse = self.newBlock()
        else:
            lElse = end
        self.set_lineno(node)
        self.emit('SETUP_EXCEPT', handlers)
        self.nextBlock(body)
        self.setups.push((EXCEPT, body))
        self.visit(node.body)
        self.emit('POP_BLOCK')
        self.setups.pop()
        self.emit('JUMP_FORWARD', lElse)
        self.startBlock(handlers)
        
        last = len(node.handlers) - 1
        for i in range(len(node.handlers)):
            expr, target, body = node.handlers[i]
            self.set_lineno(expr)
            if expr:
                self.emit('DUP_TOP')
                self.visit(expr)
                self.emit('COMPARE_OP', 'exception match')
                next = self.newBlock()
                self.emit('JUMP_IF_FALSE', next)
                self.nextBlock()
                self.emit('POP_TOP')
            self.emit('POP_TOP')
            if target:
                self.visit(target)
            else:
                self.emit('POP_TOP')
            self.emit('POP_TOP')
            self.visit(body)
            self.emit('JUMP_FORWARD', end)
            if expr:
                self.nextBlock(next)
            else:
                self.nextBlock()
            self.emit('POP_TOP')
        self.emit('END_FINALLY')
        if node.else_:
            self.nextBlock(lElse)
            self.visit(node.else_)
        self.nextBlock(end)
    
    def visitTryFinally(self, node):
        body = self.newBlock()
        final = self.newBlock()
        self.set_lineno(node)
        self.emit('SETUP_FINALLY', final)
        self.nextBlock(body)
        self.setups.push((TRY_FINALLY, body))
        self.visit(node.body)
        self.emit('POP_BLOCK')
        self.setups.pop()
        self.emit('LOAD_CONST', None)
        self.nextBlock(final)
        self.setups.push((END_FINALLY, final))
        self.visit(node.final)
        self.emit('END_FINALLY')
        self.setups.pop()

    # misc

    def visitDiscard(self, node):
        self.set_lineno(node)
        self.visit(node.expr)
        self.emit('POP_TOP')

    def visitConst(self, node):
        self.emit('LOAD_CONST', node.value)

    def visitKeyword(self, node):
        self.emit('LOAD_CONST', node.name)
        self.visit(node.expr)

    def visitGlobal(self, node):
        # no code to generate
        pass

    def visitName(self, node):
        self.set_lineno(node)
        self.loadName(node.name)
        
    def visitPass(self, node):
        self.set_lineno(node)

    def visitImport(self, node):
        self.set_lineno(node)
        for name, alias in node.names:
            if VERSION > 1:
                self.emit('LOAD_CONST', None)
            self.emit('IMPORT_NAME', name)
            mod = string.split(name, ".")[0]
            self.storeName(alias or mod)

    def visitFrom(self, node):
        self.set_lineno(node)
        fromlist = map(lambda (name, alias): name, node.names)
        if VERSION > 1:
            self.emit('LOAD_CONST', tuple(fromlist))
        self.emit('IMPORT_NAME', node.modname)
        for name, alias in node.names:
            if VERSION > 1:
                if name == '*':
                    self.namespace = 0
                    self.emit('IMPORT_STAR')
                    # There can only be one name w/ from ... import *
                    assert len(node.names) == 1
                    return
                else:
                    self.emit('IMPORT_FROM', name)
                    self._resolveDots(name)
                    self.storeName(alias or name)
            else:
                self.emit('IMPORT_FROM', name)
        self.emit('POP_TOP')

    def _resolveDots(self, name):
        elts = string.split(name, ".")
        if len(elts) == 1:
            return
        for elt in elts[1:]:
            self.emit('LOAD_ATTR', elt)

    def visitGetattr(self, node):
        self.visit(node.expr)
        self.emit('LOAD_ATTR', self.mangle(node.attrname))

    # next five implement assignments

    def visitAssign(self, node):
        self.set_lineno(node)
        self.visit(node.expr)
        dups = len(node.nodes) - 1
        for i in range(len(node.nodes)):
            elt = node.nodes[i]
            if i < dups:
                self.emit('DUP_TOP')
            if isinstance(elt, ast.Node):
                self.visit(elt)

    def visitAssName(self, node):
        if node.flags == 'OP_ASSIGN':
            self.storeName(node.name)
        elif node.flags == 'OP_DELETE':
            self.set_lineno(node)
            self.delName(node.name)
        else:
            print "oops", node.flags

    def visitAssAttr(self, node):
        self.visit(node.expr)
        if node.flags == 'OP_ASSIGN':
            self.emit('STORE_ATTR', self.mangle(node.attrname))
        elif node.flags == 'OP_DELETE':
            self.emit('DELETE_ATTR', self.mangle(node.attrname))
        else:
            print "warning: unexpected flags:", node.flags
            print node

    def _visitAssSequence(self, node, op='UNPACK_SEQUENCE'):
        if findOp(node) != 'OP_DELETE':
            self.emit(op, len(node.nodes))
        for child in node.nodes:
            self.visit(child)

    if VERSION > 1:
        visitAssTuple = _visitAssSequence
        visitAssList = _visitAssSequence
    else:
        def visitAssTuple(self, node):
            self._visitAssSequence(node, 'UNPACK_TUPLE')

        def visitAssList(self, node):
            self._visitAssSequence(node, 'UNPACK_LIST')

    # augmented assignment

    def visitAugAssign(self, node):
        self.set_lineno(node)
        aug_node = wrap_aug(node.node)
        self.visit(aug_node, "load")
        self.visit(node.expr)
        self.emit(self._augmented_opcode[node.op])
        self.visit(aug_node, "store")

    _augmented_opcode = {
        '+=' : 'INPLACE_ADD',
        '-=' : 'INPLACE_SUBTRACT',
        '*=' : 'INPLACE_MULTIPLY',
        '/=' : 'INPLACE_DIVIDE',
        '%=' : 'INPLACE_MODULO',
        '**=': 'INPLACE_POWER',
        '>>=': 'INPLACE_RSHIFT',
        '<<=': 'INPLACE_LSHIFT',
        '&=' : 'INPLACE_AND',
        '^=' : 'INPLACE_XOR',
        '|=' : 'INPLACE_OR',
        }

    def visitAugName(self, node, mode):
        if mode == "load":
            self.loadName(node.name)
        elif mode == "store":
            self.storeName(node.name)

    def visitAugGetattr(self, node, mode):
        if mode == "load":
            self.visit(node.expr)
            self.emit('DUP_TOP')
            self.emit('LOAD_ATTR', self.mangle(node.attrname))
        elif mode == "store":
            self.emit('ROT_TWO')
            self.emit('STORE_ATTR', self.mangle(node.attrname))

    def visitAugSlice(self, node, mode):
        if mode == "load":
            self.visitSlice(node, 1)
        elif mode == "store":
            slice = 0
            if node.lower:
                slice = slice | 1
            if node.upper:
                slice = slice | 2
            if slice == 0:
                self.emit('ROT_TWO')
            elif slice == 3:
                self.emit('ROT_FOUR')
            else:
                self.emit('ROT_THREE')
            self.emit('STORE_SLICE+%d' % slice)

    def visitAugSubscript(self, node, mode):
        if len(node.subs) > 1:
            raise SyntaxError, "augmented assignment to tuple is not possible"
        if mode == "load":
            self.visitSubscript(node, 1)
        elif mode == "store":
            self.emit('ROT_THREE')
            self.emit('STORE_SUBSCR')

    def visitExec(self, node):
        self.visit(node.expr)
        if node.locals is None:
            self.emit('LOAD_CONST', None)
        else:
            self.visit(node.locals)
        if node.globals is None:
            self.emit('DUP_TOP')
        else:
            self.visit(node.globals)
        self.emit('EXEC_STMT')

    def visitCallFunc(self, node):
        pos = 0
        kw = 0
        self.set_lineno(node)
        self.visit(node.node)
        for arg in node.args:
            self.visit(arg)
            if isinstance(arg, ast.Keyword):
                kw = kw + 1
            else:
                pos = pos + 1
        if node.star_args is not None:
            self.visit(node.star_args)
        if node.dstar_args is not None:
            self.visit(node.dstar_args)
        have_star = node.star_args is not None
        have_dstar = node.dstar_args is not None
        opcode = callfunc_opcode_info[have_star, have_dstar]
        self.emit(opcode, kw << 8 | pos)

    def visitPrint(self, node, newline=0):
        self.set_lineno(node)
        if node.dest:
            self.visit(node.dest)
        for child in node.nodes:
            if node.dest:
                self.emit('DUP_TOP')
            self.visit(child)
            if node.dest:
                self.emit('ROT_TWO')
                self.emit('PRINT_ITEM_TO')
            else:
                self.emit('PRINT_ITEM')
        if node.dest and not newline:
            self.emit('POP_TOP')

    def visitPrintnl(self, node):
        self.visitPrint(node, newline=1)
        if node.dest:
            self.emit('PRINT_NEWLINE_TO')
        else:
            self.emit('PRINT_NEWLINE')

    def visitReturn(self, node):
        self.set_lineno(node)
        self.visit(node.value)
        self.emit('RETURN_VALUE')

    # slice and subscript stuff

    def visitSlice(self, node, aug_flag=None):
        # aug_flag is used by visitAugSlice
        self.visit(node.expr)
        slice = 0
        if node.lower:
            self.visit(node.lower)
            slice = slice | 1
        if node.upper:
            self.visit(node.upper)
            slice = slice | 2
        if aug_flag:
            if slice == 0:
                self.emit('DUP_TOP')
            elif slice == 3:
                self.emit('DUP_TOPX', 3)
            else:
                self.emit('DUP_TOPX', 2)
        if node.flags == 'OP_APPLY':
            self.emit('SLICE+%d' % slice)
        elif node.flags == 'OP_ASSIGN':
            self.emit('STORE_SLICE+%d' % slice)
        elif node.flags == 'OP_DELETE':
            self.emit('DELETE_SLICE+%d' % slice)
        else:
            print "weird slice", node.flags
            raise

    def visitSubscript(self, node, aug_flag=None):
        self.visit(node.expr)
        for sub in node.subs:
            self.visit(sub)
        if aug_flag:
            self.emit('DUP_TOPX', 2)
        if len(node.subs) > 1:
            self.emit('BUILD_TUPLE', len(node.subs))
        if node.flags == 'OP_APPLY':
            self.emit('BINARY_SUBSCR')
        elif node.flags == 'OP_ASSIGN':
            self.emit('STORE_SUBSCR')
        elif node.flags == 'OP_DELETE':
            self.emit('DELETE_SUBSCR')

    # binary ops

    def binaryOp(self, node, op):
        self.visit(node.left)
        self.visit(node.right)
        self.emit(op)

    def visitAdd(self, node):
        return self.binaryOp(node, 'BINARY_ADD')

    def visitSub(self, node):
        return self.binaryOp(node, 'BINARY_SUBTRACT')

    def visitMul(self, node):
        return self.binaryOp(node, 'BINARY_MULTIPLY')

    def visitDiv(self, node):
        return self.binaryOp(node, 'BINARY_DIVIDE')

    def visitMod(self, node):
        return self.binaryOp(node, 'BINARY_MODULO')

    def visitPower(self, node):
        return self.binaryOp(node, 'BINARY_POWER')

    def visitLeftShift(self, node):
        return self.binaryOp(node, 'BINARY_LSHIFT')

    def visitRightShift(self, node):
        return self.binaryOp(node, 'BINARY_RSHIFT')

    # unary ops

    def unaryOp(self, node, op):
        self.visit(node.expr)
        self.emit(op)

    def visitInvert(self, node):
        return self.unaryOp(node, 'UNARY_INVERT')

    def visitUnarySub(self, node):
        return self.unaryOp(node, 'UNARY_NEGATIVE')

    def visitUnaryAdd(self, node):
        return self.unaryOp(node, 'UNARY_POSITIVE')

    def visitUnaryInvert(self, node):
        return self.unaryOp(node, 'UNARY_INVERT')

    def visitNot(self, node):
        return self.unaryOp(node, 'UNARY_NOT')

    def visitBackquote(self, node):
        return self.unaryOp(node, 'UNARY_CONVERT')

    # bit ops

    def bitOp(self, nodes, op):
        self.visit(nodes[0])
        for node in nodes[1:]:
            self.visit(node)
            self.emit(op)

    def visitBitand(self, node):
        return self.bitOp(node.nodes, 'BINARY_AND')

    def visitBitor(self, node):
        return self.bitOp(node.nodes, 'BINARY_OR')

    def visitBitxor(self, node):
        return self.bitOp(node.nodes, 'BINARY_XOR')

    # object constructors

    def visitEllipsis(self, node):
        self.emit('LOAD_CONST', Ellipsis)

    def visitTuple(self, node):
        self.set_lineno(node)
        for elt in node.nodes:
            self.visit(elt)
        self.emit('BUILD_TUPLE', len(node.nodes))

    def visitList(self, node):
        self.set_lineno(node)
        for elt in node.nodes:
            self.visit(elt)
        self.emit('BUILD_LIST', len(node.nodes))

    def visitSliceobj(self, node):
        for child in node.nodes:
            self.visit(child)
        self.emit('BUILD_SLICE', len(node.nodes))

    def visitDict(self, node):
        lineno = getattr(node, 'lineno', None)
        if lineno:
            self.emit('SET_LINENO', lineno)
        self.emit('BUILD_MAP', 0)
        for k, v in node.items:
            lineno2 = getattr(node, 'lineno', None)
            if lineno2 is not None and lineno != lineno2:
                self.emit('SET_LINENO', lineno2)
                lineno = lineno2
            self.emit('DUP_TOP')
            self.visit(v)
            self.emit('ROT_TWO')
            self.visit(k)
            self.emit('STORE_SUBSCR')

class NestedScopeCodeGenerator(CodeGenerator):
    __super_visitModule = CodeGenerator.visitModule
    __super_visitClass = CodeGenerator.visitClass
    __super__visitFuncOrLambda = CodeGenerator._visitFuncOrLambda

    def parseSymbols(self, tree):
        s = symbols.SymbolVisitor()
        walk(tree, s)
        return s.scopes

    def visitModule(self, node):
        self.scopes = self.parseSymbols(node)
        self.scope = self.scopes[node]
        self.__super_visitModule(node)

    def _nameOp(self, prefix, name):
        name = self.mangle(name)
        scope = self.scope.check_name(name)
        if scope == SC_LOCAL:
            if not self.optimized:
                self.emit(prefix + '_NAME', name)
            else:
                self.emit(prefix + '_FAST', name)
        elif scope == SC_GLOBAL:
            self.emit(prefix + '_GLOBAL', name)
        elif scope == SC_FREE or scope == SC_CELL:
            self.emit(prefix + '_DEREF', name)
        else:
            raise RuntimeError, "unsupported scope for var %s: %d" % \
                  (name, scope)

    def _visitFuncOrLambda(self, node, isLambda=0):
        gen = self.FunctionGen(node, self.filename, self.scopes, isLambda,
                               self.class_name)
        walk(node.code, gen)
        gen.finish()
        self.set_lineno(node)
        for default in node.defaults:
            self.visit(default)
        frees = gen.scope.get_free_vars()
        if frees:
            for name in frees:
                self.emit('LOAD_CLOSURE', name)
            self.emit('LOAD_CONST', gen)
            self.emit('MAKE_CLOSURE', len(node.defaults))
        else:
            self.emit('LOAD_CONST', gen)
            self.emit('MAKE_FUNCTION', len(node.defaults))

    def visitClass(self, node):
        gen = self.ClassGen(node, self.scopes, self.filename)
        walk(node.code, gen)
        gen.finish()
        self.set_lineno(node)
        self.emit('LOAD_CONST', node.name)
        for base in node.bases:
            self.visit(base)
        self.emit('BUILD_TUPLE', len(node.bases))
        frees = gen.scope.get_free_vars()
        for name in frees:
            self.emit('LOAD_CLOSURE', name)
        self.emit('LOAD_CONST', gen)
        if frees:
            self.emit('MAKE_CLOSURE', 0)
        else:
            self.emit('MAKE_FUNCTION', 0)
        self.emit('CALL_FUNCTION', 0)
        self.emit('BUILD_CLASS')
        self.storeName(node.name)
        

class LGBScopeMixin:
    """Defines initClass() for Python 2.1-compatible scoping"""
    def initClass(self):
        self.__class__.NameFinder = LocalNameFinder
        self.__class__.FunctionGen = FunctionCodeGenerator
        self.__class__.ClassGen = ClassCodeGenerator

class NestedScopeMixin:
    """Defines initClass() for nested scoping (Python 2.2-compatible)"""
    def initClass(self):
        self.__class__.NameFinder = LocalNameFinder
        self.__class__.FunctionGen = NestedFunctionCodeGenerator
        self.__class__.ClassGen = NestedClassCodeGenerator

class ModuleCodeGenerator(LGBScopeMixin, CodeGenerator):
    __super_init = CodeGenerator.__init__

    scopes = None
    
    def __init__(self, filename):
        self.graph = pyassem.PyFlowGraph("<module>", filename)
        self.__super_init(filename)

class NestedScopeModuleCodeGenerator(NestedScopeMixin,
                                     NestedScopeCodeGenerator):
    __super_init = CodeGenerator.__init__
    
    def __init__(self, filename):
        self.graph = pyassem.PyFlowGraph("<module>", filename)
        self.__super_init(filename)
        self.graph.setFlag(CO_NESTED)

class AbstractFunctionCode:
    optimized = 1
    lambdaCount = 0

    def __init__(self, func, filename, scopes, isLambda, class_name):
        self.scopes = scopes
        self.scope = scopes[func]
        self.class_name = class_name
        if isLambda:
            klass = FunctionCodeGenerator
            name = "<lambda.%d>" % klass.lambdaCount
            klass.lambdaCount = klass.lambdaCount + 1
        else:
            name = func.name
        args, hasTupleArg = generateArgList(func.argnames)
        self.graph = pyassem.PyFlowGraph(name, filename, args, 
                                         optimized=1) 
        self.isLambda = isLambda
        self.super_init(filename)

        if not isLambda and func.doc:
            self.setDocstring(func.doc)

        lnf = walk(func.code, self.NameFinder(args), verbose=0)
        self.locals.push(lnf.getLocals())
        if func.varargs:
            self.graph.setFlag(CO_VARARGS)
        if func.kwargs:
            self.graph.setFlag(CO_VARKEYWORDS)
        self.set_lineno(func)
        if hasTupleArg:
            self.generateArgUnpack(func.argnames)

    def finish(self):
        self.graph.startExitBlock()
        if not self.isLambda:
            self.emit('LOAD_CONST', None)
        self.emit('RETURN_VALUE')

    def generateArgUnpack(self, args):
        for i in range(len(args)):
            arg = args[i]
            if type(arg) == types.TupleType:
                self.emit('LOAD_FAST', '.%d' % (i * 2))
                self.unpackSequence(arg)
                        
    def unpackSequence(self, tup):
        if VERSION > 1:
            self.emit('UNPACK_SEQUENCE', len(tup))
        else:
            self.emit('UNPACK_TUPLE', len(tup))
        for elt in tup:
            if type(elt) == types.TupleType:
                self.unpackSequence(elt)
            else:
                self._nameOp('STORE', elt)

    unpackTuple = unpackSequence

class FunctionCodeGenerator(LGBScopeMixin, AbstractFunctionCode,
                            CodeGenerator): 
    super_init = CodeGenerator.__init__ # call be other init
    scopes = None

class NestedFunctionCodeGenerator(AbstractFunctionCode,
                                  NestedScopeMixin,
                                  NestedScopeCodeGenerator):
    super_init = NestedScopeCodeGenerator.__init__ # call be other init
    __super_init = AbstractFunctionCode.__init__

    def __init__(self, func, filename, scopes, isLambda, class_name):
        self.scopes = scopes
        self.scope = scopes[func]
        self.__super_init(func, filename, scopes, isLambda, class_name)
        self.graph.setFreeVars(self.scope.get_free_vars())
        self.graph.setCellVars(self.scope.get_cell_vars())
        self.graph.setFlag(CO_NESTED)

class AbstractClassCode:

    def __init__(self, klass, scopes, filename):
        assert isinstance(filename, types.StringType)
        assert isinstance(scopes, types.DictType)
        self.graph = pyassem.PyFlowGraph(klass.name, filename,
                                           optimized=0)
        self.super_init(filename)
        lnf = walk(klass.code, self.NameFinder(), 0)
        self.locals.push(lnf.getLocals())
        self.graph.setFlag(CO_NEWLOCALS)
        if klass.doc:
            self.setDocstring(klass.doc)

    def _nameOp(self, prefix, name):
        # Class namespaces are always unoptimized
        self.emit(prefix + '_NAME', name)

    def finish(self):
        self.graph.startExitBlock()
        self.emit('LOAD_LOCALS')
        self.emit('RETURN_VALUE')

class ClassCodeGenerator(LGBScopeMixin, AbstractClassCode, CodeGenerator):
    super_init = CodeGenerator.__init__
    scopes = None

    __super_init = AbstractClassCode.__init__

    def __init__(self, klass, scopes, filename):
        self.scopes = scopes
        self.scope = scopes[klass]
        self.__super_init(klass, scopes, filename)
        self.graph.setFreeVars(self.scope.get_free_vars())
        self.graph.setCellVars(self.scope.get_cell_vars())

class NestedClassCodeGenerator(AbstractClassCode,
                               NestedScopeMixin,
                               NestedScopeCodeGenerator):
    super_init = NestedScopeCodeGenerator.__init__ # call be other init
    __super_init = AbstractClassCode.__init__

    def __init__(self, klass, scopes, filename):
        assert isinstance(filename, types.StringType)
        self.scopes = scopes
        self.scope = scopes[klass]
        self.__super_init(klass, scopes, filename)
        self.graph.setFreeVars(self.scope.get_free_vars())
        self.graph.setCellVars(self.scope.get_cell_vars())
        self.graph.setFlag(CO_NESTED)

def generateArgList(arglist):
    """Generate an arg list marking TupleArgs"""
    args = []
    extra = []
    count = 0
    for i in range(len(arglist)):
        elt = arglist[i]
        if type(elt) == types.StringType:
            args.append(elt)
        elif type(elt) == types.TupleType:
            args.append(TupleArg(i * 2, elt))
            extra.extend(misc.flatten(elt))
            count = count + 1
        else:
            raise ValueError, "unexpect argument type:", elt
    return args + extra, count

def findOp(node):
    """Find the op (DELETE, LOAD, STORE) in an AssTuple tree"""
    v = OpFinder()
    walk(node, v, verbose=0)
    return v.op

class OpFinder:
    def __init__(self):
        self.op = None
    def visitAssName(self, node):
        if self.op is None:
            self.op = node.flags
        elif self.op != node.flags:
            raise ValueError, "mixed ops in stmt"
    visitAssAttr = visitAssName
    visitSubscript = visitAssName

class Delegator:
    """Base class to support delegation for augmented assignment nodes

    To generator code for augmented assignments, we use the following
    wrapper classes.  In visitAugAssign, the left-hand expression node
    is visited twice.  The first time the visit uses the normal method
    for that node .  The second time the visit uses a different method
    that generates the appropriate code to perform the assignment.
    These delegator classes wrap the original AST nodes in order to
    support the variant visit methods.
    """
    def __init__(self, obj):
        self.obj = obj

    def __getattr__(self, attr):
        return getattr(self.obj, attr)

class AugGetattr(Delegator):
    pass

class AugName(Delegator):
    pass

class AugSlice(Delegator):
    pass

class AugSubscript(Delegator):
    pass


wrapper = {
    ast.Getattr: AugGetattr,
    ast.Name: AugName,
    ast.Slice: AugSlice,
    ast.Subscript: AugSubscript,
    }

def wrap_aug(node):
    return wrapper[node.__class__](node)

if __name__ == "__main__":
    import sys

    for file in sys.argv[1:]:
        compile(file)
