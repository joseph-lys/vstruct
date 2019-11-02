"""
"""
import collections
import inspect
import tokenize
import io
from collections import OrderedDict


class TData(object):
    def __init__(self, name, value):
        self.name = name
        self.value = value


class Type(object):
    uint8_t = TData("uint8_t", 8)
    int8_t = TData("int8_t", 8)
    uint16_t = TData("uint16_t", 16)
    int16_t = TData("int16_t", 16)
    uint32_t = TData("uint32_t", 32)
    int32_t = TData("int32_t", 32)
    uint64_t = TData("uint64_t", 64)
    int64_t = TData("int64_t", 64)
    float = TData("float", 32)
    double = TData("double", 64)


class _Item(object):
    def __init__(self, bit_size=1, array_size=1):
        self._lineno = inspect.currentframe().f_back.f_back.f_lineno
        self._start_bit = None
        self._next_bit = None
        self._name = None
        self._code = None
        self._comments = []
        self._bit_size = bit_size
        self._array_size = array_size
        self._total_bits = bit_size * array_size

    def set_code(self, prior):
        raise NotImplementedError("Subclass MUST implement this")

    def get_code(self):
        return self._code

    def set_name(self, name):
        self._name = name

    def get_name(self):
        return self._name

    def append_comment(self, comment_line):
        self._comments.append(comment_line)

    def get_comments(self):
        return self._comments

    def get_type_info(self):
        raise NotImplementedError("Subclass MUST implement this")

    def extend(self, prior=None):
        if prior is None:
            self._start_bit = 0
            self._next_bit = self._bit_size * self._array_size
        else:
            self._start_bit = prior._next_bit
            self._next_bit = self._start_bit + self._bit_size * self._array_size


class BoolItem(_Item):
    def __init__(self):
        super(BoolItem, self).__init__()

    def set_code(self, prior):
        if prior is None:
            prior_name = "vstruct::Root"
        else:
            prior_name = prior.get_name()
        self._code = "typename vstruct::BoolItem<{}>::type {}".format(
            prior_name, self.get_name())
        self._code += "{*this};"

    def get_type_info(self):
        return "bool"


def bit_size_check(type_param, bit_size):
    """ Applies various checks and
    :param type_param: enum of Type
    :param (int) bit_size : Size in bits. If None, item will keep original bit size
    :return (int): bit_size, or inferred bit_size
    """
    if bit_size is None:
        bit_size = type_param.value
    elif type_param == Type.double and bit_size is None:
        bit_size = 64
    elif type_param == Type.double and bit_size is None:
        bit_size = 64

    # sanity checks:
    if bit_size != type_param.value and (
            type_param is Type.float or type_param is Type.double):
        raise ValueError("No packing support for {}, must be size {}".format(
            type_param.name, type_param.value))
    if bit_size > type_param.value:
        raise ValueError("Argument bit_size({}) must be <= {}".format(
            bit_size, type_param.value))
    if bit_size < 1:
        raise ValueError("bit_size must be greater or equal to 1")
    return bit_size


class LEItem(_Item):
    def __init__(self, type_param, bit_size=None):
        self._type = type_param
        bit_size = bit_size_check(type_param, bit_size)
        super(LEItem, self).__init__(bit_size, 1)

    def set_code(self, prior):
        if prior is None:
            prior_name = "vstruct::Root"
        else:
            prior_name = prior.get_name()
        self._code = (
            "typename vstruct::LEItem<{}, {}, {}>::type {}".format(
                prior_name,
                self._type.name,
                self._bit_size,
                self.get_name()))
        self._code += "{*this};"

    def get_type_info(self):
        return "{} : {}".format(
            self._type.name,
            self._bit_size)


class BoolArray(_Item):
    def __init__(self, array_size):
        self._type = "bool"
        super(BoolArray, self).__init__(1, array_size)

    def set_code(self, prior):
        if prior is None:
            prior_name = "vstruct::Root"
        else:
            prior_name = prior.get_name()
        self._code = (
            "typename vstruct::BoolArray<{}, {}>::type {}".format(
                prior_name,
                self._array_size,
                self.get_name()))
        self._code += "{*this};"

    def get_type_info(self):
        return "bool[{}]".format(
            self._array_size)


class LEArray(_Item):
    def __init__(self, type_param, bit_size, array_size):
        self._type = type_param
        bit_size = bit_size_check(type_param, bit_size)
        super(LEArray, self).__init__(bit_size, array_size)

    def set_code(self, prior):
        if prior is None:
            prior_name = "vstruct::Root"
        else:
            prior_name = prior.get_name()
        self._code = (
            "typename vstruct::LEArray<{}, {}, {}, {}>::type {}".format(
                prior_name,
                self._type.name,
                self._bit_size,
                self._array_size,
                self.get_name()))
        self._code += "{*this};"

    def get_type_info(self):
        return "{}[{}] : {}".format(
            self._type.name,
            self._array_size,
            self._bit_size)


class AlignPad(_Item):
    def __init__(self, byte_alignment):
        self._byte_alignment = byte_alignment
        super(AlignPad, self).__init__(0, 0)

    def set_code(self, prior):
        if prior is None:
            prior_name = "vstruct::Root"
        else:
            prior_name = prior.get_name()
        self._code = (
            "typename vstruct::AlignPad<{}, {}>::type {}".format(
                prior_name,
                self._byte_alignment,
                self.get_name()))
        self._code += ";"

    def get_type_info(self):
        return "padding[{}]".format(
            self._next_bit - self._start_bit)

    def extend(self, prior=None):
        if prior is None:
            self._start_bit = 0
            self._next_bit = 0
        else:
            self._start_bit = prior._next_bit
            align_bits = self._byte_alignment * 8
            fill = self._start_bit % align_bits
            if fill > 0:
                self._next_bit = self._start_bit + (align_bits - fill)
            else:
                self._next_bit = self._start_bit


class _OrderedClassMembers(type):
    @classmethod
    def __prepare__(self, name, bases):
        return collections.OrderedDict()

    def __new__(self, name, bases, classdict):
        classdict['__items__'] = []
        for key in classdict.keys():
            obj = classdict[key]

            if inspect.isclass(obj) and issubclass(obj, _Item):
                raise ValueError(
                    "Member must be assigned as an instance, not class. {}".format(key))
            elif not isinstance(obj, _Item):
                pass
            elif key.startswith('_'):
                raise ValueError(
                    "Member name must not start with an underscore. {}".format(key))
            else:  # all checks pass. put it in!
                classdict['__items__'].append(key)
        return type.__new__(self, name, bases, classdict)


class VStruct(metaclass=_OrderedClassMembers):
    _comments = []

    @classmethod
    def _update_item_names(cls):
        for key in cls.__items__:
            obj = getattr(cls, key)
            obj.set_name(key)

    @classmethod
    def _update_item_extension(cls):
        prior = None
        for obj in cls.items():
            obj.extend(prior)
            prior = obj

    @classmethod
    def _update_item_code(cls):
        prior = None
        for obj in cls.items():
            obj.set_code(prior)
            prior = obj

    @classmethod
    def _update_item_comments(cls):
        struct_code, struct_lineno = inspect.getsourcelines(cls)
        tokens = OrderedDict()
        for i in range(len(struct_code)):
            tokens[i + 1] = []
        for token_data in tokenize.generate_tokens(
                io.StringIO(''.join(struct_code)).readline):
            tok_type, tok_string, start, end, line = token_data
            if tok_type not in [
                    tokenize.ENDMARKER,
                    tokenize.NEWLINE,
                    tokenize.INDENT,
                    tokenize.DEDENT
                    ] and tok_type <= tokenize.COMMENT:
                tokens[start[0]].append((tok_type, tok_string))
        item_linenos = set()
        for obj in cls.items():
            item_linenos.add(obj._lineno - struct_lineno + 1)
        comment_blocks = []
        current_block = []
        item_hit = False  # flag that an item declaration has already been parsed
        for lineno in list(tokens.keys())[::-1]:
            token_data_list = tokens[lineno]
            comments = []
            # check if line has other tokens other than comments
            line_has_non_comments = False
            for tok_type, tok_string in token_data_list:
                # clean and flag non-comment line
                if tok_type == tokenize.COMMENT:
                    cleaned_comment = tok_string.split('#')[-1].lstrip()
                    comments.append(cleaned_comment)
                else:
                    line_has_non_comments = True
            # add to current or next
            if lineno in item_linenos:
                # hit an item
                if item_hit:
                    # current block is used by another time.
                    # create new block
                    comment_blocks.insert(0, current_block)
                    current_block = [comments]
                else:
                    # block currently not used by any time
                    # continue using blcok
                    current_block.insert(0, comments)
                item_hit = True
            elif item_hit and line_has_non_comments:
                # line contains non-comment lines,
                # likely part of another items, definition
                # start a new block.
                comment_blocks.insert(0, current_block)
                current_block = [comments]
                item_hit = False
            else:
                current_block.insert(0, comments)
        for i, obj in enumerate(cls.items()):
            current_block = comment_blocks[i]
            for block_lines in current_block:
                for comment in block_lines:
                    obj.append_comment(comment)

    @classmethod
    def _update_struct_comments(cls):
        for comment in cls.__doc__.splitlines():
            cls._comments.append(comment)
        unaligned_comments = []
        for item in cls.items():
            byte0 = item._start_bit // 8
            byte1 = (item._next_bit  - 1 )// 8
            bit0 = item._start_bit & 7
            bit1 = (item._next_bit - 1) & 7
            if (item._next_bit - item._start_bit) > 1:
                unaligned_comments.append((
                    "[{}].{}".format(byte0, bit0),
                    "[{}].{}".format(byte1, bit1),
                    item.get_name(),
                    item.get_type_info()))
            else:
                unaligned_comments.append((
                    "[{}].{}".format(byte0, bit0),
                    None,
                    item.get_name(),
                    item.get_type_info()))
        # find alignment
        max0, max1 = 0, 0
        for s0, s1, _, _ in unaligned_comments:
            if len(s0) > max0:
                max0 = len(s0)
            if s1 is not None and len(s1) > max1:
                max1 = len(s1)
        # generate aligned strings
        for s0, s1, name, type_info in unaligned_comments:
            aligned0 = s0 + (max0 - len(s0)) * ' '
            if s1 is not None:
                aligned1 = " ... " + s1 + (max1 - len(s1)) * ' '
            else:
                aligned1 = ' ' * 5 + max1 * ' '
            cls._comments.append("{}{} : {} ({})".format(
                aligned0, aligned1, name, type_info))

    @classmethod
    def build(cls):
        cls._comments = []
        for item in cls.items():
            item._comments = []
        cls._update_item_names()
        cls._update_item_extension()
        cls._update_item_code()
        cls._update_item_comments()
        cls._update_struct_comments()

    @classmethod
    def items(cls):
        for key in cls.__items__:
            obj = getattr(cls, key)
            yield obj

    def __new__(cls, *args, **kwargs):
        temp = super(VStruct, cls).__new__(*args, **kwargs)
        return temp


del _OrderedClassMembers
