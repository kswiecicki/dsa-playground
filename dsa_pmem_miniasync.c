#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dml/dml.h>
#include <libminiasync.h>
#include <libminiasync-dml.h>
#include <libpmem2.h>

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "usage: dsa_pmem_miniasync <file>");
		exit(-1);
	}

	char *file = argv[1];
	printf("file: %s\n", file);

	int n_vals = 8;
	int buf_alloc_size = sizeof(char) * n_vals;
	char *buf = malloc(buf_alloc_size);
	memset(buf, 111, n_vals);

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

	/* MINIASYNC */
	struct runtime *r = runtime_new();
	struct vdm *dml_mover = vdm_new(vdm_descriptor_dml_async());
	struct vdm_memcpy_future dml_memcpy_future = vdm_memcpy(dml_mover,
			map_addr, buf, buf_alloc_size, MINIASYNC_DML_F_MEM_DURABLE);

	runtime_wait(r, FUTURE_AS_RUNNABLE(&dml_memcpy_future));

	/* validation */
	for (int i = 0; i < n_vals; i++) {
		printf("dst[%d]: %d\n", i, ((char *)map_addr)[i]);
	}

	/* cleanup */
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
