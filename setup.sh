#!/bin/bash
# Extracts binaries for all solvers archived by the SAT heritage initiative.

if [[ ! -d docker-images ]]; then
    git clone https://github.com/sat-heritage/docker-images
fi

cd docker-images || exit
pyenv local 3.12.6
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install -U satex
cd ..

for solver in $(satex list); do
    yes "" | satex extract "$solver" .
done

# old script version that directly worked on the files without satex:
# for year in $(seq 2002 3 2021); do
#     echo -n "Random solver from year $year: "
#     SOLVER_NAME=$(cat "$year/solvers.json" | grep '"name"' | cut -d\" -f4 | shuf -n1)
#     echo "$SOLVER_NAME"
#     download_url=$(cat "$year/setup.json" | grep '"download_url"' | cut -d\" -f4 | shuf -n1)
#     download_url=${download_url//\{SOLVER_NAME\}/$SOLVER_NAME}
#     wget "$download_url" -O "../$year-$SOLVER_NAME"
# done
# for solver in ../*-*; do
#     if [[ ! -s "$solver" ]]; then
#         rm -f "$solver"
#     elif file "$solver" | grep -q "Zip archive data"; then
#         mv "$solver" "$solver.zip"
#         unzip -o "$solver.zip" -d "$solver"
#         rm -f "$solver.zip"
#     else
#         chmod +x "$solver"
#     fi
# done
