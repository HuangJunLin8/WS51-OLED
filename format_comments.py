"""
Comment formatting script for C/H files.

Rules:
1. Inline comments: /* text */ → // text  (with space after //)
2. Multi-line /* ... */ and /** ... */ → keep as-is
3. Section dividers: /* ===...=== Title ===...=== */ → // -------------------- Title --------------------
4. #endif /* TAG */ → #endif // TAG
5. // without space → // with space (for comments, not commented-out code)
"""

import re
import os


def process_line(line):
    """Process a single line, returning the formatted version."""
    orig = line

    # ---- Rule 4: #endif /* TAG */ → #endif // TAG ----
    m = re.match(r'^(\s*#endif)\s+/\*\s*(.*?)\s*\*/\s*$', line)
    if m:
        return f'{m.group(1)} // {m.group(2)}'

    # ---- Rule 3: Section dividers with = or - signs ----
    # Matches: /* ========== text ========= */ or /* --- text --- */ etc.
    # The pattern: /*  separator_chars  title_text  separator_chars  */
    m = re.match(r'^(\s*)/\*\s*([=\-]{3,})\s*(.+?)\s*\2\s*\*/\s*$', line)
    if m:
        indent = m.group(1)
        title = m.group(3).strip()
        return f'{indent}// -------------------- {title} --------------------'

    # ---- Rule 1: Standalone single-line /* text */ → // text ----
    # Must start with optional whitespace, then /*, then content, then */, then optional whitespace EOL
    m = re.match(r'^(\s*)/\*\s(.+?)\s\*/\s*$', line)
    if m:
        # Skip if it starts with /** (doc comment start — multi-line)
        if line.strip().startswith('/**'):
            return line
        indent = m.group(1)
        comment = m.group(2)
        return f'{indent}// {comment}'

    # ---- Rule 1: Trailing inline /* text */ after code → // text ----
    # Line has code, then /* comment */ at end
    # Careful: don't match /* inside string literals
    m = re.match(r'^(.*\S)\s+/\*\s(.+?)\s\*/\s*$', line)
    if m:
        code_part = m.group(1)
        comment = m.group(2)
        # Skip if comment looks like a divider
        if re.match(r'^[=\-]{3,}$', comment):
            return line
        return f'{code_part} // {comment}'

    return line


def format_file(filepath):
    """Read, process, and write back a single C/H file."""
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    lines = content.split('\n')
    result = []

    in_multiline = False
    for line in lines:
        # Track multi-line comments
        stripped = line.strip()

        # Check if this line starts a multi-line comment (/* without */)
        has_start = '/*' in line
        has_end = '*/' in line

        if has_start and not has_end:
            in_multiline = True
            result.append(line)
            continue
        elif has_end and not has_start:
            in_multiline = False
            result.append(line)
            continue
        elif has_start and has_end:
            # Single-line comment — process it
            result.append(process_line(line))
        else:
            result.append(line)

    # Write back using the same encoding
    with open(filepath, 'w', encoding='utf-8', newline='') as f:
        f.write('\n'.join(result))

    return True


def main():
    base_dir = r'E:\电路\WS51F6240\Code\04.OLED\OLED_IIC'

    files_processed = 0
    for root, dirs, files in os.walk(base_dir):
        for f in files:
            if f.endswith('.c') or f.endswith('.h'):
                filepath = os.path.join(root, f)
                # Skip this script and any build artifacts
                if 'format_comments' in f:
                    continue
                print(f'Processing: {os.path.relpath(filepath, base_dir)}')
                format_file(filepath)
                files_processed += 1

    print(f'\nDone. {files_processed} files processed.')


if __name__ == '__main__':
    main()
