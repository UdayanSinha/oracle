#!/bin/bash

# Script to handle creation of multiple QEMU instances
# QEMU_SIM_DIR = Root of the working directories for the instances.
#		 Contains copies of the kernel, rootfs, etc.
#		 Basically anything that an instance needs.
#		 Working directories for each instance are created here and numbered accordingly.
#
# QEMU_UTIL_DIR = Source directory containing the kernel, rootfs, etc.
#		  Basically anything that an instance needs.
#		  These are copied to the working directory of an instance.
#
# QEMU_MAX_INSTANCE = Max. number of allowed QEMU instances.

QEMU_SIM_DIR=/tmp/qemu
QEMU_UTIL_DIR=/home/${USER}/Workspace/kernel/qemu-utils
QEMU_ROOTFS=${QEMU_UTIL_DIR}/disk.img
QEMU_KERN=${QEMU_UTIL_DIR}/Image.gz
QEMU_MAX_INSTANCE=4
QEMU_INSTANCE=0
QEMU_CPU="max"
QEMU_NUM_CPU=2
QEMU_RAM="256M"
QEMU_SSH_PORT_BASE=5554
QEMU_SSH_PORT=${QEMU_SSH_PORT_BASE}

if [[ ${1} = "sync" ]]
then
	# run a QEMU instance using the original rootfs and kernel
	# useful to sync. changes in rootfs
	sudo qemu-system-arm64 -nographic -machine virt \
		-cpu ${QEMU_CPU} -smp ${QEMU_NUM_CPU} -m ${QEMU_RAM} \
		-kernel ${QEMU_KERN} \
		-append "root=/dev/vda rw console=ttyAMA0" \
		-drive if=none,file=${QEMU_ROOTFS},format=raw,id=hd0 \
		-device virtio-blk-device,drive=hd0 \
		--device e1000,netdev=net0 \
		-netdev user,id=net0,hostfwd=tcp::${QEMU_SSH_PORT_BASE}-:22
else
	# assign an instance number for the QEMU instance
	for (( i=1; i<=${QEMU_MAX_INSTANCE}; i++ ))
	do
		if [ -d "${QEMU_SIM_DIR}/${i}" ]
		then
			echo "Already running QEMU instance ${i}: ${QEMU_SIM_DIR}/${i}"
		else
			echo "Creating QEMU instance ${i}: ${QEMU_SIM_DIR}/${i}"
			QEMU_INSTANCE=${i}
			QEMU_SSH_PORT=$((QEMU_SSH_PORT_BASE + i))
			break
		fi
	done

	# create a QEMU instance if the max. number of possible instances is not reached yet
	if [[ ${QEMU_INSTANCE} -eq 0 ]]
	then
		echo "Already running max. number[${QEMU_MAX_INSTANCE}] of allowed QEMU instances!"
		echo "If you are not running QEMU and still see this message,"
		echo "then there may be leftover files from a previous run which did not exit gracefully."
		echo "You can clean these files up at ${QEMU_SIM_DIR} to resolve this issue:"
		echo " "
		echo "rm -rf ${QEMU_SIM_DIR}/*"
		echo " "
	else
		mkdir -p ${QEMU_SIM_DIR}/${QEMU_INSTANCE}
		cp ${QEMU_KERN} ${QEMU_SIM_DIR}/${QEMU_INSTANCE}/Image.gz
		cp ${QEMU_ROOTFS} ${QEMU_SIM_DIR}/${QEMU_INSTANCE}/disk.img

		sudo qemu-system-arm64 -nographic -machine virt \
			-cpu ${QEMU_CPU} -smp ${QEMU_NUM_CPU} -m ${QEMU_RAM} \
			-kernel ${QEMU_SIM_DIR}/${QEMU_INSTANCE}/Image.gz \
			-append "root=/dev/vda rw console=ttyAMA0" \
			-drive if=none,file=${QEMU_SIM_DIR}/${QEMU_INSTANCE}/disk.img,format=raw,id=hd0 \
			-device virtio-blk-device,drive=hd0 \
			--device e1000,netdev=net0 \
			-netdev user,id=net0,hostfwd=tcp::${QEMU_SSH_PORT}-:22

		echo "Cleaning up ${QEMU_SIM_DIR}/${QEMU_INSTANCE}"
		rm -rf ${QEMU_SIM_DIR}/${QEMU_INSTANCE}
	fi
fi
