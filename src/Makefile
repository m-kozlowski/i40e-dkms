 # SPDX-License-Identifier: GPL-2.0-only
# Copyright (C) 2013-2025 Intel Corporation

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
#
# Makefile for the Intel(R) 40-10 Gigabit Ethernet Connection Network Driver
#

ccflags-y += -I$(src)
subdir-ccflags-y += -I$(src)

obj-m += i40e.o
i40e-y := i40e_main.o \
	i40e_ethtool.o \
	i40e_xsk.o \
	i40e_adminq.o \
	i40e_common.o \
	i40e_hmc.o \
	i40e_lan_hmc.o \
	i40e_nvm.o \
	i40e_debugfs.o \
	i40e_diag.o \
	i40e_txrx.o \
	i40e_ptp.o \
	i40e_filters.o \
	i40e_ddp.o \
	i40e_client.o \
	i40e_virtchnl_pf.o
i40e-$(CONFIG_DCB) += i40e_dcb.o i40e_dcb_nl.o
i40e-y += kcompat.o
i40e-y += kcompat_vfd.o
ifeq (${NEED_AUX_BUS},2)
intel_auxiliary-objs := auxiliary.o
obj-m += intel_auxiliary.o
$(warning ${NEED_AUX_BUS})
endif
else	# ifneq($(KERNELRELEASE),)
# normal makefile

DRIVER := i40e
# If the user just wants to print the help output, don't include common.mk or
# perform any other checks. This ensures that running "make help" will always
# work even if kernel-devel is not installed, or if the common.mk fails under
# any other error condition.
ifneq ($(MAKECMDGOALS),help)
include common.mk
# i40e does not support building on kernels older than 2.6.32
$(call minimum_kver_check,2,6,32)
endif
# Command to update initramfs or display a warning message
ifeq (${cmd_initrd},)
define cmd_initramfs
@echo "Unable to update initramfs. You may need to do this manaully."
endef
else
define cmd_initramfs
@echo "Updating initramfs..."
-@$(call cmd_initrd)
endef
endif
###############
# Build rules #
###############

# Standard compilation, with regular output
default:
	@+$(call kernelbuild,modules)

# Noisy output, for extra debugging
noisy:
	@+$(call kernelbuild,modules,V=1)

# Silence any output generated
silent:
	@+$(call kernelbuild,modules,>/dev/null)

# Enable higher warning level
checkwarnings: clean
	@+$(call kernelbuild,modules,W=1)

# Run sparse static analyzer
sparse: clean
	@+$(call kernelbuild,modules,C=2 CF="-D__CHECK_ENDIAN__ -Wbitwise -Wcontext")

# Run coccicheck static analyzer
ccc: clean
	@+$(call kernelbuild,modules,coccicheck MODE=report)

# Build manfiles
manfile:
	@gzip -c ../${DRIVER}.${MANSECTION} > ${DRIVER}.${MANSECTION}.gz
# Clean the module subdirectories
clean:
	@+$(call kernelbuild,clean)
	@-rm -rf *.${MANSECTION}.gz *.ko
sign:
	@echo "Signing driver..."
	@$(call info_signed_modules)
	@$(call sign_driver)

mandocs_install: manfile
	@echo "Copying manpages..."
	@install -D -m 644 ${DRIVER}.${MANSECTION}.gz ${INSTALL_MOD_PATH}${MANDIR}/man${MANSECTION}/${DRIVER}.${MANSECTION}.gz
# Install kernel module files. It is not expected to modify files outside of
# the build root. Thus, it must not update initramfs, or run depmod.
modules_install: default
	@echo "Installing modules..."
	@+$(call kernelbuild,modules_install)
	$(MAKE) auxiliary_install
# Install kernel module files without auxiliary. This target is called by the
# RPM specfile when  generating binary RPMs, and is not expected to modify
# files outside of the build root. Thus, it must not update initramfs, or run depmod.
modules_install_no_aux:
	@echo "Installing modules..."
	@+$(call kernelbuild,modules_install)

# After installing all the files, perform necessary work to ensure the system
# will use the new modules. This includes running depmod to update module
# dependencies and updating the initramfs image in case the module is loaded
# during early boot.
install: modules_install
	$(call cmd_depmod)
	$(call cmd_initramfs)
	$(MAKE) mandocs_install
# Target used by rpmbuild spec file
rpm: modules_install
	$(MAKE) mandocs_install
mandocs_uninstall:
	if [ -e ${INSTALL_MOD_PATH}${MANDIR}/man${MANSECTION}/${DRIVER}.${MANSECTION}.gz ] ; then \
		rm -f ${INSTALL_MOD_PATH}${MANDIR}/man${MANSECTION}/${DRIVER}.${MANSECTION}.gz ; \
	fi;

# Remove installed module files. This target is called by the RPM specfile when
# generating binary RPMs, and is not expected to modify files outside of the
# build root. Thus, it must not update the initramfs image or run depmod.
modules_uninstall:
	rm -f ${INSTALL_MOD_PATH}/lib/modules/${KVER}/${INSTALL_MOD_DIR}/${DRIVER}.ko;
	$(MAKE) auxiliary_uninstall
# After uninstalling all the files, perform necessary work to restore the
# system back to using the default kernel modules. This includes running depmod
# to update module dependencies and updating the initramfs image.
uninstall: modules_uninstall mandocs_uninstall
	$(call cmd_depmod)
	$(call cmd_initramfs)

auxiliary_info:
	@../scripts/check_aux_bus --verbose --ksrc="${KSRC}" --build-kernel="${BUILD_KERNEL}"
auxiliary_install:
	@echo "Installing auxiliary..."
	${auxiliary_post_install}

auxiliary_uninstall:
	${auxiliary_post_uninstall}

ifeq (${NEED_AUX_BUS},1)
default: auxiliary_info
endif
########
# Help #
########
help:
	@echo 'Build targets:'
	@echo '  default             - Build module(s) with standard verbosity'
	@echo '  noisy               - Build module(s) with V=1 verbosity -- very noisy'
	@echo '  silent              - Build module(s), squelching all output'
	@echo ''
	@echo 'Static Analysis:'
	@echo '  checkwarnings       - Clean, then build module(s) with W=1 warnings enabled'
	@echo '  sparse              - Clean, then check module(s) using sparse'
	@echo '  ccc                 - Clean, then check module(s) using coccicheck'
	@echo ''
	@echo 'Cleaning targets:'
	@echo '  clean               - Clean files generated by kernel module build'
	@echo ''
	@echo 'Other targets:'
	@echo '  manfile             - Generate a gzipped manpage'
	@echo '  modules_install     - install the module(s) only'
	@echo '  mandocs_install     - install the manpage only'
	@echo '  install             - Build then install the module(s) and manpage, and update initramfs'
	@echo '  modules_uninstall   - uninstall the module(s) only'
	@echo '  mandocs_uninstall   - uninstall the manpage only'
	@echo '  uninstall           - Uninstall the module(s) and manpage, and update initramfs'
	@echo '  auxiliary_info      - Print information about the auxiliary module'
	@echo '  auxiliary_install   - Install compiled auxiliary module'
	@echo '  auxiliary_uninstall - Uninstall auxiliary module'
	@echo '  help                - Display this help message'
	@echo ''
	@echo 'Variables:'
	@echo '  LINUX_VERSION       - Debug tool to force kernel LINUX_VERSION_CODE. Use at your own risk.'
	@echo '  W=N                 - Kernel variable for setting warning levels'
	@echo '  V=N                 - Kernel variable for setting output verbosity'
	@echo '  INSTALL_MOD_PATH    - Add prefix for the module and manpage installation path'
	@echo '  INSTALL_MOD_DIR     - Use module directory other than updates/drivers/net/ethernet/intel/${DRIVER}'
	@echo '  KSRC                - Specifies the full path to the kernel tree to build against'
	@echo ' Other variables may be available for tuning make process, see'
	@echo ' Kernel Kbuild documentation for more information'
.PHONY: default noisy clean manfile silent sparse ccc install uninstall help auxiliary_info
endif	# ifneq($(KERNELRELEASE),)
