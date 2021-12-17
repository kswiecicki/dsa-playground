#include <benchmark/benchmark.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <dml/dml.h>
#include <libminiasync.h>
#include <libminiasync-dml.h>
#include <libpmem2.h>

#define BENCHMARK_ERR_SKIP(func, err) \
	sprintf(error_msg, "%s failed with error %d", #func, err); \
	state.SkipWithError(error_msg)

char error_msg[256];

class MINIASYNCBenchmark : public benchmark::Fixture {
 public:
 	void SetUp(benchmark::State& state) {
		 /* inputs */
		int data_size = state.range(0);
		assert(data_size > 0);

		/* source buffer */
		buf = (char *)malloc(data_size);
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

		dst = pmem2_map_get_address(map);
		memset(dst, 0, data_size);

		/* MINIASYNC */
		r = runtime_new();

		int nfutures = state.range(1);
		futures = (struct vdm_memcpy_future *)malloc(sizeof(*futures) * nfutures);

		/* custom counter */
		state.counters["before"] = ((uint8_t *)dst)[0];
	}

	void TearDown(benchmark::State& state) {
		void *dst = pmem2_map_get_address(map);

		/* custom counter */
		state.counters["after"] = ((uint8_t *)dst)[0];

		free(futures);
		runtime_delete(r);

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

	/* MINIASYNC */
	struct runtime *r;
	struct vdm_memcpy_future *futures;

	/* source */
	char *buf;

	/* destination */
	void *dst;
	int fd;
};

BENCHMARK_DEFINE_F(MINIASYNCBenchmark, Memcpy)(benchmark::State& state) {
	int data_size = state.range(0);
	int nfutures = state.range(1);

	struct future **base_futures = (struct future **)malloc(sizeof(struct future *) * nfutures);
	struct vdm *dml_mover = vdm_new(vdm_descriptor_dml());

	for (auto _ : state) {
		state.PauseTiming();
		for (int i = 0; i < nfutures; i++) {
			futures[i] = vdm_memcpy(dml_mover, dst, buf, data_size, MINIASYNC_DML_F_MEM_DURABLE);
			base_futures[i] = FUTURE_AS_RUNNABLE(&futures[i]);
		}

		state.ResumeTiming();
		runtime_wait_multiple(r, base_futures, nfutures);
		state.PauseTiming();
	}
}

BENCHMARK_DEFINE_F(MINIASYNCBenchmark, MemcpyAsync)(benchmark::State& state) {
	int data_size = state.range(0);
	int nfutures = state.range(1);

	struct future **base_futures = (struct future **)malloc(sizeof(struct future *) * nfutures);
	struct vdm *dml_mover = vdm_new(vdm_descriptor_dml_async());

	for (auto _ : state) {
		state.PauseTiming();
		for (int i = 0; i < nfutures; i++) {
			futures[i] = vdm_memcpy(dml_mover, dst, buf, data_size, MINIASYNC_DML_F_MEM_DURABLE);
			base_futures[i] = FUTURE_AS_RUNNABLE(&futures[i]);
		}

		state.ResumeTiming();
		runtime_wait_multiple(r, base_futures, nfutures);
		state.PauseTiming();
	}
}

BENCHMARK_REGISTER_F(MINIASYNCBenchmark, MemcpyAsync)->UseRealTime()->ArgsProduct({benchmark::CreateRange(64, 8 << 12, 2), benchmark::CreateRange(1, 16, 2)});
BENCHMARK_REGISTER_F(MINIASYNCBenchmark, Memcpy)->UseRealTime()->ArgsProduct({benchmark::CreateRange(64, 8 << 12, 2), benchmark::CreateRange(1, 16, 2)});

BENCHMARK_MAIN();
