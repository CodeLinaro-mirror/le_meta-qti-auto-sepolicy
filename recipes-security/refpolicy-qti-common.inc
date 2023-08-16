FILESEXTRAPATHS_append := "${THISDIR}:"

SRC_URI += "file://auto-sepolicy/common/ \
            file://auto-sepolicy/${BASEMACHINE}/ \
            file://auto-sepolicy/${MACHINE}/ "

SRC_URI += "${@bb.utils.contains('DISTRO_FEATURES', 'vbleima', 'file://auto-sepolicy/imaevm/', '', d)}"
SRC_URI += "${@bb.utils.contains('MACHINE_FEATURES', 'qti-vm-tele', 'file://auto-sepolicy/televm/', '', d)}"
SRC_URI += "${@bb.utils.contains('MACHINE_FEATURES', 'qti-vm-fota', 'file://auto-sepolicy/fotavm/', '', d)}"

#these modules are not applicable for telematics targets
REMOVE_MODULES ?= "adk_message_monitor adk_message_send adkcfg alljoyn allplay mediaplayer_cli mediaplayer_test qcameradaemon splitxr_service gpu iotd iotsys btdaemon button_manager demuratnservice irsc_util msdc"

do_patch_append() {
    install_device_policy(d, os.path.join("auto-sepolicy", "common"))
    if os.path.exists(os.path.join(d.getVar("WORKDIR"), "auto-sepolicy", d.getVar("BASEMACHINE"))):
        install_device_policy(d, os.path.join("auto-sepolicy", d.getVar("BASEMACHINE")))
    if os.path.exists(os.path.join(d.getVar("WORKDIR"), "auto-sepolicy", d.getVar("MACHINE"))):
        install_device_policy(d, os.path.join("auto-sepolicy", d.getVar("MACHINE")))
    if bb.utils.contains('DISTRO_FEATURES', 'vbleima', True, False, d):
        install_device_policy(d, os.path.join("auto-sepolicy", "imaevm"))
    if bb.utils.contains('MACHINE_FEATURES', 'qti-vm-tele', True, False, d):
        install_device_policy(d, os.path.join("auto-sepolicy", "televm"))
    if bb.utils.contains('MACHINE_FEATURES', 'qti-vm-fota', True, False, d):
        install_device_policy(d, os.path.join("auto-sepolicy", "fotavm"))
}

do_install_append() {
    rm -rf ${D}/${datadir}/selinux/${POLICY_NAME}/*.pp
    rm -rf ${D}/var/lib/selinux/mls/active/modules
    rm -rf ${D}/var/lib/selinux/mls/active/policy.kern
    rm -rf ${D}/var/lib/selinux/mls/active/policy.linked
}

python remove_modules_conf () {
    remove_modules = d.getVar("REMOVE_MODULES").split()
    module_conf_name = os.path.join(d.getVar("S"), "policy", "modules.conf")
    new_file = ""

    with open(module_conf_name, 'r') as modules_file:
        layer = None
        for line in modules_file.readlines():
            if line.startswith('# Layer'):
                layer = line.split(':')[-1].strip()
            elif '=' in line:
                module_name = line.split('=')[0].strip()
                if (module_name in remove_modules):
                    new_file += module_name + ' = off\n'
                    continue
            new_file += line

    # Save the updated modules.conf
    with open(module_conf_name, 'w') as modules_file:
        modules_file.write(new_file)
}

do_compile[postfuncs] += "remove_modules_conf"
