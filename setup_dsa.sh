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

setup_dsa()
{
	if [[ $# != 2 ]]; then
		echo "Usage: ${FUNCNAME} from_dsa_id to_dsa_id"
		exit
	fi

	local max_wq_id=7
	local max_group_id=3
	local max_engine_id=3

	for dsa_id in $(seq $1 $2)
	do
		disable_device ${dsa_id}

		for wq_id in $(seq 0 ${max_wq_id})
		do
			configure_shared_wq ${dsa_id} ${wq_id} $((${wq_id} % ${max_group_id}))
		done

		for engine_id in $(seq 0 ${max_engine_id})
		do
			configure_engine ${dsa_id} ${engine_id} $((${engine_id} % ${max_group_id}))
		done

		enable_device ${dsa_id}

		for wq_id in $(seq 0 ${max_wq_id})
		do
			enable_wq ${dsa_id} ${wq_id}
		done
	done

	local dsa_id=$1
	local wq_id=$2
	local dsa="dsa${dsa_id}"
	local wq="wq${dsa_id}.${wq_id}"

	accel-config disable-wq ${dsa}/${wq}
}

# WQ 0

# disable_wq 0 1
# disable_wq 0 2
# disable_wq 0 3
# disable_wq 0 4
# disable_wq 0 5
# disable_wq 0 6
# disable_wq 0 7

# disable_device 0

# configure_shared_wq 0 0 0
# configure_shared_wq 0 1 1
# configure_shared_wq 0 2 2
# configure_shared_wq 0 3 3
# configure_shared_wq 0 4 0
# configure_shared_wq 0 5 1
# configure_shared_wq 0 6 2
# configure_shared_wq 0 7 3

# configure_engine 0 0 0
# configure_engine 0 1 1
# configure_engine 0 2 2
# configure_engine 0 3 3

# enable_device 0

# enable_wq 0 0
# enable_wq 0 1
# enable_wq 0 2
# enable_wq 0 3
# enable_wq 0 4
# enable_wq 0 5
# enable_wq 0 6
# enable_wq 0 7

setup_dsa 0 5