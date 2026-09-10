"""Use the real library inline functions with host MMIO substitutes."""
import pathlib
import re
import sys

source = pathlib.Path(sys.argv[1]).read_text()
functions = re.findall(r"static inline void vgs_mode7_\w+\([^)]*\)\n\{.*?\n\}", source, re.S)
if len(functions) != 8:
    raise SystemExit("Expected eight Mode 7 APIs; update the host test extraction")
pathlib.Path(sys.argv[2]).write_text("\n\n".join(functions) + "\n")
