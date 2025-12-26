import sqlite3
import json
import os
import hashlib

DB_PATH = os.path.join(os.path.dirname(__file__), '..', 'tickets.db')
USERS_JSON = os.path.join(os.path.dirname(__file__), 'users.json')

def hash_password(password):
    return hashlib.sha256(password.encode('utf-8')).hexdigest()

def insert_users(cursor, users):
    for user in users:
        password_hash = hash_password(user['password'])
        cursor.execute(
            '''
            INSERT INTO users (username, password_hash, role) VALUES (?, ?, ?)
            ''',
            (user['username'], password_hash, user['role'])
        )


if __name__ == '__main__':
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    with open(USERS_JSON, 'r', encoding='utf-8') as f:
        users = json.load(f)
    insert_users(cursor, users)
    conn.commit()
    conn.close()
