#include <stdlib.h>
#include <stdio.h>

#include "dml/dml.h"

int main(int argc, char **argv) {
	int dst[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	int src[8] = { 1, 1, 1, 1, 1, 1, 1, 1 };
	int num = sizeof(dst) / sizeof(dst[0]);

	dml_status_t status;
	uint32_t *job_size_ptr;
	dml_job_t *dml_job_ptr;

	status = dml_get_job_size(DML_PATH_HW, job_size_ptr);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "Status error %d", status);
		return status;
	}

	dml_job_ptr = (dml_job_t *) malloc(*job_size_ptr);

	status = dml_init_job(DML_PATH_HW, dml_job_ptr);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "Status error %d", status);
		return status;
	}

	dml_job_ptr->operation = DML_OP_MEM_MOVE;
	dml_job_ptr->source_first_ptr = (uint8_t *)src;
	dml_job_ptr->destination_first_ptr = (uint8_t *)dst;
	dml_job_ptr->source_length = sizeof(src);
	dml_job_ptr->destination_length = sizeof(dst);

	status = dml_execute_job(dml_job_ptr);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "Status error %d", status);
		return status;
	}

	for (int i = 0; i < num; i++) {
		printf("dst[%d]: %d\n", i, dst[i]);
	}

	return 0;
}
