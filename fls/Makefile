#
# Flow stats Makefile
#

obj-m += qca-nss-fls.o

qca-nss-fls-y := fls_debug.o fls_init.o fls_flow.o fls_chardev.o

ifneq "$(FLS_MEM_PROFILE_LOW)" "y"
qca-nss-fls-y += fls_rfs.o fls_conn.o fls_def_sensor.o fls_sensor_manager.o
endif

ccflags-$(FLS_ECM_CLASSIFIER_EMESH_ENABLE) += -DFLS_ECM_CLASSIFIER_EMESH_ENABLE
ccflags-$(FLS_MEM_PROFILE_LOW) += -DFLS_MEM_PROFILE_LOW

ccflags-y += -Werror -Wall -Iexports/
