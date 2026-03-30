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

CREATE TABLE IF NOT EXISTS options_eod_quotes (
    quote_date TEXT NOT NULL,
    expiration_date TEXT NOT NULL,
    strike_price REAL NOT NULL,
    underlying_ticker TEXT NOT NULL,
    contract_type TEXT NOT NULL,
    bid REAL,
    ask REAL,
    PRIMARY KEY (quote_date, expiration_date, strike_price, underlying_ticker, contract_type)
);

CREATE INDEX IF NOT EXISTS idx_options_eod_underlying_quote
    ON options_eod_quotes(underlying_ticker, quote_date);

PRAGMA user_version = 2;

