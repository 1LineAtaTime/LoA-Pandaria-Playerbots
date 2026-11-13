"""Real 5.4.8 creation -> immediate enumeration -> login, on one connection.

Run only with a disposable account. Credentials come from MOP_ACCOUNT and
MOP_PASSWORD; protocol logging is disabled to avoid exposing session material.
The caller removes the test account/character through normal server commands.
"""
import argparse
import json
import os
import time
import headless


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--name', required=True)
    p.add_argument('--race', type=int, default=1)
    p.add_argument('--class-id', type=int, default=4)
    a = p.parse_args()
    account, password = os.environ['MOP_ACCOUNT'], os.environ['MOP_PASSWORD']
    host = os.environ.get('MOP_HOST', '127.0.0.1')
    headless.log = lambda *args, **kwargs: None
    key = headless.auth_login(host, int(os.environ.get('MOP_AUTH_PORT', '3724')), account, password)
    world = headless.WorldClient(host, int(os.environ.get('MOP_WORLD_PORT', '8085')), key, account)
    try:
        world.handshake(key, realm_id=1)
        before = world.char_enum()
        if before:
            raise RuntimeError('Use an empty disposable account')
        world.char_create(a.name, a.race, a.class_id)
        deadline = time.monotonic() + 30
        while time.monotonic() < deadline:
            opcode, payload = world.read_packet()
            if opcode == headless.SMSG_CHAR_CREATE:
                if payload != bytes([47]):
                    raise RuntimeError('Character creation did not report success')
                break
        else:
            raise RuntimeError('No creation response')
        # Deliberately no sleep and no reconnect between acknowledgement and enum.
        after = world.char_enum()
        matches = [c for c in after if c['name'].lower() == a.name.lower()]
        if len(matches) != 1:
            raise RuntimeError('Successful creation missing from immediate character list')
        if not world.player_login(matches[0]['guid']):
            raise RuntimeError('New character did not enter the world')
        print(json.dumps(dict(result='passed', race=a.race, name=a.name,
                              creation_response=47, immediate_enum_count=len(after),
                              same_connection=True, login_map=world.map_id)))
    finally:
        world.s.close()


if __name__ == '__main__':
    main()
