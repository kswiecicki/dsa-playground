#include <benchmark/benchmark.h>
#include <errno.h>
#include <iostream>
#include <libpmem2.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "dml/dml.h"

#define BENCHMARK_ERR_SKIP(func, err) \
	sprintf(error_msg, "%s failed with error %d", #func, err); \
	state.SkipWithError(error_msg)

char error_msg[256];

class DMLBenchmark : public benchmark::Fixture {
 public:
 	void SetUp(benchmark::State& state) {
		 /* inputs */
		int data_size = state.range(0);
		assert(data_size > 0);

		/* source buffer */
		char *buf = (char *)malloc(data_size);
		memset(buf, 1, data_size);

		/* destination file */
		char file[] = "/mnt/pmem1/testfile";
		fd = open(file, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
		if (fd < 0) {
			perror("open");
			BENCHMARK_ERR_SKIP(open, errno);
		}

		int ret = ftruncate(fd, data_size);
		if (ret == -1) {
			perror("ftruncate");
			BENCHMARK_ERR_SKIP(ftruncate, errno);
		}

		/* PMEM2 */
		ret = pmem2_source_from_fd(&src, fd);
		if (ret) {
			pmem2_perror("pmem2_source_from_fd");
			BENCHMARK_ERR_SKIP(pmem2_source_from_fd, ret);
		}

		ret = pmem2_config_new(&cfg);
		if (ret) {
			pmem2_perror("pmem2_config_new");
			BENCHMARK_ERR_SKIP(pmem2_config_new, ret);
		}

		ret = pmem2_config_set_required_store_granularity(cfg,
				PMEM2_GRANULARITY_CACHE_LINE);
		if (ret) {
			pmem2_perror("pmem2_config_set_required_store_granularity");
			BENCHMARK_ERR_SKIP(pmem2_config_set_required_store_granularity, ret);
		}

		ret = pmem2_map_new(&map, cfg, src);
		if (ret) {
			pmem2_perror("pmem2_map_new");
			BENCHMARK_ERR_SKIP(pmem2_map_new, ret);
		}

		void *dst = pmem2_map_get_address(map);
		memset(dst, 0, data_size);

		/* DML */
		uint32_t job_size_ptr;
		dml_status_t status;

		status = dml_get_job_size(DML_PATH_HW, &job_size_ptr);
		if (status != DML_STATUS_OK) {
			BENCHMARK_ERR_SKIP(dml_get_job_size, status);
		}

		dml_job_ptr = (dml_job_t *)malloc(job_size_ptr);

		status = dml_init_job(DML_PATH_HW, dml_job_ptr);
		if (status != DML_STATUS_OK) {
			BENCHMARK_ERR_SKIP(dml_init_job, status);
		}

		dml_job_ptr->operation = DML_OP_MEM_MOVE;
		dml_job_ptr->source_first_ptr = (uint8_t *)buf;
		dml_job_ptr->destination_first_ptr = (uint8_t *)dst;
		dml_job_ptr->source_length = data_size;
		dml_job_ptr->destination_length = data_size;

		/* custom counter */
		state.counters["initial data"] = ((uint8_t *)dst)[0];
	}

	void TearDown(benchmark::State& state) {
		void *dst = pmem2_map_get_address(map);

		/* custom counter */
		state.counters["final data"] = ((uint8_t *)dst)[0];

		dml_status_t status;
		status = dml_finalize_job(dml_job_ptr);
		if (status != DML_STATUS_OK) {
			BENCHMARK_ERR_SKIP(dml_finalize_job, status);
		}

		int ret = pmem2_map_delete(&map);
		if (ret) {
			pmem2_perror("pmem2_map_delete");
			BENCHMARK_ERR_SKIP(pmem2_map_delete, ret);
		}

		ret = pmem2_config_delete(&cfg);
		if (ret) {
			pmem2_perror("pmem2_config_delete");
			BENCHMARK_ERR_SKIP(pmem2_config_delete, ret);
		}

		ret = pmem2_source_delete(&src);
		if (ret) {
			pmem2_perror("pmem2_source_delete");
			BENCHMARK_ERR_SKIP(pmem2_source_delete, ret);
		}

		ret = close(fd);
		if (ret == -1) {
			perror("close");
			BENCHMARK_ERR_SKIP(close, errno);
		}

		free(buf);
	}

public:
	/* PMEM2 */
	struct pmem2_source *src;
	struct pmem2_config *cfg;
	struct pmem2_map *map;

	/* DML */
	dml_job_t *dml_job_ptr;

	/* source */
	char *buf;

	/* destination */
	int fd;
};

BENCHMARK_DEFINE_F(DMLBenchmark, Memcpy)(benchmark::State& state) {
	dml_status_t status;
	for (auto _ : state) {
		status = dml_execute_job(dml_job_ptr);
		if (status != DML_STATUS_OK) {
			BENCHMARK_ERR_SKIP(dml_execute_job, status);
		}
	}
}

BENCHMARK_REGISTER_F(DMLBenchmark, Memcpy)->UseRealTime()->ArgsProduct({benchmark::CreateRange(64, 8 << 12, 2)});

BENCHMARK_MAIN();
