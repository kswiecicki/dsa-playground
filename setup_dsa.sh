#!/usr/bin/env bash

configure_shared_wq()
{
	if [[ $# != 3 ]]; then
		echo "Usage: ${FUNCNAME} dsa_id wq_id group_id"
		exit
	fi

	local dsa_id=$1
	local wq_id=$2
	local group_id=$3
	local wq_name="shared_queue${dsa_id}.${wq_id}"
	local dsa="dsa${dsa_id}"
	local wq="wq${dsa_id}.${wq_id}"

	accel-config config-wq ${dsa}/${wq} --mode=shared --priority=10 --wq-size=16 --threshold=16 --block-on-fault=1 --type=user --name=${wq_name} --group-id=${group_id}
}

configure_engine()
{
	if [[ $# != 3 ]]; then
		echo "Usage: ${FUNCNAME} dsa_id engine_id group_id"
		exit
	fi

	local dsa_id=$1
	local engine_id=$2
	local group_id=$3
	local dsa="dsa${dsa_id}"
	local engine="engine${dsa_id}.${engine_id}"

	accel-config config-engine ${dsa}/${engine} --group-id=${group_id}
}

enable_device()
{
	if [[ $# != 1 ]]; then
		echo "Usage: ${FUNCNAME} dsa_id"
		exit
	fi

	local dsa_id=$1
	local dsa="dsa${dsa_id}"

	accel-config enable-device ${dsa}
}

disable_device()
{
	if [[ $# != 1 ]]; then
		echo "Usage: ${FUNCNAME} dsa_id"
		exit
	fi

	local dsa_id=$1
	local dsa="dsa${dsa_id}"

	accel-config disable-device ${dsa}
}

enable_wq()
{
	if [[ $# != 2 ]]; then
		echo "Usage: ${FUNCNAME} dsa_id wq_id"
		exit
	fi

	local dsa_id=$1
	local wq_id=$2
	local dsa="dsa${dsa_id}"
	local wq="wq${dsa_id}.${wq_id}"

	accel-config enable-wq ${dsa}/${wq}
}

disable_wq()
{
	if [[ $# != 2 ]]; then
		echo "Usage: ${FUNCNAME} dsa_id wq_id"
		exit
	fi

	local dsa_id=$1
	local wq_id=$2
	local dsa="dsa${dsa_id}"
	local wq="wq${dsa_id}.${wq_id}"

	accel-config disable-wq ${dsa}/${wq}
}

configure_shared_wq 1 0 0
configure_shared_wq 1 1 1
configure_shared_wq 1 2 2
configure_shared_wq 1 3 3

configure_engine 1 0 0
configure_engine 1 1 1
configure_engine 1 2 2
configure_engine 1 3 3

enable_device 1

enable_wq 1 0
enable_wq 1 1
enable_wq 1 2
enable_wq 1 3
