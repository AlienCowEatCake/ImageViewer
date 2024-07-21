#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import plistlib
import re
import shutil
import subprocess
import sys


def _is_bundle(dir):
    return os.path.exists(os.path.join(dir, "Contents", "MacOS"))


def _get_frameworks_location(dir):
    if _is_bundle(dir):
        return os.path.join(dir, "Contents/Frameworks")
    return dir


def _get_framework_info_plist(framework_path):
    for subdir in ["Resources", "Contents"]:
        plist_path = os.path.join(framework_path, subdir, "Info.plist")
        if os.path.exists(plist_path):
            return plist_path
    return None


def _get_framework_bundle_executable(framework_path):
    plist_path = _get_framework_info_plist(framework_path)
    if not plist_path:
        return None
    try:
        info = None
        if sys.version_info[0] > 2:
            with open(plist_path, "rb") as f:
                info = plistlib.load(f)
        else:
            info = plistlib.readPlist(plist_path)
        executable = info["CFBundleExecutable"]
        return executable
    except Exception:
        try:
            with open(plist_path, "r") as f:
                s = re.search('<key>CFBundleExecutable</key>[^<]*<string>([^<]*)</string>', f.read(), re.MULTILINE)
                if s:
                    return s.group(1)
        except Exception:
            pass
    return None


def _get_framework_binary(framework_path):
    executable = _get_framework_bundle_executable(framework_path)
    if not executable:
        return None
    try:
        executable_paths = [os.path.join(framework_path, executable)]
        if executable_paths[0].endswith("_debug"):
            executable_paths.append(executable_paths[0][:-6])
        for executable_path in executable_paths:
            if os.path.exists(executable_path):
                executable_path = os.path.realpath(executable_path)
                executable_path = re.sub('.*\\.framework/', '', executable_path)
                executable_path = os.path.join(framework_path, executable_path)
                if os.path.exists(executable_path):
                    return executable_path
    except Exception:
        pass
    return None


def _run_otool(filepath):
    result = []
    otool_process = subprocess.Popen(['otool', '-L', '-arch', 'all', os.path.abspath(filepath)],
                                     stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    otool_output = otool_process.communicate()[0]
    if sys.version_info[0] > 2:
        otool_output = otool_output.decode('utf-8')
    otool_process.wait()
    if otool_process.returncode == 0:
        for string in otool_output.splitlines():
            string = re.sub('\\(compatibility version [0-9\\.]*, current version [0-9\\.]*\\)', '', string)
            if string.startswith(' ') or string.startswith('\t'):
                string = string.strip()
                result.append(string)
    return result


def _int_add_rpath(filepath, new_rpath):
    int_process = subprocess.Popen(['install_name_tool', '-add_rpath', new_rpath, os.path.abspath(filepath)],
                                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    int_process.wait()
    return int_process.returncode == 0


def _int_id(filepath, new_id):
    int_process = subprocess.Popen(['install_name_tool', '-id', new_id, os.path.abspath(filepath)],
                                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    int_process.wait()
    return int_process.returncode == 0


def _int_change(filepath, old, new):
    int_process = subprocess.Popen(['install_name_tool', '-change', old, new, os.path.abspath(filepath)],
                                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    int_process.wait()
    return int_process.returncode == 0


def _collect_binaries(dir):
    result = []
    if _is_bundle(dir):
        for subdir in ["Contents/MacOS", "Contents/Frameworks"]:
            subdir_path = os.path.abspath(os.path.join(dir, subdir))
            if os.path.exists(subdir_path):
                result += _collect_binaries(subdir_path)
        plugins_dir = os.path.join(dir, "Contents/PlugIns")
        if os.path.exists(plugins_dir):
            for root, _, files in os.walk(plugins_dir):
                for file in files:
                    plugin_path = os.path.abspath(os.path.join(root, file))
                    result.append(plugin_path)
        frameworks_dir = os.path.join(dir, "Contents/Frameworks")
        if os.path.exists(frameworks_dir):
            for framework_dir in os.listdir(frameworks_dir):
                framework_path = os.path.abspath(os.path.join(frameworks_dir, framework_dir))
                framework_bin = _get_framework_binary(framework_path)
                if framework_bin and os.path.exists(framework_bin):
                    result.append(os.path.abspath(framework_bin))
    else:
        for file in os.listdir(dir):
            file_path = os.path.abspath(os.path.join(dir, file))
            if os.path.isfile(file_path):
                result.append(file_path)
    return result


def _can_use_rpath(dir):
    # @todo LC_RPATH is incompatible with < 10.5
    return False


def get_binary_deps(filepath):
    result = []
    for string in _run_otool(filepath):
        path_components = string.split('/')
        if filepath.split('/')[-1] == path_components[-1]:
            continue
        if path_components[-1].endswith('.dylib'):
            result.append(path_components[-1].strip())
        elif len(path_components) > 3 and path_components[-4].endswith('.framework'):
            result.append(path_components[-4].strip())
    return list(set(result))


def get_deps(dir):
    deps = []
    for binary in _collect_binaries(dir):
        deps += get_binary_deps(binary)
    return list(set(deps))


def check_dll(dir, dll):
    return os.path.exists(os.path.join(_get_frameworks_location(dir), dll))


def copy_dll(dir_in, dir_out, dll):
    in_path = os.path.join(dir_in, dll)
    out_path = _get_frameworks_location(dir_out)
    if not os.path.exists(in_path):
        return False
    if dll.endswith('.dylib'):
        shutil.copy2(in_path, out_path, follow_symlinks=True)
    elif dll.endswith('.framework'):
        ignore = shutil.ignore_patterns('Headers', '*.prl')
        shutil.copytree(in_path, os.path.join(out_path, dll), symlinks=True, ignore=ignore)
    return os.path.exists(os.path.join(out_path, dll))


def copy_dlls(dir, search_paths):
    i = 0
    while True:
        i += 1
        print("======= Iteration", i, "=======")
        changed = False
        deps = get_deps(dir)
        for it in deps:
            resolved = False
            if check_dll(dir, it):
                continue
            for jt in search_paths:
                if not check_dll(jt, it):
                    continue
                if not copy_dll(jt, dir, it):
                    continue
                print("Resolved:", it, "in", os.path.abspath(jt))
                resolved = True
                changed = True
                break
            if not resolved:
                print("Unresolved:", it)
        if not changed:
            break
    print("======= Done =======")


def fixup_binaries(dir):
    print("======= Fixup =======")
    is_bundle = _is_bundle(dir)
    use_rpath = _can_use_rpath(dir)
    rpath = '@executable_path/../Frameworks' if is_bundle else '@executable_path'
    prefix = '@rpath' if use_rpath else rpath
    binaries = _collect_binaries(dir)
    for binary in binaries:
        otool_results = _run_otool(binary)
        if len(otool_results) < 1:
            continue
        print(binary)
        binary_components = binary.split('/')
        if binary_components[-1].endswith('.dylib'):
            new_id = binary_components[-1]
            if not is_bundle or binary_components[-2] == 'Frameworks':
                new_id = '{0}/{1}'.format(prefix, new_id)
            old_ids = []
            for otool_result in otool_results:
                if otool_result.endswith(binary_components[-1]) and otool_result not in old_ids:
                    old_ids.append(otool_result)
            if len(old_ids) > 0 and old_ids[0] != new_id:
                if _int_id(binary, new_id):
                    print('  -id', new_id)
        elif len(binary_components) > 3 and binary_components[-4].endswith('.framework'):
            binary_rel = re.sub('.*/{0}'.format(binary_components[-4]), binary_components[-4], binary)
            new_id = '{0}/{1}'.format(prefix, binary_rel)
            old_ids = []
            for otool_result in otool_results:
                if otool_result.endswith(binary_rel) and otool_result not in old_ids:
                    old_ids.append(otool_result)
            if len(old_ids) > 0 and old_ids[0] != new_id:
                if _int_id(binary, new_id):
                    print('  -id', new_id)
            framework_path = os.path.dirname(os.path.dirname(os.path.dirname(binary)))
            framework_bundle_executable = _get_framework_bundle_executable(framework_path)
            framework_binary = binary_components[-1]
            if framework_bundle_executable and framework_bundle_executable != framework_binary:
                plist_path = _get_framework_info_plist(framework_path)
                if sys.version_info[0] > 2:
                    info = None
                    with open(plist_path, "rb") as f:
                        info = plistlib.load(f)
                    info["CFBundleExecutable"] = framework_binary
                    with open(plist_path, "wb") as f:
                        plistlib.dump(info, f)
                    print('  CFBundleExecutable', framework_bundle_executable, "=>", framework_binary)
                else:
                    info = plistlib.readPlist(plist_path)
                    info["CFBundleExecutable"] = framework_binary
                    plistlib.writePlist(info, plist_path)
                    print('  CFBundleExecutable', framework_bundle_executable, "=>", framework_binary)
        deps = get_binary_deps(binary)
        for dep in deps:
            if not check_dll(dir, dep):
                continue
            otool_currents = []
            for otool_result in otool_results:
                if dep.endswith('.framework') and (re.match('.*/{0}/'.format(dep), otool_result) or re.match('^{0}/'.format(dep), otool_result)):
                    otool_currents.append(otool_result)
                elif dep.endswith('.dylib') and (re.match('.*/{0}$'.format(dep), otool_result) or re.match('^{0}$'.format(dep), otool_result)):
                    otool_currents.append(otool_result)
            if len(otool_currents) < 1:
                continue
            dep_rel = dep
            if dep.endswith('.framework'):
                dep_rel = os.path.join(_get_frameworks_location(dir), dep_rel)
                dep_rel = _get_framework_binary(dep_rel)
                dep_rel = re.sub('.*/{0}'.format(dep), dep, dep_rel)
            otool_new = '{0}/{1}'.format(prefix, dep_rel)
            for otool_old in set(otool_currents):
                if otool_old == otool_new:
                    continue
                if _int_change(binary, otool_old, otool_new):
                    print('  -change', otool_old, otool_new)
        if use_rpath:
            if _int_add_rpath(binary, rpath):
                print('  -add_rpath', rpath)
    print("======= Done =======")


def main():
    if len(sys.argv) < 3:
        print("Usage:", sys.argv[0], "<target directory> <search directory #1> [search directory #2] ...")
        return 1
    dir = sys.argv[1]
    arguments = sys.argv[2:]
    print("Target Directory:", dir)
    print("Search Directories:", arguments)
    copy_dlls(dir, arguments)
    fixup_binaries(dir)
    return 0


if __name__ == '__main__':
    sys.exit(main())
