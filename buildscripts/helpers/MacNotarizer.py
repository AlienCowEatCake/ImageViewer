#!/usr/bin/env python3

import argparse
import json
import os
import subprocess
import sys
import time


def extract_password(password):
    if password[0:10] == '@keychain:':
        return subprocess.check_output(
            ['security', 'find-generic-password', '-s', password[10:], '-w']
        ).decode('utf-8').strip()
    return password


def main():
    parser = argparse.ArgumentParser(description='Notarizer')
    parser.add_argument('--application',                metavar='PATH', type=str, required=True,    help='Path to application.')
    parser.add_argument('--primary-bundle-id',          metavar='STR',  type=str, required=False,   help='[deprecated] Primary Bundle ID.')
    parser.add_argument('--apple-id', '--username',     metavar='STR',  type=str, required=False,   help='Apple ID.')
    parser.add_argument('--password',                   metavar='STR',  type=str, required=False,   help='Application password.')
    parser.add_argument('--team-id', '--asc-provider',  metavar='STR',  type=str, required=False,   help='Developer Team ID.')
    parser.add_argument('--keychain-profile',           metavar='STR',  type=str, required=False,   help='Profile name from notarytool store-credentials.')
    args = parser.parse_args(sys.argv[1:])

    uploadable_app = args.application
    if os.path.isdir(uploadable_app) or os.path.splitext(uploadable_app)[1] not in ['.zip', '.pkg', '.dmg']:
        uploadable_app = args.application + '.zip'
        subprocess.call(['/usr/bin/ditto', '-c', '-k', '--keepParent', args.application, uploadable_app])

    notarize_request_output = json.loads(subprocess.check_output([
        'xcrun', 'notarytool',
        'submit', uploadable_app,
        '--output-format', 'json']
    + (['--apple-id', args.apple_id]                    if args.apple_id            else [])
    + (['--password', extract_password(args.password)]  if args.password            else [])
    + (['--team-id', args.team_id]                      if args.team_id             else [])
    + (['--keychain-profile', args.keychain_profile]    if args.keychain_profile    else [])
    ))
    request_id = None
    try:
        request_id = notarize_request_output['id']
        print("REQUEST_ID = {}".format(request_id))
    except Exception:
        print("REQUEST_ID is empty!")
        print(json.dumps(notarize_request_output, sort_keys=True, indent=4))
        return 1

    if uploadable_app != args.application:
        os.remove(uploadable_app)

    notarize_check_output = None
    notarization_status = None
    while True:
        notarization_status = None
        notarize_check_process = subprocess.run([
            'xcrun', 'notarytool',
            'log', request_id,
            '--output-format', 'json']
        + (['--apple-id', args.apple_id]                    if args.apple_id            else [])
        + (['--password', extract_password(args.password)]  if args.password            else [])
        + (['--team-id', args.team_id]                      if args.team_id             else [])
        + (['--keychain-profile', args.keychain_profile]    if args.keychain_profile    else [])
        , check=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        notarize_check_output = None
        try:
            notarize_check_output = json.loads(notarize_check_process.stdout)
            print(json.dumps(notarize_check_output, sort_keys=True, indent=4))
        except Exception:
            print("notarytool log output is not JSON!")
            print(notarize_check_process.stdout)
            return 1
        if notarize_check_process.returncode == 0:
            try:
                notarization_status = notarize_check_output['status']
                print('NOTARIZATION_STATUS = {}'.format(notarization_status))
            except Exception:
                print("NOTARIZATION_STATUS is empty!")
                print(json.dumps(notarize_check_output, sort_keys=True, indent=4))
                return 1
            break
        time.sleep(60)

    if notarization_status != 'Accepted':
        print('NOTARIZATION_STATUS is not success!')
        return 1

    subprocess.call(['xcrun', 'stapler', 'staple', args.application])
    return 0


if __name__ == '__main__':
    sys.exit(main())
