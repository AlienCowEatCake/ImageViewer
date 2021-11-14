#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse
import os
import plistlib
import subprocess
import sys
import time

'''
# @note https://stackoverflow.com/questions/56890749/macos-notarize-in-script
'''


def plistlib_loads(string):
    try:
        return plistlib.readPlistFromString(string)
    except AttributeError:
        return plistlib.loads(string.encode('utf-8'))


def plistlib_dumps(plist):
    try:
        return plistlib.writePlistToString(plist)
    except AttributeError:
        return plistlib.dumps(plist).decode('utf8')


def main():
    parser = argparse.ArgumentParser(description='Notarizer')
    parser.add_argument('--application',        metavar='PATH', type=str, required=True,    help='Path to application.')
    parser.add_argument('--primary-bundle-id',  metavar='STR',  type=str, required=True,    help='Primary Bundle ID.')
    parser.add_argument('--username',           metavar='STR',  type=str, required=True,    help='Apple ID.')
    parser.add_argument('--password',           metavar='STR',  type=str, required=True,    help='Application password.')
    parser.add_argument('--asc-provider',       metavar='STR',  type=str, required=False,   help='Provider shortname.')
    args = parser.parse_args(sys.argv[1:])

    uploadable_app = args.application
    if os.path.isdir(uploadable_app):
        uploadable_app = args.application + '.zip'
        subprocess.call(['/usr/bin/ditto', '-c', '-k', '--keepParent', args.application, uploadable_app])

    notarize_request_output = plistlib_loads(subprocess.check_output([
        'xcrun', 'altool',
        '--notarize-app',
        '--primary-bundle-id', args.primary_bundle_id,
        '--username', args.username,
        '--password', args.password,
        '--file', uploadable_app,
        '--output-format', 'xml'
    ] + (['--asc-provider', args.asc_provider] if args.asc_provider else [])).decode('utf8'))
    request_uuid = None
    try:
        request_uuid = notarize_request_output['notarization-upload']['RequestUUID']
        print("REQUEST_UUID = {}".format(request_uuid))
    except Exception:
        print("REQUEST_UUID is empty!")
        print(plistlib_dumps(notarize_request_output))
        return 1

    if uploadable_app != args.application:
        os.remove(uploadable_app)

    time.sleep(10)  # Otherwise altool --notarization-info will shit himself

    notarize_check_output = None
    notarization_status = None
    while True:
        notarization_status = None
        notarize_check_output = plistlib_loads(subprocess.check_output([
            'xcrun', 'altool',
            '--notarization-info', request_uuid,
            '--username', args.username,
            '--password', args.password,
            '--output-format', 'xml'
        ]).decode('utf8'))
        try:
            notarization_status = notarize_check_output['notarization-info']['Status']
            print('NOTARIZATION_STATUS = {}'.format(notarization_status))
        except Exception:
            print("NOTARIZATION_STATUS is empty!")
            print(plistlib_dumps(notarize_check_output))
            return 1
        if notarization_status == 'in progress':
            time.sleep(60)
        else:
            break

    print(plistlib_dumps(notarize_check_output))

    if notarization_status != 'success':
        print('NOTARIZATION_STATUS is not success!')
        return 1

    subprocess.call(['xcrun', 'stapler', 'staple', args.application])
    return 0


if __name__ == '__main__':
    sys.exit(main())
