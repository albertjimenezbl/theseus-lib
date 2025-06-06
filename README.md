# Theseus-lib

## 1. Introduction

### 1.1. What is Theseus?
Theseus-lib is an optimized library implementing the ideas from the Theseus algorithm []. Theseus, in its turn, is a fast, scalable and exact affine-gap sequence-to-graph aligner, based on the WaveFront Alignment algorithm (WFA) []. Notably, Theseus extends the ideas behind WFA's success to the sequence-to-graph framework. In this regard, it also behaves as an output-sensitive algorithm and, thus, its performance really shines when the aligned data shows high levels of similarity. Theseus has been designed as a general purpose aligner, with tow main functionalities:
1. Multiple Sequence Alignment (MSA): Theseus is capable of performing MSA based on the Partial Order Alignment (POA) approach. That is, the user can provide a set of sequences and Theseus will progressively build a POA graph representing the variation between them in a compact manner.
2. Mapping to a graph: Alternatively, Theseus can be provided with a graph, a sequence and a starting position and perform global alignment of the given sequence against the given graph, starting at the provided initial position.

// Add Theseus image

### 1.2. What is WFA?
TODO: Copiado literalmente de la librería de Santiago. Lo tendría que cambiar o no cuenta como plagio jaja?
The wavefront alignment (WFA) algorithm is an exact gap-affine algorithm that takes advantage of homologous regions between the sequences to accelerate the alignment process. Unlike traditional dynamic programming algorithms that run in quadratic time complexity, the WFA runs in time O(ns+s^2), proportional to the sequence length n and the alignment score s, using O(s^2) memory (or O(s) using the ultralow/BiWFA mode). Moreover, the WFA algorithm exhibits simple computational patterns that the modern compilers can automatically vectorize for different architectures without adapting the code. To intuitively illustrate why the WFA algorithm is so interesting, look at the following figure. The left panel shows the cells computed by a classical dynamic programming based algorithm (like Smith-Waterman or Needleman Wunsch). In contrast, the right panel shows the cells computed by the WFA algorithm to obtain the same result (i.e., the optimal alignment)

// Add Images

### 1.3. Getting started
Git clone and compile the library, tools, and examples (by default, use `cmake` for the library and benchmark build).

```
git clone https://github.com/
cd theseus-lib
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

If you need to restart the process, you can execute again the `cmake` file and compile the code with `make`.
```
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

## 2. Using Theseus

### 2.1. Multiple Sequence Alignment (MSA)
The problem of MSA consists on comparing a set of sequences and find an alignment for all of them. Given m sequences of length n, finding the optimal alignment of all sequences has an algorithmic cost of O(n^m). For this reason, practical implementations of MSA use several strategies to find good enough solutions to this problem. Lee [] proposed in 2003 the POA approach, which utilizes a Directed Acyclic Graph (DAG) to represent the diverse alignments, compacting redundant information. Its proposal, extended the Smith-Waterman algorithm of sequence-to-sequence alignment to the context of DAGs, converting the MSA problem into a problem of progressively aligning more sequences to the graph reference. At each step, this approach essentially aligns a new sequence to the current DAG, which encapsulates the variation in the sequences processed so far. Theseus is able to perform such an MSA faster, taking advantage of the expected high similarity between the aligned sequences.

CREATING THE MSA ALIGNER:


ALIGNING A NEW SEQUENCE TO THE POA GRAPH:


VISUALIZING THE RESULTS IN FASTA FORMAT:


### 2.2. Mapping a sequence to a graph



## <a name="theseus.other.notes"></a> 3. Some technical notes

- Thanks to Eizenga's formulation, Theseus-lib can operate with any match score. In practice, M=0 is often the most efficient choice.


## <a name="theseus.bugs"></a> 4. REPORTING BUGS AND FEATURE REQUEST

Feedback and bug reporting is highly appreciated. Please report any issue or suggestion on github or email to the main developer (albert.jimenez1@bsc.es). Don't hesitate to contact us
if:
  - You experience any bug or crash.
  - You want to request a feature or have any suggestion.
  - Your application using the library is running slower than it should or you expected.
  - Need help integrating the library into your tool.

## <a name="theseus.licence"></a> 5. LICENSE

Theseus-lib is distributed under MIT licence.

## <a name="theseus.authors"></a> 6. AUTHORS

[Santiago Marco-Sola](https://github.com/smarco) (santiagomsola@gmail.com) is the main developer and the person you should address your complaints.

[Andrea Guarracino](https://github.com/AndreaGuarracino) and [Erik Garrison](https://github.com/ekg) have contributed to the design of new features and intensive testing of the library.

[Pjotr Prins](https://thebird.nl/) contributed the CMake build system, preventing of leaking variables in include headers and other tweaks.

Miquel Moretó has contributed with fruitful technical discussions and tireless efforts seeking funding, so we could keep working on this project.

## <a name="theseus.ack"></a> 7. ACKNOWLEDGEMENTS

???

## <a name="theseus.cite"></a> 8. CITATION

**Albert Jimenez-Blanco, Lorien Lopez-, Juan Carlos Moure, Miquel Moreto, Santiago Marco-Sola**. ["Fast, Scalable and Affine-Gap Sequence to Graph
Alignment using Theseus"](). , 2025.