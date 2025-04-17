// #pragma once

// #include <vector>

// #include "../../include/theseus/graph.h"
// #include "../../include/theseus/penalties.h"

// namespace theseus {

// class DPAligner {
// public:

//   // Structs
//   struct DP_data
//   {
//     std::vector<std::vector<std::vector<int>>> M_matrices; // Set of M matrices
//     std::vector<std::vector<std::vector<int>>> I_matrices; // Set of I matrices
//     std::vector<std::vector<std::vector<int>>> D_matrices; // Set of D matrices
//   };

//   // Functions
//   /**
//    * @brief TODO:
//    * 
//    * @param G 
//    * @param data 
//    * @param topo_order 
//    * @param v 
//    * @param m 
//    * @param n 
//    */
//   void compute_DP_first_column(
//       Graph &G,
//       DP_data &data,
//       std::vector<int> &topo_order,
//       int v,
//       int m,
//       int n)
//   {

//       std::vector<int> in_vertices = G._vertices[v].in_vertices;
//       int v_pos = topo_order[v];

//       // Compute initial scores
//       for (int i = 0; i <= m; ++i)
//       {
//           for (int k = 0; k < in_vertices.size(); ++k)
//           {
//               int v_pos_in = topo_order[in_vertices[k]];
//               int last_column_in = G._vertices[in_vertices[k]].value.size();

//               // I
//               int prev_score_I = data.I_matrices[v_pos_in][last_column_in][i];
//               data.I_matrices[v_pos][0][i] = std::min(data.I_matrices[v_pos][0][i], prev_score_I);

//               // D
//               int prev_score_D = data.D_matrices[v_pos_in][last_column_in][i];
//               data.D_matrices[v_pos][0][i] = std::min(data.D_matrices[v_pos][0][i], prev_score_D);

//               // M
//               int prev_score_M = data.M_matrices[v_pos_in][last_column_in][i];
//               data.M_matrices[v_pos][0][i] = std::min(data.M_matrices[v_pos][0][i], prev_score_M);
//           }
//       }
//   }

//   // Compute initial conditions
//   void compute_DP_first_column_start_v(
//       Graph &G,
//       DP_data &data,
//       std::vector<int> &topo_order,
//       int v,
//       int m,
//       int n)
//   {

//       std::vector<int> in_vertices = G._vertices[v].in_vertices;
//       int v_pos = topo_order[v];

//       // Compute initial scores
//       data.M_matrices[v_pos][0][0] = 0; // i == 0
//       for (int i = 1; i <= m; ++i)
//       {
//           // I will have the default value
//           // D
//           data.D_matrices[v_pos][0][i] = std::min(data.M_matrices[v_pos][0][i - 1] + INDEL_OPEN + INDEL_EXTEND, data.D_matrices[v_pos][0][i - 1] + INDEL_EXTEND);

//           // M
//           data.M_matrices[v_pos][0][i] = data.D_matrices[v_pos][0][i];
//       }
//   }

//   void compute_I_value(
//       DP_data &data,
//       int v,
//       int v_pos,
//       int m,
//       int n,
//       int i,
//       int j)
//   {

//       int prev_M = data.M_matrices[v_pos][j - 1][i];
//       int prev_I = data.I_matrices[v_pos][j - 1][i];
//       data.I_matrices[v_pos][j][i] = std::min(prev_I + INDEL_EXTEND, prev_M + INDEL_OPEN + INDEL_EXTEND);
//   }

//   void compute_D_value(
//       DP_data &data,
//       int v,
//       int v_pos,
//       int m,
//       int n,
//       int i,
//       int j)
//   {

//       if (i > 0)
//       {
//           int prev_M = data.M_matrices[v_pos][j][i - 1];

//           if (i > 1)
//           { // Value of prev_D can be used
//               int prev_D = data.D_matrices[v_pos][j][i - 1];
//               data.D_matrices[v_pos][j][i] = std::min(prev_D + INDEL_EXTEND, prev_M + INDEL_OPEN + INDEL_EXTEND);
//           }
//           else
//           { // i == 1
//               data.D_matrices[v_pos][j][i] = prev_M + INDEL_OPEN + INDEL_EXTEND;
//           }
//       }
//   }

//   void compute_M_value(
//       Graph &G,
//       std::string &seq,
//       DP_data &data,
//       int v,
//       int v_pos,
//       int m,
//       int n,
//       int i,
//       int j)
//   {

//       int curr_I = data.I_matrices[v_pos][j][i];
//       int curr_D = data.D_matrices[v_pos][j][i];

//       if (i > 0)
//       {
//           int increment;
//           if (G.vertices[v].value[j - 1] == seq[i - 1])
//               increment = MATCH;
//           else
//               increment = MISMATCH;

//           int prev_M = data.M_matrices[v_pos][j - 1][i - 1];
//           data.M_matrices[v_pos][j][i] = std::min(std::min(curr_I, curr_D), prev_M + increment);
//       }
//       else
//       { // i == 0
//           data.M_matrices[v_pos][j][i] = std::min(curr_I, curr_D);
//       }
//   }

//   // Compute DP vertex
//   void compute_DP_vertex(
//       Graph &G,
//       std::string &seq,
//       DP_data &data,
//       int v,
//       int m,
//       int n)
//   {

//       // Compute initial conditions
//       if (v != 0)
//       {
//           compute_DP_first_column(G, data, v, m, n);
//       }
//       else
//       {
//           compute_DP_first_column_start_v(G, data, v, m, n);
//       }

//       // Compute full DP matrix column by column
//       int v_pos = G.vertices[v].pos;
//       for (int j = 1; j <= n; ++j)
//       {
//           for (int i = 0; i <= m; ++i)
//           {
//               compute_I_value(data, v, v_pos, m, n, i, j);         // Compute I
//               compute_D_value(data, v, v_pos, m, n, i, j);         // Compute D
//               compute_M_value(G, seq, data, v, v_pos, m, n, i, j); // Compute M
//           }
//       }
//   }

//   // Compute wavefronts
//   void DP_compute_alignment(
//       Graph &G,
//       DP_data &data,
//       std::string &seq,
//       int m)
//   {

//       // Process all vertices
//       for (int k = 0; k < G.vertices.size(); ++k)
//       {
//           int v = k;
//           int n = G.vertices[v].value.size();

//           // Allocate the necessary memory
//           std::vector<std::vector<int>> M(n + 1, std::vector<int>(m + 1, INT_MAX / 2));
//           std::vector<std::vector<int>> I(n + 1, std::vector<int>(m + 1, INT_MAX / 2));
//           std::vector<std::vector<int>> D(n + 1, std::vector<int>(m + 1, INT_MAX / 2));
//           data.M_matrices.push_back(M);
//           data.I_matrices.push_back(I);
//           data.D_matrices.push_back(D);

//           G.vertices[v].pos = k;                    // Store v_pos in the graph
//           compute_DP_vertex(G, seq, data, v, m, n); // Compute the DP matrices of vertex v
//       }
//   }

//   void backtrace_inside_vertex_M(
//       Graph &G,
//       position &pos,
//       DP_data &data,
//       backtrack_seq &back,
//       std::string &seq)
//   {

//       int v_pos = G.vertices[pos.v].pos;
//       int curr_score = data.M_matrices[v_pos][pos.j][pos.i];

//       // Check the three options D, I and M
//       if (curr_score == data.D_matrices[v_pos][pos.j][pos.i])
//       { // DELETION
//           pos.matrix = 'D';
//       }
//       else if (curr_score == data.I_matrices[v_pos][pos.j][pos.i])
//       { // INSERTION
//           pos.matrix = 'I';
//       }
//       else if (pos.i > 0 && curr_score == data.M_matrices[v_pos][pos.j - 1][pos.i - 1] + MISMATCH)
//       { // MISMATCH
//           pos.i = pos.i - 1;
//           pos.j = pos.j - 1;
//           pos.score = pos.score - MISMATCH;
//           back.edit_op.push_back('X');
//           back.rec_seq.push_back(G.vertices[pos.v].value[pos.j]);
//       }
//       else if (pos.i > 0 && curr_score == data.M_matrices[v_pos][pos.j - 1][pos.i - 1] + MATCH)
//       { // MATCH
//           pos.i = pos.i - 1;
//           pos.j = pos.j - 1;
//           pos.score = pos.score - MATCH;
//           back.edit_op.push_back('M');
//           back.rec_seq.push_back(seq[pos.i]);
//       }
//   }

//   void backtrace_inside_vertex_D(
//       Graph &G,
//       position &pos,
//       DP_data &data,
//       backtrack_seq &back,
//       std::string &seq)
//   {

//       int v_pos = G.vertices[pos.v].pos;
//       int curr_score = data.D_matrices[v_pos][pos.j][pos.i];

//       // Check the two options D and M
//       if (curr_score == data.M_matrices[v_pos][pos.j][pos.i - 1] + INDEL_OPEN + INDEL_EXTEND)
//       {
//           pos.i = pos.i - 1;
//           pos.score = pos.score - INDEL_OPEN - INDEL_EXTEND;
//           pos.matrix = 'M';
//           back.edit_op.push_back('D');
//       }
//       else if (curr_score == data.D_matrices[v_pos][pos.j][pos.i - 1] + INDEL_EXTEND)
//       {
//           pos.i = pos.i - 1;
//           pos.score = pos.score - INDEL_EXTEND;
//           back.edit_op.push_back('D');
//       }
//   }

//   void backtrace_inside_vertex_I(
//       Graph &G,
//       position &pos,
//       DP_data &data,
//       backtrack_seq &back,
//       std::string &seq)
//   {

//       int v_pos = G.vertices[pos.v].pos;
//       int curr_score = data.I_matrices[v_pos][pos.j][pos.i];

//       // Check the two options I and M
//       if (curr_score == data.M_matrices[v_pos][pos.j - 1][pos.i] + INDEL_OPEN + INDEL_EXTEND)
//       {
//           pos.j = pos.j - 1;
//           pos.score = pos.score - INDEL_OPEN - INDEL_EXTEND;
//           pos.matrix = 'M';
//           back.rec_seq.push_back(G.vertices[pos.v].value[pos.j]);
//           back.edit_op.push_back('I');
//       }
//       else if (curr_score == data.I_matrices[v_pos][pos.j - 1][pos.i] + INDEL_EXTEND)
//       {
//           pos.j = pos.j - 1;
//           pos.score = pos.score - INDEL_EXTEND;
//           back.rec_seq.push_back(G.vertices[pos.v].value[pos.j]);
//           back.edit_op.push_back('I');
//       }
//   }

//   // Backtrace inside of a vertex
//   void backtrace_inside_vertex(
//       Graph &G,
//       position &pos,
//       DP_data &data,
//       backtrack_seq &back,
//       std::string &seq)
//   {

//       if (pos.matrix == 'I')
//           backtrace_inside_vertex_I(G, pos, data, back, seq);
//       else if (pos.matrix == 'M')
//           backtrace_inside_vertex_M(G, pos, data, back, seq);
//       else if (pos.matrix == 'D')
//           backtrace_inside_vertex_D(G, pos, data, back, seq);
//       ;
//   }

//   // Backtrace in a jump when in M matrix: where do I come from?
//   void backtrace_jump_M(
//       Graph &G,
//       position &pos,
//       DP_data &data #include "../../include/theseus/graph.h",
//       backtrack_seq &back)
//   {

//       // Consider first coming from a deletion
//       int curr_v_pos = G.vertices[pos.v].pos;
//       int curr_value = data.M_matrices[curr_v_pos][pos.j][pos.i];
//       if (curr_value == data.D_matrices[curr_v_pos][pos.j][pos.i])
//       {
//           pos.matrix = 'D';
//       }
//       else if (curr_value == data.I_matrices[curr_v_pos][pos.j][pos.i])
//       {
//           pos.matrix = 'I';
//       }
//       else
//       { // You come from a jump
//           std::vector<int> in_vertices = G.vertices[pos.v].in_vertices;
//           for (int k = 0; k < in_vertices.size(); ++k)
//           {
//               int new_v = in_vertices[k];
//               int new_j = G.vertices[new_v].value.size();
//               int prev_v_pos = G.vertices[new_v].pos;
//               int prev_value = data.M_matrices[prev_v_pos][new_j][pos.i];
//               if (curr_value == prev_value)
//               {
//                   back.path.push_back(new_v);
//                   pos.v = new_v;
//                   pos.j = new_j;
//                   break;
//               }
//           }
//       }
//   }

//   // Backtrace in a jump when in I matrix: where do I come from?
//   void backtrace_jump_I(
//       Graph &G,
//       position &pos,
//       DP_data &data,
//       backtrack_seq &back)
//   {

//       std::vector<int> in_vertices = G.vertices[pos.v].in_vertices;
//       for (int k = 0; k < in_vertices.size(); ++k)
//       {
//           int new_v = in_vertices[k];
//           int new_j = G.vertices[new_v].value.size();
//           int curr_v_pos = G.vertices[pos.v].pos;
//           int prev_v_pos = G.vertices[new_v].pos;
//           int curr_value = data.I_matrices[curr_v_pos][pos.j][pos.i];
//           int prev_value = data.I_matrices[prev_v_pos][new_j][pos.i];
//           if (curr_value == prev_value)
//           {
//               back.path.push_back(new_v);
//               pos.v = new_v;
//               pos.j = new_j;
//               break;
//           }
//       }
//   }

//   // Backtrace in a jump: where do I come from?
//   void backtrace_jump(
//       Graph &G,
//       position &pos,
//       DP_data &data,
//       backtrack_seq &back,
//       std::string &seq,
//       int m)
//   {

//       if (pos.matrix == 'I')
//           backtrace_jump_I(G, pos, data, back);
//       else if (pos.matrix == 'M')
//           backtrace_jump_M(G, pos, data, back);
//       else if (pos.matrix == 'D')
//           backtrace_inside_vertex_D(G, pos, data, back, seq);
//   }

//   // Compute wavefronts
//   void backtrace_affine_gap_DP(
//       Graph &G,
//       DP_data &data,
//       backtrack_seq &back,
//       std::string &seq,
//       int m,
//       int end_vertex)
//   {

//       // Set start position (end vertex and row)
//       position pos;
//       pos.v = end_vertex;
//       pos.i = m;
//       pos.j = 0;
//       pos.score = data.M_matrices[G.vertices[pos.v].pos][pos.j][pos.i];
//       pos.matrix = 'M';
//       back.path.push_back(end_vertex); // Store the last vertex

//       // Recover the backtrace of the alignment (the CIGAR)
//       while (pos.v != 0)
//       {
//           if (pos.j == 0)
//           {
//               backtrace_jump(G, pos, data, back, seq, m);
//           }
//           else
//           {
//               backtrace_inside_vertex(G, pos, data, back, seq);
//           }
//       }

//       // Complete the necessary D's
//       while (pos.i > 0)
//       { // Check correctness of score?
//           back.edit_op.push_back('D');
//           pos.i = pos.i - 1;
//       }

//       std::reverse(back.edit_op.begin(), back.edit_op.end());
//       std::reverse(back.rec_seq.begin(), back.rec_seq.end());
//       std::reverse(back.path.begin(), back.path.end());

//       // for (int i = 0; i < back.edit_op.size(); ++i) {
//       //   if (back.edit_op[i] != 'M') {
//       //     std::cout << back.edit_op[i] << " in position " << i << std::endl;
//       //   }
//       // }
//   }
// }