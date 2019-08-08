#include <stdio.h>
#include <inttypes.h>

#include <mpi.h>

#include <p4est_connectivity.h>
#include <p6est.h>
#include <p6est_extended.h>
#include <p6est_ghost.h>
#include <p6est_lnodes.h>
#include <p6est_vtk.h>

p4est_connectivity_t *
p4est_connectivity_new_cube_s (void)
{
  const p4est_topidx_t num_vertices = 8;
  const p4est_topidx_t num_trees = 6;
  const p4est_topidx_t num_ctt = 0;
  const double vertices[8 * 3] = {
    -1, -1, -1,
     1, -1, -1,
    -1,  1, -1,
     1,  1, -1,
    -1, -1,  1,
     1, -1,  1,
    -1,  1,  1,
     1,  1,  1,
  };
  const p4est_topidx_t tree_to_vertex[6 * 4] = {
    0, 2, 1, 3,
    2, 6, 3, 7,
    0, 4, 2, 6,
    4, 5, 6, 7,
    0, 1, 4, 5,
    1, 3, 5, 7,
  };
  const p4est_topidx_t tree_to_tree[6 * 4] = {
    4, 1, 2, 5,
    0, 3, 2, 5,
    0, 3, 4, 1,
    2, 5, 4, 1,
    2, 5, 0, 3,
    4, 1, 0, 3,
  };
  const int8_t        tree_to_face[6 * 4] = {
    2, 0, 0, 2,
    1, 3, 3, 1,
    2, 0, 0, 2,
    1, 3, 3, 1,
    2, 0, 0, 2,
    1, 3, 3, 1,
  };

  return p4est_connectivity_new_copy (num_vertices, num_trees, 0,
                                      vertices, tree_to_vertex,
                                      tree_to_tree, tree_to_face,
                                      NULL, &num_ctt, NULL, NULL);
}

int
main (int argc, char **argv)
{
  int ret, rank, size;
  MPI_Comm comm = MPI_COMM_WORLD;

  /* initialize MPI and p4est internals */
  ret = MPI_Init(&argc, &argv); SC_CHECK_MPI(ret);
  ret = MPI_Comm_size(comm, &size); SC_CHECK_MPI(ret);
  ret = MPI_Comm_rank(comm, &rank); SC_CHECK_MPI(ret);

  sc_init(comm, 1, 1, NULL, SC_LP_DEFAULT);
  p4est_init(NULL, SC_LP_DEFAULT);

  p6est_connectivity_t *connectivity;
  {
    p4est_connectivity_t *connectivity4 = p4est_connectivity_new_cube_s();

    p4est_connectivity_t *refineconn4 = p4est_connectivity_refine(connectivity4, 3);

    double *topvertices = malloc(sizeof(double)*refineconn4->num_vertices*3);
    for(p4est_topidx_t i = 0; i < refineconn4->num_vertices*3; ++i) {
      topvertices[i] = 2*refineconn4->vertices[i];
    }

    connectivity = p6est_connectivity_new(refineconn4, topvertices, NULL);

    p4est_connectivity_destroy(refineconn4);
    p4est_connectivity_destroy(connectivity4);
    free(topvertices);
  }

  p6est_t* p6est =
    p6est_new_ext(comm, connectivity, 0, 1, 4, 3, 0, 0, NULL, NULL);

  p6est_partition(p6est, NULL);
  p6est_ghost_t *ghost = p6est_ghost_new(p6est, P4EST_CONNECT_FULL);
  p6est_lnodes_t *lnodes = p6est_lnodes_new(p6est, ghost, 1);

  char filename[BUFSIZ];
  snprintf(filename, BUFSIZ, "rank_%02d.txt", rank);
  FILE *f = fopen(filename, "w");
  if (f == NULL)
  {
    fprintf(stderr, "Error opening file %s!\n", filename);
    exit(1);
  }

  for(p4est_locidx_t e = 0; e < lnodes->num_local_elements; ++e) {
    for(int v = 0; v < lnodes->vnodes; ++v) {
      p4est_locidx_t ln = lnodes->element_nodes[e*lnodes->vnodes + v];

      p4est_gloidx_t gn = (ln < lnodes->owned_count) ?
        lnodes->global_offset + ln :
        lnodes->nonlocal_nodes[ln-lnodes->owned_count];

      fprintf(f, " %2" PRId64, gn);
    }

    fprintf(f, "\n");
  }

  p6est_vtk_write_file(p6est, "cubesphere");

  p6est_lnodes_destroy(lnodes);
  p6est_ghost_destroy(ghost);
  p6est_destroy(p6est);
  p6est_connectivity_destroy(connectivity);
  sc_finalize();

  ret = MPI_Finalize(); SC_CHECK_MPI (ret);

  return 0;
}
