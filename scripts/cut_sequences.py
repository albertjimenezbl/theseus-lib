import argparse
from Bio import SeqIO

def cut_sequences(input_file, output_file, n):
  with open(input_file, "r") as infile, open(output_file, "w") as outfile:
    for record in SeqIO.parse(infile, "fasta"):
      record.seq = record.seq[:n]  # Cut the sequence to the first n characters
      SeqIO.write(record, outfile, "fasta")

if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument('-i', '--input', required=True, help='Input file')
  parser.add_argument('-o', '--output', required=True, help='Output file')
  parser.add_argument('-n', '--length', required=True, type=int, help='Number of base pairs to cut')

  args = parser.parse_args()
  cut_sequences(args.input, args.output, args.length)