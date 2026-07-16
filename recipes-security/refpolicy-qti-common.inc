FILESEXTRAPATHS:append := "${THISDIR}:"

SRC_URI += "${@bb.utils.contains('MLPREFIX', 'lib32-', 'file://auto-sepolicy/patches/0001-Add-32bit-busybox-substitution-paths.patch', '', d)}"
SRC_URI += "file://auto-sepolicy/common/ \
            file://auto-sepolicy/${BASEMACHINE}/ \
            "

SRC_URI += "${@bb.utils.contains('DISTRO_FEATURES', 'vbleima', 'file://auto-sepolicy/imaevm/', '', d)}"
SRC_URI += "${@bb.utils.contains('MACHINE_FEATURES', 'qti-vm-tele', 'file://auto-sepolicy/televm/', '', d)}"
SRC_URI += "${@bb.utils.contains('MACHINE_FEATURES', 'qti-vm-fota', 'file://auto-sepolicy/fotavm/', '', d)}"

#these modules are not applicable for telematics targets
REMOVE_MODULES ?= "adk_message_monitor adk_message_send adkcfg alljoyn allplay mediaplayer_cli mediaplayer_test qcameradaemon splitxr_service gpu iotd iotsys btdaemon button_manager demuratnservice irsc_util msdc cloudinit dlsc fs_scrub_daemon pdmappersvc sosreport sblim samhain rkhunter puppet mcelog logwatch logrotate Kudzu kismet kdump kdumpgui firstboot fakehwclock ddcprobe dbadm chkrootkit cfengine certwatch bootloader bootlogd amtu aide alex"

ADD_MODULES ?= "cron"

do_patch:append() {
    install_device_policy(d, os.path.join("auto-sepolicy", "common"))
    if os.path.exists(os.path.join(d.getVar("UNPACKDIR"), "auto-sepolicy", d.getVar("BASEMACHINE"))):
        install_device_policy(d, os.path.join("auto-sepolicy", d.getVar("BASEMACHINE")))
    if os.path.exists(os.path.join(d.getVar("UNPACKDIR"), "auto-sepolicy", d.getVar("MACHINE"))):
        install_device_policy(d, os.path.join("auto-sepolicy", d.getVar("MACHINE")))
    if bb.utils.contains('DISTRO_FEATURES', 'vbleima', True, False, d):
        install_device_policy(d, os.path.join("auto-sepolicy", "imaevm"))
    if bb.utils.contains('MACHINE_FEATURES', 'qti-vm-tele', True, False, d):
        install_device_policy(d, os.path.join("auto-sepolicy", "televm"))
    if bb.utils.contains('MACHINE_FEATURES', 'qti-vm-fota', True, False, d):
        install_device_policy(d, os.path.join("auto-sepolicy", "fotavm"))
}

do_install:append() {
    rm -rf ${D}/${datadir}/selinux/${POLICY_NAME}/*.pp
    rm -rf ${D}/var/lib/selinux/mls/active/policy.kern
    rm -rf ${D}/var/lib/selinux/mls/active/policy.linked
    if ${@bb.utils.contains('COMBINED_FEATURES', 'qti-nad-telaf', 'false', 'true', d)}; then
        rm -rf ${D}/var/lib/selinux/mls/active/modules
    fi
}

create_data_ubi_mounted_contexts:append() {
    # Create seperate file_contexts file for /systemrw
    sed -n '/^\/systemrw/p' \
        ${D}/etc/selinux/mls/contexts/files/file_contexts | \
        sed 's/^\/systemrw//g' > \
        ${D}/etc/selinux/mls/contexts/files/systemrw_file_contexts
}

python remove_modules_conf () {
    remove_modules = d.getVar("REMOVE_MODULES").split()
    add_modules = d.getVar("ADD_MODULES").split()
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
                elif (module_name in add_modules):
                    new_file += module_name + ' = module\n'
                    continue
            new_file += line
    # Save the updated modules.conf
    with open(module_conf_name, 'w') as modules_file:
        modules_file.write(new_file)
}

python remove_cil () {
    import os
    rootdir = os.path.join(d.getVar("D"), "var", "lib", "selinux", "mls", "active", "modules" , "100")
    for subdir, dirs, files in os.walk(rootdir):
        for file in files :
            if file == "hll":
                os.remove(os.path.join(subdir, file))
}

python print_modules_conf () {
    import os
    import bb

    module_conf_name = os.path.join(d.getVar("S"), "policy", "modules.conf")
    if not os.path.exists(module_conf_name):
        bb.warn("SELinux: modules.conf not found at %s" % module_conf_name)
        return

    enabled_base = []
    enabled_mod  = []
    disabled_off = []

    with open(module_conf_name, 'r') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#') or '=' not in line:
                continue
            name, val = [x.strip() for x in line.split('=', 1)]
            if val == "base":
                enabled_base.append(name)
            elif val == "module":
                enabled_mod.append(name)
            elif val == "off":
                disabled_off.append(name)

    bb.note("SELinux: modules.conf summary: base=%d module=%d off=%d" %
            (len(enabled_base), len(enabled_mod), len(disabled_off)))

    bb.debug("SELinux: enabled base modules:")
    for m in sorted(enabled_base):
        bb.debug("  %s = base" % m)

    bb.debug("SELinux: enabled loadable modules:")
    for m in sorted(enabled_mod):
        bb.debug("  %s = module" % m)

    bb.debug("SELinux: disabled modules:")
    for m in sorted(disabled_off):
        bb.debug("  %s = module" % m)
}


do_compile[postfuncs] += "remove_modules_conf print_modules_conf"

POLICY_CUSTOM_BUILDOPT:append = "${@bb.utils.contains('COMBINED_FEATURES', 'qti-mvm', '"qti-mvm "', '', d)}"
do_install[postfuncs] += "${@bb.utils.contains('COMBINED_FEATURES', 'qti-nad-telaf', 'remove_cil', '', d)}"

PURGE_POLICY_MODULES += "needrestart"
