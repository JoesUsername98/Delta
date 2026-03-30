PRAGMA journal_mode=WAL;

CREATE TABLE IF NOT EXISTS options_contracts (
    ticker TEXT PRIMARY KEY NOT NULL,
    underlying_ticker TEXT NOT NULL,
    expiration_date TEXT NOT NULL,
    strike_price REAL,
    contract_type TEXT NOT NULL,
    exercise_style TEXT
);

CREATE INDEX IF NOT EXISTS idx_options_contracts_underlying ON options_contracts(underlying_ticker);
CREATE INDEX IF NOT EXISTS idx_options_contracts_expiration ON options_contracts(expiration_date);

PRAGMA user_version = 1;
