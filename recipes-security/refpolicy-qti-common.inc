FILESEXTRAPATHS_append := "${THISDIR}:"

SRC_URI += "file://auto-sepolicy/patches/ \
	    file://auto-sepolicy/common/ \
            file://auto-sepolicy/${BASEMACHINE}/ \
            file://auto-sepolicy/${MACHINE}/ "

SRC_URI += "${@bb.utils.contains('DISTRO_FEATURES', 'vbleima', 'file://auto-sepolicy/imaevm/', '', d)}"

do_patch_append() {
    install_device_policy(d, os.path.join("auto-sepolicy", "common"))
    if os.path.exists(os.path.join(d.getVar("WORKDIR"), "auto-sepolicy", d.getVar("BASEMACHINE"))):
        install_device_policy(d, os.path.join("auto-sepolicy", d.getVar("BASEMACHINE")))
    if os.path.exists(os.path.join(d.getVar("WORKDIR"), "auto-sepolicy", d.getVar("MACHINE"))):
        install_device_policy(d, os.path.join("auto-sepolicy", d.getVar("MACHINE")))
    if bb.utils.contains('DISTRO_FEATURES', 'vbleima', True, False, d):
        install_device_policy(d, os.path.join("auto-sepolicy", "imaevm"))
}

do_install_append() {
    rm -rf ${D}/${datadir}/selinux/${POLICY_NAME}/*.pp
}
