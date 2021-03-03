FILESEXTRAPATHS_append := "${THISDIR}:"

SRC_URI += "file://auto-sepolicy/common/ \
            file://auto-sepolicy/${BASEMACHINE}/ "

do_patch_append() {
    install_device_policy(d, os.path.join("auto-sepolicy", "common"))
    if os.path.exists(os.path.join(d.getVar("WORKDIR"), "auto-sepolicy", d.getVar("BASEMACHINE"))):
        install_device_policy(d, os.path.join("auto-sepolicy", d.getVar("BASEMACHINE")))
}
