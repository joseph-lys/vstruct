import sys
import os
import argparse
from collections import OrderedDict
import inspect
import importlib.util
import vstruct


def header_guard(args):
    if args.guard is None:
        s = args.output.name.upper().replace(
            "\\", "_").replace("/", "_").replace(" ", "_").replace(
            ".", "_") + "_"
    else:
        s = args.guard
    return s


def header_start(args, code_obj, modules):
    c = code_obj
    d = OrderedDict()
    c.comment("*** THIS FILE IS AUTO GENERATED BY {} ***".format(
        os.path.split(__file__)[-1]))
    c.comment('')
    for m in modules:
        d[m.__file__] = m.__doc__
    for f, doc in d.items():
        c.comment(
            os.path.split(f)[-1])
        c.comments(doc.splitlines())
        c.comment("")
        c.comment("")
    c.code("#ifndef {}".format(header_guard(args)))
    c.code("#define {}".format(header_guard(args)))
    c.code("#include \"vstruct.h\"")
    c.blank_line()
    # File level namespace
    if args.namespace is not None:
        for n in args.namespace:
            c.code("namespace {}".format(n) + " {")
    c.blank_lines(2)


def header_end(args, code_obj):
    c = code_obj
    # close namespaces
    c.blank_lines(1)
    if args.namespace is not None:
        for n in list(args.namespace)[::-1]:
            c.code("}")
            c.inline_comment("namespace {}".format(n))
    c.blank_lines(2)
    c.code("#endif")
    c.inline_comment(header_guard(args))
    c.blank_line()


def header_structs(args, code_obj, struct):
    c = code_obj
    S = struct
    S.build()
    c.comments(S._comments)
    c.code("struct {} : public vstruct::VStruct".format(
        S.__name__) + " {")
    c.indent()
    for item in S.items():
        c.comments(item.get_comments())
        c.code(item.get_code())
        c.blank_line()
    c.dedent()
    c.code("};")
    c.inline_comment(S.__name__)
    c.blank_lines(2)


def main():
    parser = argparse.ArgumentParser(
        description="generate C++ header based on vstructs in source file")
    parser.add_argument(
        '-f', '--files',  nargs='*', type=argparse.FileType('r'),
        help="Source .py files to parse")
    parser.add_argument(
        '-o', '--output', type=argparse.FileType('w'),
        help="Output header file to generate")
    parser.add_argument(
        '-n', '--namespace', nargs='*',
        help="namespaces for header")
    parser.add_argument(
        '-g', '--guard', type=str,
        help="custom header guard, if option is not given,"
             " defaults to a pattern generated from the output filename")
    args = parser.parse_args()

    assert args.output is not None, "Missing argument --output"

    code_obj = Code()
    modules = []
    for i, f in enumerate(args.files):
        dir_path = os.path.abspath(os.path.split(f.name)[0])
        module_name = os.path.split(f.name)[-1].split('.')[0]
        sys.path.insert(0, dir_path)
        try:
            m = importlib.import_module(module_name, module_name)
            modules.append(m)
        finally:
            del sys.path[0]
    header_start(args, code_obj, modules)
    for m in modules:
        f = inspect.getfile(m)
        for obj_name in dir(m):
            try:
                obj = m.__dict__[obj_name]
                issubclass(obj, vstruct.VStruct)
            except:
                continue
            if (inspect.isclass(obj)   # only classes
                    and issubclass(obj, vstruct.VStruct)
                    and obj != vstruct.VStruct):  # expect not equal
                header_structs(args, code_obj, obj)
    header_end(args, code_obj)
    for cc in code_obj._code:
        args.output.writelines(cc + '\n')
        print(cc)


class Code(object):
    def __init__(self):
        self._indent = 0
        self._code = []
        self._indent_char = "  "

    def set_indent_char(self, str):
        self._indent_char = str

    def indent(self):
        self._indent += 1

    def dedent(self):
        if self._indent > 0:
            self._indent -= 1

    def comments(self, list_of_str):
        for str in list_of_str:
            self.comment(str)

    def comment(self, str):
        self._code.append(r"{}/// {}".format(
            self._indent * self._indent_char, str.rstrip()))

    def codes(self, list_of_str):
        for str in list_of_str:
            self.code(str)

    def code(self, str):
        self._code.append(r"{}{}".format(
            self._indent * self._indent_char, str.rstrip()))

    def inline_comment(self, str):
        self._code[-1] += r"  // {}".format(
            str.rstrip())

    def blank_lines(self, number_of_lines):
        for _ in range(number_of_lines):
            self.blank_line()

    def blank_line(self):
        self._code.append("{}".format(
            self._indent * self._indent_char))

if __name__ == "__main__":
    main()
