#!/bin/bash
# One-off setup for the ENG3014 notebooks on this Mac.
#
#   1. Creates a Python virtual environment in ~/.venvs/eng3014. It lives
#      outside OneDrive on purpose: OneDrive syncs the thousands of files in a
#      venv badly and can make them "online-only", which breaks imports.
#   2. Installs the packages listed in requirements.txt.
#   3. Registers the environment as a Jupyter kernel called "ENG3014 (Python)",
#      so VS Code lists it under Select Kernel.
#
# Run it from this folder in VS Code's terminal:   bash setup_env.sh
# Running it again is safe: it reuses the environment and updates the packages.

set -e
cd "$(dirname "$0")"
VENV="$HOME/.venvs/eng3014"

# Use the newest Python 3 on this Mac
PY=""
for candidate in python3.13 python3.12 python3.11 python3.10 python3; do
    if command -v "$candidate" >/dev/null 2>&1; then
        PY="$candidate"
        break
    fi
done
if [ -z "$PY" ]; then
    echo "No Python 3 found. Install it from python.org, then run this again."
    exit 1
fi
echo "Using $("$PY" --version) from $(command -v "$PY")"

if [ ! -x "$VENV/bin/python" ]; then
    echo "Creating the environment in $VENV ..."
    mkdir -p "$(dirname "$VENV")"
    "$PY" -m venv "$VENV"
fi

echo "Installing packages (this takes a minute the first time) ..."
"$VENV/bin/python" -m pip install --quiet --upgrade pip
"$VENV/bin/python" -m pip install --quiet -r requirements.txt

"$VENV/bin/python" -m ipykernel install --user --name eng3014 --display-name "ENG3014 (Python)"

echo ""
echo "All set: $("$VENV/bin/python" --version), numpy $("$VENV/bin/python" -c 'import numpy; print(numpy.__version__)')"
echo "In VS Code, open channel_simulation.ipynb, click Select Kernel (top right)"
echo "and choose Jupyter Kernel... > ENG3014 (Python)."
