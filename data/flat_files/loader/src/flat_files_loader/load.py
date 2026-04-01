"""
Load SPX EOD flat files into `options_long`, then upsert into repo `data/marketDB.sqlite`
(table `options_eod_quotes`), using the same DDL as `Delta++DB/sql/market_schema.sql`.

Run from repo (or any cwd)::

    cd data/flat_files/loader
    uv run load-flat-files --load options
    uv run load-flat-files --load equities
"""
from __future__ import annotations

import argparse
import sqlite3
from pathlib import Path

import pandas as pd


def _find_repo_root() -> Path:
    here = Path(__file__).resolve()
    for p in [here.parent, *here.parents]:
        if (p / "Delta++DB" / "sql" / "market_schema.sql").is_file():
            return p
    raise RuntimeError("Could not locate repository root (missing Delta++/DB/sql/market_schema.sql).")


REPO_ROOT = _find_repo_root()
DATA_DIR = REPO_ROOT / "data"
FLAT_FILES_DIR = DATA_DIR / "flat_files"
MARKET_DB_PATH = DATA_DIR / "marketDB.sqlite"
MARKET_SCHEMA_SQL = REPO_ROOT / "Delta++DB" / "sql" / "market_schema.sql"

OPTIONS_UPSERT_SQL = """
INSERT INTO options_eod_quotes (
  quote_date, expiration_date, strike_price, underlying_ticker, contract_type, bid, ask, volume
) VALUES (?, ?, ?, ?, ?, ?, ?, ?)
ON CONFLICT(quote_date, expiration_date, strike_price, underlying_ticker, contract_type) DO UPDATE SET
  bid = excluded.bid,
  ask = excluded.ask,
  volume = excluded.volume
"""

EQUITIES_UPSERT_SQL = """
INSERT INTO equities (
  quote_date, ticker, last
) VALUES (?, ?, ?)
ON CONFLICT(quote_date, ticker) DO UPDATE SET
  last = excluded.last
"""

CHUNK_ROWS = 10000


def _ensure_options_eod_volume_column(conn: sqlite3.Connection) -> None:
    """Add `volume` when upgrading DBs created before schema v4."""
    cur = conn.execute("PRAGMA table_info(options_eod_quotes)")
    if any(row[1] == "volume" for row in cur.fetchall()):
        return
    conn.execute("ALTER TABLE options_eod_quotes ADD COLUMN volume REAL")


def _ensure_schema(conn: sqlite3.Connection) -> None:
    sql = MARKET_SCHEMA_SQL.read_text(encoding="utf-8")
    conn.executescript(sql)
    _ensure_options_eod_volume_column(conn)


def read_spx_eod_2023(flat_files_dir: Path = FLAT_FILES_DIR) -> pd.DataFrame:
    frames: list[pd.DataFrame] = []
    for i in range(1, 13):
        path = flat_files_dir / f"spx_eod_2023{i:02d}.txt"
        df = pd.read_csv(path, skipinitialspace=True)
        df["source_file"] = path.name
        frames.append(df)
    return pd.concat(frames, ignore_index=True)


def _normalize_dates(df: pd.DataFrame) -> pd.DataFrame:
    out = df.copy()
    for col in ("quote_date", "expiration_date"):
        out[col] = pd.to_datetime(out[col], errors="coerce").dt.strftime("%Y-%m-%d")
    return out


def build_options_long(spx_eod_2023: pd.DataFrame) -> pd.DataFrame:
    useful_cols = [
        "[QUOTE_DATE]",
        "[EXPIRE_DATE]",
        "[STRIKE]",
        "[C_BID]",
        "[C_ASK]",
        "[C_VOLUME]",
        "[P_BID]",
        "[P_ASK]",
        "[P_VOLUME]",
    ]
    df = spx_eod_2023[useful_cols].copy()
    df["underlying_ticker"] = "SPX"
    df.rename(
        columns={
            "[QUOTE_DATE]": "quote_date",
            "[EXPIRE_DATE]": "expiration_date",
            "[STRIKE]": "strike_price",
            "[C_BID]": "c_bid",
            "[C_ASK]": "c_ask",
            "[C_VOLUME]": "c_volume",
            "[P_BID]": "p_bid",
            "[P_ASK]": "p_ask",
            "[P_VOLUME]": "p_volume",
        },
        inplace=True,
    )

    common = ["quote_date", "expiration_date", "strike_price", "underlying_ticker"]
    calls = df[common + ["c_bid", "c_ask", "c_volume"]].copy().rename(
        columns={"c_bid": "bid", "c_ask": "ask", "c_volume": "volume"}
    )
    calls["contract_type"] = "call"
    puts = df[common + ["p_bid", "p_ask", "p_volume"]].copy().rename(
        columns={"p_bid": "bid", "p_ask": "ask", "p_volume": "volume"}
    )
    puts["contract_type"] = "put"
    return pd.concat([calls, puts], ignore_index=True)


def upsert_options_long(options_long: pd.DataFrame, db_path: Path | None = None) -> int:
    """Upsert all rows from `options_long` into `options_eod_quotes`. Returns row count written."""
    path = db_path if db_path is not None else MARKET_DB_PATH
    path.parent.mkdir(parents=True, exist_ok=True)

    df = _normalize_dates(options_long)
    cols = [
        "quote_date",
        "expiration_date",
        "strike_price",
        "underlying_ticker",
        "contract_type",
        "bid",
        "ask",
        "volume",
    ]
    df = df[cols]
    df["volume"] = pd.to_numeric(df["volume"], errors="coerce").fillna(0.0)
    non_volume = [c for c in cols if c != "volume"]
    df[non_volume] = df[non_volume].where(pd.notna(df[non_volume]), None)

    with sqlite3.connect(path) as conn:
        _ensure_schema(conn)
        cur = conn.cursor()
        n = 0
        for start in range(0, len(df), CHUNK_ROWS):
            chunk = df.iloc[start : start + CHUNK_ROWS]
            rows = [
                (
                    r["quote_date"],
                    r["expiration_date"],
                    float(r["strike_price"]),
                    r["underlying_ticker"],
                    r["contract_type"],
                    r["bid"],
                    r["ask"],
                    float(r["volume"]),
                )
                for _, r in chunk.iterrows()
            ]
            cur.executemany(OPTIONS_UPSERT_SQL, rows)
            n += len(rows)
        conn.commit()
    return n


def build_equities(spx_eod_2023: pd.DataFrame) -> pd.DataFrame:
    df = spx_eod_2023[["[QUOTE_DATE]", "[UNDERLYING_LAST]"]].copy()
    df["ticker"] = "SPX"
    df.rename(columns={"[QUOTE_DATE]": "quote_date", "[UNDERLYING_LAST]": "last"}, inplace=True)
    df["quote_date"] = pd.to_datetime(df["quote_date"], errors="coerce").dt.strftime("%Y-%m-%d")
    df = df[["quote_date", "ticker", "last"]].drop_duplicates()
    return df


def upsert_equities(equities: pd.DataFrame, db_path: Path | None = None) -> int:
    """Upsert all rows into `equities`. Returns row count written."""
    path = db_path if db_path is not None else MARKET_DB_PATH
    path.parent.mkdir(parents=True, exist_ok=True)

    df = equities.copy()
    df = df[["quote_date", "ticker", "last"]]
    df = df.where(pd.notna(df), None)

    with sqlite3.connect(path) as conn:
        _ensure_schema(conn)
        cur = conn.cursor()
        n = 0
        for start in range(0, len(df), CHUNK_ROWS):
            chunk = df.iloc[start : start + CHUNK_ROWS]
            rows = [(r["quote_date"], r["ticker"], r["last"]) for _, r in chunk.iterrows()]
            cur.executemany(EQUITIES_UPSERT_SQL, rows)
            n += len(rows)
        conn.commit()
    return n


def _parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    p = argparse.ArgumentParser(description="Load SPX EOD flat files into marketDB.sqlite.")
    p.add_argument("--load", choices=["options", "equities"], default="options")
    p.add_argument("--db", type=Path, default=MARKET_DB_PATH)
    return p.parse_args(argv)


def main() -> None:
    args = _parse_args()
    spx_eod_2023 = read_spx_eod_2023()

    if args.load == "options":
        options_long = build_options_long(spx_eod_2023)
        n = upsert_options_long(options_long, db_path=args.db)
        print(f"Upserted {n} row(s) into {args.db} (options_eod_quotes)")
        return

    equities = build_equities(spx_eod_2023)
    n = upsert_equities(equities, db_path=args.db)
    print(f"Upserted {n} row(s) into {args.db} (equities)")


if __name__ == "__main__":
    main()
