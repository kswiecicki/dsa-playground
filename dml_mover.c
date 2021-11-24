#include <assert.h>
#include <dml/dml.h>
#include <libuasync.h>
#include <stdlib.h>

#include "dml_mover.h"

static void *
sync_memcpy_dml(void *dst, void *src, size_t n)
{
	dml_status_t status;
	uint32_t job_size_ptr;
	dml_job_t *dml_job_ptr;

	status = dml_get_job_size(DML_PATH_HW, &job_size_ptr);
	assert(status == DML_STATUS_OK);

	dml_job_ptr = (dml_job_t *) malloc(job_size_ptr);

	status = dml_init_job(DML_PATH_HW, dml_job_ptr);
	assert(status == DML_STATUS_OK);

	dml_job_ptr->operation = DML_OP_MEM_MOVE;
	dml_job_ptr->source_first_ptr = (uint8_t *)src;
	dml_job_ptr->destination_first_ptr = (uint8_t *)dst;
	dml_job_ptr->source_length = n;
	dml_job_ptr->destination_length = n;

	status = dml_execute_job(dml_job_ptr);
	assert(status == DML_STATUS_OK);

	return dst;
}

static void
memcpy_dml_sync(void *runner, struct future_context *context)
{
	struct mover_memcpy_data *data = future_context_get_data(context);
	struct mover_memcpy_output *output = future_context_get_output(context);
	output->dest = sync_memcpy_dml(data->dest, data->src, data->n);
	data->mover_cb(context);
}

static struct mover_runner dml_synchronous_runner = {
	.runner_data = NULL,
	.memcpy = memcpy_dml_sync,
};

struct mover_runner *
mover_runner_dml_synchronous(void)
{
	return &dml_synchronous_runner;
}
