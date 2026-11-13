"""Small lexer for trusted checked-in migration SQL; no database execution."""
def statements(text):
    result, current = [], []
    quote = None
    i = 0
    while i < len(text):
        char = text[i]
        if quote:
            current.append(char)
            if char == '\\' and quote != '`' and i + 1 < len(text):
                i += 1
                current.append(text[i])
            elif char == quote:
                if i + 1 < len(text) and text[i + 1] == quote:
                    i += 1
                    current.append(text[i])
                else:
                    quote = None
        elif char in "'\"`":
            quote = char
            current.append(char)
        elif text.startswith('--', i) or char == '#':
            while current and current[-1] in (' ', '\t'):
                current.pop()
            end = text.find('\n', i)
            i = len(text) if end < 0 else end
            current.append('\n')
            continue
        elif text.startswith('/*', i):
            end = text.find('*/', i + 2)
            if end < 0:
                raise ValueError('Unterminated SQL comment')
            i = end + 2
            current.append(' ')
            continue
        elif char == ';':
            if ''.join(current).strip():
                result.append(''.join(current).strip())
            current = []
        else:
            current.append(char)
        i += 1
    if quote:
        raise ValueError('Unterminated SQL quote')
    if ''.join(current).strip():
        result.append(''.join(current).strip())
    return result


def comma_values(text):
    result, start, depth, quote, i = [], 0, 0, None, 0
    while i < len(text):
        char = text[i]
        if quote:
            if char == '\\' and quote != '`':
                i += 1
            elif char == quote:
                if i + 1 < len(text) and text[i + 1] == quote:
                    i += 1
                else:
                    quote = None
        elif char in "'\"`":
            quote = char
        elif char == '(':
            depth += 1
        elif char == ')':
            depth -= 1
        elif char == ',' and depth == 0:
            result.append(text[start:i].strip())
            start = i + 1
        i += 1
    if quote or depth:
        raise ValueError('Unbalanced SQL values')
    result.append(text[start:].strip())
    return result
