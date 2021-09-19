
// mpi header
#include <mpi.h>

#include <op_lib_c.h>
#include <op_lib_mpi.h>
#include <op_util.h>

#include <op_mpi_core.h>

int** aug_part_range;
int* aug_part_range_size;
int* aug_part_range_cap;

int*** foreign_aug_part_range;
int** foreign_aug_part_range_size;

halo_list* OP_export_part_range_list;
halo_list* OP_import_part_range_list;

halo_list* OP_aug_export_exec_lists[10];  // 10 levels for now
halo_list* OP_aug_import_exec_lists[10];  // 10 levels for now


void print_array(int* arr, int size, const char* name, int my_rank){
  for(int i = 0; i < size; i++){
    printf("array my_rank=%d name=%s size=%d value[%d]=%d\n", my_rank, name, size, i, arr[i]);
  }
}

void print_halo(halo_list h_list, const char* name, int my_rank){
  printf("print_halo my_rank=%d, name=%s\n", my_rank, name);
  printf("print_halo my_rank=%d, name=%s, set=%s, size=%d\n", my_rank, name, h_list->set->name, h_list->size);

  for(int i = 0; i < h_list->ranks_size; i++){
    printf("print_halo my_rank=%d, name=%s, to_rank=%d, size=%d, disp=%d start>>>>>>>>>\n", my_rank, name, h_list->ranks[i], h_list->sizes[i], h_list->disps[i]);

    for(int j = h_list->disps[i]; j < h_list->disps[i] + h_list->sizes[i]; j++){
      printf("print_halo my_rank=%d, name=%s, to_rank=%d, size=%d, disp=%d value[%d]=%d\n", 
      my_rank, name, h_list->ranks[i], h_list->sizes[i], h_list->disps[i], j, h_list->list[j]);
    }

    printf("print_halo my_rank=%d, name=%s, to_rank=%d, size=%d, disp=%d end<<<<<<<<<<\n", my_rank, name, h_list->ranks[i], h_list->sizes[i], h_list->disps[i]);
  }
}

void print_aug_part_list(int** aug_range, int* aug_size, int my_rank){
  for(int i = 0; i < OP_set_index; i++){
    for(int j = 0; j < aug_size[i]; j++){
      printf("aug_range my_rank=%d set=%s size=%d value[%d]=%d\n", my_rank, OP_set_list[i]->name, aug_size[i], j, aug_range[i][j]);
    }
  }
}

void print_foreign_aug_part_list(int*** aug_range, int** aug_size, int comm_size, int my_rank){
  printf("foreign_aug_range start my_rank=%d\n", my_rank);
  for(int r = 0; r < comm_size; r++){
    for(int s = 0; s < OP_set_index; s++){
      for(int i = 0; i < aug_size[r][s]; i++){
        printf("foreign_aug_range my_rank=%d foreign=%d set=%s size=%d value[%d]=%d\n", my_rank, r, OP_set_list[s]->name, aug_size[r][s], i, aug_range[r][s][i]);
      }
    }
  }
  printf("foreign_aug_range end my_rank=%d\n", my_rank);
}

/*******************************************************************************
 * Routine to create an nonexec-import extended list (only a wrapper)
 *******************************************************************************/

static void create_nonexec_ex_import_list(op_set set, int *temp_list,
                                       halo_list h_list, int size,
                                       int comm_size, int my_rank) {
  create_export_list(set, temp_list, h_list, size, comm_size, my_rank);
}

/*******************************************************************************
 * Routine to create an nonexec-export extended list (only a wrapper)
 *******************************************************************************/

static void create_nonexec_ex_export_list(op_set set, int *temp_list,
                                       halo_list h_list, int total_size,
                                       int *ranks, int *sizes, int ranks_size,
                                       int comm_size, int my_rank) {
  create_import_list(set, temp_list, h_list, total_size, ranks, sizes,
                     ranks_size, comm_size, my_rank);
}

int get_max_value(int* arr, int from, int to){
  int max = 0;  // assumption: max >= 0
  for(int i = from; i < to; i++){
    if(max < arr[i]){
      max = arr[i];
    }  
  }
  return max;
}

void check_augmented_part_range(int* parts, int set_index, int value, int my_rank, int comm_size){
  for(int r = 0; r < comm_size; r++){
    parts[r] = -1;
    if(r != my_rank){
      for(int i = 0; i < foreign_aug_part_range_size[r][set_index]; i++){
        if(value == foreign_aug_part_range[r][set_index][i]){
          parts[r] = 1;
          break;
        }
      }
    }
  }
}

halo_list* create_handshake_h_list(halo_list * h_lists, int **part_range, int my_rank, int comm_size){

  halo_list* handshake_h_list = (halo_list *)xmalloc(OP_set_index * sizeof(halo_list));

  int *neighbors, *sizes;
  int ranks_size;

  for (int s = 0; s < OP_set_index; s++) { // for each set
    op_set set = OP_set_list[s];

    //-----Discover neighbors-----
    ranks_size = 0;
    neighbors = (int *)xmalloc(comm_size * sizeof(int));
    sizes = (int *)xmalloc(comm_size * sizeof(int));

    halo_list list = h_lists[set->index];

    find_neighbors_set(list, neighbors, sizes, &ranks_size, my_rank, comm_size,
                       OP_MPI_WORLD);
    MPI_Request request_send[list->ranks_size];

    int *rbuf, cap = 0, index = 0;

    for (int i = 0; i < list->ranks_size; i++) {
      int *sbuf = &list->list[list->disps[i]];
      MPI_Isend(sbuf, list->sizes[i], MPI_INT, list->ranks[i], s, OP_MPI_WORLD,
                &request_send[i]);
    }

    for (int i = 0; i < ranks_size; i++)
      cap = cap + sizes[i];
    int *temp = (int *)xmalloc(cap * sizeof(int));

    // import this list from those neighbors
    for (int i = 0; i < ranks_size; i++) {
      rbuf = (int *)xmalloc(sizes[i] * sizeof(int));
      MPI_Recv(rbuf, sizes[i], MPI_INT, neighbors[i], s, OP_MPI_WORLD,
               MPI_STATUS_IGNORE);
      memcpy(&temp[index], (void *)&rbuf[0], sizes[i] * sizeof(int));
      index = index + sizes[i];
      op_free(rbuf);
    }

    MPI_Waitall(list->ranks_size, request_send, MPI_STATUSES_IGNORE);

    // create import lists
    halo_list h_list = (halo_list)xmalloc(sizeof(halo_list_core));
    create_import_list(set, temp, h_list, index, neighbors, sizes, ranks_size,
                       comm_size, my_rank);
    handshake_h_list[set->index] = h_list;
  }
  return handshake_h_list;
}

void create_aug_part_range(int exec_level, int **part_range, int my_rank, int comm_size){
  
  int exec_size = 0;

  for (int s = 0; s < OP_set_index; s++) { // for each set
    op_set set = OP_set_list[s];

    exec_size = 0;
    for(int l = 0; l <= exec_level; l++){
      exec_size += OP_aug_import_exec_lists[l][set->index]->size;
    }
    int start = 0;
    if(exec_level > 0){
      start = set->size + exec_size - OP_aug_import_exec_lists[exec_level][set->index]->size;
    }
    int end = set->size + exec_size;
   
    for (int e = start; e < end; e++) {      // for each elment of this set
      for (int m = 0; m < OP_map_index; m++) { // for each maping table
        op_map map = OP_map_list[m];

        if (compare_sets(map->from, set) == 1) { // need to select mappings from this set
        
          int part, local_index;
          for (int j = 0; j < map->dim; j++) { // for each element
                                               // pointed at by this entry
            part = get_partition(map->map[e * map->dim + j],
                                 part_range[map->to->index], &local_index,
                                 comm_size);
            if (aug_part_range_size[map->to->index] >= aug_part_range_cap[map->to->index]) {
              aug_part_range_cap[map->to->index] *= 2;
              aug_part_range[map->to->index] = (int *)xrealloc(aug_part_range[map->to->index], aug_part_range_cap[map->to->index] * sizeof(int));
            }

            if (part != my_rank) {
              aug_part_range[map->to->index][aug_part_range_size[map->to->index]++] = map->map[e * map->dim + j];
            }
          }
        }
      }
    }
  }

  for (int s = 0; s < OP_set_index; s++) {
    if(aug_part_range_size[s] > 0){
      quickSort(aug_part_range[s], 0, aug_part_range_size[s] - 1);
      aug_part_range_size[s] = removeDups(aug_part_range[s], aug_part_range_size[s]);
    }
    
  }
  // print_aug_part_list(aug_part_range, aug_part_range_size, my_rank);
}

void exchange_aug_part_ranges(int **part_range, int my_rank, int comm_size){

  OP_export_part_range_list = (halo_list*) xmalloc(OP_set_index * sizeof(halo_list));

  int* set_list;
  int s_i = 0;
  int cap_s = 1000;

  for (int s = 0; s < OP_set_index; s++) {
    s_i = 0;
    cap_s = 1000;
    op_set set = OP_set_list[s];
    set_list = (int*)xmalloc(aug_part_range_size[s] * comm_size * 2 * sizeof(int));

    for(int r = 0; r < comm_size; r++){
      if(r != my_rank){
        for(int i = 0; i < aug_part_range_size[s]; i++){
          set_list[s_i++] = r;
          set_list[s_i++] = aug_part_range[s][i];
        }
      }
    }

    halo_list h_list = (halo_list)xmalloc(sizeof(halo_list_core));
    create_export_list(set, set_list, h_list, s_i, comm_size, my_rank);
    OP_export_part_range_list[set->index] = h_list;
    op_free(set_list);
  }

  OP_import_part_range_list = create_handshake_h_list(OP_export_part_range_list, part_range, my_rank, comm_size);

  for (int s = 0; s < OP_set_index; s++) {
    halo_list h_list = OP_import_part_range_list[s];

    for(int r = 0; r < h_list->ranks_size; r++){
      if(foreign_aug_part_range_size[h_list->ranks[r]][s] < h_list->sizes[r]){
        foreign_aug_part_range[h_list->ranks[r]][s] = (int*)xrealloc(foreign_aug_part_range[h_list->ranks[r]][s], h_list->sizes[r] * sizeof(int));
      }
      
      foreign_aug_part_range_size[h_list->ranks[r]][s] = h_list->sizes[r];
      for(int i = 0; i < h_list->sizes[r]; i++){
        foreign_aug_part_range[h_list->ranks[r]][s][i] = h_list->list[h_list->disps[r] + i];
      }
    }
  }
  // print_foreign_aug_part_list(foreign_aug_part_range, foreign_aug_part_range_size, comm_size, my_rank);
}

bool is_in_prev_export_exec_lists(int exec_level, int set_index, int export_rank, int export_value, int my_rank){

  for(int i = 0; i < exec_level; i++){
    halo_list h_list = OP_aug_export_exec_lists[i][set_index];

    int rank_index = binary_search(h_list->ranks, export_rank, 0, h_list->ranks_size - 1);

    if(rank_index >= 0){
      for(int j = h_list->disps[rank_index]; j < h_list->disps[rank_index] + h_list->sizes[rank_index]; j++){
        if(h_list->list[j] == export_value){
          return true;
        }
      }
    }
  }
  return false;
}

halo_list* step1_create_aug_export_exec_list(int exec_level, int **part_range, int my_rank, int comm_size){

  halo_list* aug_export_exec_list = (halo_list *)xmalloc(OP_set_index * sizeof(halo_list));
  // declare temporaty scratch variables to hold set export lists and mapping
  // table export lists
  int s_i;
  int *set_list;

  int cap_s = 1000; // keep track of the temp array capacities
  int* parts;

  for (int s = 0; s < OP_set_index; s++) { // for each set
    op_set set = OP_set_list[s];

    // create a temporaty scratch space to hold export list for this set
    s_i = 0;
    cap_s = 1000;
    set_list = (int *)xmalloc(cap_s * sizeof(int));
    parts = (int*)xmalloc(comm_size * sizeof(int));

    for (int e = 0; e < set->size; e++) {      // iterating upto set->size rather than upto nonexec size will eliminate duplicated being exported
      for (int m = 0; m < OP_map_index; m++) { // for each maping table
        op_map map = OP_map_list[m];

        if (compare_sets(map->from, set) == 1) { // need to select mappings
                                                 // FROM this set
          int part, local_index;
          for (int j = 0; j < map->dim; j++) { // for each element
                                               // pointed at by this entry
            part = get_partition(map->map[e * map->dim + j],
                                 part_range[map->to->index], &local_index,
                                 comm_size);

            check_augmented_part_range(parts, map->to->index, map->map[e * map->dim + j],
                                    my_rank, comm_size);
            if (s_i + comm_size * 2 >= cap_s) {
              cap_s = cap_s * 2;
              set_list = (int *)xrealloc(set_list, cap_s * sizeof(int));
            }

            if (part != my_rank && !is_in_prev_export_exec_lists(exec_level, set->index, part, e, my_rank)) {
              set_list[s_i++] = part; // add to set export list
              set_list[s_i++] = e;
            }

            for(int r = 0; r < comm_size; r++){
              if(r != part && parts[r] == 1 && !is_in_prev_export_exec_lists(exec_level, set->index, r, e, my_rank)){
                set_list[s_i++] = r; // add to set export list
                set_list[s_i++] = e;
              }
            }
          }
        }
      }
    }

    // create set export list
    halo_list h_list = (halo_list)xmalloc(sizeof(halo_list_core));
    create_export_list(set, set_list, h_list, s_i, comm_size, my_rank);
    aug_export_exec_list[set->index] = h_list;
    op_free(set_list);
    op_free(parts);
  }
  return aug_export_exec_list;
}

void step3_exchange_exec_mappings(int exec_level, int **part_range, int my_rank, int comm_size){

  for (int m = 0; m < OP_map_index; m++) { // for each maping table
    op_map map = OP_map_list[m];
    halo_list i_list = OP_aug_import_exec_lists[exec_level][map->from->index];
    halo_list e_list = OP_aug_export_exec_lists[exec_level][map->from->index];

    MPI_Request request_send[e_list->ranks_size];

    // prepare bits of the mapping tables to be exported
    int **sbuf = (int **)xmalloc(e_list->ranks_size * sizeof(int *));

    for (int i = 0; i < e_list->ranks_size; i++) {
      sbuf[i] = (int *)xmalloc(e_list->sizes[i] * map->dim * sizeof(int));
      for (int j = 0; j < e_list->sizes[i]; j++) {
        for (int p = 0; p < map->dim; p++) {
          sbuf[i][j * map->dim + p] =
              map->map[map->dim * (e_list->list[e_list->disps[i] + j]) + p];
        }
      }
      MPI_Isend(sbuf[i], map->dim * e_list->sizes[i], MPI_INT, e_list->ranks[i],
                m, OP_MPI_WORLD, &request_send[i]);
    }

    // prepare space for the incomming mapping tables - realloc each
    // mapping tables in each mpi process

    int prev_exec_size = 0;
    for(int i = 0; i < exec_level; i++){
      halo_list prev_h_list = OP_aug_import_exec_lists[i][map->from->index];
      prev_exec_size += prev_h_list->size;
    }

    OP_map_list[map->index]->map = (int *)xrealloc(
        OP_map_list[map->index]->map,
        (map->dim * (map->from->size + prev_exec_size + i_list->size)) * sizeof(int));

    int init = map->dim * (map->from->size + prev_exec_size);
    for (int i = 0; i < i_list->ranks_size; i++) {
      MPI_Recv(
          &(OP_map_list[map->index]->map[init + i_list->disps[i] * map->dim]),
          map->dim * i_list->sizes[i], MPI_INT, i_list->ranks[i], m,
          OP_MPI_WORLD, MPI_STATUS_IGNORE);
    }

    MPI_Waitall(e_list->ranks_size, request_send, MPI_STATUSES_IGNORE);
    for (int i = 0; i < e_list->ranks_size; i++)
      op_free(sbuf[i]);
    op_free(sbuf);
  }
}

void step6_exchange_exec_data(int exec_level, int **part_range, int my_rank, int comm_size){
  for (int s = 0; s < OP_set_index; s++) { // for each set
    op_set set = OP_set_list[s];
    halo_list i_list = OP_aug_import_exec_lists[exec_level][set->index];
    halo_list e_list = OP_aug_export_exec_lists[exec_level][set->index];

    // for each data array
    op_dat_entry *item;
    int d = -1; // d is just simply the tag for mpi comms
    TAILQ_FOREACH(item, &OP_dat_list, entries) {
      d++; // increase tag to do mpi comm for the next op_dat
      op_dat dat = item->dat;

      if (compare_sets(set, dat->set) == 1) { // if this data array
                                              // is defined on this set
        MPI_Request request_send[e_list->ranks_size];

        // prepare execute set element data to be exported
        char **sbuf = (char **)xmalloc(e_list->ranks_size * sizeof(char *));

        for (int i = 0; i < e_list->ranks_size; i++) {
          sbuf[i] = (char *)xmalloc(e_list->sizes[i] * dat->size);
          for (int j = 0; j < e_list->sizes[i]; j++) {
            int set_elem_index = e_list->list[e_list->disps[i] + j];
            memcpy(&sbuf[i][j * dat->size],
                   (void *)&dat->data[dat->size * (set_elem_index)], dat->size);
          }
          MPI_Isend(sbuf[i], dat->size * e_list->sizes[i], MPI_CHAR,
                    e_list->ranks[i], d, OP_MPI_WORLD, &request_send[i]);
        }

        // prepare space for the incomming data - realloc each
        // data array in each mpi process

        int prev_exec_size = 0;
        for(int i = 0; i < exec_level; i++){
          halo_list prev_h_list = OP_aug_import_exec_lists[i][set->index];
          prev_exec_size += prev_h_list->size;
        }

        dat->data =
            (char *)xrealloc(dat->data, (set->size + prev_exec_size + i_list->size) * dat->size);

        int init = (set->size + prev_exec_size) * dat->size;
        for (int i = 0; i < i_list->ranks_size; i++) {
          MPI_Recv(&(dat->data[init + i_list->disps[i] * dat->size]),
                   dat->size * i_list->sizes[i], MPI_CHAR, i_list->ranks[i], d,
                   OP_MPI_WORLD, MPI_STATUS_IGNORE);
        }

        MPI_Waitall(e_list->ranks_size, request_send, MPI_STATUSES_IGNORE);
        for (int i = 0; i < e_list->ranks_size; i++)
          op_free(sbuf[i]);
        op_free(sbuf);
      }
    }
  }
}

void step8_renumber_mappings(int exec_levels, int **part_range, int my_rank, int comm_size){

  int* parts;
  for (int s = 0; s < OP_set_index; s++) { // for each set
    op_set set = OP_set_list[s];
    parts = (int*)xmalloc(comm_size * sizeof(int));

    for (int m = 0; m < OP_map_index; m++) { // for each maping table
      op_map map = OP_map_list[m];

      if (compare_sets(map->to, set) == 1) { // need to select
                                             // mappings TO this set

        halo_list nonexec_set_list = OP_import_nonexec_list[set->index];
        //get exec level size
        int exec_map_len = 0;
        for(int l = 0; l < exec_levels; l++){
          exec_map_len += OP_aug_import_exec_lists[l][map->from->index]->size;
        }

        // for each entry in this mapping table: original+execlist
        int len = map->from->size + exec_map_len;
        for (int e = 0; e < len; e++) {
          for (int j = 0; j < map->dim; j++) { // for each element
                                               // pointed at by this entry
            int part;
            int local_index = 0;
            part = get_partition(map->map[e * map->dim + j],
                                 part_range[map->to->index], &local_index,
                                 comm_size);

            check_augmented_part_range(parts, map->to->index, map->map[e * map->dim + j],
                                    my_rank, comm_size);

            if (part == my_rank) {
              OP_map_list[map->index]->map[e * map->dim + j] = local_index;
            } else {
              int found = -1;
              int rank1 = -1;
              for(int l = 0; l < exec_levels; l++){
                  found = -1;
                  rank1 = -1;

                  halo_list exec_set_list = OP_aug_import_exec_lists[l][set->index];
                  rank1 = binary_search(exec_set_list->ranks, part, 0,
                                      exec_set_list->ranks_size - 1);

                  if (rank1 >= 0) {
                    found = binary_search(exec_set_list->list, local_index,
                                        exec_set_list->disps[rank1],
                                        exec_set_list->disps[rank1] +
                                            exec_set_list->sizes[rank1] - 1);
                  }
                  //only one found should happen in this loop
                  if (found >= 0) {
                    int prev_exec_set_list_size = 0;
                    for(int l1 = 0; l1 < l; l1++){  //take the size of prev exec levels
                      prev_exec_set_list_size += OP_aug_import_exec_lists[l1][set->index]->size;
                    }
                    OP_map_list[map->index]->map[e * map->dim + j] =
                        found + map->to->size + prev_exec_set_list_size;
                    break;
                  }
              }
              // check in nonexec list
              int rank2 = binary_search(nonexec_set_list->ranks, part, 0,
                                        nonexec_set_list->ranks_size - 1);

              

              if (rank2 >= 0 && found < 0) {
                found = binary_search(nonexec_set_list->list, local_index,
                                      nonexec_set_list->disps[rank2],
                                      nonexec_set_list->disps[rank2] +
                                          nonexec_set_list->sizes[rank2] - 1);
                if (found >= 0) {
                  int exec_set_list_size = 0;
                  for(int l = 0; l < exec_levels; l++){
                    exec_set_list_size += OP_aug_import_exec_lists[l][set->index]->size;
                  }
                  OP_map_list[map->index]->map[e * map->dim + j] =
                      found + set->size + exec_set_list_size;
                }
              }

              if (found < 0)
                printf("ERROR: Set %10s Element %d needed on rank %d \
                    from partition %d\n",
                       set->name, local_index, my_rank, part);
            }
          }
        }
      }
    }
  }
}

void step4_import_nonexec(int exec_levels, int **part_range, int my_rank, int comm_size){

  OP_import_nonexec_list =
      (halo_list *)xmalloc(OP_set_index * sizeof(halo_list));
  OP_export_nonexec_list =
      (halo_list *)xmalloc(OP_set_index * sizeof(halo_list));

  // declare temporaty scratch variables to hold non-exec set export lists
  int s_i;
  int *set_list;
  int cap_s = 1000; // keep track of the temp array capacities

  s_i = 0;
  set_list = NULL;
  cap_s = 1000; // keep track of the temp array capacity

  for (int s = 0; s < OP_set_index; s++) { // for each set
    op_set set = OP_set_list[s];

    // create a temporaty scratch space to hold nonexec export list for this set
    s_i = 0;
    set_list = (int *)xmalloc(cap_s * sizeof(int));

    for (int m = 0; m < OP_map_index; m++) { // for each maping table
      op_map map = OP_map_list[m];
      int exec_size = 0;
      for(int l = 0; l < exec_levels; l++){
        exec_size += OP_aug_import_exec_lists[l][map->from->index]->size;
      }

      if (compare_sets(map->to, set) == 1) { // need to select
                                             // mappings TO this set

        // for each entry in this mapping table: original+execlist
        int len = map->from->size + exec_size;
        for (int e = 0; e < len; e++) {
          int part;
          int local_index;
          for (int j = 0; j < map->dim; j++) { // for each element pointed
                                               // at by this entry
            part = get_partition(map->map[e * map->dim + j],
                                 part_range[map->to->index], &local_index,
                                 comm_size);

            if (part != my_rank) {
              int found = -1;
              int rank = -1;

              for(int l = 0; l < exec_levels; l++){
                found = -1;
                rank = -1;

                halo_list exec_set_list = OP_aug_import_exec_lists[l][set->index];
                rank = binary_search(exec_set_list->ranks, part, 0,
                                      exec_set_list->ranks_size - 1);

                if (rank >= 0) {
                  found = binary_search(exec_set_list->list, local_index,
                                        exec_set_list->disps[rank],
                                        exec_set_list->disps[rank] +
                                            exec_set_list->sizes[rank] - 1);
                }
                if(found >= 0){
                  break;
                }
              }
              if (s_i >= cap_s) {
                cap_s = cap_s * 2;
                set_list = (int *)xrealloc(set_list, cap_s * sizeof(int));
              }

              if (found < 0) {
                // not in this partition and not found in
                // exec list
                // add to non-execute set_list
                set_list[s_i++] = part;
                set_list[s_i++] = local_index;
              }
            }
          }
        }
      }
    }

    // create non-exec set import list
    halo_list h_list = (halo_list)xmalloc(sizeof(halo_list_core));
    create_nonexec_ex_import_list(set, set_list, h_list, s_i, comm_size, my_rank);
    op_free(set_list); // free temp list
    OP_import_nonexec_list[set->index] = h_list;
  }
}


void step7_halo(int exec_levels, int **part_range, int my_rank, int comm_size){

  for (int s = 0; s < OP_set_index; s++) { // for each set
    op_set set = OP_set_list[s];
    halo_list i_list = OP_import_nonexec_list[set->index];
    halo_list e_list = OP_export_nonexec_list[set->index];

    // for each data array
    op_dat_entry *item;
    int d = -1; // d is just simply the tag for mpi comms
    TAILQ_FOREACH(item, &OP_dat_list, entries) {
      d++; // increase tag to do mpi comm for the next op_dat
      op_dat dat = item->dat;

      if (compare_sets(set, dat->set) == 1) { // if this data array is
                                              // defined on this set

        MPI_Request request_send[e_list->ranks_size];

        // prepare non-execute set element data to be exported
        char **sbuf = (char **)xmalloc(e_list->ranks_size * sizeof(char *));

        for (int i = 0; i < e_list->ranks_size; i++) {
          sbuf[i] = (char *)xmalloc(e_list->sizes[i] * dat->size);
          for (int j = 0; j < e_list->sizes[i]; j++) {
            int set_elem_index = e_list->list[e_list->disps[i] + j];
            memcpy(&sbuf[i][j * dat->size],
                   (void *)&dat->data[dat->size * (set_elem_index)], dat->size);
          }
          MPI_Isend(sbuf[i], dat->size * e_list->sizes[i], MPI_CHAR,
                    e_list->ranks[i], d, OP_MPI_WORLD, &request_send[i]);
        }

        int exec_size = 0;
        for(int l = 0; l < exec_levels; l++){
          exec_size += OP_aug_import_exec_lists[l][set->index]->size;
        }
        // prepare space for the incomming nonexec-data - realloc each
        // data array in each mpi process
        dat->data = (char *)xrealloc(
            dat->data,
            (set->size + exec_size + i_list->size) * dat->size);

        int init = (set->size + exec_size) * dat->size;
        for (int i = 0; i < i_list->ranks_size; i++) {
          MPI_Recv(&(dat->data[init + i_list->disps[i] * dat->size]),
                   dat->size * i_list->sizes[i], MPI_CHAR, i_list->ranks[i], d,
                   OP_MPI_WORLD, MPI_STATUS_IGNORE);
        }

        MPI_Waitall(e_list->ranks_size, request_send, MPI_STATUSES_IGNORE);
        for (int i = 0; i < e_list->ranks_size; i++)
          op_free(sbuf[i]);
        op_free(sbuf);
      }
    }
  }
}

void step9_halo(int exec_levels, int **part_range, int my_rank, int comm_size){
  op_dat_entry *item;
  TAILQ_FOREACH(item, &OP_dat_list, entries) {
    op_dat dat = item->dat;

    op_mpi_buffer mpi_buf = (op_mpi_buffer)xmalloc(sizeof(op_mpi_buffer_core));

    int exec_e_size = 0;
    int exec_e_ranks_size = 0;
    for(int l = 0; l < exec_levels; l++){
      exec_e_size += OP_aug_export_exec_lists[l][dat->set->index]->size;
      exec_e_ranks_size += OP_aug_export_exec_lists[l][dat->set->index]->ranks_size;
    }
    
    halo_list nonexec_e_list = OP_export_nonexec_list[dat->set->index];
    halo_list nonexec_i_list = OP_import_nonexec_list[dat->set->index];

    mpi_buf->buf_exec = (char *)xmalloc((exec_e_size) * dat->size);
    mpi_buf->buf_nonexec = (char *)xmalloc((nonexec_e_list->size + nonexec_i_list->size) * dat->size);

    int exec_i_ranks_size = 0;
    for(int l = 0; l < exec_levels; l++){
      exec_i_ranks_size += OP_aug_import_exec_lists[l][dat->set->index]->ranks_size;
    }

    mpi_buf->s_req = (MPI_Request *)xmalloc(
        sizeof(MPI_Request) *
        (exec_e_ranks_size + nonexec_e_list->ranks_size));
    mpi_buf->r_req = (MPI_Request *)xmalloc(
        sizeof(MPI_Request) *
        (exec_i_ranks_size + nonexec_i_list->ranks_size));

    mpi_buf->s_num_req = 0;
    mpi_buf->r_num_req = 0;
    dat->mpi_buffer = mpi_buf;
  }

  // set dirty bits of all data arrays to 0
  // for each data array
  item = NULL;
  TAILQ_FOREACH(item, &OP_dat_list, entries) {
    op_dat dat = item->dat;
    dat->dirtybit = 0;
  }
}

void step10_halo(int exec_levels, int **part_range, int **core_elems, int **exp_elems, int my_rank, int comm_size){
 
  for (int s = 0; s < OP_set_index; s++) { // for each set
    op_set set = OP_set_list[s];

    int exec_size = 0;
    halo_list exec[exec_levels];
    for(int l = 0; l < exec_levels; l++){
      exec[l] = OP_aug_export_exec_lists[l][set->index];
      exec_size += exec[l]->size;
    }

    halo_list nonexec = OP_export_nonexec_list[set->index];

    if (exec_size > 0) {
      exp_elems[set->index] = (int *)xmalloc(exec_size * sizeof(int));
      int prev_exec_size = 0;
      for(int l = 0; l < exec_levels; l++){
        memcpy(&exp_elems[set->index][prev_exec_size], exec[l]->list, exec[l]->size * sizeof(int));
        prev_exec_size += exec[l]->size;
      }
      quickSort(exp_elems[set->index], 0, exec_size - 1);

      int num_exp = removeDups(exp_elems[set->index], exec_size);
      core_elems[set->index] = (int *)xmalloc(set->size * sizeof(int));
      int count = 0;
      for (int e = 0; e < set->size; e++) { // for each elment of this set

        if ((binary_search(exp_elems[set->index], e, 0, num_exp - 1) < 0)) {
          core_elems[set->index][count++] = e;
        }
      }
      quickSort(core_elems[set->index], 0, count - 1);

      if (count + num_exp != set->size)
        printf("sizes not equal\n");
      set->core_size = count;

      // for each data array defined on this set seperate its elements
      op_dat_entry *item;
      TAILQ_FOREACH(item, &OP_dat_list, entries) {
        op_dat dat = item->dat;

        if (compare_sets(set, dat->set) == 1) // if this data array is
        // defined on this set
        {
          char *new_dat = (char *)xmalloc(set->size * dat->size);
          for (int i = 0; i < count; i++) {
            memcpy(&new_dat[i * dat->size],
                   &dat->data[core_elems[set->index][i] * dat->size],
                   dat->size);
          }
          for (int i = 0; i < num_exp; i++) {
            memcpy(&new_dat[(count + i) * dat->size],
                   &dat->data[exp_elems[set->index][i] * dat->size], dat->size);
          }
          memcpy(&dat->data[0], &new_dat[0], set->size * dat->size);
          op_free(new_dat);
        }
      }

      // for each mapping defined from this set seperate its elements
      for (int m = 0; m < OP_map_index; m++) { // for each set
        op_map map = OP_map_list[m];

        if (compare_sets(map->from, set) == 1) { // if this mapping is
                                                 // defined from this set
          int *new_map = (int *)xmalloc(set->size * map->dim * sizeof(int));
          for (int i = 0; i < count; i++) {
            memcpy(&new_map[i * map->dim],
                   &map->map[core_elems[set->index][i] * map->dim],
                   map->dim * sizeof(int));
          }
          for (int i = 0; i < num_exp; i++) {
            memcpy(&new_map[(count + i) * map->dim],
                   &map->map[exp_elems[set->index][i] * map->dim],
                   map->dim * sizeof(int));
          }
          memcpy(&map->map[0], &new_map[0], set->size * map->dim * sizeof(int));
          op_free(new_map);
        }
      }

      
      for(int l = 0; l < exec_levels; l++){
        for (int i = 0; i < exec[l]->size; i++) {
          int index =
              binary_search(exp_elems[set->index], exec[l]->list[i], 0, num_exp - 1);
          if (index < 0)
            printf("Problem in seperating core elements - exec list\n");
          else
            exec[l]->list[i] = count + index;
        }
      }

      for (int i = 0; i < nonexec->size; i++) {
        int index = binary_search(core_elems[set->index], nonexec->list[i], 0,
                                  count - 1);
        if (index < 0) {
          index = binary_search(exp_elems[set->index], nonexec->list[i], 0,
                                num_exp - 1);
          if (index < 0)
            printf("Problem in seperating core elements - nonexec list\n");
          else
            nonexec->list[i] = count + index;
        } else
          nonexec->list[i] = index;
      }

    } else {
      core_elems[set->index] = (int *)xmalloc(set->size * sizeof(int));
      exp_elems[set->index] = (int *)xmalloc(0 * sizeof(int));
      for (int e = 0; e < set->size; e++) { // for each elment of this set
        core_elems[set->index][e] = e;
      }
      set->core_size = set->size;
    }
  }

  // now need to renumber mapping tables as the elements are seperated
  for (int m = 0; m < OP_map_index; m++) { // for each set
    op_map map = OP_map_list[m];

    int imp_exec_size = 0;
    for(int l = 0; l < exec_levels; l++){
      imp_exec_size += OP_aug_import_exec_lists[l][map->from->index]->size;
    }
    // for each entry in this mapping table: original+execlist
    int len = map->from->size + imp_exec_size;
    for (int e = 0; e < len; e++) {
      for (int j = 0; j < map->dim; j++) { // for each element pointed
                                           // at by this entry
        if (map->map[e * map->dim + j] < map->to->size) {
          int index = binary_search(core_elems[map->to->index],
                                    map->map[e * map->dim + j], 0,
                                    map->to->core_size - 1);
          if (index < 0) {
            index = binary_search(exp_elems[map->to->index],
                                  map->map[e * map->dim + j], 0,
                                  (map->to->size) - (map->to->core_size) - 1);
            if (index < 0)
              printf("Problem in seperating core elements - \
                  renumbering map\n");
            else
              OP_map_list[map->index]->map[e * map->dim + j] =
                  map->to->core_size + index;
          } else
            OP_map_list[map->index]->map[e * map->dim + j] = index;
        }
      }
    }
  }
}

void step11_halo(int exec_levels, int **part_range, int **core_elems, int **exp_elems, int my_rank, int comm_size){
  // if OP_part_list is empty, (i.e. no previous partitioning done) then
  // create it and store the seperation of elements using core_elems
  // and exp_elems
  if (OP_part_index != OP_set_index) {
    // allocate memory for list
    OP_part_list = (part *)xmalloc(OP_set_index * sizeof(part));

    for (int s = 0; s < OP_set_index; s++) { // for each set
      op_set set = OP_set_list[s];
      // printf("set %s size = %d\n", set.name, set.size);
      int *g_index = (int *)xmalloc(sizeof(int) * set->size);
      int *partition = (int *)xmalloc(sizeof(int) * set->size);
      for (int i = 0; i < set->size; i++) {
        g_index[i] =
            get_global_index(i, my_rank, part_range[set->index], comm_size);
        partition[i] = my_rank;
      }
      decl_partition(set, g_index, partition);

      // combine core_elems and exp_elems to one memory block
      int *temp = (int *)xmalloc(sizeof(int) * set->size);
      memcpy(&temp[0], core_elems[set->index], set->core_size * sizeof(int));
      memcpy(&temp[set->core_size], exp_elems[set->index],
             (set->size - set->core_size) * sizeof(int));

      // update OP_part_list[set->index]->g_index
      for (int i = 0; i < set->size; i++) {
        temp[i] = OP_part_list[set->index]->g_index[temp[i]];
      }
      op_free(OP_part_list[set->index]->g_index);
      OP_part_list[set->index]->g_index = temp;
    }
  } else { // OP_part_list exists (i.e. a partitioning has been done)
           // update the seperation of elements

    for (int s = 0; s < OP_set_index; s++) { // for each set
      op_set set = OP_set_list[s];

      // combine core_elems and exp_elems to one memory block
      int *temp = (int *)xmalloc(sizeof(int) * set->size);
      memcpy(&temp[0], core_elems[set->index], set->core_size * sizeof(int));
      memcpy(&temp[set->core_size], exp_elems[set->index],
             (set->size - set->core_size) * sizeof(int));

      // update OP_part_list[set->index]->g_index
      for (int i = 0; i < set->size; i++) {
        temp[i] = OP_part_list[set->index]->g_index[temp[i]];
      }
      op_free(OP_part_list[set->index]->g_index);
      OP_part_list[set->index]->g_index = temp;
    }
  }

  /*for(int s=0; s<OP_set_index; s++) { //for each set
    op_set set=OP_set_list[s];
    printf("Original Index for set %s\n", set->name);
    for(int i=0; i<set->size; i++ )
    printf(" %d",OP_part_list[set->index]->g_index[i]);
    }*/

  // set up exec and nonexec sizes
  for (int s = 0; s < OP_set_index; s++) { // for each set
    op_set set = OP_set_list[s];
    for(int l = 0; l < exec_levels; l++){
      set->exec_size += OP_aug_import_exec_lists[l][set->index]->size;
    }
    set->nonexec_size = OP_import_nonexec_list[set->index]->size;
  }
}

/*******************************************************************************
 * Main MPI halo creation routine
 *******************************************************************************/

void op_halo_create_comm_avoid() {
  // declare timers
  double cpu_t1, cpu_t2, wall_t1, wall_t2;
  double time;
  double max_time;
  op_timers(&cpu_t1, &wall_t1); // timer start for list create

  // create new communicator for OP mpi operation
  int my_rank, comm_size;
  // MPI_Comm_dup(OP_MPI_WORLD, &OP_MPI_WORLD);
  MPI_Comm_rank(OP_MPI_WORLD, &my_rank);
  MPI_Comm_size(OP_MPI_WORLD, &comm_size);

  /* Compute global partition range information for each set*/
  int **part_range = (int **)xmalloc(OP_set_index * sizeof(int *));
  get_part_range(part_range, my_rank, comm_size, OP_MPI_WORLD);

  // save this partition range information if it is not already saved during
  // a call to some partitioning routine
  if (orig_part_range == NULL) {
    orig_part_range = (int **)xmalloc(OP_set_index * sizeof(int *));
    for (int s = 0; s < OP_set_index; s++) {
      op_set set = OP_set_list[s];
      orig_part_range[set->index] = (int *)xmalloc(2 * comm_size * sizeof(int));
      for (int j = 0; j < comm_size; j++) {
        orig_part_range[set->index][2 * j] = part_range[set->index][2 * j];
        orig_part_range[set->index][2 * j + 1] =
            part_range[set->index][2 * j + 1];
      }
    }
  }

  aug_part_range = (int **)xmalloc(OP_set_index * sizeof(int *));
  aug_part_range_size = (int *)xmalloc(OP_set_index * sizeof(int));
  aug_part_range_cap = (int *)xmalloc(OP_set_index * sizeof(int));
  for (int s = 0; s < OP_set_index; s++) {
    aug_part_range_cap[s] = 1000;
    aug_part_range[s] = (int *)xmalloc(aug_part_range_cap[s] * sizeof(int));
    aug_part_range_size[s] = 0;
  }

  foreign_aug_part_range = (int ***)xmalloc(comm_size * sizeof(int **));
  foreign_aug_part_range_size = (int **)xmalloc(comm_size * sizeof(int *));
  for(int i = 0; i < comm_size; i++){
    foreign_aug_part_range[i] = (int **)xmalloc(OP_set_index * sizeof(int *));
    foreign_aug_part_range_size[i] = (int *)xmalloc(OP_set_index * sizeof(int));

    for(int j = 0; j < OP_set_index; j++){
      foreign_aug_part_range[i][j] = NULL;
      foreign_aug_part_range_size[i][j] = 0;
    }
  }

  int exec_levels = 5;
  
  for(int l = 0; l < exec_levels; l++){
    /*----- STEP 1 - Construct export lists for execute set elements and related
    mapping table entries -----*/
    OP_aug_export_exec_lists[l] = step1_create_aug_export_exec_list(l, part_range, my_rank, comm_size);

    /*---- STEP 2 - construct import lists for mappings and execute sets------*/
    OP_aug_import_exec_lists[l] = create_handshake_h_list(OP_aug_export_exec_lists[l], part_range, my_rank, comm_size);

    /*--STEP 3 -Exchange mapping table entries using the import/export lists--*/
    step3_exchange_exec_mappings(l, part_range, my_rank, comm_size);

    /*-STEP 6 - Exchange execute set elements/data using the import/export lists--*/
    step6_exchange_exec_data(l, part_range, my_rank, comm_size);

    create_aug_part_range(l, part_range, my_rank, comm_size);
    exchange_aug_part_ranges(part_range, my_rank, comm_size);
  }

  OP_export_exec_list = OP_aug_export_exec_lists[0];
  OP_import_exec_list = OP_aug_import_exec_lists[0];

  /*-- STEP 4 - Create import lists for non-execute set elements using mapping
    table entries including the additional mapping table entries --*/
  step4_import_nonexec(exec_levels, part_range, my_rank, comm_size);

  /*----------- STEP 5 - construct non-execute set export lists -------------*/
  step5(part_range, my_rank, comm_size);

  /*-STEP 6 - Exchange execute set elements/data using the import/export
   * lists--*/
  // this step is moved upwards

  /*-STEP 7 - Exchange non-execute set elements/data using the import/export
   * lists--*/
  step7_halo(exec_levels, part_range, my_rank, comm_size);

  /*-STEP 8 ----------------- Renumber Mapping tables-----------------------*/
  step8_renumber_mappings(exec_levels, part_range, my_rank, comm_size);

  /*-STEP 9 ---------------- Create MPI send Buffers-----------------------*/
  step9_halo(exec_levels, part_range, my_rank, comm_size);

  /*-STEP 10 -------------------- Separate core
   * elements------------------------*/
  int **core_elems = (int **)xmalloc(OP_set_index * sizeof(int *));
  int **exp_elems = (int **)xmalloc(OP_set_index * sizeof(int *));
  step10_halo(exec_levels, part_range, core_elems, exp_elems, my_rank, comm_size);

  /*-STEP 11 ----------- Save the original set element
   * indexes------------------*/
  step11_halo(exec_levels, part_range, core_elems, exp_elems, my_rank, comm_size);

  /*-STEP 12 ---------- Clean up and Compute rough halo size
   * numbers------------*/
  for (int i = 0; i < OP_set_index; i++) {
    op_free(part_range[i]);
    op_free(core_elems[i]);
    op_free(exp_elems[i]);
  }
  op_free(part_range);
  op_free(exp_elems);
  op_free(core_elems);

  // releasing augmented part range data structures
  for (int s = 0; s < OP_set_index; s++) {
    op_free(aug_part_range[s]);
  }
  op_free(aug_part_range);
  op_free(aug_part_range_size);
  op_free(aug_part_range_cap);

  for(int i = 0; i < comm_size; i++){
    for(int j = 0; j < OP_set_index; j++){
      op_free(foreign_aug_part_range[i][j]);
    }
    op_free(foreign_aug_part_range[i]);
    op_free(foreign_aug_part_range_size[i]);
  }
  op_free(foreign_aug_part_range);
  op_free(foreign_aug_part_range_size);

  op_timers(&cpu_t2, &wall_t2); // timer stop for list create
  // compute import/export lists creation time
  time = wall_t2 - wall_t1;
  MPI_Reduce(&time, &max_time, 1, MPI_DOUBLE, MPI_MAX, MPI_ROOT, OP_MPI_WORLD);

  step12(part_range, max_time, my_rank, comm_size);
}

void op_single_halo_destroy(halo_list* h_list){

  for (int s = 0; s < OP_set_index; s++) {
    op_set set = OP_set_list[s];

    op_free(h_list[set->index]->ranks);
    op_free(h_list[set->index]->disps);
    op_free(h_list[set->index]->sizes);
    op_free(h_list[set->index]->list);
    op_free(h_list[set->index]);
  }
  op_free(h_list);
}

void op_halo_destroy_comm_avoid() {

  op_halo_destroy();

  for(int i = 0; i < 10; i++){
    if(OP_aug_import_exec_lists[i])
      op_single_halo_destroy(OP_aug_import_exec_lists[i]);
    if(OP_aug_export_exec_lists[i])
      op_single_halo_destroy(OP_aug_import_exec_lists[i]);
  }

}

int find_element_in(int* arr, int element){
  for(int i = 1; i <= arr[0]; i++){
    if(arr[i] == element){
      return i;
    }
  }
  return -1;
}

void calculate_max_values(op_set from_set, op_set to_set, int map_dim, int* map_values,
int* to_sets, int* to_set_to_core_max, int* to_set_to_exec_max, int* to_set_to_nonexec_max, int my_rank){

  int set_index = find_element_in(to_sets, to_set->index);
  if(set_index < 0){
    to_sets[0] = to_sets[0] + 1;
    to_sets[to_sets[0]] = to_set->index;
    set_index = to_sets[0];
  }
  
  int core_max = get_max_value(map_values, 0, from_set->core_size * map_dim);
  if(to_set_to_core_max[set_index] < core_max){
    to_set_to_core_max[set_index] = core_max;
  }

  int exec_max = get_max_value(map_values, from_set->core_size * map_dim,
      (from_set->size +  from_set->exec_size) * map_dim);
  if(to_set_to_exec_max[set_index] < exec_max){
    to_set_to_exec_max[set_index] = exec_max;
  }

  int nonexec_max = get_max_value(map_values, (from_set->size +  from_set->exec_size) * map_dim,
      (from_set->size +  from_set->exec_size + from_set->nonexec_size) * map_dim);
  if(to_set_to_nonexec_max[set_index] < nonexec_max){
    to_set_to_nonexec_max[set_index] = nonexec_max;
  }
  
}

int get_core_size(op_set set, int* to_sets, int* to_set_to_core_max){

  int index = find_element_in(to_sets, set->index);
  if(index != -1){
    return to_set_to_core_max[index] + 1;
  }else{
    return set->core_size;
  }
}

int get_exec_size(op_set set, int* to_sets, int* to_set_to_core_max, int* to_set_to_exec_max){

  int index = find_element_in(to_sets, set->index);
  if(index != -1){
    int it_core = to_set_to_core_max[index];
    int it_exec = to_set_to_exec_max[index];
    return ((it_exec - it_core) > 0) ? (it_exec - it_core) : 0;
  }else{
    return (set->size - set->core_size) + OP_import_exec_list[set->index]->size;
  }
}

int get_nonexec_size(op_set set, int* to_sets, int* to_set_to_exec_max, int* to_set_to_nonexec_max){

  int index = find_element_in(to_sets, set->index);
  if(index != -1){
    int it_exec = to_set_to_exec_max[index];
    int it_nonexec = to_set_to_nonexec_max[index];
    return ((it_nonexec - it_exec) > 0) ? (it_nonexec - it_exec) : 0;
  }else{
    return OP_import_nonexec_list[set->index]->size;
  }
}