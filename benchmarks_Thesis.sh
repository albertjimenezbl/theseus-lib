#!/bin/bash

DATASET_PATH=datasets/MSA/amphora_genes
OUTPUT_PATH=results
GENES=(dnaG nusA pgk pyrG rpoB)

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

if [ ! -d $OUTPUT_PATH ]; then
    mkdir -p $OUTPUT_PATH
fi

# Get the times
set=0
for gene in "${GENES[@]}"; do
  echo "Processing gene $gene"
  for folder in $SCRIPT_DIR/$DATASET_PATH/$gene/*; do
    # General work
    if [ ! -f  $folder/align_seq.fna ]; then
      gzip -d $folder/align_seq.fna.gz  # Unzip the file
    fi
    FILE_PATH=$folder/align_seq.fna

    echo "Processing set $set"
    # Execute SPOA
    # echo "SPOA"
    # \time -p $SCRIPT_DIR/bin/spoa -m 0 -n -2 -g -3 -e -1 -l 1 -r 1 -d output.txt $FILE_PATH 2>&1 >/dev/null | tail -n 3
    # Execute POASTA
    echo "POASTA"
    \time -p $SCRIPT_DIR/SOTA/poasta/target/release/poasta align -o graph.poasta $FILE_PATH 2>&1 >/dev/null | tail -n 3
    # Execute Teresa DAG (segments)
    echo "Theseus DAG diagonals"
    \time -p $SCRIPT_DIR/build/tests/interactive/align_msa_benchmark -s $FILE_PATH 2>&1 >/dev/null | tail -n 3
    # Execute Theseus voracious
    # echo "Theseus voracious"
    # \time -p $SCRIPT_DIR/bin/theseus_voracious -s $FILE_PATH -f fasta -o ./results/MSA/align_seq.al 2>&1 >/dev/null | tail -n 3
    # # Execute DP
    # echo "DP"
    # \time -p $SCRIPT_DIR/bin/DP_gap_affine -s $FILE_PATH -f fasta -o ./results/MSA/align_seq.al 2>&1 >/dev/null | tail -n 3
    # echo "DP_checker"
    # $SCRIPT_DIR/bin/DP_checker -s $FILE_PATH -f fasta -o ./results/MSA/align_seq.al
    set=$((set+1))
  done
done