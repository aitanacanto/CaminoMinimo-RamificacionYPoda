"""Comprueba el coste mínimo contra los casos de referencia conservados."""
import argparse
from pathlib import Path
import subprocess

def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--binary", type=Path, required=True)
    p.add_argument("--timeout", type=float, default=10)
    a = p.parse_args()
    binary = a.binary.resolve()
    if not binary.is_file():
        p.error("No existe el ejecutable; compila primero")
    failures = 0
    maps = sorted((Path(__file__).parent / "data").glob("*.map"))
    if not maps:
        p.error("No hay casos de prueba")
    for case in maps:
        expected = Path(str(case) + ".sol_bb").read_text().splitlines()[0].strip()
        try:
            result = subprocess.run([str(binary), "-f", str(case)], capture_output=True,
                                    text=True, timeout=a.timeout, check=True)
            actual = result.stdout.splitlines()[0].strip()
            ok = actual == expected
            print(f"{'OK' if ok else 'FALLO'} {case.name}: {actual} / {expected}")
            failures += int(not ok)
        except (subprocess.SubprocessError, IndexError) as error:
            failures += 1
            print(f"FALLO {case.name}: {error}")
    raise SystemExit(1 if failures else 0)

if __name__ == "__main__":
    main()
