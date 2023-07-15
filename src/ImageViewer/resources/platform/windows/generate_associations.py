#!/usr/bin/env python3

import io
import os
import plistlib
import sys


def main():
    cur_dir = os.path.dirname(os.path.realpath(__file__))
    plist = os.path.join(os.path.dirname(cur_dir), 'macosx', 'Info.plist')
    with open(plist, 'rb') as f:
        plist_data = plistlib.load(f)
    f_open = io.open('open.wxi', 'w', newline='\n')
    f_open_with = io.open('open_with.wxi', 'w', newline='\n')
    for f in [f_open, f_open_with]:
        f.write('<?xml version="1.0" encoding="utf-8" ?>\n')
        f.write('<Include>\n')
    for dt in plist_data['CFBundleDocumentTypes']:
        type_name = dt['CFBundleTypeName']
        for ext in dt['CFBundleTypeExtensions']:
            f_open_with.write('    <RegistryValue\n')
            f_open_with.write('      Root="HKCR"\n')
            f_open_with.write('      Type="string"\n')
            f_open_with.write('      Key="$(var.ProductNameSafe).{}"\n'.format(ext))
            f_open_with.write('      Value="{}"\n'.format(type_name))
            f_open_with.write('      Id="reg_type_{}" />\n'.format(ext))
            f_open_with.write('    <RegistryValue\n')
            f_open_with.write('      Root="HKCR"\n')
            f_open_with.write('      Type="string"\n')
            f_open_with.write('      Key="$(var.ProductNameSafe).{}\\DefaultIcon"\n'.format(ext))
            # https://github.com/AlienCowEatCake/ImageViewer/issues/7
            if ext not in ['ico', 'cur', 'ani']:
                f_open_with.write('      Value="[INSTALLLOCATION]$(var.ExeProcessName),0"\n')
            else:
                f_open_with.write('      Value="%1"\n')
            f_open_with.write('      Id="reg_icon_{}" />\n'.format(ext))
            f_open_with.write('    <RegistryValue\n')
            f_open_with.write('      Root="HKCR"\n')
            f_open_with.write('      Type="string"\n')
            f_open_with.write('      Key="$(var.ProductNameSafe).{}\\shell\\open"\n'.format(ext))
            f_open_with.write('      Value="Open with $(var.ProductName)"\n')
            f_open_with.write('      Id="reg_shell_open_{}" />\n'.format(ext))
            f_open_with.write('    <RegistryValue\n')
            f_open_with.write('      Root="HKCR"\n')
            f_open_with.write('      Type="string"\n')
            f_open_with.write('      Key="$(var.ProductNameSafe).{}\\shell\\open\\command"\n'.format(ext))
            f_open_with.write('      Value="&quot;[INSTALLLOCATION]$(var.ExeProcessName)&quot; &quot;%1&quot;"\n')
            f_open_with.write('      Id="reg_shell_open_command_{}" />\n'.format(ext))
            f_open_with.write('    <RegistryValue\n')
            f_open_with.write('      Root="HKCR"\n')
            f_open_with.write('      Type="string"\n')
            f_open_with.write('      Key=".{}\\OpenWithProgids"\n'.format(ext))
            f_open_with.write('      Value=""\n')
            f_open_with.write('      Name="$(var.ProductNameSafe).{}"\n'.format(ext))
            f_open_with.write('      Id="reg_{}" />\n'.format(ext))
            # https://github.com/AlienCowEatCake/ImageViewer/issues/6
            if ext not in ['ico', 'cur', 'ani', 'psd']:
                f_open.write('    <RegistryValue\n')
                f_open.write('      Root="HKCR"\n')
                f_open.write('      Type="string"\n')
                f_open.write('      Key=".{}"\n'.format(ext))
                f_open.write('      Value="$(var.ProductNameSafe).{}"\n'.format(ext))
                f_open.write('      Id="reg_assoc_{}" />\n'.format(ext))
    for f in [f_open, f_open_with]:
        f.write('</Include>\n')
        f.close()
    return 0


if __name__ == '__main__':
    sys.exit(main())
