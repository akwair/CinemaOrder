import sqlite3
import json
import os

DB_PATH = os.path.join(os.path.dirname(__file__), '..', 'tickets.db')
MOVIES_JSON = os.path.join(os.path.dirname(__file__), 'movies.json')

def insert_movies(cursor, movies):
    for movie in movies:
        # 合并movie_details、actors、description
        details = movie.get('movie_details', '')
        if 'actors' in movie:
            details += f"\n主演: {movie['actors']}"
        if 'description' in movie:
            details += f"\n剧情简介: {movie['description']}"
        cursor.execute(
            '''
            INSERT INTO tickets (
                movieName, cinemaName, showDate, showTime, duration, price, hall, capacity, remain, sold, movie_details
            ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
            ''',
            (
                movie['movieName'], movie['cinemaName'], movie['showDate'], movie['showTime'],
                movie['duration'], movie['price'], movie['hall'], movie['capacity'],
                movie['remain'], movie['sold'], details
            )
        )


if __name__ == '__main__':
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    with open(MOVIES_JSON, 'r', encoding='utf-8') as f:
        movies = json.load(f)
    insert_movies(cursor, movies)
    conn.commit()
    conn.close()
