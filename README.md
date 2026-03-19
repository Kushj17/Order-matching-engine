# Low Latency Order Matching Engine (C++)

A simple order matching engine written in C++ that simulates how trades are executed in real-world exchanges. The focus was on keeping it efficient, handling concurrent orders, and maintaining price-time priority.

---

## What it does

- Matches buy and sell orders in real time  
- Supports both **limit** and **market** orders  
- Maintains an order book with proper priority rules  
- Handles multiple orders using threads (with mutex for safety)  
- Logs executed trades to a file  
- Tracks execution latency per order  

---

## How it works (high level)

Orders are stored in two books:
- Buy side → highest price first  
- Sell side → lowest price first  

When a new order comes in:
- It tries to match against the opposite side  
- Trades are executed if prices are compatible  
- Remaining quantity (if any) goes back into the book  

For same price levels, orders are matched in FIFO order.

---

## Tech used

- C++ (C++17)
- STL (map, queue, etc.)
- std::thread, std::mutex
- chrono (for latency tracking)
- file I/O for logging

---

## Build & Run

```bash
g++ src/main.cpp -o main -std=c++17 -pthread
./main
