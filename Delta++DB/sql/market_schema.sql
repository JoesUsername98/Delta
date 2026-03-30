PRAGMA journal_mode=WAL;

CREATE TABLE IF NOT EXISTS treasury_yields (
    date TEXT PRIMARY KEY NOT NULL,
    yield_1_month REAL,
    yield_3_month REAL,
    yield_6_month REAL,
    yield_1_year REAL,
    yield_2_year REAL,
    yield_3_year REAL,
    yield_5_year REAL,
    yield_7_year REAL,
    yield_10_year REAL,
    yield_20_year REAL,
    yield_30_year REAL
);

PRAGMA user_version = 1;

