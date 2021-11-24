#!/bin/bash

remount_device()
{
	if [[ $# != 1 ]]; then
		echo "Usage: ${FUNCNAME} dev_path"
		exit
	fi

	local dev_path=$1
	local dev_id=${dev_path#*pmem}
	local mount="/mnt/pmem${dev_id}"

	sudo umount ${mount}
	sudo mkfs.ext4 ${dev_path}
	sudo mount -o dax ${dev_path} ${mount}
}

remount_device "/dev/pmem1"
