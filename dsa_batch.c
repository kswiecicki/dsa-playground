#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dml/dml.h>
#include <libpmem2.h>

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "usage: dsa_batch <file>");
		exit(-1);
	}

	char *file = argv[1];
	printf("file: %s\n", file);

	int n_vals = 8;
	int buf_alloc_size = sizeof(char) * n_vals;
	char *buf = malloc(buf_alloc_size);
	memset(buf, 9, n_vals);
	unsigned complete = 0;

	struct pmem2_source *src;
	struct pmem2_config *cfg;
	struct pmem2_map *map;

	int fd = open(file, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
	if (fd < 0) {
		perror("open");
		return errno;
	}

	int ret = ftruncate(fd, sizeof(buf));
	if (ret == -1) {
		perror("ftruncate");
		return errno;
	}

	/* PMEM2 */
	ret = pmem2_source_from_fd(&src, fd);
	if (ret) {
		pmem2_perror("pmem2_source_from_fd");
		return ret;
	}

	ret = pmem2_config_new(&cfg);
	if (ret) {
		pmem2_perror("pmem2_config_new");
		return ret;
	}

	ret = pmem2_config_set_required_store_granularity(cfg,
			PMEM2_GRANULARITY_CACHE_LINE);
	if (ret) {
		pmem2_perror("pmem2_config_set_required_store_granularity");
		return ret;
	}

	ret = pmem2_map_new(&map, cfg, src);
	if (ret) {
		pmem2_perror("pmem2_map_new");
		return ret;
	}

	void *map_addr = pmem2_map_get_address(map);
	size_t map_size = pmem2_map_get_size(map);

	/* DML */
	dml_status_t status;
	uint32_t job_size_ptr;
	dml_job_t *dml_job_ptr;

	status = dml_get_job_size(DML_PATH_HW, &job_size_ptr);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "dml_get_job_size error: %d", status);
		return status;
	}

	dml_job_ptr = (dml_job_t *)malloc(job_size_ptr);

	status = dml_init_job(DML_PATH_HW, dml_job_ptr);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "dml_init_job error: %d", status);
		return status;
	}

	uint32_t operations_count = 2;
	uint32_t batch_buffer_length = 0;
    	status = dml_get_batch_size(dml_job_ptr, operations_count, &batch_buffer_length);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "dml_get_batch_size error: %d", status);
		return status;
	}

	uint8_t * batch_buffer_ptr = (uint8_t *) malloc(batch_buffer_length);

	dml_job_ptr->operation = DML_OP_BATCH;
    	dml_job_ptr->destination_first_ptr = batch_buffer_ptr;
    	dml_job_ptr->destination_length = batch_buffer_length;

	status = dml_batch_set_mem_move_by_index(dml_job_ptr, 0, (uint8_t *)buf,
                                             (uint8_t *)map_addr,
                                             buf_alloc_size, 0);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "dml_batch_set_mem_move_by_index error: %d", status);
		return status;
	}

	uint8_t fill = 1;
	status = dml_batch_set_fill_by_index(dml_job_ptr, 1, &fill, (uint8_t *)&complete, sizeof(uint8_t), 0);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "dml_batch_set_fill_by_index error: %d", status);
		return status;
	}

	status = dml_submit_job(dml_job_ptr);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "dml_execute_job error: %d", status);
		return status;
	}

	status = dml_check_job(dml_job_ptr);
	printf("dml_check_job status: %d\n", status);

	dml_wait_job(dml_job_ptr);

	status = dml_check_job(dml_job_ptr);
	printf("dml_check_job status: %d\n", status);

	/* validation */
	for (int i = 0; i < n_vals; i++) {
		printf("dst[%d]: %d\n", i, ((char *)map_addr)[i]);
	}

	printf("complete: %d\n", complete);

	/* cleanup */
	status = dml_finalize_job(dml_job_ptr);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "dml_finalize_job error: %d", status);
		return status;
	}

	ret = pmem2_map_delete(&map);
	if (ret) {
		pmem2_perror("pmem2_map_delete");
		return ret;
	}

	ret = pmem2_config_delete(&cfg);
	if (ret) {
		pmem2_perror("pmem2_config_delete");
		return ret;
	}

	ret = pmem2_source_delete(&src);
	if (ret) {
		pmem2_perror("pmem2_source_delete");
		return ret;
	}

	ret = close(fd);
	if (ret == -1) {
		perror("close");
		return errno;
	}

	free(buf);

	return 0;
}
