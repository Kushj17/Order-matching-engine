#include <bits/stdc++.h>
#include <thread>
#include <mutex>
#include <fstream>
#include <chrono>
using namespace std;

// ---------------- ENUMS ----------------
enum OrderType { BUY, SELL };
enum OrderKind { LIMIT, MARKET };

// ---------------- ORDER ----------------
struct Order {
    int id;
    int price;
    int quantity;
    OrderType type;
    OrderKind kind;
    long long timestamp;

    Order(int id, int price, int quantity, OrderType type, OrderKind kind, long long ts)
        : id(id), price(price), quantity(quantity), type(type), kind(kind), timestamp(ts) {}
};

// ---------------- LOGGER ----------------
class Logger {
private:
    ofstream file;
    mutex logMutex;

public:
    Logger() {
        file.open("trades.log", ios::app);
    }

    void log(string msg) {
        lock_guard<mutex> lock(logMutex);
        file << msg << endl;
    }

    ~Logger() {
        file.close();
    }
};

// ---------------- ORDER BOOK ----------------
class OrderBook {
public:
    map<int, queue<Order>, greater<int>> buyOrders;
    map<int, queue<Order>> sellOrders;
};

// ---------------- MATCHING ENGINE ----------------
class MatchingEngine {
private:
    OrderBook orderBook;
    mutex mtx;
    Logger logger;
    int currentTime = 0;

public:
    void addOrder(int id, int price, int quantity, OrderType type, OrderKind kind) {
        auto start = chrono::high_resolution_clock::now();

        lock_guard<mutex> lock(mtx);

        Order order(id, price, quantity, type, kind, currentTime++);

        if (type == BUY) matchBuy(order);
        else matchSell(order);

        auto end = chrono::high_resolution_clock::now();
        auto latency = chrono::duration_cast<chrono::microseconds>(end - start);

        cout << "Latency: " << latency.count() << " microseconds\n";
    }

private:
    void matchBuy(Order& buy) {
        while (buy.quantity > 0 && !orderBook.sellOrders.empty()) {
            auto it = orderBook.sellOrders.begin();

            if (buy.kind == LIMIT && it->first > buy.price) break;

            auto& q = it->second;

            while (!q.empty() && buy.quantity > 0) {
                Order& sell = q.front();

                int qty = min(buy.quantity, sell.quantity);
                executeTrade(buy, sell, qty);

                buy.quantity -= qty;
                sell.quantity -= qty;

                if (sell.quantity == 0) q.pop();
            }

            if (q.empty()) orderBook.sellOrders.erase(it);
        }

        if (buy.quantity > 0 && buy.kind == LIMIT) {
            orderBook.buyOrders[buy.price].push(buy);
        }
    }

    void matchSell(Order& sell) {
        while (sell.quantity > 0 && !orderBook.buyOrders.empty()) {
            auto it = orderBook.buyOrders.begin();

            if (sell.kind == LIMIT && it->first < sell.price) break;

            auto& q = it->second;

            while (!q.empty() && sell.quantity > 0) {
                Order& buy = q.front();

                int qty = min(sell.quantity, buy.quantity);
                executeTrade(buy, sell, qty);

                sell.quantity -= qty;
                buy.quantity -= qty;

                if (buy.quantity == 0) q.pop();
            }

            if (q.empty()) orderBook.buyOrders.erase(it);
        }

        if (sell.quantity > 0 && sell.kind == LIMIT) {
            orderBook.sellOrders[sell.price].push(sell);
        }
    }

    void executeTrade(Order& buy, Order& sell, int qty) {
        string msg = "Trade: BuyID=" + to_string(buy.id) +
                     " SellID=" + to_string(sell.id) +
                     " Price=" + to_string(sell.price) +
                     " Qty=" + to_string(qty);

        cout << msg << endl;
        logger.log(msg);
    }
};

// ---------------- REST SIMULATION ----------------
class API {
private:
    MatchingEngine engine;

public:
    void placeOrder(int id, int price, int qty, string side, string kind) {
        OrderType type = (side == "BUY") ? BUY : SELL;
        OrderKind k = (kind == "MARKET") ? MARKET : LIMIT;

        engine.addOrder(id, price, qty, type, k);
    }
};

// ---------------- MAIN ----------------
int main() {
    API api;

    thread t1([&]() {
        api.placeOrder(1, 100, 10, "BUY", "LIMIT");
    });

    thread t2([&]() {
        api.placeOrder(2, 95, 5, "SELL", "LIMIT");
    });

    thread t3([&]() {
        api.placeOrder(3, 0, 10, "SELL", "MARKET");
    });

    t1.join();
    t2.join();
    t3.join();

    return 0;
}