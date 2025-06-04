import sys
import re

def gfa_to_dot(gfa_file, dot_file):
  with open(gfa_file, 'r') as gfa, open(dot_file, 'w') as dot:
    dot.write("digraph G {\n")

    for line in gfa:
      if line.startswith('S'):  # Segment line
        parts = re.split(r'[ \t]+', line.strip())
        node_id = parts[1]
        node_label = parts[2]
        dot.write(f'{node_id} [label=\"{node_label}\"]\n')
      elif line.startswith('L'):  # Link line
        parts = re.split(r'[ \t]+', line.strip())
        from_node = parts[1]
        to_node = parts[3]
        dot.write(f'{from_node} -> {to_node};\n')

    dot.write("}\n")

if __name__ == "__main__":
  if len(sys.argv) != 3:
    print("Usage: python gfa_to_dot.py <input.gfa> <output.dot>")
    sys.exit(1)

  gfa_file = sys.argv[1]
  dot_file = sys.argv[2]
  gfa_to_dot(gfa_file, dot_file)