#!/usr/bin/env python3
"""
Usage: python3 run_tests.py ./my_program ./reference_program tests/*
"""
import sys
import subprocess
import os
from pathlib import Path
from colorama import init, Fore, Style

init()

def run_program(prog, input_file):
    """Запускает программу, подавая input_file на stdin. Возвращает stdout."""
    try:
        with open(input_file, 'r') as f:
            result = subprocess.run(
                [prog],
                stdin=f,
                capture_output=True,
                text=True,
                timeout=10
            )
        return result.stdout.strip().splitlines(), None
    except subprocess.TimeoutExpired:
        return None, "TIMEOUT"
    except Exception as e:
        return None, str(e)

def sort_output(lines):
    """Сортирует непустые строки, так как порядок вхождений не важен."""
    return sorted(line.strip() for line in lines if line.strip())

def main():
    if len(sys.argv) < 4:
        print("Usage: python3 run_tests.py <my_program> <reference_program> <test_files...>")
        print("Example: python3 run_tests.py ./aho_corasick ./reference tests/*")
        sys.exit(1)

    my_prog = sys.argv[1]
    ref_prog = sys.argv[2]
    test_files = sys.argv[2:]

    # Отфильтровываем: только существующие обычные файлы, исключаем .out файлы
    tests = []
    for f in test_files:
        p = Path(f)
        if p.is_file() and p.suffix != '.out':
            tests.append(p)

    if not tests:
        print(f"{Fore.YELLOW}No valid test files found.{Style.RESET_ALL}")
        sys.exit(1)

    passed = 0
    failed = 0
    generated = 0

    for test_path in sorted(tests):
        test_name = test_path.name
        out_file = test_path.with_suffix('.out')
        # Если файл без расширения, .out будет просто file.out
        if test_path.suffix == '':
            out_file = Path(str(test_path) + '.out')

        print(f"Test '{test_name}': ", end='', flush=True)

        # Шаг 1: Генерируем эталонный ответ (если .out файла нет)
        if True:
        # if not out_file.exists():
            # print(f"{Fore.RED}ANSWER FILE NOT FOUND: \"{out_file.name}\"")
            print(f"{Fore.YELLOW}GENERATING{Style.RESET_ALL} ", end='', flush=True)
            ref_lines, ref_err = run_program(ref_prog, test_path)
            if ref_lines is None:
                print(f"{Fore.RED}FAIL (reference program error: {ref_err}){Style.RESET_ALL}")
                failed += 1
                continue
            
            # Сохраняем эталонный вывод
            ref_sorted = sort_output(ref_lines)
            with open(out_file, 'w') as f:
                for line in ref_sorted:
                    f.write(line + '\n')
            generated += 1
            print(f"{Fore.GREEN}→ {out_file.name}{Style.RESET_ALL} ", end='', flush=True)

        # Шаг 2: Запускаем мою программу
        my_lines, my_err = run_program(my_prog, test_path)
        if my_lines is None:
            print(f"{Fore.RED}FAIL (your program error: {my_err}){Style.RESET_ALL}")
            failed += 1
            continue

        # Шаг 3: Сравниваем с эталоном
        with open(out_file, 'r') as f:
            expected = [line.strip() for line in f if line.strip()]

        my_sorted = sort_output(my_lines)

        if my_sorted == expected:
            print(f"{Fore.GREEN}PASSED{Style.RESET_ALL}")
            passed += 1
        else:
            print(f"{Fore.RED}FAILED{Style.RESET_ALL}")
            print("  --- Your output:")
            for l in my_sorted:
                print(f"    {l}")
            print("  --- Expected:")
            for l in expected:
                print(f"    {l}")
            # Показываем diff
            print("  --- Diff:")
            import difflib
            diff = difflib.unified_diff(expected, my_sorted, fromfile='expected', tofile='yours')
            for line in diff:
                print(f"    {line}")
            failed += 1

    print(f"\n{Fore.CYAN}Results: {Fore.GREEN}{passed} passed{Fore.CYAN}, {Fore.RED}{failed} failed{Style.RESET_ALL}")
    if generated > 0:
        print(f"{Fore.YELLOW}{generated} reference file(s) generated{Style.RESET_ALL}")
    print(f"Total: {passed + failed}")

    sys.exit(1 if failed > 0 else 0)

if __name__ == "__main__":
    main()