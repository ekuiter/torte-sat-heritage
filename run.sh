#!/usr/bin/env bash

solver="$1"
solver_file="$(dirname "$0")/$solver/solvers.json"
file="$2"

read -r solver_name solver_call solver_args < <(
    jq -r '. | to_entries[0].value | [.name, .call, (.args | join(" "))] | @tsv' "$solver_file"
)

solver_name=$(jq -r '. | to_entries[0].value.name' "$solver_file")
solver_call=$(jq -r '. | to_entries[0].value.call' "$solver_file")

if [[ $solver_call != */* ]]; then
  solver_call="./$solver_call"
fi

mapfile -t solver_args < <(
    jq -r '. | to_entries[0].value.args[]' "$solver_file"
)

pushd "$(dirname "$0")/$solver/$solver_name" > /dev/null || exit

FILECNF="$(readlink -f -- "$file")"
FILEPROOF=""
RANDOMSEED=${RANDOMSEED:-1234567}
MAXNBTHREAD=${MAXNBTHREAD:-1}
MEMLIMIT=${MEMLIMIT:-1024}
TIMEOUT=${TIMEOUT:-99999999}

for (( i=0; i<${#solver_args[@]}; ++i )); do
    a="${solver_args[$i]/FILECNF/$FILECNF}"
    a="${a/RANDOMSEED/$RANDOMSEED}"
    a="${a/MAXNBTHREAD/$MAXNBTHREAD}"
    a="${a/MEMLIMIT/$MEMLIMIT}"
    a="${a/TIMEOUT/$TIMEOUT}"
    solver_args[$i]="${a/FILEPROOF/$FILEPROOF}"
done

echo "$solver_call ${solver_args[*]}"
"$solver_call" "${solver_args[@]}"

popd > /dev/null || exit
