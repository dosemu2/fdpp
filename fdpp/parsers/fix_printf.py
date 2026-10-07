#!/usr/bin/python

import sys
import re

from os.path import relpath
from pathlib import Path
from textwrap import dedent


# Combined transformation configuration map
TRANSFORMS_MAP = {
    'S': {
        'wrapper': 'GET_PTR',
        'replacement': 's'
    },
    'Fs': {
        'wrapper': 'GET_PTR',
        'replacement': 's'
    },
    'P': {
        'wrapper': 'GET_FP32',
        'replacement': None
    },
    'Fp': {
        'wrapper': 'GET_FP32',
        'replacement': 'P',
    },
# Example only
#    'Y': {
#        'wrapper': None,          # Leaves the variable exactly as written
#        'replacement': 'p'        # Mutates %Y -> %p inside the string literal
#    }
}

FORMAT_INDEX_MAP = {
# Std
    'printf': 0,
    'fprintf': 1,
    'sprintf': 1,
    'snprintf': 2,

# FreeDOS specific
    'DebugPrintf': 0,
    'HMAInitPrintf': 0,
    'log': 0,
    'tn_printf': 0,
    '_printf': 0,
    '_fprintf': 1,
    '_sprintf': 1,
    '_snprintf': 2,

# FDPP
    'fdloudprintf': 0,
}

FUNC_PATTERN = re.compile(rf"\b({'|'.join(FORMAT_INDEX_MAP.keys())})\s*\(", re.MULTILINE)

CLEAN_MSG = f"C source clean up opportunity"


def usage(error=None):
    """
    Usage: python fix_printf.py (-i|-I) <input.c> (-o|-O) <output.cpp>
       -i: Use input.c as input file
       -I: Use stdin as input, input.c is the name used in warnings/errors
       -o: Use output.c as output file
       -O: Use stdout as output, output.c is the name used in warnings/errors
    """

    print(dedent(usage.__doc__), file=sys.stderr)
    if error:
        print(error, file=sys.stderr)


def extract_balanced_args_with_positions(text, start_pos):
    """
    Steps through characters starting at an open parenthesis.
    Returns a list of tuples containing: (arg_text, global_start_idx, global_end_idx).
    Preserves all internal whitespace, tabs, and newlines exactly.
    """
    args = []
    current_arg = []
    paren_depth = 0
    in_string = False
    escape = False

    arg_start = start_pos + 1
    i = start_pos

    while i < len(text):
        char = text[i]

        if in_string:
            current_arg.append(char)
            if escape:
                escape = False
            elif char == '\\':
                escape = True
            elif char == '"':
                in_string = False
        else:
            if char == '"':
                in_string = True
                current_arg.append(char)
            elif char == '(':
                paren_depth += 1
                if paren_depth > 1:
                    current_arg.append(char)
            elif char == ')':
                paren_depth -= 1
                if paren_depth == 0:
                    if current_arg or len(args) > 0:
                        args.append(("".join(current_arg), arg_start, i))
                    return i + 1, args
                current_arg.append(char)
            elif char == ',' and paren_depth == 1:
                args.append(("".join(current_arg), arg_start, i))
                current_arg = []
                arg_start = i + 1
            else:
                current_arg.append(char)
        i += 1
    return -1, []


def process_file(argv):

    def relative(name):
        current_dir = Path.cwd().resolve()
        full_path = Path(name).resolve()
        return relpath(full_path, current_dir)

    input_name = relative(argv[2])    # for any error messages we issue
    if argv[1] == '-i':
        try:
            input_code = Path(argv[2]).read_text(encoding='cp437')
            print(f"Reading {input_name}", file=sys.stderr)
        except FileNotFoundError:
            usage(f"Could not open input file '{input_name}")
            sys.exit(2)
    elif argv[1] == '-I':
        print(f"Reading stdin (named as {input_name})", file=sys.stderr)
        sys.stdin.reconfigure(encoding='cp437', newline=None)
        input_code = sys.stdin.read()
    else:
        usage(f"Invalid {argv[1]}")
        sys.exit(2)

    output_name = relative(argv[4])    # for any error messages we issue
    if argv[3] == '-o':
        output_stdout = False
    elif argv[3] == '-O':
        output_stdout = True
    else:
        usage(f"Invalid {argv[3]}")
        sys.exit(2)

    has_errors = False
    modifications = []
    pos = 0

    while True:
        match = FUNC_PATTERN.search(input_code, pos)
        if not match:
            break

        func_name = match.group(1)
        func_start = match.start()
        open_paren_pos = match.end() - 1

        end_pos, args = extract_balanced_args_with_positions(input_code, open_paren_pos)

        if end_pos == -1 or not args:
            pos = match.end()
            continue

        # Double-parentheses macro unpacking layer
        if len(args) == 1 and args[0][0].strip().startswith('(') and args[0][0].strip().endswith(')'):
            inner_text_raw, inner_start, inner_end = args[0]
            first_paren = inner_text_raw.find('(')
            _, inner_args = extract_balanced_args_with_positions(inner_text_raw, first_paren)
            if inner_args:
                args = [(txt, inner_start + s, inner_start + e) for txt, s, e in inner_args]


        fmt_index = FORMAT_INDEX_MAP[func_name]
        if fmt_index >= len(args):
            pos = end_pos
            continue

        fmt_arg_text, fmt_start, fmt_end = args[fmt_index]
        fmt_arg_stripped = fmt_arg_text.strip()

        if not (fmt_arg_stripped.startswith('"') and fmt_arg_stripped.endswith('"')):
            pos = end_pos
            continue

        fmt_str = fmt_arg_stripped[1:-1]

        specifier_pattern = re.compile(r'%(?:%|[0-9.+\-*#lhzj]*[a-zA-Z]+)')
        specifiers = [t for t in specifier_pattern.findall(fmt_str) if t != '%%']
        vargs = args[fmt_index + 1:]

        updated_fmt_str = fmt_str
        format_string_modified = False

        for i, spec in enumerate(specifiers):
            if i >= len(vargs):
                break

            arg_text, arg_start, arg_end = vargs[i]
            arg_stripped = arg_text.strip()

            target_token = next((token for token in TRANSFORMS_MAP if token in spec), None)

            line_no = input_code.count('\n', 0, func_start) + 1
            file_msg = f"[{input_name}:{line_no}/{func_name}]"

            if target_token:
                transform_config = TRANSFORMS_MAP[target_token]
                wrapper = transform_config['wrapper']
                replacement = transform_config['replacement']

                # Enforce the strict policy check ONLY if we are actively wrapping an argument
                if wrapper is not None:
                    if '(' in arg_stripped and not re.match(r'^\s*\([^)]+\)\s*[a-zA-Z_]', arg_stripped):
                        func_msg = f"'%{target_token} / {arg_stripped}'"
                        # Check if it is already wrapped in the target macro
                        if arg_stripped.startswith(f"{wrapper}(") and arg_stripped.endswith(')'):
                            print(f"Info: {file_msg} {CLEAN_MSG}, '{spec}' already applied so could remove the {wrapper}() around '{arg_stripped}'", file=sys.stderr)

                            # Clean the format string specifier even if the variable was already wrapped
                            if replacement is not None:
                                replaced_spec = spec.replace(target_token, replacement)
                                updated_fmt_str = updated_fmt_str.replace(spec, replaced_spec, 1)
                                format_string_modified = True
                            continue
                        else:
                            # It's an unauthorized nested macro, throw a hard build error
                            print(f"Error: {file_msg} Found restricted macro/function call {func_msg} - Aborting.", file=sys.stderr)
                            has_errors = True
                            continue

                    # Safe whitespace preservation wrap
                    leading_spaces = arg_text[:len(arg_text)-len(arg_text.lstrip())]
                    trailing_spaces = arg_text[len(arg_text.rstrip()):]
                    wrapped_arg_text = f"{leading_spaces}{wrapper}({arg_stripped}){trailing_spaces}"
                    modifications.append((arg_start, arg_end, wrapped_arg_text))

                # Process the string literal replacement rule (e.g. %Y -> %p)
                if replacement is not None:
                    replaced_spec = spec.replace(target_token, replacement)
                    updated_fmt_str = updated_fmt_str.replace(spec, replaced_spec, 1)
                    format_string_modified = True

            else:
                # C++ism DETECTION PASS:
                # The specifier is already a standard format token (%s or %p)
                # Check if the developer manually added a C++ wrapper macro
                for token, config in TRANSFORMS_MAP.items():
                    wrapper = config['wrapper']
                    replacement = config['replacement']

                    if wrapper and arg_stripped.startswith(f"{wrapper}(") and arg_stripped.endswith(')'):
                        print(f"Info: {file_msg} {CLEAN_MSG}, '{spec}' could be replaced with '%{token}' and the {wrapper}() removed around '{arg_stripped}'", file=sys.stderr)
                        break

        if format_string_modified and not has_errors:
            leading_fmt_spaces = fmt_arg_text[:len(fmt_arg_text)-len(fmt_arg_text.lstrip())]
            trailing_fmt_spaces = fmt_arg_text[len(fmt_arg_text.rstrip()):]

            new_fmt_arg_text = f'{leading_fmt_spaces}"{updated_fmt_str}"{trailing_fmt_spaces}'
            modifications.append((fmt_start, fmt_end, new_fmt_arg_text))

        pos = end_pos

    if has_errors:
        sys.exit(1)

    modifications.sort(key=lambda x: x, reverse=True)

    output_code = list(input_code)
    for start, end, new_text in modifications:
        output_code[start:end] = list(new_text)

    if output_stdout:
        sys.stdout.reconfigure(encoding='utf-8')
        with sys.stdout as f:
            f.write("".join(output_code))
    else:
        with open(output_name, 'w', encoding='utf-8') as f:
            f.write("".join(output_code))

if __name__ == "__main__":
    if len(sys.argv) < 5:
        usage("Not enough arguments")
        sys.exit(1)
    process_file(sys.argv)

