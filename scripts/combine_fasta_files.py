import os
import re
import shutil
import zipfile
import argparse
from pathlib import Path

def decompress_gzip(input_file, output_file):
    with gzip.open(input_file, 'rb') as f_in:
        with open(output_file, 'wb') as f_out:
            shutil.copyfileobj(f_in, f_out)

def decompress_zip(input_file, output_folder):
    with zipfile.ZipFile(input_file, 'r') as zip_ref:
        zip_ref.extractall(output_folder)

def is_compressed_file(filepath):
    compressed_extensions = {".gz", ".zip"}
    return Path(filepath).suffix.lower() in compressed_extensions

def concatenate_fasta_files(input_folder, output_file):
    if Path(input_folder).suffix.lower() == ".zip":
      # If the input is a zip file, extract it first
      decompress_zip(input_folder, input_folder + "_decompressed")
      input_path = Path(input_folder + "_decompressed")
    elif Path(input_folder).suffix.lower() == ".gz":
      # If the input is a gzip file, decompress it first
      decompress_gzip(input_folder, input_folder + "_decompressed")
      input_path = Path(input_folder + "_decompressed")
    else:
      input_path = Path(input_folder)

    if not input_path.is_dir():
        raise ValueError(f"The input path '{input_folder}' is not a valid directory.")

    if not input_path.exists():
        raise ValueError(f"The input path '{input_folder}' does not exist.")

    # Ensure the output directory exists
    output_dir = Path(output_file).parent
    os.makedirs(output_dir, exist_ok=True)  # Create directory if it doesn't exist

    fasta_files = list(input_path.rglob("*.fna"))
    with open(output_file, 'w') as outfile:
        for fasta_file in fasta_files:
            with open(fasta_file, 'r') as infile:
                contents = infile.read()
                outfile.write(contents)
                if not contents.endswith('\n'):
                    outfile.write('\n')  # Ensure newline between files

    print(f"Concatenated {len(fasta_files)} FASTA files into '{output_file}'.")

    if (input_path != Path(input_folder)):
        # Clean up the decompressed folder if it was created (the file ends with _decompressed)
        shutil.rmtree(input_path)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Concatenate all .fasta files in a folder into one file.")
    parser.add_argument("input_folder", help="Path to the folder containing .fasta files")
    parser.add_argument("output_file", help="Path to the output .fasta file")

    args = parser.parse_args()

    concatenate_fasta_files(args.input_folder, args.output_file)
