load("//build/bazel_common_rules/dist:dist.bzl", "copy_to_dist_dir")
load("//build/kernel/kleaf:kernel.bzl", "ddk_module")

def define_modules(target, variant):
    kernel_build_variant = "{}_{}".format(target, variant)

    #The below will take care of the defconfig
    include_defconfig = ":{}_defconfig".format(variant)

    mod_list = []

    ddk_module(
        name = "{}-defconfig_qca_nss_fls".format(kernel_build_variant),
        out = "qca-nss-fls.ko",
        srcs = [
            "fls_flow.c",
            "fls_init.c",
            "fls_sensor_manager.c",
            "fls_def_sensor.c",
            "fls_conn.c",
            "fls_debug.c",
            "fls_chardev.c",
            "fls_rfs.c",
            "fls_stats.c",
        ],
        kernel_build = "//msm-kernel:{}-defconfig".format(kernel_build_variant),
        includes = [
		    ".",
        ],
        copts = [
            #"-DFLS_ECM_CLASSIFIER_EMESH_ENABLE=y",
        ],
        deps = [
            ":fls_headers",
            "//msm-kernel:all_headers",
            "//build_dir/target-aarch64_cortex-a53_musl/linux-sdx85/qca-nss-sfe-1.0:{}-defconfig_qca_nss_sfe".format(kernel_build_variant),
        ],
    )
    mod_list.append("{}-defconfig_qca_nss_fls".format(kernel_build_variant))

    copy_to_dist_dir(
        name = "{}-defconfig_qca_nss_fls_module_dist".format(kernel_build_variant),
        data = mod_list,
        dist_dir = "out/target/product/{}/dlkm/lib/modules/".format(target),
        flat = True,
        wipe_dist_dir = False,
        allow_duplicate_filenames = False,
        mode_overrides = {"**/*": "644"},
        log = "info",
    )

