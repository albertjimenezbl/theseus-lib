import argparse


def compute_time_vg(input_file):

    # Simulating some processing on the input file
    with open(input_file, 'r') as file:
        data = file.read()

    # Read all lines of file and add the time values
    execution_time = 0.0
    for line in data.splitlines():
        if line.strip():  # Skip empty lines
            if line.startswith("Local alignment took "):
                time_str = line.split(" ")[3]
                try:
                    time_value = float(time_str)
                    execution_time += time_value
                except ValueError:
                    print(f"Invalid time value in line: {line}")

    # Write the total execution time to terminal
    execution_time /= 1000000 # Convert microseconds to seconds
    print(f'Execution Time: {execution_time} seconds')

if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument('-i', '--input', required=True, help='Input file')

  args = parser.parse_args()
  compute_time_vg(args.input)