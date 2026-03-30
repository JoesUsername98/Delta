# optionsDX Field Definitions

| Field | Definition | Example |
|---|---|---|
| QUOTE_UNIXTIME | Time of quote as a Unix Timestamp | 1583508900 |
| QUOTE_READTIME | Time of quote in a readable form (year-month-day hour:minute) [yyyy-mm-dd hh:MM) | 2020-03-06 10:35 |
| QUOTE_DATE | Date of quote in a readable form (year-month-day) [yyyy-mm-dd] | 2020-03-06 |
| QUOTE_TIME_HOURS | Time of quoted day in hours | 10.58333 |
| UNDERLYING_LAST | Last price of underlying asset | 296.38 |
| EXPIRE_DATE | Expiry date of contract in a readable form (year-month-day) [yyyy-mm-dd] | 2020-09-18 |
| EXPIRE_UNIX | Expiry date of contract as a Unix Timestamp | 1600459200 |
| DTE | Days till Expiry in hours (assumes options expire on expiry date at 16:00) | 196.18 |
| C_DELTA | Derived Option Greek: Delta of Call option | 0.12682 |
| C_GAMMA | Derived Option Greek: Gamma of Call option | 0.00508 |
| C_VEGA | Derived Option Greek: Vega of Call option | 0.45103 |
| C_THETA | Derived Option Greek: Theta of Call option | -0.01843 |
| C_RHO | Derived Option Greek: Rho of Call option | 0.184 |
| C_IV | Implied Volatility of Call option | 0.18953 |
| C_VOLUME | Days traded volume of Call option | 1 |
| C_LAST | Last traded price of Call option | 2.25 |
| C_SIZE | Current size of Call option | 113 x 100 |
| C_BID | Bid price of Call option | 2.05 |
| C_ASK | Ask price of Call option | 2.82 |
| STRIKE | Option strike price | 345 |
| P_BID | Bid price of Put option | 52.58 |
| P_ASK | Ask price of Put option | 54.68 |
| P_SIZE | Current size of Put option | 113 x 113 |
| P_LAST | Last traded price of Put option | 43 |
| P_DELTA | Derived Option Greek: Delta of Put option | -0.95081 |
| P_GAMMA | Derived Option Greek: Gamma of Put option | 0.00277 |
| P_VEGA | Derived Option Greek: Vega of Put option | 0.16209 |
| P_THETA | Derived Option Greek: Theta of Put option | -0.02739 |
| P_RHO | Derived Option Greek: Rho of Put option | -1.80885 |
| P_IV | Implied Volatility of Put option | 0.12222 |
| P_VOLUME | Days traded volume of Put option | 6 |
| STRIKE_DISTANCE | Distance between strike price and current underlying price | 48.6 |
| STRIKE_DISTANCE_PCT | Distance as a percentage between strike price and current underlying price | 0.164 |
