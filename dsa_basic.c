#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dml/dml.h>

int main(int argc, char **argv) {
	int n_vals = 8;
	int buf_alloc_size = sizeof(char) * n_vals;
	char *buf = malloc(buf_alloc_size);
	memset(buf, 7, n_vals);

	char *dst = malloc(buf_alloc_size);
	memset(dst, 0, n_vals);

	dml_status_t status;
	uint32_t job_size_ptr;
	dml_job_t *dml_job_ptr;

	status = dml_get_job_size(DML_PATH_HW, &job_size_ptr);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "Status error %d", status);
		return status;
	}

	dml_job_ptr = (dml_job_t *)malloc(job_size_ptr);

	status = dml_init_job(DML_PATH_HW, dml_job_ptr);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "Status error %d", status);
		return status;
	}

	dml_job_ptr->operation = DML_OP_MEM_MOVE;
	dml_job_ptr->source_first_ptr = (uint8_t *)buf;
	dml_job_ptr->destination_first_ptr = (uint8_t *)dst;
	dml_job_ptr->source_length = buf_alloc_size;
	dml_job_ptr->destination_length = buf_alloc_size;

	status = dml_execute_job(dml_job_ptr);
	if (status != DML_STATUS_OK) {
		fprintf(stderr, "Status error %d", status);
		return status;
	}

	for (int i = 0; i < n_vals; i++) {
		printf("dst[%d]: %d\n", i, dst[i]);
	}

	free(dst);
	free(buf);

	return 0;
}
