FILESEXTRAPATHS_append := "${THISDIR}:"

SRC_URI += "file://auto-sepolicy/common/ \
            file://auto-sepolicy/${BASEMACHINE}/ "

POLICY_MONOLITHIC = "${@oe.utils.conditional('BASEMACHINE', 'sdxpoorwills', 'y', 'n', d)}"

do_patch_append() {
    install_device_policy(d, os.path.join("auto-sepolicy", "common"))
    if os.path.exists(os.path.join(d.getVar("WORKDIR"), "auto-sepolicy", d.getVar("BASEMACHINE"))):
        install_device_policy(d, os.path.join("auto-sepolicy", d.getVar("BASEMACHINE")))
}

do_install_sdxpoorwills() {
    oe_runmake 'DESTDIR=${D}' 'prefix=${D}${prefix}' install
    install_misc_files
    install_config
}
