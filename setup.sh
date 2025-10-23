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
